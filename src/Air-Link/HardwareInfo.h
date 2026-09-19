#ifndef HARDWARE_INFO_H
#define HARDWARE_INFO_H

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// Struttura dati per le informazioni Hardware
struct HardwareMetrics {
  uint32_t cpuFreq;
  uint32_t freeHeap;
  uint32_t flashSize;
  uint32_t flashSpeed;
  String sdkVersion;
  String resetReason;
  String chipId;
  String macAddress;
  String hostname;
  String localIP;
  String gatewayIP;
  int32_t rssi;
  int32_t channel;
  String uptime;
};

// Funzione helper per formattare l'uptime
inline String getUptimeFormatted() {
  unsigned long totalSeconds = millis() / 1000;
  unsigned long days = totalSeconds / 86400;
  unsigned long hours = (totalSeconds % 86400) / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  unsigned long seconds = totalSeconds % 60;

  String res = "";
  if (days > 0) res += String(days) + "g ";
  if (hours > 0 || days > 0) res += String(hours) + "h ";
  res += String(minutes) + "m ";
  res += String(seconds) + "s";
  return res;
}

// Genera il JSON con tutte le info hardware
inline String getHardwareJson() {
  DynamicJsonDocument doc(512);

  doc["cpuFreq"] = String(ESP.getCpuFreqMHz()) + " MHz";
  doc["freeHeap"] = String(ESP.getFreeHeap() / 1024) + " KB";
  doc["flashSize"] = String(ESP.getFlashChipRealSize() / (1024 * 1024)) + " MB";
  doc["flashSpeed"] = String(ESP.getFlashChipSpeed() / 1000000) + " MHz";
  doc["sdk"] = ESP.getSdkVersion();
  doc["resetReason"] = ESP.getResetReason();
  doc["chipId"] = String(ESP.getChipId(), HEX);
  doc["mac"] = WiFi.macAddress();
  doc["hostname"] = WiFi.hostname();
  doc["ip"] = WiFi.localIP().toString();
  doc["gateway"] = WiFi.gatewayIP().toString();
  doc["rssi"] = String(WiFi.RSSI()) + " dBm";
  doc["rssiVal"] = WiFi.RSSI();
  doc["channel"] = WiFi.channel();
  doc["uptime"] = getUptimeFormatted();

  String output;
  serializeJson(doc, output);
  return output;
}

#endif // HARDWARE_INFO_H
