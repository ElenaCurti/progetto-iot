#include <WiFiClient.h>
#include "modalita_comunicazione.h"
#include <PubSubClient.h>
#include "BluetoothSerial.h"
#include <connessione_wifi.h>

// TODO cambia tutto in CamelCase o snake

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

String device_name_g ="";
BluetoothSerial SerialBT;

// const size_t size_foto = 9600;
// WiFiClient net;
// MQTTClient mqtt_client(size_foto*2+100);
WiFiClient espClient;
PubSubClient mqtt_client(espClient);
String* lista_topic_subscription_g;
int num_topic_subscription_g;
void messageReceived2(String &topic, String &payload) ;



void messaggio_arrivato(char* topic, byte* payload, unsigned int length) ;



void mqtt_ble_setup(String device_name, const String lista_topic_subscription[], int num_topic_subscription){
    // Salvo le variabili globali
    device_name_g = device_name;
    num_topic_subscription_g = num_topic_subscription;
    lista_topic_subscription_g = new String[num_topic_subscription];
    for (int i = 0; i < num_topic_subscription; i++) 
        lista_topic_subscription_g[i] = lista_topic_subscription[i];

    // Inizializzo Wifi e MQTT
    if (WiFi.status() != WL_CONNECTED) 
        connessione_wifi();

    IPAddress ip_broker = IPAddress(192,168,1,23);    // TODO mettere broker come configurazione

    mqtt_client.setServer(ip_broker, 1883);
    mqtt_client.setCallback(messaggio_arrivato);

    // Inizialzzo Bluetooth
    // SerialBT.begin(device_name_g); //Bluetooth device name

    
}


void gestisciComunicazioneIdle(){

    // Serial.print("gc ");
    
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
            String clientId =  "elenaId-" +  device_name_g + String(random(0xffff), HEX); // TODO forse togli elenaId
            Serial.print("Wifi OK. Attempting MQTT re-connection as " + clientId + " ...");
            if (mqtt_client.connect(clientId.c_str())) {
                Serial.println("reconnected as " + clientId );

                for (int i=0; i<num_topic_subscription_g; i++)
                    mqtt_client.subscribe(lista_topic_subscription_g[i].c_str());

            } else {
                Serial.print("failed, rc=" + (String) mqtt_client.state());
                /*Serial.print("failed, lastError=");
                Serial.print(mqtt_client.lastError());
                Serial.print("\t returnCode=");
                Serial.print(mqtt_client.returnCode());*/
                Serial.println("\t try again in " + (String) MILLISECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT + " milliseconds");
            
            }

            

            last_mqtt_reconnection_attempt = millis();
        }
    }


    // Serial.print("loop");
    mqtt_client.loop();


    // TODO -> se BLE rotto, risolvi
}


void inviaMessaggio(String topic, String messaggio){
    
    // if (WiFi.status() == WL_CONNECTED && msqtt_client.connected()){
    if (mqtt_client.connected()){
        Serial.print("(MQTT) " );
        // Serial.print(mqtt_client.publish(topic.c_str(), messaggio.c_str(), messaggio.length(), 0));
        if (messaggio.length() > 1000) {
            Serial.print("(long mess)");

            mqtt_client.beginPublish(topic.c_str(), messaggio.length(), false);
            mqtt_client.print(messaggio.c_str());
            Serial.print(mqtt_client.endPublish());
        } else 
            Serial.print(mqtt_client.publish(topic.c_str(), messaggio.c_str()));

    } /*else if (true) {  // TODO metti condizione del tipo "se dispositivo connesso"
        Serial.print("(BLE) " );
        String msg = "["+topic+"] " + messaggio ;
        SerialBT.print(msg.c_str());

    }*/
    else 
        Serial.print("disconnesso");
    
}


