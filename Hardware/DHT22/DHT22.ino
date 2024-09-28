// Description:
// This Arduino code connects an ESP32 to a DHT22 sensor, Firebase, and Wi-Fi 
// to monitor temperature and humidity. It reads temperature and humidity values, 
// then uploads this data to Firebase in real-time. If the temperature exceeds 
// the defined upper or lower limits, the system sends alerts to Firebase. 
// The alerts are reset when the temperature returns to normal. 
// The code uses Firebase anonymous authentication for secure data transfers. 
// The main goal is to continuously monitor environmental conditions and trigger 
// alerts when certain thresholds are breached.
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "DHT.h"
#include "credentials.h"

// DHT settings
#define DHTPIN 4  // Pin where DHT is connected
#define DHTTYPE DHT22  // Change to DHT11 if necessary

// Define temperature limits
#define TEMP_UPPER_LIMIT 50.0  // Set your upper temperature limit in Celsius
#define TEMP_LOWER_LIMIT 10.0   // Set your lower temperature limit in Celsius

DHT dht(DHTPIN, DHTTYPE);
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// State variables for alerts
bool tempExceededAlertSent = false;
bool tempUnderAlertSent = false;

void setup() {
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");

  // Configure Firebase
  config.api_key = API_KEY;
  config.database_url = FIREBASE_HOST;

  // Sign in anonymously
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase Anonymous Authentication succeeded");
  } else {
    Serial.printf("Firebase sign up failed, %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Read temperature and humidity from DHT sensor
  float temp = dht.readTemperature();  // Temperature in Celsius
  float humidity = dht.readHumidity();  // Humidity in %

  // Check if readings are valid
  if (isnan(temp) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Push temperature and humidity data to Firebase
    if (Firebase.setFloat(firebaseData, "/sensor/temperature", temp)) {
      Serial.println("Temperature sent to Firebase");
    } else {
      Serial.println("Failed to send temperature");
      Serial.println(firebaseData.errorReason());
    }

    if (Firebase.setFloat(firebaseData, "/sensor/humidity", humidity)) {
      Serial.println("Humidity sent to Firebase");
    } else {
      Serial.println("Failed to send humidity");
      Serial.println(firebaseData.errorReason());
    }

    // Check if temperature exceeds the upper limit
    if (temp > TEMP_UPPER_LIMIT) {
      // Send alert to Firebase if not already sent
      if (!tempExceededAlertSent) {
        if (Firebase.setString(firebaseData, "/alerts/temperature_alert", "Temperature limit exceeded!")) {
          Serial.println("Temperature exceeded alert sent to Firebase");
          tempExceededAlertSent = true;  // Set alert sent state
        } else {
          Serial.println("Failed to send temperature exceeded alert");
          Serial.println(firebaseData.errorReason());
        }
      }
    } else {
      // Reset the exceeded alert if temperature falls below the upper limit
      if (tempExceededAlertSent) {
        if (Firebase.setString(firebaseData, "/alerts/temperature_alert", "Temperature is back to normal.")) {
          Serial.println("Temperature exceeded alert reset in Firebase");
          tempExceededAlertSent = false;  // Reset alert state
        } else {
          Serial.println("Failed to reset temperature exceeded alert");
          Serial.println(firebaseData.errorReason());
        }
      }
    }

    // Check if temperature falls below the lower limit
    if (temp < TEMP_LOWER_LIMIT) {
      // Send alert to Firebase if not already sent
      if (!tempUnderAlertSent) {
        if (Firebase.setString(firebaseData, "/alerts/temperature_alert", "Temperature limit under!")) {
          Serial.println("Temperature under alert sent to Firebase");
          tempUnderAlertSent = true;  // Set alert sent state
        } else {
          Serial.println("Failed to send temperature under alert");
          Serial.println(firebaseData.errorReason());
        }
      }
    } else {
      // Reset the under alert if temperature rises above the lower limit
      if (tempUnderAlertSent) {
        if (Firebase.setString(firebaseData, "/alerts/temperature_alert", "Temperature is back to normal.")) {
          Serial.println("Temperature under alert reset in Firebase");
          tempUnderAlertSent = false;  // Reset alert state
        } else {
          Serial.println("Failed to reset temperature under alert");
          Serial.println(firebaseData.errorReason());
        }
      }
    }
  }

  // Wait 5 seconds before reading again
  delay(5000);
}
