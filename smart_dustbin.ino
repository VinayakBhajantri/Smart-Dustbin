#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Virat"
#define WIFI_PASSWORD "Virat@18"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBrOjC33cW0VfL7q7ikqOvHA__PNx-7ggY"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://garbage-monitor-46f39-default-rtdb.firebaseio.com/"

//Define Firebase Data object
FirebaseData fbd1;

FirebaseAuth auth;
FirebaseConfig config;
#include <ESP32Servo.h>
const int servoPin = 18;
const int pirPin = 19;
const int TRIG_PIN = 21;  // GPIO pin connected to Ultrasonic Sensor's TRIG pin
const int ECHO_PIN = 5;   // GPIO pin connected to Ultrasonic Sensor's ECHO pin
const int led =23;

float duration_us, distance_cm;
Servo servo;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;



void setup() {
  pinMode(pirPin, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(led,OUTPUT);
  Serial.begin(9600);
  servo.attach(servoPin, 500, 2400);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.begin(115200);
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

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration_us = pulseIn(ECHO_PIN, HIGH);
  distance_cm = duration_us * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");

  if (Firebase.ready()) {
    //for(int i=0;i<180;i++)
    {
      if (Firebase.RTDB.setInt(&fbd1, "/Fill_level2", distance_cm)) {
        Serial.println("Data sent to Firebase successfully");
      } else {
        Serial.println("Failed to send data to Firebase");
      }

      if (distance_cm > 8 && distance_cm < 100) {
        int pirState = digitalRead(pirPin);
        Serial.println("PIR State: " + String(pirState));
        if (pirState == HIGH) {
          for (int pos = 0; pos <= 360; pos++) {
            servo.write(pos);
            delay(10);
          }
          Serial.println("Dustbin opened");
          delay(1000);
        }
      } else if (distance_cm < 5) {
        Serial.println("Dustbin is filled");
        digitalWrite(led, HIGH);
      }
      digitalWrite(led, LOW);
    }
  } else {
    Serial.println("Firebase not ready");
    // You may want to attempt to reconnect to Firebase here
    // or implement a reconnection mechanism
  }
}