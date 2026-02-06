#ifndef TELEGRAM_HELPER_H
#define TELEGRAM_HELPER_H

#ifdef UNIT_TEST
#include <string>
std::string buildTelegramAlertMessage(int sensorValue, bool isClear);
#else
#include <Arduino.h>
String buildTelegramAlertMessage(int sensorValue, bool isClear);
#endif

#endif
