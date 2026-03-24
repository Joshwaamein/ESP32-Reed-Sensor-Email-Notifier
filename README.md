# ESP32 Reed Switch Email Notifier

This project uses an ESP32 microcontroller to monitor a magnetic reed switch, which is commonly used as a door or window sensor. When the sensor's state changes (e.g., a door is opened or closed), the ESP32 connects to Wi-Fi and sends an email notification to a specified address.

Two versions of the sketch are provided — one for **Gmail** and one for **Brevo (Sendinblue)** — so you can choose the email provider that best suits your needs.

## Features

- **Real-time Monitoring**: Detects when a door or window is opened or closed.
- **Email Alerts**: Sends immediate email notifications for any change in the sensor's status.
- **Multi-Network Wi-Fi**: Can attempt to connect to a list of pre-configured Wi-Fi networks.
- **LED Status Indicator**: Uses the built-in LED to indicate the sensor's status (ON for open, OFF for closed) and blinks to confirm an email has been sent.
- **Simple Debouncing**: Implements a software debounce timer to prevent false triggers from sensor "bouncing."
- **Two SMTP Providers**: Choose between Gmail (App Password) or Brevo (SMTP key) for sending emails.

## Project Structure

```
├── README.md
├── gmail/
│   └── sketch.ino       # Gmail version (SMTP over SSL, port 465)
└── brevo/
    └── sketch.ino        # Brevo version (SMTP over STARTTLS, port 587)
```

## Hardware Requirements

- ESP32 Development Board
- Magnetic Reed Switch Sensor (2-wire type)
- Jumper Wires
- Micro-USB Cable for power and programming

## Wiring Diagram

The wiring is minimal because the project utilizes the ESP32's internal pull-up resistor, so no external resistors are needed.

- Connect one terminal of the reed switch to a **GND** pin on the ESP32.
- Connect the other terminal of the reed switch to **GPIO 23** on the ESP32.

```
+---------------------+         +-----------------+
|  ESP32 Dev Board    |         |   Reed Switch   |
|                     |         |                 |
|                 GND |<------->|   Terminal 1    |
|                     |         |                 |
|              GPIO23 |<------->|   Terminal 2    |
|                     |         +-----------------+
+---------------------+
```

*Note: The built-in LED is on GPIO 2, which requires no external wiring.*

## Software & Libraries

1. **Arduino IDE** or **PlatformIO**.
2. **ESP32 Board Support Package**: If using the Arduino IDE, ensure you have the ESP32 boards installed via the Board Manager.
3. **ESP_Mail_Client Library**: Install via the Arduino Library Manager — search for `ESP_Mail_Client`.

## Configuration

Before uploading, open the sketch for your chosen provider and fill in the `USER CONFIGURATION` section.

### Wi-Fi Credentials (both versions)

Add your Wi-Fi network SSID(s) and password(s) to the `credentials` array:

```cpp
WifiCredential credentials[] = {
  {"YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD"},
  // {"AnotherSSID", "AnotherPassword"}
};
```

---

### Option 1: Gmail (`gmail/sketch.ino`)

Uses Gmail's SMTP server with SSL on port 465. Requires a Google **App Password**.

| Setting | Value |
|---|---|
| SMTP Host | `smtp.gmail.com` |
| SMTP Port | `465` (SSL) |
| Auth Email | Your Gmail address |
| Auth Password | A Google App Password |
| Sender | Same as your Gmail address |

**How to get a Gmail App Password:**

1. Go to your [Google Account Security settings](https://myaccount.google.com/security).
2. Enable **2-Step Verification** if not already enabled.
3. Go to **App passwords** (search for it in your Google Account settings).
4. Generate a new app password for "Mail" / "Other (ESP32)".
5. Copy the 16-character password into `AUTHOR_PASSWORD`.

```cpp
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "your.email@gmail.com"
#define AUTHOR_PASSWORD "YOUR_GMAIL_APP_PASSWORD"
#define RECIPIENT_EMAIL "recipient@example.com"
```

---

### Option 2: Brevo (`brevo/sketch.ino`)

Uses Brevo's SMTP relay with STARTTLS on port 587. Requires a Brevo **SMTP key**.

| Setting | Value |
|---|---|
| SMTP Host | `smtp-relay.brevo.com` |
| SMTP Port | `587` (STARTTLS) |
| Auth Email | Your Brevo login email |
| Auth Password | Your Brevo SMTP key |
| Sender | A verified sender address from your Brevo account |

**How to get a Brevo SMTP key:**

1. Sign up or log in at [brevo.com](https://www.brevo.com/).
2. Go to **Settings → SMTP & API**.
3. Copy your SMTP key (starts with `xsmtpsib-`).
4. Make sure your sender domain/email is verified in Brevo.

```cpp
#define SMTP_HOST "smtp-relay.brevo.com"
#define SMTP_PORT 587
#define AUTHOR_EMAIL "your.brevo.login@example.com"
#define AUTHOR_PASSWORD "YOUR_BREVO_SMTP_KEY"
#define SENDER_EMAIL "alerts@yourdomain.com"
#define RECIPIENT_EMAIL "recipient@example.com"
```

> **Note:** With Brevo, the sender email (`SENDER_EMAIL`) can be different from the authentication email (`AUTHOR_EMAIL`), as long as the sender domain is verified in your Brevo account.

---

## How It Works

1. **Setup**: On startup, the ESP32 initializes the serial monitor, configures the reed switch pin (`GPIO 23`) with an internal pull-up resistor, and sets the LED pin as an output. It then connects to one of the specified Wi-Fi networks.
2. **Loop**: The main loop continuously reads the state of the reed switch.
3. **State Change Detection**: A simple 50ms debounce timer checks for a stable state change. When a confirmed change from `HIGH` (open) to `LOW` (closed) or vice-versa occurs, it triggers an action.
4. **Action**:
   - The built-in LED is turned ON if the sensor state is `HIGH` (door open) and OFF if `LOW` (door closed).
   - The `sendEmailNotification()` function is called.
5. **Email Notification**: The ESP32 connects to the SMTP server, composes an HTML email indicating the new sensor status ("OPEN" or "CLOSED"), and sends it to the recipient. The LED blinks three times as a visual confirmation.
6. **Wi-Fi Handling**: If the Wi-Fi connection is lost, the device will attempt to reconnect before trying to send another email. If it cannot connect to any of the provided networks, it will reboot after 30 seconds.

## Upload and Run

1. Install the necessary software and libraries as described above.
2. Choose either the `gmail/` or `brevo/` sketch and fill in your credentials.
3. Connect your ESP32 to your computer via USB.
4. In the Arduino IDE, select your ESP32 board model and the correct COM port.
5. Click **Upload** to flash the code to the device.
6. Open the Serial Monitor at a baud rate of `115200` to see status messages and debug information.
