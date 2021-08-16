#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>

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
const String led_topic = "/topic";
const int LED_PIN = 15;
const int LED_COUNT = 22;

/////////////////////////////////////////////////////////

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel neoPixel = Adafruit_NeoPixel(LED_COUNT, LED_PIN , NEO_GRB + NEO_KHZ800);

bool led_on = false;
int led_brig = 50;
String led_color = "#FFFFFF";

void setPixel(int p, byte r, byte g, byte b) {
  r = map(r, 0, 255, 0, led_brig);
  g = map(g, 0, 255, 0, led_brig);
  b = map(b, 0, 255, 0, led_brig);
  neoPixel.setPixelColor(p, neoPixel.Color(r, g, b));
}

void setAll(byte r, byte g, byte b) {
  neoPixel.clear();
  for(int i = 0; i <= LED_COUNT; i++ ) {
    setPixel(i, r, g, b);
  }
  neoPixel.show();
}

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

    int lamp_r = color >> 16;
    int lamp_g = color >> 8 & 0xFF;
    int lamp_b = color & 0xFF;


    if(led_on){
        setAll(lamp_r, lamp_g, lamp_b);
    }else{
      setAll(0, 0, 0);
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
  Serial.begin(115200);
  neoPixel.begin();
  setAll(0, 0, 0);
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
