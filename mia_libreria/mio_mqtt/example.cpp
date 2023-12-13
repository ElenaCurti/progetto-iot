//#include <Arduino.h>

/*
#include <mio_mqtt.h>

extern PubSubClient mqtt_client;

void setup() {
  Serial.begin(9600);
  delay(1000);

  mqtt_inizializzaServerAndCallback("broker.mqtt.cool", "eleClient");
  mqtt_client.subscribe("inTopic");  
  mqtt_client.publish("outTopic", "ciaooo");  
}

void loop() {
  mqtt_clientLoop();

}


void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.println((String) topic + " provaa");

}*/