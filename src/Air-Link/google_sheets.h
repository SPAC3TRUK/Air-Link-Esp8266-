#ifndef GOOGLE_SHEETS_H
#define GOOGLE_SHEETS_H

#include "config.h"
#include "web_serial.h"

// INVIO RECORD GIORNALIERI A GOOGLE SHEETS VIA APPS SCRIPT
inline void InvioDatiGoogle() {
  yield(); // Previene il blocco del Watchdog Timer
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); // Salva RAM evitando la catena dei certificati TLS pesanti

    HTTPClient http;
    logWebSerial(F("[GoogleSheets] Connessione in corso..."));
    
    http.begin(client, scriptURL);
    http.addHeader(F("Content-Type"), F("application/json"));

    // Pre-allocazione memoria JSON
    String payload;
    payload.reserve(256);
    payload = F("{\"tempMin\":");
    payload += String(tempMin, 1);
    payload += F(",\"tempMax\":");
    payload += String(tempMax, 1);
    payload += F(",\"umidMin\":");
    payload += String(umidMin, 1);
    payload += F(",\"umidMax\":");
    payload += String(umidMax, 1);
    payload += F(",\"co2Min\":");
    payload += String(co2Min, 1);
    payload += F(",\"co2Max\":");
    payload += String(co2Max, 1);
    payload += F(",\"pressioneMin\":");
    payload += String(pressioneMin, 1);
    payload += F(",\"pressioneMax\":");
    payload += String(pressioneMax, 1);
    payload += F("}");

    int httpResponseCode = http.POST(payload);
    
    // stampa il numero effettivo del codice HTTP
    logWebSerial("[GoogleSheets] Risposta HTTP: " + String(httpResponseCode));

    http.end();

    // Ripristino limiti per la nuova giornata
    tempMin = 100.0; tempMax = -100.0;
    umidMin = 100.0; umidMax = -100.0;
    co2Min = 10000.0; co2Max = -1.0;
    pressioneMin = 2000.0; pressioneMax = -1.0;
    logWebSerial(F("[GoogleSheets] Limiti resettati per la nuova giornata"));
  } else {
    logWebSerial(F("[GoogleSheets] Errore: WiFi non connesso"));
  }
  yield();
}

// CONTROLLO SCHEDULATO A MEZZANOTTE PER INVIO GOOGLE SHEETS
void checkMezzanotte() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  int ora = timeinfo->tm_hour;
  int minuto = timeinfo->tm_min;

  // Se scatta la mezzanotte e non abbiamo ancora inviato i dati riassuntivi oggi
  if (ora == 0 && minuto == 0 && !inviatoOggi) {
    logWebSerial("Scattata la mezzanotte! Invio dati a Google Sheets...");
    InvioDatiGoogle();
    inviatoOggi = true;
  }

  // Resetta il flag di invio all'una di notte per il ciclo successivo
  if (ora == 1 && inviatoOggi) {
    inviatoOggi = false;
  }
}

#endif // GOOGLE_SHEETS_H
