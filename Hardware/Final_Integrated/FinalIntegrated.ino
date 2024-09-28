/*
  This Arduino code connects an ESP32 microcontroller to a DHT22 sensor, Firebase, and a Wi-Fi network to continuously monitor environmental conditions, specifically temperature and humidity. 

  Key Features:
  - **Wi-Fi Connectivity**: The ESP32 connects to the specified Wi-Fi network using credentials defined in the "credentials.h" file.
  - **Firebase Integration**: 
    - Utilizes Firebase's real-time database to send and receive data.
    - Implements anonymous authentication for secure data transfers.
    - Initializes values in the Firebase database for fuel theft alerts and fuel level differences.
  - **Distance Measurement**: 
    - Uses an HC-SR04 ultrasonic sensor to measure the distance, which is interpreted as the fuel level in this context.
    - Compares current distance readings to previous ones to detect significant changes that may indicate refueling or theft.
  - **Alert System**: 
    - Monitors distance values to determine if fuel refueling or theft is occurring based on a defined threshold.
    - Sends real-time alerts to Firebase if fuel levels change significantly, indicating possible refueling or theft, along with timestamps for reference.
    - Alerts are reset when the situation returns to normal.
  - **NTP Time Sync**: 
    - Uses an NTPClient to obtain the current time from an NTP server, providing accurate timestamps for alerts.

  The code runs in a loop where it continually:
  1. Fetches the current status of the car (whether it is stopped) from Firebase.
  2. Measures the distance using the HC-SR04 sensor to represent the fuel level.
  3. Compares the current distance to the previous distance to check for significant changes.
  4. Sends the distance data and any relevant alerts to Firebase.
  5. Resets alerts when the conditions normalize.

  This system is designed for real-time monitoring and alerting, enhancing safety and oversight regarding fuel levels in vehicles.
*/
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <time.h>  // Include time.h for current time
#include <WiFiUdp.h>
#include <NTPClient.h>  // Include NTPClient library
#include "credentials.h"


// Wi-Fi credentials


// HC-SR04 connections
#define TRIG_PIN 23  // Trig pin connected to GPIO 23
#define ECHO_PIN 22  // Echo pin connected to GPIO 22

FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // GMT+5:30 offset

long duration;
float distance;
float previousDistance = 0;  // To store the previous distance reading
bool carStopped = false;      // To track if the car is stopped
const float THRESHOLD = 2.0;  // Threshold for detecting a significant decrease/increase in distance (in cm)

void setup() {
  Serial.begin(115200);

  // Initialize GPIO pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

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

  // Set initial values in Firebase for fuel_theft
  Serial.println("Attempting to set fuel_theft to 'No alert'...");
  if (Firebase.setString(firebaseData, "/alerts/fuel_theft", "No alert")) {
    Serial.println("Successfully initialized fuel_theft to 'No alert'.");
  } else {
    Serial.println("Failed to initialize fuel_theft.");
    Serial.println(firebaseData.errorReason());  // Print detailed error message
  }

  // Set initial values in Firebase for fuel_level_difference
  Serial.println("Attempting to set fuel_level_difference to 0...");
  if (Firebase.setFloat(firebaseData, "/alerts/fuel_level_difference", 0.0)) {
    Serial.println("Successfully initialized fuel_level_difference to 0.");
  } else {
    Serial.println("Failed to initialize fuel_level_difference.");
    Serial.println(firebaseData.errorReason());  // Print detailed error message
  }
}

void loop() {
  // Fetch the carStopped status from Firebase in every loop iteration
  if (Firebase.getBool(firebaseData, "/car_status/stopped")) {
    carStopped = firebaseData.boolData();  // Update the carStopped variable based on Firebase data
    Serial.print("Car stopped status: ");
    Serial.println(carStopped ? "Yes" : "No");
  } else {
    Serial.println("Failed to get car status from Firebase");
    Serial.println(firebaseData.errorReason());
  }

  // Fetch the is_resolved status from Firebase
  bool isResolved;
  if (Firebase.getBool(firebaseData, "/alerts/is_resolved")) {
    isResolved = firebaseData.boolData();  // Update the isResolved variable based on Firebase data
    Serial.print("Alert resolved status: ");
    Serial.println(isResolved ? "True" : "False");

    // If alert is resolved, reset fuel_theft and fuel_level_difference
    if (isResolved) {
      Serial.println("Alert resolved, resetting fuel_theft and fuel_level_difference...");

      // Reset fuel_theft to "No alert"
      if (Firebase.setString(firebaseData, "/alerts/fuel_theft", "No alert")) {
        Serial.println("Fuel theft alert reset to 'No alert'.");
      } else {
        Serial.println("Failed to reset fuel theft alert.");
        Serial.println(firebaseData.errorReason());
      }

      // Reset fuel_level_difference to 0
      if (Firebase.setFloat(firebaseData, "/alerts/fuel_level_difference", 0.0)) {
        Serial.println("Fuel level difference reset to 0.");
      } else {
        Serial.println("Failed to reset fuel level difference.");
        Serial.println(firebaseData.errorReason());
      }
    }
  } else {
    Serial.println("Failed to get is_resolved status from Firebase");
    Serial.println(firebaseData.errorReason());
  }

  // Clear the trigger pin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);  // Short delay to ensure the pin is low

  // Send a 10 microsecond pulse to trigger the sensor
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);  // Keep the pin high for 10 microseconds
  digitalWrite(TRIG_PIN, LOW);  // Set the pin low again

  // Measure the echo time in microseconds
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate distance in centimeters (speed of sound = 343 m/s)
  distance = duration * 0.034 / 2;  // Convert echo time to distance

  // Update the NTP Client to get the current time
  timeClient.update();  // Update time from NTP server
  
  // Get formatted time and date
  String timeString = timeClient.getFormattedTime();  // Get current time as string
  unsigned long epochTime = timeClient.getEpochTime();  // Get epoch time
  struct tm *ptm = gmtime((time_t *)&epochTime);  // Convert to broken-down time structure

  int day = ptm->tm_mday;  // Get day of the month
  int month = ptm->tm_mon + 1;  // Get month (tm_mon is 0-11, so add 1)
  int year = ptm->tm_year + 1900;  // Get year (tm_year is years since 1900, so add 1900)

  String dateString = String(day) + "/" + String(month) + "/" + String(year);  // Create date string

  // Check if the car is stopped
  if (carStopped) {
    Serial.println("Car is stopped");
    
    // Print the distance (Fuel level)
    Serial.print("Distance (Fuel level): ");
    Serial.print(distance);
    Serial.println(" cm");

    // Send the distance to Firebase (fuel level data)
    if (Firebase.setFloat(firebaseData, "/sensor/fuel_level", distance)) {
      Serial.println("Fuel level sent to Firebase");
    } else {
      Serial.println("Failed to send fuel level");
      Serial.println(firebaseData.errorReason());
    }

    // Check if distance has decreased significantly
    if (previousDistance != 0) { // Only compare if previousDistance has been set
      Serial.print("Previous Distance: ");
      Serial.println(previousDistance);
      Serial.print("Current Distance: ");
      Serial.println(distance);
      
      if (distance < (previousDistance - THRESHOLD)) {
        float difference = previousDistance - distance;  // Calculate the difference
        Serial.println("Fuel Refueling Occurred");  // Alert for fuel refueling
        
        // Send alert to Firebase
        if (Firebase.setString(firebaseData, "/alerts/fuel_theft", "Fuel Refueling Occurred at Time: " + timeString + " Date: 29/09/2024")) {
          Serial.println("Alert sent to Firebase");
        } else {
          Serial.println("Failed to send alert");
          Serial.println(firebaseData.errorReason());
        }

        // Send the difference in fuel level to Firebase
        if (Firebase.setFloat(firebaseData, "/alerts/fuel_level_difference", difference)) {
          Serial.println("Fuel level difference sent to Firebase");
        } else {
          Serial.println("Failed to send fuel level difference.");
          Serial.println(firebaseData.errorReason());
        }

        // Set is_resolved to false
        if (Firebase.setBool(firebaseData, "/alerts/is_resolved", false)) {
          Serial.println("is_resolved set to false");
        } else {
          Serial.println("Failed to set is_resolved to false.");
          Serial.println(firebaseData.errorReason());
        }
      } else if (distance > (previousDistance + THRESHOLD)) {  // Check for significant increase
        float difference = distance - previousDistance;  // Calculate the difference
        Serial.println("Alert: Possible Fuel Theft Detected");  // Alert for possible fuel theft
        
        // Send alert to Firebase
        if (Firebase.setString(firebaseData, "/alerts/fuel_theft", "Possible Fuel Theft Detected at Time: " + timeString + " Date: 29/09/2024")) {
          Serial.println("Alert sent to Firebase");
        } else {
          Serial.println("Failed to send alert.");
          Serial.println(firebaseData.errorReason());
        }

        // Send the difference in fuel level to Firebase
        if (Firebase.setFloat(firebaseData, "/alerts/fuel_level_difference", difference)) {
          Serial.println("Fuel level difference sent to Firebase");
        } else {
          Serial.println("Failed to send fuel level difference.");
          Serial.println(firebaseData.errorReason());
        }

        // Set is_resolved to false
        if (Firebase.setBool(firebaseData, "/alerts/is_resolved", false)) {
          Serial.println("is_resolved set to false.");
        } else {
          Serial.println("Failed to set is_resolved to false.");
          Serial.println(firebaseData.errorReason());
        }
      }
    }
  }

  previousDistance = distance;  // Update the previous distance

  delay(5000);  // Delay for 5 seconds before next iteration
}
