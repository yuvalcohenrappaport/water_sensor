# ESP32 Water Sensor Alert System

A smart water level monitoring system that uses an ESP32-WROOM-32D microcontroller and an **XKC-Y25-PNP non-contact capacitive sensor** to send **Telegram alerts** when water is (or isn't) detected.

## Features

- 🚨 **Instant Telegram Alerts** - Get notified immediately when water is detected
- 📊 **On-Demand Readings** - Send "test" to the bot to get current sensor status
- 💡 **LED Feedback** - Built-in LED flashes when messages are received
- 🔄 **Auto-Reconnect** - Automatically reconnects WiFi if connection is lost
- 🧪 **Startup Test Phase** - 1-minute test with readings every 10 seconds on boot

## Components Required

- **ESP32-WROOM-32D**: Main microcontroller
- **XKC-Y25-PNP**: Non-contact capacitive water level sensor
- **10kΩ Resistor**: For voltage divider (signal line)
- **20kΩ Resistor**: For voltage divider (signal line)
- **Micro USB Cable**: For ESP32 programming and power
- **Jumper Wires**: For connections
- **Power Supply**: 5V USB or external power

## Wiring

Connect the XKC-Y25-PNP to the ESP32 **with a voltage divider** on the signal line:

| XKC-Y25-PNP Wire | Connection |
|-------------------|------------|
| Brown/Red (VCC)   | ESP32 5V   |
| Blue/Black (GND)  | ESP32 GND  |
| Yellow (Signal)   | → 10kΩ resistor → ESP32 GPIO 34 |
|                   | GPIO 34 → 20kΩ resistor → GND |

> ⚠️ **The voltage divider is required!** The sensor outputs 5V but ESP32 GPIOs only tolerate 3.3V.

## Setup Instructions

### 1. Install PlatformIO

- Download and install [Visual Studio Code](https://code.visualstudio.com/)
- Install the PlatformIO IDE extension from the VS Code marketplace

### 2. Create a Telegram Bot

1. Open Telegram and search for **@BotFather**
2. Send `/newbot` and follow the prompts to create your bot
3. Copy the **Bot Token** (looks like `123456789:ABCdefGHIjklMNOpqrsTUVwxyz`)
4. Search for **@myidbot** and send `/getid` to get your **Chat ID**

### 3. Configure Credentials

Edit [include/config.h](include/config.h) with your settings:

```cpp
// WiFi Configuration
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Telegram Configuration
#define BOT_TOKEN "YOUR_BOT_TOKEN"    // From BotFather
#define CHAT_ID "YOUR_CHAT_ID"        // From myidbot

// Sensor Configuration (XKC-Y25-PNP digital sensor)
#define SENSOR_PIN 34              // Digital input pin (GPIO34)
#define CHECK_INTERVAL 300000      // Check every 5 minutes (ms)
#define DEBOUNCE_TIME 60000        // Min time between alerts (ms)
```

### 4. Mount & Verify the Sensor

1. Mount the XKC-Y25-PNP flat against the outside of the tank wall
2. Flash the code and open Serial Monitor (115200 baud)
3. Verify: with water at sensor level → reads HIGH
4. Verify: without water at sensor level → reads LOW
5. Adjust the sensor's built-in sensitivity potentiometer if needed

> No threshold calibration needed — the XKC-Y25-PNP is a digital (on/off) sensor.

### 5. Build and Upload

In VS Code with PlatformIO:

1. Press `Ctrl+Shift+P` (or `Cmd+Shift+P` on Mac)
2. Type "PlatformIO: Build" and run
3. Type "PlatformIO: Upload" to flash to ESP32
4. Open Serial Monitor to see debug output

Or use terminal:
```bash
pio run --target upload -e esp32-wroom-32d
pio device monitor -b 115200
```

## Telegram Bot Commands

| Command | Description |
|---------|-------------|
| `test` or `/test` | Get current sensor reading |
| `help` or `/help` | Show available commands |
| `/start` | Welcome message with commands |

## How It Works

1. **Startup**: System connects to WiFi and syncs time via NTP
2. **Test Phase**: Runs a 1-minute test with readings every 10 seconds
3. **Monitoring**: Checks sensor every 5 minutes (configurable)
4. **Alerts**: Sends Telegram message when water is detected
5. **Clear Notification**: Notifies when water clears
6. **Interactive**: Responds to Telegram commands anytime

### Alert Flow

```
Water Detected → 🚨 ALERT sent to Telegram
Water Cleared  → ✅ Clear notification sent
```

## LED Behavior

- **3 flashes**: Telegram message received
- The built-in LED (GPIO 2) provides visual feedback

## Troubleshooting

### No Telegram messages
- Verify BOT_TOKEN and CHAT_ID in config.h
- Check WiFi connection in Serial Monitor
- Ensure you've started a chat with your bot first

### WiFi won't connect
- Double-check SSID and password
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- Check Serial Monitor for connection status

### Sensor readings incorrect
- Verify the voltage divider is wired correctly (measure ~3.3V at GPIO 34 when HIGH)
- Ensure sensor is mounted flat against a non-metallic wall
- Adjust the sensor's built-in sensitivity potentiometer
- Ensure proper 5V power supply to the sensor

### ESP32 won't upload
- Hold BOOT button while uploading starts
- Check USB cable (use data cable, not charge-only)
- Ensure correct COM port is selected

## Configuration Reference

| Parameter | Default | Description |
|-----------|---------|-------------|
| `SENSOR_PIN` | 34 | Digital input pin for water sensor |
| `CHECK_INTERVAL` | 300000 | Sensor check interval (5 min) |
| `DEBOUNCE_TIME` | 60000 | Min time between alerts (1 min) |
| `BOT_CHECK_INTERVAL` | 1000 | Telegram polling interval (1 sec) |

## Project Structure

```
water_sensor/
├── include/
│   └── config.h          # WiFi, Telegram, and sensor settings
├── src/
│   └── main.cpp          # Main application code
├── platformio.ini        # PlatformIO configuration
└── README.md             # This file
```

## Dependencies

Managed automatically by PlatformIO:
- **UniversalTelegramBot** - Telegram Bot API
- **ArduinoJson** - JSON parsing for Telegram
- **WiFi** - ESP32 WiFi library
- **WiFiClientSecure** - HTTPS connections

## License

This project is open source and available for personal use.

