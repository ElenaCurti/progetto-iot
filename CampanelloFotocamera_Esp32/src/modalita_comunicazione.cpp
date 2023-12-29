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


bool stato_ble2 = false;


modalita_comunicazione getModalitaComunicazioneUsata(){
    return modalita_usata;
}

const size_t size_foto = 9600;
WiFiClient net;
MQTTClient mqtt_client(size_foto*2+100);
void messageReceived2(String &topic, String &payload) ;

HardwareBLESerial &bleSerial = HardwareBLESerial::getInstance();

void init_mqtt_ble(){
    IPAddress ip_broker = IPAddress(192,168,43,252);    // TODO mettere broker come configurazione

    mqtt_client.begin(ip_broker, net);
    mqtt_client.onMessage(messageReceived2);
    // mqtt_client.setKeepAlive(90);
    // mqtt_client.setCleanSession(false);

    // Inizialzzo Bluetooth
    if (!bleSerial.beginAndSetupBLE("Esp32provaa")) {
        Serial.println("failed to initialize HardwareBLESerial!");
        return;
    } else {
        Serial.println(" HardwareBLESerial inizializzata correttamente.");
    }
    
}


void gestisciComunicazioneIdle(){

    Serial.print("gc ");
    
    if (WiFi.status() != WL_CONNECTED) {
        //  il Wifi
        if (millis() - last_wifi_reconnection_attempt >= MILLISECONDI_DISCONNESSIONE_WIFI_AMMESSA){
            Serial.println("Disconnetto e riconnetto Wifi...");
            // WiFi.disconnect();  // TODO forse disconnect e reconnect vanno tolti
            setup_wifi();
            // WiFi.reconnect();

            last_wifi_reconnection_attempt = millis();
        }
        

    }

        
    if (mqtt_client.connected()){ 
        // Client MQTT connesso -> tutto ok, chiamo il loop
        last_stable_mqtt_connection = millis();
        
        // return;
        // TODO eventualmente vai in deep sleep
    } else if (WiFi.status() == WL_CONNECTED){
        // client non comunica con MQTT, ma e' connesso al Wifi -> se sono passati piu' di SECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT, ritento la connessione
        if (millis() - last_mqtt_reconnection_attempt >= MILLISECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT){
            String clientId = "elenaId-foto-" + String(random(0xffff), HEX);
            Serial.print("Wifi OK. Attempting MQTT re-connection as " + clientId + " ...");
            if (mqtt_client.connect(clientId.c_str())) {
                Serial.println("reconnected as " + clientId );
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

    mqtt_client.loop();
/*
lastError=-3    returnCode=0        -> non stampati da mosquitto
lastError=-9   returnCode=6         -> esp chiude la connessione
lastError=-3   returnCode=6         ->
*/


//   // Write data to Bluetooth Serial
//   if (Serial.available()) {
//     String sendData = Serial.readString();
//     SerialBT.print("Sent from ESP32: ");
//     SerialBT.println(sendData);
//   }

    // // Se ble ok -> chiama poll, altrimenti sistema connessione
    // HardwareBLESerial &bleSerial = *bleSerial2;
    // Serial.print("a");
    // if (bleSerial2 == NULL)
        // return;
    bleSerial.poll();
    // Serial.print("b");

    if (stato_ble2 != bleSerial){
        Serial.print("Nuovo stato BLE:");
        Serial.println(bleSerial ? "Connesso" : "NON connesso");
        stato_ble2=bleSerial;
    }


    if (bleSerial && bleSerial.available()){
        // Serial.print("a");

        Serial.print("dati in arrivo: ");
        String dati_arrivati = "";

        while (bleSerial.available() > 0) {
            // Leggo i dati dalla ble serial
            // Serial.println("qui3");
            dati_arrivati += (char) bleSerial.read();

        }
        Serial.println(dati_arrivati);

        Serial.println("Mando ciao: ");
        // FIXME (a volte) quando wifi si scollega, la print del bluetooth non funziona. la ricezione pero' funziona ancora
        bleSerial.println("ciao");    
    }
  

}


void inviaMessaggio(String topic, String messaggio){
    if (mqtt_client.connected()){
        Serial.print("(MQTT) " );
        Serial.print(mqtt_client.publish(topic.c_str(), messaggio.c_str(), messaggio.length(), 0));
    } else if (bleSerial) {
        Serial.print("(BLE) " );
        String msg = "["+topic+"] " + messaggio ;
        Serial.print(bleSerial.println(msg.c_str())); 
    } else 
        Serial.print("disconnesso");
    
}