/************************** Environment **************************/
#include "env.h"



/************************** WiFi Setup **************************/
#include <ESP8266WiFi.h>
WiFiClientSecure espClient;



/************************** MQTT Setup **************************/
#include <PubSubClient.h>
PubSubClient client(espClient);



/************************** Temp/IR Setup **************************/
#include "DHT.h"
#include <DaikinHeatpumpIR.h>

#define DHTTYPE DHT22
#define DHTPIN  D5
#define IRPIN   D6

IRSenderBitBang irSender(IRPIN);
DaikinHeatpumpIR *heatpumpIR;
DHT dht(DHTPIN, DHTTYPE);



/************************** Setup! **************************/

void setup_wifi() {
  delay(500);

  Serial.print("Attempting WiFi connection");

  delay(10);

  // Begin connecting to wifi
  if (WiFi.SSID() != WIFI_SSID) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    WiFi.persistent(true);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(" Connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void setup_pubsub() {
  client.setServer(MQTT_SERVER, MQTT_PORT);

  while(!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(WiFi.macAddress().c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println(" Connected.");
      client.publish("connection", "Connection.");
    } else {
      Serial.print("Connection failed, error code: ");
      Serial.print(client.state());
      Serial.println(". Will retry in 5 seconds.");

      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  setup_pubsub();

  // Begin Temp/IR
  dht.begin();
  heatpumpIR = new DaikinHeatpumpIR();

  delay(1000);
  Serial.println("Starting...");
}

/************************** Loop! **************************/

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float i = dht.computeHeatIndex(t, h, false);

	print_climate(h, t, i);
  publish_climate(h, t, i);

  heatpumpIR->send(irSender, POWER_ON, MODE_COOL, FAN_AUTO, 24, VDIR_AUTO, HDIR_AUTO);
  delay(5000);
}

void print_climate(float h, float t, float i) {
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println("°C");

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println("%");

  Serial.print("Heat index: ");
  Serial.print(i);
  Serial.println("°C");
}

void publish_climate(float h, float t, float i) {
  // Temp: XX.XX
  char roomTemp[60], hStr[5], tStr[6], heStr[6];
  
  dtostrf(h, 2, 1, hStr);
  dtostrf(t, 2, 2, tStr);
  dtostrf(i, 2, 2, heStr);

  sprintf(roomTemp, "{\"humidity\":%s,\"temperature\":%s,\"heatIndex\":%s}", hStr, tStr, heStr);

  client.publish("climate", roomTemp);
}
