#include <WiFiManager.h> // WiFiManager library
#include <PubSubClient.h> // MQTT library
#include <DHT.h>

// Define DHT11 pin and type
#define DHTPIN 17
#define DHTTYPE DHT11

// Define LED pin
#define LED_PIN 16

// MQTT settings
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
const char* temperatureTopic = "StudentID/DHT11"; // Replace StudentID with your actual Student ID
const char* ledTopic = "StudentID/LED1"; // Replace StudentID with your actual Student ID

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();

  // Set LED pin as output
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize WiFiManager for Wi-Fi setup
  WiFiManager wifiManager;
  wifiManager.autoConnect("StudentID"); // Replace StudentID with your actual Student ID
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Configure MQTT client
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);

  // Connect to MQTT broker
  mqttConnect();
}

void loop() {
  // Reconnect to MQTT broker if necessary
  if (!client.connected()) {
    mqttConnect();
  }
  client.loop();

  // Read temperature from DHT11 and publish it
  float temperature = dht.readTemperature();
  if (!isnan(temperature)) {
    char tempStr[8];
    dtostrf(temperature, 6, 2, tempStr);
    client.publish(temperatureTopic, tempStr);
    Serial.print("Published temperature: ");
    Serial.println(tempStr);
  } else {
    Serial.println("Failed to read from DHT sensor");
  }

  delay(5000); // Wait 5 seconds before the next temperature reading
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);

  if (strcmp(topic, ledTopic) == 0) {
    if (length == 1 && payload[0] == '1') {
      digitalWrite(LED_PIN, HIGH); // Turn on LED
      Serial.println("LED ON");
    } else if (length == 1 && payload[0] == '0') {
      digitalWrite(LED_PIN, LOW); // Turn off LED
      Serial.println("LED OFF");
    } else {
      Serial.println("Invalid payload");
    }
  }
}

void mqttConnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (client.connect("StudentID")) {
      Serial.println("Connected to MQTT broker");
      client.subscribe(ledTopic);
      Serial.print("Subscribed to topic: ");
      Serial.println(ledTopic);
    } else {
      Serial.print("Failed to connect. State: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}
