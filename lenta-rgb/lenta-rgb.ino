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

// LENTA
const String led_topic = "/home/lenta-rgb";
const int CHANNEL_R = 2;
const int CHANNEL_G = 12;
const int CHANNEL_B = 16;

/////////////////////////////////////////////////////////



WiFiClient espClient;
PubSubClient client(espClient);

bool led_on = false;
int led_brig = 0;
String led_color = "#000000";


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
    long color = strtol( &led_color[1], NULL, 16);

    long lamp_r = color >> 16;
    long lamp_g = color >> 8 & 0xFF;
    long lamp_b = color & 0xFF;

    lamp_b = lamp_b * led_brig / 100;
    lamp_g = lamp_g * led_brig / 100;
    lamp_r = lamp_r * led_brig / 100;


    if(led_on){
      analogWrite(CHANNEL_R, map(lamp_r, 0, 255, 1, 1024));
      analogWrite(CHANNEL_G, map(lamp_g, 0, 255, 1, 1024));
      analogWrite(CHANNEL_B, map(lamp_b, 0, 255, 1, 1024));
    }else{
      analogWrite(CHANNEL_R, 0);
      analogWrite(CHANNEL_G, 0);
      analogWrite(CHANNEL_B, 0);
    }

}

void callback(char* topic, byte* payload, unsigned int length) {
    
  String data_pay;
  for (int i = 0; i < length; i++) {
    data_pay += String((char)payload[i]);
  }

    Serial.println(data_pay);
    
  if( String(topic) == led_topic ){
        if(data_pay == "ON") led_on = true;
        if(data_pay == "OFF") led_on = false;
    }

  if( String(topic) == (led_topic + "/brig") ){
        led_brig = data_pay.toInt();
    }

    if( String(topic) == (led_topic + "/color") ){
        led_color = data_pay;
    }


    updateStatePins();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266-" + WiFi.macAddress();
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password) ) {
      Serial.println("connected");
      
      client.subscribe( (led_topic + "/#").c_str() );

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(CHANNEL_R, OUTPUT);
  pinMode(CHANNEL_G, OUTPUT);
  pinMode(CHANNEL_B, OUTPUT);

  digitalWrite(CHANNEL_R, LOW);
  digitalWrite(CHANNEL_G, LOW);
  digitalWrite(CHANNEL_B, LOW);

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
