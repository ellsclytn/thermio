#include "DHT.h"
#include <DaikinHeatpumpIR.h>

#define DHTTYPE DHT22
#define DHTPIN D5
#define IRPIN D6

IRSenderBitBang irSender(IRPIN);
DaikinHeatpumpIR *heatpumpIR;
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  heatpumpIR = new DaikinHeatpumpIR();
  
  delay(1000);
  Serial.println("Starting...");
}

void loop() {
  heatpumpIR->send(irSender, POWER_ON, MODE_COOL, FAN_AUTO, 24, VDIR_AUTO, HDIR_AUTO);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C");

  // Get humidity event and print its value.
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println("%");

  Serial.print("Heat index: ");
  Serial.print(dht.computeHeatIndex(t, h, false));
  Serial.println(" *C");

  delay(5000);
}
