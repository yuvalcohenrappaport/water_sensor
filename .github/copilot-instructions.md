# Water Sensor Alert System - Project Setup

## Project Overview
ESP32-based water tank monitoring system with email alerts using an Elegoo water sensor.

## Setup Checklist

- [x] Project Requirements Clarified
  - ESP32-WROOM-32D microcontroller
  - Elegoo water sensor for water level detection
  - Email alerting when sensor stops detecting water

- [x] Project Structure Scaffolded
  - PlatformIO project structure created
  - Configuration files in place
  - Source code implemented

- [x] Configuration Files Created
  - platformio.ini: Build configuration for ESP32
  - config.h: WiFi and email settings (requires customization)
  - main.cpp: Core firmware logic
  - README.md: Complete setup and troubleshooting guide

## Next Steps for User

1. **Configure Credentials** (REQUIRED)
   - Edit `include/config.h`
   - Add WiFi SSID and password
   - Add Gmail credentials (use App Password for Gmail)

2. **Calibrate Sensor**
   - Upload initial code
   - Check Serial Monitor for sensor readings
   - Adjust SENSOR_THRESHOLD based on water/air readings

3. **Build and Upload**
   - Use PlatformIO: Build
   - Use PlatformIO: Upload to flash ESP32

4. **Monitor**
   - Use PlatformIO Serial Monitor (115200 baud)
   - System checks every 5 minutes and alerts on water loss

## Key Features

- WiFi connectivity with automatic reconnection
- SMTP-based email alerts
- Configurable sensor threshold with debouncing
- Detailed serial logging for debugging
- Low power monitoring every 5 minutes
