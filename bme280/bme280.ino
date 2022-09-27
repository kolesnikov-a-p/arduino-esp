#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

/////////////////// SETTINGS /////////////////////////////

// Wi-Fi
const char* ssid = "********";
const char* password = "********";

// MQTT
const char* mqtt_server = "xx.wqtt.ru";
const int mqtt_port = 1234;
const char* mqtt_user = "****";
const char* mqtt_password = "****";

// SENSOR
const int sending_period = 5;
const bool retain_flag = false;
const char* temperature_topic = "bme280/temperature";
const char* humidity_topic = "bme280/humidity";
const char* pressure_topic = "bme280/pressure";
/////////////////////////////////////////////////////////


WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_BME280 bme;
uint32_t tmr1;

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266-" + WiFi.macAddress();
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password) ) {
      Serial.println("connected");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void sendData() {
  float temperature = bme.readTemperature();
  float pressure = bme.readPressure() / 133.3224;
  float humidity = bme.readHumidity();

  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" mm");
  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.println(" %");

  client.publish(temperature_topic, String(temperature).c_str(), retain_flag);
  client.publish(humidity_topic, String(humidity).c_str(), retain_flag);
  client.publish(pressure_topic, String(pressure).c_str(), retain_flag);
}

void setup() {
  Serial.begin(115200);

  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    while (1){
      delay(10);
    }
  }

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - tmr1 >= (sending_period * 1000)) {
    tmr1 = millis();
    sendData();
  }
}