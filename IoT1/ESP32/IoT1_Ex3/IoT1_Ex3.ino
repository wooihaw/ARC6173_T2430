#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>

#define LED_PIN 16 // GPIO pin connected to the LED

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Use WiFiManager for Wi-Fi connection
  WiFiManager wifiManager;
  wifiManager.autoConnect("StudentID"); // Replace StudentID with your actual Student ID
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Turn off the LED initially
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Define the URL for dweet.io
    String thingName = "StudentID"; // Replace StudentID with your actual Student ID
    String url = "http://dweet.io/get/latest/dweet/for/" + thingName;

    // Make the HTTP GET request
    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Response: " + payload);

      // Parse the received data to determine LED state
      int index = payload.indexOf("\"LED\":");
      if (index != -1) {
        char binaryValue = payload.charAt(index + 6); // Extract the binary value (0 or 1)
        if (binaryValue == '1') {
          digitalWrite(LED_PIN, HIGH); // Turn on the LED
          Serial.println("LED ON");
        } else if (binaryValue == '0') {
          digitalWrite(LED_PIN, LOW); // Turn off the LED
          Serial.println("LED OFF");
        } else {
          Serial.println("Unexpected value received");
        }
      } else {
        Serial.println("Failed to find 'LED' key in the payload");
      }
    } else {
      Serial.print("Error in HTTP request: ");
      Serial.println(http.errorToString(httpResponseCode));
    }

    http.end(); // Close the connection
  } else {
    Serial.println("WiFi not connected");
  }

  delay(5000); // Wait 5 seconds before the next request
}
