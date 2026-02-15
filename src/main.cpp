#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "config.h" 
#include <time.h>
// Ensure config.h has WIFI_SSID, WIFI_PASSWORD, BOT_TOKEN, CHAT_ID defined

// -- CONFIGURATION --
// Add these to your config.h if not there:
// #define BOT_TOKEN "YOUR_BOT_TOKEN_FROM_BOTFATHER"
// #define CHAT_ID "YOUR_NUMERIC_ID_FROM_MYIDBOT"

// Initialize Telegram objects
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// Internal LED pin (ESP32 built-in LED is typically on GPIO 2)
#define LED_PIN 2

// Global Variables
unsigned long lastAlertTime = 0;
unsigned long lastBotCheck = 0;
unsigned long lastSensorCheck = 0;
unsigned long lastKeepAlive = 0;
unsigned long bootTime = 0;
const unsigned long BOT_CHECK_INTERVAL = 1000;  // Check for messages every 1 second
const unsigned long KEEPALIVE_EARLY = 15 * 60 * 1000;    // Every 15 min for first 2 hours
const unsigned long KEEPALIVE_NORMAL = 3 * 60 * 60 * 1000; // Every 3 hours after that
const unsigned long KEEPALIVE_EARLY_END = 2 * 60 * 60 * 1000; // 2 hours
bool tankEmpty = false; // Start assuming tank has water
unsigned long keepAliveOverride = 0; // 0 = use default schedule, >0 = custom interval in ms
// Rate limiting: max 10 messages per minute from any user
int msgCount = 0;
unsigned long msgWindowStart = 0;
const int MAX_MSGS_PER_MINUTE = 10;
// Configuration values are provided in include/config.h:
// SENSOR_PIN, CHECK_INTERVAL, DEBOUNCE_TIME
// XKC-Y25-PNP: analog read since output is very weak (~0.02V)

// Read sensor once and return both raw value and wet status
int lastSensorRaw = 0;

int readSensor() {
  lastSensorRaw = analogRead(SENSOR_PIN);
  return lastSensorRaw;
}

bool isWet(int raw) {
  return raw <= SENSOR_THRESHOLD;
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  // specific to ESP32: Ignore SSL certificate validation
  client.setInsecure();
  // Synchronize time for TLS certificate validation
  Serial.print("Syncing time with NTP");
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  time_t now = 0;
  int retries = 0;
  while ((now = time(nullptr)) < 1600000000 && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  if (now < 1600000000) {
    Serial.println("\nWarning: time sync failed or timed out. TLS certificate validation may fail.");
  } else {
    Serial.println("\nTime is synced.");
  }

  // Configure TLS: use insecure mode for simplicity (debug only)
  client.setInsecure();
  Serial.println("Using insecure TLS (debug only).");
}

void ensureWiFiConnected() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.println("WiFi disconnected, attempting reconnect...");
  WiFi.disconnect();
  WiFi.reconnect();
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi reconnected.");
  } else {
    Serial.println("\nWiFi reconnect failed.");
  }
}

void flashLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
    delay(150);
  }
}

void sendTelegramMessage(String message) {
  Serial.println("Sending Telegram: " + message);
  if (WiFi.status() != WL_CONNECTED) {
    ensureWiFiConnected();
  }
  bool ok = bot.sendMessage(CHAT_ID, message, "");
  if (ok) {
    Serial.println("Message sent successfully");
  } else {
    Serial.println("Failed to send message");
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    // Security: only allow authorized chat ID
    if (chat_id != CHAT_ID) {
      Serial.println("Unauthorized message from " + from_name + " (chat_id: " + chat_id + ")");
      bot.sendMessage(chat_id, "⛔ Unauthorized. Access denied.", "");
      continue;
    }

    // Rate limiting: reset window every 60 seconds
    if (millis() - msgWindowStart > 60000) {
      msgCount = 0;
      msgWindowStart = millis();
    }
    msgCount++;
    if (msgCount > MAX_MSGS_PER_MINUTE) {
      Serial.println("Rate limit exceeded, ignoring message");
      continue;
    }

    Serial.println("Message received from " + from_name + ": " + text);

    // Flash LED 3 times when any message is received
    flashLED(3);

    // Convert to lowercase for case-insensitive comparison
    text.toLowerCase();

    if (text == "test" || text == "/test") {
      // Read current sensor value (XKC-Y25-PNP: analog read)
      int raw = readSensor();
      bool wet = isWet(raw);

      String response = "📊 Current Sensor Reading:\n";
      response += "• Raw ADC: " + String(raw) + " (threshold: " + String(SENSOR_THRESHOLD) + ")\n";
      response += "• Status: " + String(wet ? "FULL 💧" : "EMPTY ⚠️");
      
      bot.sendMessage(chat_id, response, "");
      Serial.println("Sent sensor reading response");
    } else if (text.startsWith("change timer_")) {
      int minutes = text.substring(13).toInt();
      if (minutes > 0) {
        keepAliveOverride = (unsigned long)minutes * 60 * 1000;
        String response = "⏱ Keep-alive interval changed to " + String(minutes) + " minute(s).";
        bot.sendMessage(chat_id, response, "");
        Serial.println("Keep-alive interval changed to " + String(minutes) + " minutes");
        lastKeepAlive = millis(); // Reset timer so new interval starts now
      } else {
        bot.sendMessage(chat_id, "❌ Invalid timer value. Use: change timer_[minutes]\nExample: change timer_30", "");
      }
    } else if (text == "help" || text == "/help" || text == "/start") {
      String helpMsg = "🤖 Water Tank Bot Commands:\n";
      helpMsg += "• test - Get current tank level reading\n";
      helpMsg += "• change timer_[min] - Set keep-alive interval (e.g. change timer_30)\n";
      helpMsg += "• help - Show this help message";
      bot.sendMessage(chat_id, helpMsg, "");
    }
  }
}

void checkTelegramMessages() {
  if (millis() - lastBotCheck > BOT_CHECK_INTERVAL) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastBotCheck = millis();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  connectToWiFi();

  // Send a startup message so you know it works
  sendTelegramMessage("💧 Water Tank Monitor is Online!");

  // Startup test phase: continuous serial output for 2 minutes
  Serial.println("\n=== Starting 2-minute continuous sensor test (serial only) ===");
  Serial.println("Time(s) | ADC Raw | Threshold | Status");
  Serial.println("--------|---------|-----------|-------");
  sendTelegramMessage("🔬 Starting 2-min sensor test (check serial monitor)");

  const unsigned long testDuration = 2 * 60 * 1000;  // 2 minutes
  const unsigned long testInterval = 500;              // every 500ms
  unsigned long testStartTime = millis();

  while (millis() - testStartTime < testDuration) {
    int raw = readSensor();
    float elapsed = (millis() - testStartTime) / 1000.0;

    Serial.printf("%7.1f | %7d | %9d | %s\n", elapsed, raw, SENSOR_THRESHOLD, isWet(raw) ? "FULL" : "EMPTY");

    delay(testInterval);
  }

  Serial.println("=== Sensor test phase complete ===\n");
  sendTelegramMessage("✅ Test phase complete! Switching to normal monitoring mode.");

  bootTime = millis();
  lastKeepAlive = millis();
}

void loop() {
  // Check for incoming Telegram messages
  checkTelegramMessages();
  
  // 1. Read Sensor periodically (Non-blocking)
  if (millis() - lastSensorCheck > CHECK_INTERVAL) {
    lastSensorCheck = millis();

    int raw = readSensor();
    bool wet = isWet(raw);

    // Debug printing
    Serial.printf("Sensor pin %d | ADC: %d (threshold: %d) | Wet: %s\n", SENSOR_PIN, raw, SENSOR_THRESHOLD, wet ? "YES" : "NO");

    // 2. Alert Logic — alert when tank is empty (dry)
    if (!wet) {
      if (!tankEmpty || (millis() - lastAlertTime > DEBOUNCE_TIME)) {
        Serial.println("ALARM: TANK EMPTY!");
        sendTelegramMessage("🚨 ALERT! Water tank is empty!");

        tankEmpty = true;
        lastAlertTime = millis();
      }
    } else {
      if (tankEmpty) {
        Serial.println("Tank refilled.");
        sendTelegramMessage("✅ Water tank has been refilled.");
        tankEmpty = false;
      }
    }
  }

  // 3. Keep-alive message
  unsigned long uptime = millis() - bootTime;
  unsigned long keepAliveInterval = (keepAliveOverride > 0) ? keepAliveOverride :
    ((uptime < KEEPALIVE_EARLY_END) ? KEEPALIVE_EARLY : KEEPALIVE_NORMAL);
  if (millis() - lastKeepAlive > keepAliveInterval) {
    lastKeepAlive = millis();
    int raw = readSensor();
    unsigned long hours = uptime / 3600000;
    unsigned long mins = (uptime % 3600000) / 60000;
    String msg = "💓 Keep-alive | Uptime: " + String(hours) + "h " + String(mins) + "m";
    msg += " | Tank: " + String(isWet(raw) ? "FULL" : "EMPTY");
    msg += " | ADC: " + String(raw);
    sendTelegramMessage(msg);
  }
}