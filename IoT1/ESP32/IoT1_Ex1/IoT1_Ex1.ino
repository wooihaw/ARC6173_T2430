#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// WiFi Credentials
const char *ssid = "StudentID"; // Replace this with your actual Student ID
const char *password = "12345678";

// GPIO Definitions
#define LED_PIN 16
#define DHT_PIN 17
#define DHT_TYPE DHT11

// Objects
WebServer server(80);
DHT dht(DHT_PIN, DHT_TYPE);

bool ledState = false;

// HTML for the Web Page
String generateHTML(float temperature) {
  String html = "<!DOCTYPE html><html>";
  html += "<head><title>ARC6173 IoT1 Lab</title></head>";
  html += "<body style='text-align:center;'>";
  html += "<h1>ESP32 IoT Lab</h1>";
  html += "<p>Temperature: " + String(temperature) + " &deg;C</p>";
  html += "<button onclick=\"toggleLED()\">Toggle LED</button>";
  html += "<p>LED is " + String(ledState ? "ON" : "OFF") + "</p>";
  html += "<script>";
  html += "function toggleLED() { fetch('/toggleLED').then(() => location.reload()); }";
  html += "</script>";
  html += "</body></html>";
  return html;
}

// Handle Root Page
void handleRoot() {
  float temperature = dht.readTemperature();
  if (isnan(temperature)) {
    temperature = 0.0; // Fallback if sensor reading fails
  }
  server.send(200, "text/html", generateHTML(temperature));
}

// Handle LED Toggle
void handleToggleLED() {
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  server.send(204); // No content response
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Initialize GPIOs
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  dht.begin();

  // Configure WiFi in AP mode
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.println("Access Point started");
  Serial.print("IP Address: ");
  Serial.println(IP);

  // Setup Web Server Routes
  server.on("/", handleRoot);
  server.on("/toggleLED", handleToggleLED);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
