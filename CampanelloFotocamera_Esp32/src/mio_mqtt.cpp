/*#include "mio_mqtt.h"
#include <WiFi.h>
#include <MQTTClient.h>
#include <connessione_wifi.h>

void mqtt_clientLoop_old(MQTTClient &mqtt_client, boolean skip){


  if (!mqtt_client.connected()) {

    Serial.print("checking wifi...");
    // while (WiFi.status() != WL_CONNECTED) {
    //   Serial.print(".");

    //   delay(1000);
    // }

    connessione_wifi();
    Serial.print("\nconnecting mqtt broker...");
    String clientId = "elenaId-esp32-" + String(random(0xffff), HEX);
    while (!mqtt_client.connect(clientId.c_str())) {
      Serial.print(".");
      delay(1000);
    }

    Serial.println("\nconnected!");

   }

   mqtt_client.loop();

}

void mqtt_clientLoop(MQTTClient &mqtt_client){
  const int SECONDI_DISCONNESSIONE_MQTT_AMMESSA = 2, SECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT=3; // TODO rimuovimi
  unsigned long inizio_loop = millis();

  while (!mqtt_client.connected()) {

    if (millis() - inizio_loop >= SECONDI_DISCONNESSIONE_MQTT_AMMESSA * 1000) {
      // Passo al BLE
    }

    Serial.print("Attempting MQTT connection...");
    
    // Create a random client ID
    String clientId = "elenaId-foto-" + String(random(0xffff), HEX);

    // Attempt to connect
    if (mqtt_client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, resubscribe // TODO
      // client.subscribe("inTopic");

    } else {
      Serial.print("failed, lastError=");
      Serial.print(mqtt_client.lastError());
      Serial.print("\t returnCode=");
      Serial.print(mqtt_client.returnCode());
      Serial.println("\t try again in " + (String) SECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT + "  seconds");
      delay(SECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT);
    }
  }


  mqtt_client.loop();

}


void messageReceived(String &topic, String &payload) {
  // NB: NON USARE mqtt_client QUI -> se devi usarlo, cambia una variabile globale
  Serial.println("incoming: " + topic + " - " + payload);

}
*/