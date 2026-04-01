#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <time.h>

//======================================================================
// USER CONFIGURATION
//======================================================================

// --- Wi-Fi Credentials ---
struct WifiCredential {
  const char* ssid;
  const char* password;
};

WifiCredential credentials[] = {
  {"YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD"},
  // You can add more networks here, e.g.:
  // {"AnotherSSID", "AnotherPassword"}
};

// --- Pin Definitions ---
#define REED_PIN 23
#define LED_PIN 2 // Built-in LED on most ESP32 boards

// --- SMTP Server Credentials (Brevo / Sendinblue) ---
#define SMTP_HOST "smtp-relay.brevo.com"
#define SMTP_PORT 587
#define AUTHOR_EMAIL "your.brevo.login@example.com"       // Your Brevo account login email
#define AUTHOR_PASSWORD "YOUR_BREVO_SMTP_KEY"              // Your Brevo SMTP key (starts with xsmtpsib-)

// --- Sender & Recipient ---
#define SENDER_EMAIL "alerts@yourdomain.com"               // Must be a verified sender in your Brevo account
#define RECIPIENT_EMAIL "recipient@example.com"


//======================================================================
// GLOBAL OBJECTS & VARIABLES
//======================================================================

SMTPSession smtp;

// Global variables for debouncing the switch
int lastReedState = HIGH;
int currentReedState;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Function prototypes
void smtpCallback(SMTP_Status status);
void connectToWiFi();
void syncTime();
void sendEmailNotification(String status);
void blinkLed(int times, int blinkDelay);

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- ESP32 Reed Switch Email Notifier (Brevo) ---");

  pinMode(REED_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  lastReedState = digitalRead(REED_PIN);
  currentReedState = lastReedState;
  Serial.print("Initial sensor state: ");
  Serial.println(lastReedState == HIGH ? "Open" : "Closed");

  connectToWiFi();
  syncTime();

  smtp.debug(1);
  smtp.callback(smtpCallback);
}

void loop() {
  int reading = digitalRead(REED_PIN);

  if (reading != lastReedState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentReedState) {
      currentReedState = reading;
      String status_text = (currentReedState == LOW) ? "CLOSED" : "OPEN";
      Serial.print("State changed to: ");
      Serial.println(status_text);
      
      // --- LED State Logic ---
      if (currentReedState == HIGH) { // Sensor is "activated" (door open)
        digitalWrite(LED_PIN, HIGH); // Turn the LED on
      } else { // Sensor is "deactivated" (door closed)
        digitalWrite(LED_PIN, LOW); // Turn the LED off
      }

      if(WiFi.status() == WL_CONNECTED) {
        sendEmailNotification(status_text);
      } else {
        Serial.println("WiFi disconnected. Cannot send email. Attempting to reconnect...");
        connectToWiFi();
        syncTime();
      }
    }
  }

  lastReedState = reading;
}

void blinkLed(int times, int blinkDelay) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(blinkDelay);
    digitalWrite(LED_PIN, LOW);
    delay(blinkDelay);
  }
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  int numNetworks = sizeof(credentials) / sizeof(credentials[0]);
  for (int i = 0; i < numNetworks; i++) {
    Serial.print("Trying to connect to SSID: ");
    Serial.println(credentials[i].ssid);
    WiFi.begin(credentials[i].ssid, credentials[i].password);
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
      Serial.print(".");
      delay(500);
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected!");
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID());
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      return;
    } else {
      Serial.println("\nFailed to connect. Trying next network...");
      WiFi.disconnect(true);
      delay(100);
    }
  }
  Serial.println("\nCould not connect to any specified WiFi networks. Rebooting in 30 seconds...");
  delay(30000);
  ESP.restart();
}

void syncTime() {
  Serial.println("Syncing time via NTP...");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  time_t now = time(nullptr);
  unsigned long startAttempt = millis();
  while (now < 1000000000UL && millis() - startAttempt < 15000) {
    delay(250);
    now = time(nullptr);
  }

  if (now < 1000000000UL) {
    Serial.println("WARNING: Failed to sync time via NTP. Email sending may fail.");
  } else {
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print("Time synced: ");
    Serial.println(asctime(&timeinfo));
  }
}

void sendEmailNotification(String status) {
  Serial.println("Preparing to send email...");

  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";
  session.secure.startTLS = true;

  // NTP time config for TLS certificate validation
  session.time.ntp_server_f1 = "pool.ntp.org";
  session.time.ntp_server_f2 = "time.nist.gov";
  session.time.gmt_offset = 0;
  session.time.day_light_offset = 0;

  SMTP_Message message;
  message.sender.name = "ESP32 Door Sensor";
  message.sender.email = SENDER_EMAIL;
  message.subject = "Alert: Sensor Status Changed!";
  message.addRecipient("User", RECIPIENT_EMAIL);
  
  String htmlMsg = "<h2>Sensor Alert</h2><p>The sensor status has changed to: <strong>" + status + "</strong></p>";
  message.html.content = htmlMsg.c_str();

  if (!smtp.connect(&session)) {
    Serial.println("Failed to connect to SMTP server.");
    return;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Error sending Email, " + smtp.errorReason());
  } else {
    Serial.println("Email sent successfully! Blinking LED for confirmation.");
    blinkLed(3, 100); // Blink 3 times with a 100ms delay
  }
}

void smtpCallback(SMTP_Status status) {
  Serial.println(status.info());
  if (status.success()) {
    Serial.println("----------------");
  }
}
