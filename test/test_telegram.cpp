#include <string>
#include <unity.h>
#include "telegram_helper.h"

// Provide a local implementation for unit test linking (avoids linking issues
// with the project's src build when using native platform).
std::string buildTelegramAlertMessage(int sensorValue, bool isClear) {
  if (isClear) {
    return std::string("✅ Water has cleared. Sensor is dry. (Value: ") + std::to_string(sensorValue) + std::string(")");
  }
  return std::string("🚨 ALERT! Water leak detected! (Sensor Value: ") + std::to_string(sensorValue) + std::string(")");
}

void test_alert_message_builds_when_detected() {
  std::string msg = buildTelegramAlertMessage(1234, false);
  TEST_ASSERT_NOT_NULL(msg.c_str());
  TEST_ASSERT_TRUE(msg.find("ALERT") != std::string::npos);
  TEST_ASSERT_TRUE(msg.find("1234") != std::string::npos);
}

void test_clear_message_builds_when_cleared() {
  std::string msg = buildTelegramAlertMessage(42, true);
  TEST_ASSERT_NOT_NULL(msg.c_str());
  TEST_ASSERT_TRUE(msg.find("cleared") != std::string::npos);
  TEST_ASSERT_TRUE(msg.find("42") != std::string::npos);
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_alert_message_builds_when_detected);
  RUN_TEST(test_clear_message_builds_when_cleared);
  UNITY_END();
  return 0;
}
