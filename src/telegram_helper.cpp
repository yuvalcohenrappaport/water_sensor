
#include "telegram_helper.h"

#ifdef UNIT_TEST
std::string buildTelegramAlertMessage(int sensorValue, bool isClear) {
  if (isClear) {
    return std::string("✅ Water has cleared. Sensor is dry. (Value: ") + std::to_string(sensorValue) + std::string(")");
  }
  return std::string("🚨 ALERT! Water leak detected! (Sensor Value: ") + std::to_string(sensorValue) + std::string(")");
}
#else
String buildTelegramAlertMessage(int sensorValue, bool isClear) {
  if (isClear) {
    return String("✅ Water has cleared. Sensor is dry. (Value: ") + String(sensorValue) + String(")");
  }
  return String("🚨 ALERT! Water leak detected! (Sensor Value: ") + String(sensorValue) + String(")");
}
#endif
