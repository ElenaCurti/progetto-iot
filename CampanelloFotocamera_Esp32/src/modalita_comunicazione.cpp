#include "modalita_comunicazione.h"
#include <MQTTClient.h>
#include <HardwareBLESerial.h>
#include <connessione_wifi.h>

modalita_comunicazione modalita_usata = IMMAGINE_CON_MQTT;

unsigned long last_wifi_reconnection_attempt=-1;
const int MILLISECONDI_DISCONNESSIONE_WIFI_AMMESSA = 5*1000;
const int MILLISECONDI_DISCONNESSIONE_WIFI_AMMESSA2 = 20*1000;


unsigned long last_stable_mqtt_connection=-1;
unsigned long last_mqtt_reconnection_attempt=-1;


// La disconnessione MQTT e' ammessa per un certo tempo, settabile in secondi in questa variabile. Dopo di che si passa al BLE
const int MILLISECONDI_DISCONNESSIONE_MQTT_AMMESSA = 10*1000;

// Se MQTT perde la connessione, si prova a riconnettere dopo un certo #secondi, settabili qui
const int MILLISECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT = 2*1000;



modalita_comunicazione getModalitaComunicazioneUsata(){
    return modalita_usata;
}


void gestisciComunicazioneIdle(MQTTClient &mqtt_client, HardwareBLESerial &bleSerial){
    if (WiFi.status() != WL_CONNECTED) {
        //  il Wifi
        if (millis() - last_wifi_reconnection_attempt >= MILLISECONDI_DISCONNESSIONE_WIFI_AMMESSA){
            Serial.println("Disconnetto e riconnetto Wifi...");
            WiFi.disconnect();
            setup_wifi();
            WiFi.reconnect();

            last_wifi_reconnection_attempt = millis();
        }
        

    }

        
    if (mqtt_client.connected()){ 
        // Client MQTT connesso -> tutto ok, chiamo il loop
        last_stable_mqtt_connection = millis();
        mqtt_client.loop();
        return;
        // TODO eventualmente vai in deep sleep
    } else if (WiFi.status() == WL_CONNECTED){
        // client non comunica con MQTT, ma e' connesso al Wifi -> se sono passati piu' di SECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT, ritento la connessione
        if (millis() - last_mqtt_reconnection_attempt >= MILLISECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT){
            Serial.print("Attempting MQTT re-connection...");
            String clientId = "elenaId-foto-" + String(random(0xffff), HEX);
            if (mqtt_client.connect(clientId.c_str())) {
                Serial.println("reconnected.");
                // TODO re-subscribe
                mqtt_client.subscribe("out_topic");
                Serial.print("Pub: " );
                Serial.println(mqtt_client.publish("in_topic", "ciao"));
                mqtt_client.loop();
            } else {
                Serial.print("failed, lastError=");
                Serial.print(mqtt_client.lastError());
                Serial.print("\t returnCode=");
                Serial.print(mqtt_client.returnCode());
                Serial.println("\t try again in " + (String) MILLISECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT + " milliseconds");
            }
            last_mqtt_reconnection_attempt = millis();
        }
    }


    // Se ble ok -> chiama poll, altrimenti sistema connessione
    

}