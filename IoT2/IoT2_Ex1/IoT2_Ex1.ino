#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// setting PWM properties
#define PUMP 16
#define DHTPIN 17
#define DHTTYPE DHT11
#define POTPIN 34

#define MSG_BUFFER_SIZE  (128)
char msg[MSG_BUFFER_SIZE];
char new_topic[80];
char requestID[10];

unsigned long lastTime = 0;
unsigned long timerDelay = 5000; // Publish data every 5 seconds

// Global variable to keep track of whether the startup data has been sent
bool startupDataPublished = false;

// Network settings
const char* ssid = "";      // WiFi SSID
const char* password = "";  // WiFi password
const char* mqtt_server = "demo.thingsboard.io";
const unsigned int mqtt_port = 1883;
const char* pub_topic = "v1/devices/me/telemetry";
const char* sub_topic = "v1/devices/me/rpc/request/+";
const char* attr_topic = "v1/devices/me/attributes";
const char* access_token = "";  // Thingsboard access token

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  DynamicJsonDocument doc(128);
  deserializeJson(doc, payload);

  const char* method = doc["method"];

  String responseTopic = String(topic);
  responseTopic.replace("request", "response");
  if (strcmp(method, "getPumpStatus")==0) {
      int pumpStatus = digitalRead(PUMP);
      client.publish(responseTopic.c_str(), String(pumpStatus).c_str());
      if (pumpStatus == 0) {
          client.publish(attr_topic, "{pumpStatus: 0}");
        } else {
          client.publish(attr_topic, "{pumpStatus: 1}");
      }
  } else if (strcmp(method, "setPump")==0) {
      int pumpStatus = 1 - digitalRead(PUMP);
      client.publish(responseTopic.c_str(), String(pumpStatus).c_str());
      int value = doc["params"];
      Serial.print("params =");
      Serial.println(value);          
      if (value==0) {
        digitalWrite(PUMP, LOW);
        client.publish(attr_topic, "{pumpStatus: 0}");
      } else {
        digitalWrite(PUMP, HIGH);
        client.publish(attr_topic, "{pumpStatus: 1}");
      }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), access_token, "")) {
      Serial.println("connected");

      // Check if the startup data has not been published
      if (!startupDataPublished) {
          // Publish startup data
          client.publish(attr_topic, "{pumpStatus: 0}"); // relays are reset to off initially
          startupDataPublished = true; // Set the flag so this only happens once
          Serial.println("Startup data published");
      }

      bool sub = client.subscribe(sub_topic);
      if (sub) {
        Serial.println("Subscribed");
      } else {
        Serial.println("Not subscribed");
      }
        
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publishSensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  unsigned int potValue = analogRead(POTPIN);

  bool dhtError = false;
  // Check if the reading is valid
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    dhtError = true;
  }

  // Create JSON object
  StaticJsonDocument<128> jsonDoc;
  jsonDoc["potValue"] = potValue;

  if (!dhtError) {
    jsonDoc["temperature"] = temperature;
    jsonDoc["humidity"] = humidity;
  }

  char jsonBuffer[128];
  serializeJson(jsonDoc, jsonBuffer);

  // Publish data to MQTT
  if (client.publish(pub_topic, jsonBuffer)) {
    Serial.println("Data published successfully:");
    Serial.println(jsonBuffer);
  } else {
    Serial.println("Failed to publish data");
  }
}

void setup() {
  pinMode(PUMP, OUTPUT);
  digitalWrite(PUMP, LOW);
  
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastTime > timerDelay) {
    lastTime = now;
    publishSensorData();
  }
}
