#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include "DHT.h"

// ---------------- WIFI ----------------
const char* ssid = "WiFi-SSID"; // Enter your ssid and password
const char* password = "WiFi-PASSWORD";

// ---------------- PINS ----------------
const int motorPin1 = D1;
const int motorPin2 = D2;
const int DHTPIN = D7;
#define DHTTYPE DHT11
#define sensorPin A0

// ---------------- OBJECTS ----------------
DHT dht(DHTPIN, DHTTYPE);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ---------------- FIREBASE ----------------
const char* db_url = "Firebase-url";
const char* api_key = "Firebase-API-key";

// ---------------- THRESHOLDS (BOTANICAL) ----------------
#define SOIL_DRY 80          // Soil dryness threshold
#define HOT_TEMP 30          // °C
#define LOW_HUMIDITY 40      // %

bool signupOK = false;
bool ledstatus = false;
unsigned long lastReadMillis = 0;

// --------------------------------------------------------
void setup()
{
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  Serial.begin(115200);
  dht.begin();

  // WIFI
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  // FIREBASE
  config.api_key = api_key;
  config.database_url = db_url;

  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("Firebase signup OK");
    signupOK = true;
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

// --------------------------------------------------------
void loop()
{
  if (millis() - lastReadMillis < 3000) return;
  lastReadMillis = millis();

  // -------- SENSOR READINGS --------
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int rawSoil = analogRead(sensorPin);
  int moisture = map(rawSoil, 0, 1023, 255, 0);

  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("DHT read failed");
    return;
  }

  // -------- SEND DATA TO FIREBASE --------
  Firebase.RTDB.setInt(&fbdo, "/Moisture", moisture);
  Firebase.RTDB.setFloat(&fbdo, "/Temperature", temperature);
  Firebase.RTDB.setFloat(&fbdo, "/Humidity", humidity);

  // -------- BOTANICALLY CORRECT IRRIGATION --------
  if (moisture > SOIL_DRY)   // Soil is dry → roots need water
  {
    int pumpTime = 2000;    // default duration

    // Transpiration-based adjustment
    if (temperature >= HOT_TEMP && humidity <= LOW_HUMIDITY)
    {
      pumpTime = 5000;  // High transpiration
      Serial.println("High transpiration → long watering");
    }
    else if (temperature < HOT_TEMP && humidity > 60)
    {
      pumpTime = 1500;  // Low transpiration
      Serial.println("Low transpiration → short watering");
    }
    else
    {
      Serial.println("Normal transpiration → medium watering");
    }

    // Run pump
    Firebase.RTDB.setBool(&fbdo, "/Pump-status", true);
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
    delay(pumpTime);
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
  
  }
  else
  {
    // Soil already moist
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    Firebase.RTDB.setBool(&fbdo, "/Pump-status", false);
  }

  // -------- SERIAL OUTPUT --------
  Serial.println("----- STATUS -----");
  Serial.print("Soil Moisture: "); Serial.println(moisture);
  Serial.print("Temperature : "); Serial.print(temperature); Serial.println(" C");
  Serial.print("Humidity    : "); Serial.print(humidity); Serial.println(" %");

  if(Firebase.RTDB.getBool(&fbdo, "/Pump-status"))
    {
      if(fbdo.dataType() == "boolean")
      {
        ledstatus = fbdo.boolData();
        Serial.println("Successful READ from "+fbdo.dataPath()+": "+ledstatus+" {"+fbdo.dataType() +"}");
        if (ledstatus == true)
        {
          digitalWrite(motorPin1, HIGH);
          digitalWrite(motorPin2, LOW);
          delay(4000);
          digitalWrite(motorPin1, LOW);
          digitalWrite(motorPin2, LOW);
          if(Firebase.RTDB.setFloat(&fbdo, "/Pump-status", false))
          {
            Serial.print("Pump-status: ");
            Serial.println(false);
          }
          else
          {
            Serial.print("Pump FAILED: ");
            Serial.println(fbdo.errorReason());
          }
        }
      }
    }
}
