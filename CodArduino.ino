#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

const char* WIFI_SSID = "MJMMiller";
const char* WIFI_PASSWORD = "0150303923";
const char* API_KEY = "AIzaSyAgVSPRpZErhCVV8-M3pV3TZ9mc71qR3Fs";
const char* DATABASE_URL = "https://segues-71c59-default-rtdb.firebaseio.com";
const char* USER_EMAIL = "segues1605@gmail.com";
const char* USER_PASSWORD = "segues1605";

const int pinLedGreen = 22; // LED verde
const int pinLedRed = 23;   // LED rojo (ahora en el pin 27)
const int pinEco = 19;
const int pinTrigger = 21;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
const unsigned long sendDataInterval = 500; // Intervalo de envío de datos en milisegundos

void setup_WIFI(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void setupFirebase() {
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  config.signer.tokens.legacy_token = "<database secret>";
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096, 1024);
  fbdo.setResponseSize(2048);
  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);
  config.timeout.serverResponse = 10 * 1000;
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
}

float readDistance() {
  digitalWrite(pinTrigger, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrigger, LOW);
  float duration = pulseIn(pinEco, HIGH);
  float distance = duration * 0.034 / 2;
  return distance;
}

void sendDataToFirebase(bool data) {
  if (Firebase.RTDB.setBool(&fbdo, F("/sectorA/f1/c1"), data)) {
    Serial.println("Data sent successfully.");
  } else {
    Serial.print("Failed to send data: ");
    Serial.println(fbdo.errorReason().c_str());
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(pinLedGreen, OUTPUT);
  pinMode(pinLedRed, OUTPUT);
  pinMode(pinEco, INPUT);
  pinMode(pinTrigger, OUTPUT);
  digitalWrite(pinLedGreen, HIGH); // Inicializar el LED verde encendido
  digitalWrite(pinLedRed, LOW);    // Inicializar el LED rojo apagado
  setup_WIFI();
  setupFirebase();
  
  sendDataToFirebase(true); // Enviar "true" a la base de datos al inicio
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - sendDataPrevMillis >= sendDataInterval) {
    sendDataPrevMillis = currentMillis;
    float distance = readDistance();
    bool signalReceived = (distance < 17);
    digitalWrite(pinLedGreen, !signalReceived);
    digitalWrite(pinLedRed, signalReceived);
    sendDataToFirebase(!signalReceived);
  }
}
