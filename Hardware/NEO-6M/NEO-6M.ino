// This code uses the ESP32 to retrieve GPS data from a Ublox NEO-6M GPS module 
// and sends the latitude and longitude values to a Firebase Realtime Database.
// It connects to Wi-Fi and Firebase using the provided credentials.
// The GPS data is read using the TinyGPS++ library, and the location updates are sent
// to Firebase every 2 seconds when a new GPS fix is available.
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <TinyGPS++.h>
#include "credentials.h" // Include your Wi-Fi and Firebase credentials

// GPS connections
#define GPS_RX_PIN 16  // GPS module RX pin connected to GPIO 16
#define GPS_TX_PIN 17  // GPS module TX pin connected to GPIO 17

FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// GPS object
TinyGPSPlus gps;
HardwareSerial gpsSerial(1); // Using Serial1 for GPS

void setup() {
  Serial.begin(115200);

  // Initialize GPS serial
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  // Indicate connection attempt
  }
  Serial.println("Connected to Wi-Fi");

  // Configure Firebase
  config.api_key = API_KEY;  // Set the API key
  config.database_url = FIREBASE_HOST;  // Set the Firebase database URL

  // Sign in anonymously
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase Anonymous Authentication succeeded");
  } else {
    Serial.printf("Firebase sign up failed, %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);  // Initialize Firebase
  Firebase.reconnectWiFi(true);  // Reconnect to Wi-Fi if connection is lost
}

void loop() {
  // Check if GPS data is available
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());

    // If a new GPS fix is available
    if (gps.location.isUpdated()) {
      // Get latitude and longitude
      float latitude = gps.location.lat();
      float longitude = gps.location.lng();

      // Print the values to Serial Monitor
      Serial.print("Latitude: ");
      Serial.print(latitude, 6);
      Serial.print(" | Longitude: ");
      Serial.println(longitude, 6);

      // Send latitude and longitude to Firebase
      if (Firebase.setFloat(firebaseData, "/gps/latitude", latitude)) {
        Serial.println("Latitude sent to Firebase");
      } else {
        Serial.println("Failed to send latitude");
        Serial.println(firebaseData.errorReason());
      }

      if (Firebase.setFloat(firebaseData, "/gps/longitude", longitude)) {
        Serial.println("Longitude sent to Firebase");
      } else {
        Serial.println("Failed to send longitude");
        Serial.println(firebaseData.errorReason());
      }
    }
  }

  delay(2000);  // Delay between GPS updates
}
