#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Mail_Client.h>

//======================================================================
// USER CONFIGURATION
//======================================================================

// --- Wi-Fi Credentials ---
struct WifiCredential {
  const char* ssid;
  const char* password;
};

WifiCredential credentials[] = {
  {"SSID1", "SSID1_PASSWORD"},
  {"SSID2", "SSID2_PASSWORD"},
  // You can add more networks here, e.g.:
  // {"MyOtherWifi", "ItsPassword123"}
};

// --- Pin Definitions ---
#define REED_PIN 23
#define LED_PIN 2 // Built-in LED on most ESP32 boards

// --- SMTP Server Credentials ---
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "AUTHOR_EMAIL"
#define AUTHOR_PASSWORD "APP_PASSWORD" // IMPORTANT: Use an App Password for Gmail!

// --- Recipient ---
#define RECIPIENT_EMAIL "RECIPIENT_EMAIL"


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
void sendEmailNotification(String status);
void blinkLed(int times, int blinkDelay);

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- ESP32 Reed Switch Email Notifier ---");

  pinMode(REED_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  lastReedState = digitalRead(REED_PIN);
  currentReedState = lastReedState;
  Serial.print("Initial sensor state: ");
  Serial.println(lastReedState == HIGH ? "Open" : "Closed");

  connectToWiFi();

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

void sendEmailNotification(String status) {
  Serial.println("Preparing to send email...");

  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  SMTP_Message message;
  message.sender.name = "ESP32 Notifier";
  message.sender.email = AUTHOR_EMAIL;
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
