#ifndef WEB_SERIAL_H
#define WEB_SERIAL_H

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// Dichiara che server esiste nel file .ino principale
extern ESP8266WebServer server;

// CONFIGURAZIONE BUFFER FISSO SERIALE
const size_t SER_MAX_LOGS = 15;      // Numero massimo di righe da conservare
const size_t SER_MAX_LEN  = 90;      // Lunghezza massima di ogni riga di testo

char serialBuffer[SER_MAX_LOGS][SER_MAX_LEN];
size_t serialHead = 0;  
size_t serialCount = 0; 

// Funzione per inviare log sia al Seriale fisico che al Seriale Web
inline void logWebSerial(const String& msg) {
  Serial.println(msg);

  snprintf(serialBuffer[serialHead], SER_MAX_LEN, "%s", msg.c_str());
  
  serialHead = (serialHead + 1) % SER_MAX_LOGS;
  if (serialCount < SER_MAX_LOGS) {
    serialCount++;
  }
}

// Handler HTTP per /api/serial
inline void handleApiSerial() {
  StaticJsonDocument<1024> doc;
  JsonArray logs = doc.createNestedArray("logs");

  size_t startIdx = (serialCount < SER_MAX_LOGS) ? 0 : serialHead;

  for (size_t i = 0; i < serialCount; i++) {
    size_t idx = (startIdx + i) % SER_MAX_LOGS;
    logs.add(serialBuffer[idx]);
  }

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  server.send(200, "application/json", jsonResponse);
}

// CONFIGURAZIONE STRUTTURA EVENTI DI SISTEMA
struct EventoData {
  String timestamp;
  String tipo;    // "OK", "ERROR", "INFO"
  String messaggio;
};

const int MAX_EVENTI = 15;
EventoData listaEventi[MAX_EVENTI];
int eventiIndex = 0;

// Funzione per aggiungere un Evento di Sistema
inline void aggiungiEventoSistema(String tipo, String messaggio) {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  char timeStr[20];
  if (now > 100000) {
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeinfo);
  } else {
    snprintf(timeStr, sizeof(timeStr), "Boot+%lus", millis() / 1000);
  }

  listaEventi[eventiIndex].timestamp = String(timeStr);
  listaEventi[eventiIndex].tipo = tipo;
  listaEventi[eventiIndex].messaggio = messaggio;

  eventiIndex = (eventiIndex + 1) % MAX_EVENTI;
}

// Handler HTTP per /api/events
inline void handleApiEvents() {
  StaticJsonDocument<1024> doc;
  JsonArray arr = doc.createNestedArray("events");

  for (int i = 0; i < MAX_EVENTI; i++) {
    int idx = (eventiIndex + i) % MAX_EVENTI;
    if (listaEventi[idx].messaggio.length() > 0) {
      JsonObject obj = arr.createNestedObject();
      obj["time"] = listaEventi[idx].timestamp;
      obj["type"] = listaEventi[idx].tipo;
      obj["msg"]  = listaEventi[idx].messaggio;
    }
  }

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

#endif // WEB_SERIAL_H
