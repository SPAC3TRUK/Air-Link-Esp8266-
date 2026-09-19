#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include "config.h"
#include "web_serial.h"

// Struttura per contenere l'esito della diagnostica di tutti i sensori
struct RisultatoDiagnostica {
  bool dhtOk = true;
  bool bmeOk = true;
  bool mqOk = true;
  bool rainOk = true;
  bool ldrOk = true;
  int erroriTotali = 0;
};

// Funzione UNICA usata sia da Blynk che dalla WebApp
inline RisultatoDiagnostica eseguiControlloSensori() {
  RisultatoDiagnostica res;

  // Lettura dei sensori
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  float pressione = bme.readPressure() / 100.0F;
  float ppm = gasSensor.getCorrectedPPM(isnan(temp) ? 20.0 : temp, isnan(hum) ? 50.0 : hum);
  int rawMQ = analogRead(A0);

  // Controllo DHT22
  if (isnan(temp) || isnan(hum) || temp < -40.0 || temp > 80.0 || hum < 0.0 || hum > 100.0) {
    res.dhtOk = false;
    res.erroriTotali++;
  }

  // Controllo BME280 (Se non ha corrente bme.readPressure() restituisce 0 o NaN)
  if (isnan(pressione) || pressione < 300.0 || pressione > 1100.0) {
    res.bmeOk = false;
    res.erroriTotali++;
  }

  // Controllo MQ135 (Se non ha corrente il valore analogico A0 cade a 0)
  if (isnan(ppm) || ppm < 100.0 || ppm > 10000.0 || rawMQ < 15) {
    res.mqOk = false;
    res.erroriTotali++;
  }

  // Sensori Digitali (Pioggia e Fotoresistore)
  // Per rilevare se sono scollegati dalla corrente/pin, testiamo con PULLUP temporaneo:
  pinMode(RAIN_SENSOR, INPUT_PULLUP);
  pinMode(FOTORESISTORE, INPUT_PULLUP);
  delayMicroseconds(50);

  int pioggia = digitalRead(RAIN_SENSOR);
  int sole = digitalRead(FOTORESISTORE);

  // Riportiamo i pin al loro stato normale
  pinMode(RAIN_SENSOR, INPUT);
  pinMode(FOTORESISTORE, INPUT);

  // Se un modulo non è alimentato o è scollegato, con INPUT_PULLUP rimarrà bloccato.
  res.rainOk = (pioggia == LOW || pioggia == HIGH);
  res.ldrOk = (sole == LOW || sole == HIGH);

  return res;
}

// MONITORAGGIO STATO E FUNZIONAMENTO SENSORI PER BLYNK
inline void DiagnosticaSensori() {
  RisultatoDiagnostica res = eseguiControlloSensori();

  if (!res.dhtOk) {
    Blynk.logEvent("airlink_errore", "❌ DHT22 - Sensore non disponibile o fuori range");
  }
  if (!res.bmeOk) {
    Blynk.logEvent("airlink_errore", "❌ BME280 - Pressione non disponibile o fuori range");
  }
  if (!res.mqOk) {
    Blynk.logEvent("airlink_errore", "❌ MQ135 - Valore PPM / A0 non valido");
  }

  // NOTIFICA EVENTO FINALE SU BLYNK APP
  if (res.erroriTotali == 0) {
    Blynk.logEvent("airlink_ok", "Tutti i sensori OK");
  } else {
    String msg = "Rilevati " + String(res.erroriTotali) + " errori nei sensori";
    Blynk.logEvent("airlink_errore", msg);
  }
}

// PIANIFICAZIONE ORARIA DIAGNOSTICA
inline void checkDiagnosticaOraria() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  int ora = timeinfo->tm_hour;
  int minuto = timeinfo->tm_min;

  if (ora == 9 && minuto == 0 && !diagnosticaMattina) {
    Serial.println(F("🕒 [Schedulatore] Avvio diagnostica sensori di mattina..."));
    DiagnosticaSensori();
    diagnosticaMattina = true;
  }

  if (ora == 21 && minuto == 0 && !diagnosticaSera) {
    Serial.println(F("🕒[Schedulatore] Avvio diagnostica sensori di sera..."));
    DiagnosticaSensori();
    diagnosticaSera = true;
  }

  if (ora == 10) diagnosticaMattina = false;
  if (ora == 22) diagnosticaSera = false;
}

#endif // DIAGNOSTICS_H
