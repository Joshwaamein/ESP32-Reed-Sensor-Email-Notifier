# ESP32 Reed Switch Email Notifier

This project uses an ESP32 microcontroller to monitor a magnetic reed switch (commonly used as a door or window sensor). When the sensor's state changes (e.g., a door is opened or closed), the ESP32 connects to Wi-Fi and sends an email notification to a specified address.

## Features

-   **Real-time Monitoring**: Detects when a door or window is opened or closed.
-   **Email Alerts**: Sends immediate email notifications for any change in the sensor's status.
-   **Wi-Fi Connectivity**: Can connect to a list of pre-configured Wi-Fi networks.
-   **LED Status Indicator**: Uses the built-in LED to indicate the sensor's status (ON for open, OFF for closed) and blinks to confirm an email has been sent.
-   **Robust Sensing**: Implements software debouncing to prevent false triggers from sensor "bouncing".
-   **Secure Authentication**: Designed for use with Gmail's SMTP server using an "App Password" for enhanced security.

## Hardware Requirements

-   ESP32 Development Board
-   Magnetic Reed Switch Sensor (2-wire type)
-   Jumper Wires
-   Micro-USB Cable for power and programming

## Wiring Diagram

The wiring is simple because the project uses the ESP32's internal pull-up resistor, so no external resistors are needed.

-   Connect one terminal of the reed switch to **GND** on the ESP32.
-   Connect the other terminal of the reed switch to **GPIO 23** on the ESP32.

Here is a simple text-based diagram of the connections:

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

1.  **Arduino IDE** or **PlatformIO**.
2.  **ESP32 Board Support Package**: If using Arduino IDE, make sure you have the ESP32 boards installed via the Board Manager.
3.  **ESP_Mail_Client Library**: This library is required for sending emails. You can install it through the Arduino Library Manager. Search for `ESP_Mail_Client` and install it.

## Configuration

Before uploading the code to your ESP32, you must configure the following settings in the `USER CONFIGURATION` section of the sketch:

1.  **Wi-Fi Credentials**: Add your Wi-Fi network SSID(s) and password(s) to the `credentials` array. You can add multiple networks as fallbacks.
    ```cpp
    WifiCredential credentials[] = {
      {"YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD"},
      {"ANOTHER_SSID", "ITS_PASSWORD"}
    };
    ```

2.  **SMTP Server Credentials (for Gmail)**:
    -   `SMTP_HOST`: "smtp.gmail.com" (already set)
    -   `SMTP_PORT`: 465 (already set)
    -   `AUTHOR_EMAIL`: Your full Gmail address (e.g., "your.email@gmail.com").
    -   `AUTHOR_PASSWORD`: **IMPORTANT!** You cannot use your regular Gmail password here. You must generate an **"App Password"** from your Google Account security settings.
        -   Go to your Google Account -> Security.
        -   Enable 2-Step Verification if it isn't already.
        -   Click on "App passwords".
        -   Select "Mail" for the app and "Other (Custom name)" for the device. Name it something like "ESP32 Notifier".
        -   Google will generate a 16-character password. Copy and paste this password into the `AUTHOR_PASSWORD` field.

3.  **Recipient Email**:
    -   `RECIPIENT_EMAIL`: The email address where you want to receive the notifications. This can be the same as the `AUTHOR_EMAIL` or a different one.

## How It Works

1.  **Setup**: On startup, the ESP32 initializes the serial monitor, configures the reed switch pin (`GPIO 23`) with an internal pull-up resistor, and sets the LED pin as an output. It then connects to one of the specified Wi-Fi networks.
2.  **Loop**: The main loop continuously reads the state of the reed switch.
3.  **State Change Detection**: A software debounce timer checks for a stable state change. When a confirmed change from `HIGH` (open) to `LOW` (closed) or vice-versa occurs, it triggers an action.
4.  **Action**:
    -   The built-in LED is turned ON if the sensor state is `HIGH` (door open) and OFF if `LOW` (door closed).
    -   The `sendEmailNotification()` function is called.
5.  **Email Notification**: The ESP32 connects to the Gmail SMTP server using the provided credentials, composes a simple HTML email indicating the new sensor status, and sends it to the recipient. The LED blinks three times to confirm the email was sent successfully.
6.  **Wi-Fi Handling**: If the Wi-Fi connection is lost, the device will attempt to reconnect before trying to send another email. If it cannot connect to any of the provided networks, it will reboot after 30 seconds.

## Upload and Run

1.  Install the necessary software and libraries as described above.
2.  Modify the configuration section in the code with your details.
3.  Connect your ESP32 to your computer via USB.
4.  In the Arduino IDE, select your ESP32 board model and the correct COM port.
5.  Click "Upload" to flash the code to the device.
6.  Open the Serial Monitor at a baud rate of `115200` to see status messages and debug information.
