#ifndef SENSORS_MANAGER_H
#define SENSORS_MANAGER_H

#include "diagnostics.h"
#include "config.h"
#include "web_serial.h"

RisultatoDiagnostica eseguiControlloSensori();

//LETTURA SENSORT DHT22
void Dht22() {
  float temperatura = dht.readTemperature();
  float umidita = dht.readHumidity();

  // Verifica validità letture e aggiorna i record minimi e massimi giornalieri
  if (!isnan(temperatura)) {
    if (temperatura < tempMin) tempMin = temperatura;
    if (temperatura > tempMax) tempMax = temperatura;
    Blynk.virtualWrite(V0, temperatura); // Invio a Blynk Virtual Pin V0
  } else {
    logWebSerial("❌ Errore nella lettura della temperatura da DHT22");
  }

  if (!isnan(umidita)) {
    if (umidita < umidMin) umidMin = umidita;
    if (umidita > umidMax) umidMax = umidita;
    Blynk.virtualWrite(V1, umidita);     // Invio a Blynk Virtual Pin V1
  } else {
    logWebSerial("❌ Errore nella lettura dell'umidità da DHT22");
  }
}

//LETTURA SENSORE BME280
void Bme280() {
  float pressione = bme.readPressure() / 100.0F; // hPa

  if (!isnan(pressione) && pressione > 0) {
    if (pressione < pressioneMin) pressioneMin = pressione;
    if (pressione > pressioneMax) pressioneMax = pressione;
    Blynk.virtualWrite(V2, pressione);   // Invio a Blynk Virtual Pin V2
  } else {
    logWebSerial("❌ Errore nella lettura della pressione da BME280");
  }
}

// GESTIONE PIOGGIA E FOTORESISTORE
inline void MeteoLable() {
  String meteo;
  meteo.reserve(32); // Pre-alloca la memoria necessaria per evitare la frammentazione della RAM

  // Costruzione dinamica usando la macro F() per salvare la memoria Flash
  meteo = (digitalRead(RAIN_SENSOR) == LOW) ? F("🌧 Pioggia") : F("☀ No Pioggia");
  meteo += F(" | ");
  meteo += (digitalRead(FOTORESISTORE) == LOW) ? F("🌞 Giorno") : F("🌙 Notte");

  // Invio a Blynk Virtual Pin V4
  Blynk.virtualWrite(V4, meteo);
}

//LETTURA SENSORE QUALITÀ DELL'ARIA MQ135
void SensoreMQ135() {
  float temperatura = dht.readTemperature();
  float umidita = dht.readHumidity();

  // Se i valori DHT22 non sono validi, usiamo valori standard per la calibrazione
  if (isnan(temperatura) || isnan(umidita)) {
    temperatura = 20.0;
    umidita = 50.0;
  }

  float correctedPPM = gasSensor.getCorrectedPPM(temperatura, umidita);

  if (!isnan(correctedPPM) && correctedPPM >= 0) {
    if (correctedPPM < co2Min) co2Min = correctedPPM;
    if (correctedPPM > co2Max) co2Max = correctedPPM;
    Blynk.virtualWrite(V6, correctedPPM); // Invio a Blynk Virtual Pin V6
  } else {
    logWebSerial("Errore nella lettura o calcolo PPM da MQ135");
  }
}

//MANAGER COORDINATORE DEI SENSORI
void ManagerSensori() {
  logWebSerial("Avvio lettura ciclica e aggiornamento Blynk Cloud...");

  // Esegue il controllo di salute preliminare
  RisultatoDiagnostica res = eseguiControlloSensori();

  if (res.dhtOk) {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    Blynk.virtualWrite(V0, temp);
    Blynk.virtualWrite(V1, hum);
  }

  if (res.bmeOk) {
    float pressione = bme.readPressure() / 100.0F;
    Blynk.virtualWrite(V2, pressione);
  }

  if (res.mqOk) {
    float ppm = gasSensor.getPPM();
    Blynk.virtualWrite(V3, ppm);
  }

  // Notifica l'avvenuto invio a Blynk nel Seriale Web
  logWebSerial("[BLYNK] Pacchetto dati meteo inviato con successo!");
}

#endif // SENSORS_MANAGER_H
