#include <WiFiManager.h>          // WiFiManager library
#include <HTTPClient.h>           // HTTPClient for HTTP requests
#include <DHT.h>                  // DHT library

// Pin definitions
#define DHTPIN 17                  // Pin connected to the DHT11 sensor
#define DHTTYPE DHT11             // Define the sensor type (DHT11)

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 5000; // Publish data every 5 seconds

void setup() {
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();

  // Start WiFiManager for dynamic WiFi configuration
  WiFiManager wifiManager;
  wifiManager.autoConnect("StudentID"); // Replace StudentID with your actual Student ID

  // Confirm successful WiFi connection
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Timer to publish data periodically
  if (millis() - lastTime > timerDelay) {
    lastTime = millis();

    // Read temperature from the DHT11 sensor
    float temperature = dht.readTemperature();
    if (isnan(temperature)) {
      Serial.println("Failed to read temperature from DHT11!");
      return;
    }

    // Prepare the URL for dweet.io
    String dweetUrl = "http://dweet.io/dweet/for/StudentID?temperature=" + String(temperature); // Replace StudentID with your actual Student ID

    // Publish the data using HTTP GET
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(dweetUrl);
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        Serial.println("Data sent successfully!");
        Serial.print("Response: ");
        Serial.println(http.getString());
      } else {
        Serial.print("Error sending data. HTTP Response Code: ");
        Serial.println(httpResponseCode);
      }

      http.end(); // Close the connection
    } else {
      Serial.println("WiFi disconnected! Reconnecting...");
      WiFi.reconnect();
    }
  }
}
