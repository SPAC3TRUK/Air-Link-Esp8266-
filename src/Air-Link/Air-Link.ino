/*
 * =========================================================================
 *                  STAZIONE METEOROLOGICA AIR-LINK (ESP8266)
 * =========================================================================
 * 
 * Descrizione:
 * Stazione meteo IoT basata su ESP8266. Esegue il monitoraggio in tempo reale 
 * con sensori (DHT22, BME280, MQ135, Raindrop Module, Fotoresistore), trasmette le 
 * letture all'app Blynk IoT ogni 30 minuti, esegue una diagnostica pianificata 
 * dello stato dei sensori due volte al giorno, e a mezzanotte invia i dati 
 * minimi e massimi registrati a Google Sheets tramite Google Apps Script.
 * Possibilità di visualizzare i dati anche su una webapp locale
 * 
 * Struttura dei File del Progetto:
 * - Airlink.ino         : Punto di ingresso principale, Setup e Loop.
 * - config.h            : Configurazione di Blynk, WiFi,Google, Pin e Variabili Globali.
 * - sensors_manager.h   : Logica di lettura, elaborazione e invio dei sensori.
 * - diagnostics.h       : Gestione della diagnostica dei sensori oraria.
 * - google_sheets.h     : Logica di invio dei record min/max a Google Sheets.
 * - WebApp.h            : Logica e codice Webapp locale + EndPoint + Handler
 * - HardwareInfo.h      : Logica e EndPoint per recuperare dati del hardware del microcontrollore
 * - web_serial.h        : Logica e EndPoint per recuperare dati dal monitor seriale e mandarli alla webapp
 * - 1.png.h             : Array in bite per l'icona della webapp in png
 * Autore: Christian Mosci - SPAC3TRUK - christian_mosci
 * =========================================================================
 */

#include "config.h"
#include "sensors_manager.h"
#include "diagnostics.h"
#include "google_sheets.h"
#include "1.png.h"
#include "WebApp.h"
#include "HardwareInfo.h"
#include "web_serial.h"


// Crea l'istanza del server sulla porta 80
ESP8266WebServer server(80);

void setup() {
  Serial.begin(9600);
  delay(1500);
  logWebSerial("==============================================");
  logWebSerial("AirLink in avvio... ");
  logWebSerial("==============================================");

  // Inizializzazione della connessione Blynk IoT e WiFi
  Blynk.begin(auth, ssid, pass);

  // Stampa l'IP locale sul Monitor Seriale e digitale su webApp
  Serial.print("[WiFi] Connesso! Indirizzo IP: ");
  Serial.println(WiFi.localIP());
  logWebSerial("[WiFi] Connesso! IP: " + WiFi.localIP().toString());

  // Inizializzazione dei moduli Hardware / Sensori
  dht.begin();                      // DHT22 (Temperatura & Umidità)
  Wire.begin(BME_SDA, BME_SCL);     // BME280 (SDA=Pin D6, SCL=Pin D5)
  
  if (!bme.begin(0x76)) {
    logWebSerial("[BME280] Sensore BME280 non trovato");
  } else {
    logWebSerial("[BME280] Connessione riuscita.");
  }

  pinMode(RAIN_SENSOR, INPUT);      // Pin digitale sensore pioggia
  pinMode(FOTORESISTORE, INPUT);    // Pin digitale fotoresistore
  
  //Calibrazione iniziale sensore gas MQ135
  logWebSerial("[MQ135] Calibrazione sensore in corso");
  delay(5000);
  float rzero = gasSensor.getRZero();
  logWebSerial("[MQ135] RZero calcolato: " + String(rzero));
  
  // Re-inizializza il sensore con la corretta resistenza base di calibrazione (RZero)
  gasSensor = MQ135(MQ135_PIN, rzero);

  // Configurazione orario di sistema tramite protocollo NTP con timeout anti-crash
  configTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");
  logWebSerial(F("[NTP] Sincronizzazione dell'orario con i server NTP..."));
  
  int timeoutNTP = 0;
  while (!time(nullptr) && timeoutNTP < 20) { // Max 10 secondi di attesa
    delay(500);
    logWebSerial(F("."));
    yield(); // Previene il reset del Watchdog
    timeoutNTP++;
  }

  if (time(nullptr)) {
    logWebSerial(F("\n[NTP] Orario sincronizzato con successo!"));
  } else {
    logWebSerial(F("\n[NTP] Timeout: Sincronizzazione fallita, il sistema proseguirà comunque."));
  }

  // Configurazione dei timer schedulati con BlynkTimer
  timer.setInterval(1800000L, ManagerSensori);           // Legge e invia i dati a Blynk ogni 30 MINUTI
  timer.setInterval(60000L, checkMezzanotte);         // Verifica se inviare i dati riepilogativi a mezzanotte (ogni 1 min)
  timer.setInterval(60000L, checkDiagnosticaOraria);  // Esegue la diagnostica pianificata alle 09:00 e 21:00 (ogni 1 min)

  logWebSerial("Airlink Configurazione completata! Stazione attiva.");
  logWebSerial("==============================================");

// Rotta Favicon classica da PROGMEM
server.on("/favicon.png", HTTP_GET, []() {
  server.send_P(200, "image/png", (const char*)__1_png, sizeof(__1_png));
});

//Rotta per l'icona PWA da PROGMEM
server.on("/icon-192.png", HTTP_GET, []() {
  server.send_P(200, "image/png", (const char*)__1_png, sizeof(__1_png));
});

//Rotta per il manifest.json
server.on("/manifest.json", HTTP_GET, handleManifest);

// Rotta Service Worker
server.on("/sw.js", HTTP_GET, []() {
  server.send_P(200, "application/javascript", "self.addEventListener('fetch', function(e) {});");
});


  initWebApp(); // Avvia le rotte della web app

  // Registra le rotte API
  server.on("/api/serial", HTTP_GET, handleApiSerial);
  server.on("/api/events", HTTP_GET, handleApiEvents);

  server.begin(); // Avvia il Web Server DOPO la registrazione di tutte le rotte

  logWebSerial("[SYSTEM] ESP8266 avviato correttamente!");
}


void loop() {
  Blynk.run(); // Mantiene attiva la comunicazione bidirezionale con Blynk Cloud
  timer.run(); // Gestisce ed esegue i compiti schedulati dai timer
  server.handleClient(); // Gestisce le richieste web in background
  // Rilascia la CPU al sistema operativo per evitare il crash del Watchdog
  yield();
}
