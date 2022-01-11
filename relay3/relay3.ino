#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/////////////////// SETTINGS /////////////////////////////

// Wi-Fi
const char* ssid = "********";
const char* password = "********";

// MQTT
const char* mqtt_server = "xx.wqtt.ru";
const int mqtt_port = 1234;
const char* mqtt_user = "****";
const char* mqtt_password = "****";

// RELAY
const String relay_topic = "/topic/relay";
const String relay2_topic = "/topic/relay2";
const String relay3_topic = "/topic/relay3";

const int RELAY = 14;
const int RELAY2 = 15;
const int RELAY3 = 16;

/////////////////////////////////////////////////////////



WiFiClient espClient;
PubSubClient client(espClient);

bool relay_on = false;
bool relay2_on = false;
bool relay3_on = false;

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

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void updateStatePins(void){
    if(relay_on){
      digitalWrite(RELAY, HIGH);
    }else{
      digitalWrite(RELAY, LOW);
    }

    if(relay2_on){
      digitalWrite(RELAY2, HIGH);
    }else{
      digitalWrite(RELAY2, LOW);
    }

    if(relay3_on){
      digitalWrite(RELAY3, HIGH);
    }else{
      digitalWrite(RELAY3, LOW);
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    
  String data_pay;
  for (int i = 0; i < length; i++) {
    data_pay += String((char)payload[i]);
  }

    Serial.println(data_pay);
    
  if( String(topic) == relay_topic ){
        if(data_pay == "ON" || data_pay == "1") relay_on = true;
        if(data_pay == "OFF" || data_pay == "0") relay_on = false;
    }

  if( String(topic) == relay2_topic ){
        if(data_pay == "ON" || data_pay == "1") relay2_on = true;
        if(data_pay == "OFF" || data_pay == "0") relay2_on = false;
    }

  if( String(topic) == relay3_topic ){
        if(data_pay == "ON" || data_pay == "1") relay3_on = true;
        if(data_pay == "OFF" || data_pay == "0") relay3_on = false;
    }

    updateStatePins();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266-" + WiFi.macAddress();

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password) ) {
      Serial.println("connected");
      
      client.subscribe( (relay_topic + "/#").c_str() );
      client.subscribe( (relay2_topic + "/#").c_str() );
      client.subscribe( (relay3_topic + "/#").c_str() );

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(RELAY, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);

  digitalWrite(RELAY, LOW);
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, LOW);

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
