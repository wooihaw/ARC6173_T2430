/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN ""      // The authentication token for Blynk platform

#include <WiFi.h>                // Includes the Wi-Fi library for the ESP32
#include <WiFiClient.h>          // Includes the Wi-Fi client library for network connections
#include <BlynkSimpleEsp32.h>    // Includes the Blynk library specific to ESP32
#include <string.h>              // Includes the library for using string functions
#include <DHT.h>                 // Includes the DHT sensor library

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial       // Enable debug printing to serial monitor

// DHT sensor configuration
#define DHTPIN 17                 // Pin where the DHT sensor is connected
#define DHTTYPE DHT11            // Type of DHT sensor
DHT dht(DHTPIN, DHTTYPE);        // Create a DHT sensor object

#define PUMP 16             // GPIO pin connected to the relay
#define POTPIN 34              // Analog pin connected to the soil moisture sensor

#define INTERVAL 3000            // Interval at which to send sensor data (milliseconds)
unsigned long previousMillis = 0; // Stores last time sensor data was sent

char auth[] = BLYNK_AUTH_TOKEN;  // Blynk authorization token

/* Fill-in the WiFi SSID and password */
char ssid[] = "";                // WiFi SSID
char pass[] = "";                // WiFi password

void setup()
{
  Serial.begin(115200);          // Initialize serial communication at 115200bps
  dht.begin();                   // Initialize the DHT sensor
  pinMode(PUMP, OUTPUT);    // Set the relay pin as an output
  digitalWrite(PUMP, LOW); // Initialize the relay to be OFF
  Blynk.begin(auth, ssid, pass); // Connect to Blynk
}

void loop()
{
  Blynk.run();                   // Run Blynk process
  unsigned long currentMillis = millis(); // Get the current time
  // Check if it is time to read and send sensor data
  if (currentMillis - previousMillis >= INTERVAL) {
    previousMillis = currentMillis;       // Update the last sent time
    float t = dht.readTemperature();      // Read temperature from DHT sensor
    float h = dht.readHumidity();         // Read humidity from DHT sensor

    // Check if DHT readings were successful
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!\n");
    } else {
      Blynk.virtualWrite(V0, t);          // Send temperature to Blynk
      Blynk.virtualWrite(V1, h);          // Send humidity to Blynk
      BLYNK_PRINT.printf("Humidity: %.2f, Temperature: %.2f, ", h, t);
    }

    int potValue = analogRead(POTPIN);  // Read soil moisture level
    Blynk.virtualWrite(V2, potValue);     // Send soil moisture to Blynk
    BLYNK_PRINT.printf("Soil moisture level: %d\n", potValue); 
  }
}

// Function to control the relay from the Blynk app
BLYNK_WRITE(V3)
{
  if (param.asInt() == LOW)  // Check if button in app is pressed
  {
    digitalWrite(PUMP, LOW); // Turn on relay (activate water pump)
  }
  else
  {
    digitalWrite(PUMP, HIGH); // Turn off relay (deactivate water pump)
  }
}
