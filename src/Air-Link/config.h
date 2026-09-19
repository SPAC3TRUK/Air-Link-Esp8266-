#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>


// CONFIGURAZIONE BLYNK E WEB SERVER
// Inserisci qui le tue credenziali Blynk IoT
#define BLYNK_TEMPLATE_ID " " // Specifica il tuo Template ID di Blynk
#define BLYNK_TEMPLATE_NAME " " // Specifica il tuo Template Name di Blynk
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include <ESP8266WebServer.h>

// Istanza globale del server HTTP sulla porta 80
extern ESP8266WebServer server;

// LIBRERIE SENSORI
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <MQ135.h>

//LIBRERIE SETUP GOOGLE SHEETS 
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

//LIBRERIE GESTIONE ORARIO
#include <time.h>
#include <TZ.h>

// CREDENZIALI WIFI E AUTH BLYNK 

char auth[] = " ";// Inserisci le tue credenziali blynk personali
char ssid[] = " ";// Inserisci il tuo ssid wifi
char pass[] = ""; // Inserisci la tua password wifi

// PIN DI COLLEGAMENTO SENSORI
#define DHTPIN 4                 // Pin D2 su ESP8266 per DHT22
#define DHTTYPE DHT22   
#define BME_SDA 12               // Pin D6 su ESP8266 per BME280 SDA
#define BME_SCL 14               // Pin D5 su ESP8266 per BME280 SCL
#define FOTORESISTORE 5          // Pin D1 su ESP8266 per Fotoresistore
#define RAIN_SENSOR 0            // Pin D3 su ESP8266 per Sensore Pioggia
#define MQ135_PIN A0             // Pin A0 su ESP8266 per MQ135

#define SEALEVELPRESSURE_HPA (1013.25)  // Valore standard pressione livello mare

//DICHIARAZIONE ISTANZE GLOBALI SENSORI
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BME280 bme;
MQ135 gasSensor = MQ135(MQ135_PIN);
BlynkTimer timer;

// CONFIGURAZIONE GOOGLE APP SCRIPT

const char* scriptURL = " "; // URL fornito da Google Apps Script dopo il deploy come Web App

//VARIABILI GLOBALI PER MASSIME E MINIME GIORNALIERE
float tempMin = 100.0;
float tempMax = -100.0;
float umidMin = 100.0;
float umidMax = -100.0;
float co2Min = 10000.0;
float co2Max = -1.0;
float pressioneMin = 2000.0;
float pressioneMax = -1.0;

//FLAG DI CONTROLLO STATO GIORNALIERO
bool inviatoOggi = false;
bool diagnosticaMattina = false;
bool diagnosticaSera = false;

#endif // CONFIG_H
