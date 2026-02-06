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
const unsigned long BOT_CHECK_INTERVAL = 1000;  // Check for messages every 1 second
bool waterDetected = false; // Start assuming no water
// Configuration values are provided in include/config.h:
// SENSOR_PIN, SENSOR_THRESHOLD, CHECK_INTERVAL, DEBOUNCE_TIME

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
    
    Serial.println("Message received from " + from_name + ": " + text);
    
    // Flash LED 3 times when any message is received
    flashLED(3);
    
    // Convert to lowercase for case-insensitive comparison
    text.toLowerCase();
    
    if (text == "test" || text == "/test") {
      // Read current sensor value
      int sensorValue = analogRead(SENSOR_PIN);
      bool isWet = (sensorValue > SENSOR_THRESHOLD);
      
      String response = "📊 Current Sensor Reading:\n";
      response += "• Value: " + String(sensorValue) + "\n";
      response += "• Status: " + String(isWet ? "WET 💦" : "DRY ✅") + "\n";
      response += "• Threshold: " + String(SENSOR_THRESHOLD);
      
      bot.sendMessage(chat_id, response, "");
      Serial.println("Sent sensor reading response");
    } else if (text == "help" || text == "/help" || text == "/start") {
      String helpMsg = "🤖 Water Sensor Bot Commands:\n";
      helpMsg += "• test - Get current sensor reading\n";
      helpMsg += "• help - Show this help message";
      bot.sendMessage(chat_id, helpMsg, "");
    }
  }
}

void checkTelegramMessages() {
  if (millis() - lastBotCheck > BOT_CHECK_INTERVAL) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    if (numNewMessages) {
      handleNewMessages(numNewMessages);
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
  sendTelegramMessage("💧 Water Sensor System is Online!");

  // Startup test phase: check sensor every 10 seconds for 1 minute
  Serial.println("\n=== Starting 1-minute sensor test phase ===");
  sendTelegramMessage("🔬 Starting 1-minute sensor test (readings every 10 sec)");
  
  const unsigned long testDuration = 1 * 60 * 1000;  // 1 minutes in milliseconds
  const unsigned long testInterval = 10 * 1000;       // 10 seconds in milliseconds
  unsigned long testStartTime = millis();
  int readingCount = 0;
  
  while (millis() - testStartTime < testDuration) {
    readingCount++;
    int sensorValue = analogRead(SENSOR_PIN);
    bool isWet = (sensorValue > SENSOR_THRESHOLD);
    
    String statusMsg = "📊 Test #" + String(readingCount) + 
                       " | Value: " + String(sensorValue) + 
                       " | Status: " + (isWet ? "WET 💦" : "DRY ✅") +
                       " | Threshold: " + String(SENSOR_THRESHOLD);
    
    Serial.println(statusMsg);
    sendTelegramMessage(statusMsg);
    
    // Wait 30 seconds before next reading (unless test period is over)
    unsigned long elapsed = millis() - testStartTime;
    unsigned long waitTime = 0;

    if (elapsed + testInterval < testDuration) {
      waitTime = testInterval;
    } else {
      if (testDuration > elapsed) waitTime = testDuration - elapsed;
    }

    unsigned long waitStart = millis();
    while (millis() - waitStart < waitTime) {
      checkTelegramMessages();
      delay(20);
    }
  }
  
  Serial.println("=== Sensor test phase complete ===\n");
  sendTelegramMessage("✅ Test phase complete! Switching to normal monitoring mode.");
}

void loop() {
  // Check for incoming Telegram messages
  checkTelegramMessages();
  
  // 1. Read Sensor periodically (Non-blocking)
  if (millis() - lastSensorCheck > CHECK_INTERVAL) {
    lastSensorCheck = millis();

    int sensorValue = analogRead(SENSOR_PIN);
    bool isWet = (sensorValue > SENSOR_THRESHOLD);

    // Debug printing (optional, helps tuning)
    // Serial.printf("Value: %d | Wet: %s\n", sensorValue, isWet ? "YES" : "NO");

    // 2. Alert Logic
    if (isWet) {
      // If this is a NEW detection (or enough time passed since last alert)
      if (!waterDetected || (millis() - lastAlertTime > DEBOUNCE_TIME)) {
        
        Serial.println("ALARM: WATER DETECTED!");
        sendTelegramMessage("🚨 ALERT! Water leak detected! (Sensor Value: " + String(sensorValue) + ")");
        
        waterDetected = true;
        lastAlertTime = millis();
      }
    } else {
      // If sensor is dry, reset the flag so we can alert again instantly if water returns
      if (waterDetected) {
        Serial.println("Water cleared.");
        sendTelegramMessage("✅ Water has cleared. Sensor is dry.");
        waterDetected = false;
      }
    }
  }
}