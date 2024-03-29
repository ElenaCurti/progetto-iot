#include <WiFiClient.h>
#include "modalita_comunicazione.h"
#include <PubSubClient.h>
#include <connessione_wifi.h>
#include "BluetoothSerial.h"
#include <IPAddress.h>

// Nome BT e MQTT del device + Elenco dei topic a cui la board e' sottoscritta
String device_name_g ="";
String* lista_topic_subscription_g;
int num_topic_subscription_g;

// Variabili per il wifi
unsigned long last_wifi_reconnection_attempt=-1;
const int MILLISECONDI_DISCONNESSIONE_WIFI_AMMESSA = 5*1000;
const int MILLISECONDI_DISCONNESSIONE_WIFI_AMMESSA2 = 20*1000;


// Variabili per MQTT
WiFiClient espClient;
PubSubClient mqtt_client(espClient);
String broker_mqtt = "";
String topic_will_message_g ;
const int BIG_MESSAGE_SIZE_CHUNK_BYTE = 2400;
const int PORTA_DEFAULT_BROKER = 1883;

unsigned long first_stable_mqtt_connection=-1;
unsigned long last_mqtt_reconnection_attempt=-1;

// Se MQTT perde la connessione, si prova a riconnettere dopo un certo #secondi, settabili qui
const int MILLISECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT = 2*1000;



// Variabili per il bluetooth
BluetoothSerial SerialBT;
int usa_bluetooth = 1;  // 0->non usarlo, 1->usalo solo se MQTT non va, 2->sempre acceso
bool wait_for_init = false;
bool bluetooth_is_acceso = false;
// const uint8_t remoteAddressPC[] = {0x90, 0xE8, 0x68, 0xEF, 0x95, 0x1A};
// const uint8_t remoteAddressCell[] = {0xF4, 0x7D, 0xEF, 0x70, 0x90, 0xEE};

void accendi_blt();
void spegni_blt();

int num_tentativi_connessione_mqtt = 0 ; 
int num_tentativi_connessione_wifi = 0 ;
int num_tentativi_falliti_send_messaggio = 0;  
int max_tentativi_reconnect_prima_del_restart = -1; // TODO metti a 20
const int TENTATIVI_DISCONNESSIONE_MQTT_POI_ACCENDI_BT = 3;
const int TENTATIVI_FALLITI_SEND_IMMAGINE = 5;

// Variabile per il deep sleep
// bool deep_sleep = true;
// const int SECONDI_MQTT_STABILE_POI_DEEP_SLEEP = 60;


// ************ SETUP E LOOP ****************

void mqtt_ble_setup(String device_name, const String lista_topic_subscription[], int num_topic_subscription, const String topic_will_message){
    // Salvo le variabili globali
    device_name_g = device_name;
    num_topic_subscription_g = num_topic_subscription;
    lista_topic_subscription_g = new String[num_topic_subscription];
    for (int i = 0; i < num_topic_subscription; i++) 
        lista_topic_subscription_g[i] = lista_topic_subscription[i];
    topic_will_message_g = topic_will_message;

    // Inizializzo Wifi e MQTT
    if (WiFi.status() != WL_CONNECTED) 
        connessione_wifi();

    
    IPAddress ip_broker;
    if (broker_mqtt.equals("")){
        // Board appena inizializzata, metto di default l'ip del pc
        #ifdef WIFI_CASA
        ip_broker = IPAddress(192,168,1,23);    
        #else
        ip_broker = IPAddress(192,168,43,252);  
        #endif
        mqtt_client.setServer(ip_broker, PORTA_DEFAULT_BROKER);
        broker_mqtt = ip_broker.toString(); 
        Serial.println("[MQTT] Broker iniziale: " + broker_mqtt);
    } else if (ip_broker.fromString(broker_mqtt)){
        // broker_mqtt e' un ip
        mqtt_client.setServer(ip_broker, PORTA_DEFAULT_BROKER);
    } else if (broker_mqtt.indexOf(':') != -1 ) {
        // broker_mqtt e' nel formato url:port oppure ip:port
        String url_or_ip = broker_mqtt.substring(0, broker_mqtt.indexOf(':'));
        int porta = broker_mqtt.substring(broker_mqtt.indexOf(':') + 1).toInt();
        
        Serial.println(porta);
        if (ip_broker.fromString(url_or_ip))
            mqtt_client.setServer(ip_broker, porta);
        else 
            mqtt_client.setServer(url_or_ip.c_str(), porta);

    } else  {
        // broker_mqtt e' un url senza la porta
        mqtt_client.setServer(broker_mqtt.c_str(), PORTA_DEFAULT_BROKER);
    }

    mqtt_client.setCallback(messaggio_arrivato);

    // Inizialzzo Bluetooth solo se usa_bluetooth e' 2
    if (usa_bluetooth == 2 && !bluetooth_is_acceso){
        accendi_blt();
    }
        
}


void mqtt_bt_loop(){

    // Serial.print("gc ");
    
    if (WiFi.status() != WL_CONNECTED) {
        //  Inizilizzo il Wifi
        if (millis() - last_wifi_reconnection_attempt >= MILLISECONDI_DISCONNESSIONE_WIFI_AMMESSA){
            Serial.println("[WIFI] Disconnetto e riconnetto Wifi...");
            WiFi.disconnect();  // TODO forse disconnect e reconnect vanno tolti
            setup_wifi();
            // WiFi.reconnect();

            last_wifi_reconnection_attempt = millis();
            num_tentativi_connessione_wifi ++;
        }
        

    }

        
    if (mqtt_client.connected()){ 
        // Client MQTT connesso correttamente
        
        num_tentativi_connessione_mqtt = 0;
        
        if (bluetooth_is_acceso && usa_bluetooth == 1){
            spegni_blt();
        }

        // Eventualmente vai in deep sleep
        // if (deep_sleep && millis() - first_stable_mqtt_connection < SECONDI_MQTT_STABILE_POI_DEEP_SLEEP*1000 ){
        //     Serial.println("Vado in deep sleep");
        //     esp_deep_sleep_start();
        // }
        
    } else if (WiFi.status() == WL_CONNECTED){
        num_tentativi_connessione_wifi = 0;
        
        // Il client non comunica con MQTT, ma e' connesso al Wifi -> se sono passati piu' di SECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT, ritento la connessione
        if (millis() - last_mqtt_reconnection_attempt >= MILLISECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT){
            num_tentativi_connessione_mqtt ++;
            
            String clientId =  device_name_g ; // + "-"+  String(random(0xffff), HEX); 
            Serial.print("[MQTT] Wifi OK. Attempting MQTT re-connection to "+broker_mqtt+" as " + clientId + " ...");
            
            // Provo a riconnettere il client e specifico il will message da mandare in caso di disconnessione
            String will_messaggio = "0"; 
            if (mqtt_client.connect(clientId.c_str(), "", "", topic_will_message_g.c_str(), 2, true, will_messaggio.c_str(), false)) {
                Serial.println("reconnected as " + clientId );

                for (int i=0; i<num_topic_subscription_g; i++)
                    mqtt_client.subscribe(lista_topic_subscription_g[i].c_str());
                mqtt_client.publish(topic_will_message_g.c_str(), "1", true);   

                first_stable_mqtt_connection = millis();

            } else {
                Serial.print("failed, rc=" + (String) mqtt_client.state());
                Serial.println("\t try again in " + (String) (MILLISECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT/1000) + " seconds");
            }

            last_mqtt_reconnection_attempt = millis();
        }
    }


    // Serial.print("l ");
    mqtt_client.loop();
    
    // Se bluetooth e' spento, eventualmente lo accendo
    if (!bluetooth_is_acceso && ((usa_bluetooth == 1 && !mqtt_client.connected() && max(num_tentativi_connessione_mqtt, num_tentativi_connessione_wifi) >=TENTATIVI_DISCONNESSIONE_MQTT_POI_ACCENDI_BT)  || usa_bluetooth == 2 ) ){
        accendi_blt();
    }

    // Eventualmente resetto la board
    if (max_tentativi_reconnect_prima_del_restart!=-1 &&
            (num_tentativi_connessione_mqtt >= max_tentativi_reconnect_prima_del_restart \
            || num_tentativi_connessione_wifi >= max_tentativi_reconnect_prima_del_restart)
        ) {
        Serial.println("[MQTT] Board disconnessa da troppo, faccio il restart");
        ESP.restart();
    }

    
    if (num_tentativi_falliti_send_messaggio >= TENTATIVI_FALLITI_SEND_IMMAGINE){
        Serial.println("Troppi tentativi errati per l'invio di un messaggi grandi. Reset.. ");
        ESP.restart();
    }

}

// ***************** MQTT *************************

bool mqtt_is_connected(){
    return mqtt_client.connected();
}

void cambia_broker_mqtt(String new_broker){
    if (new_broker.equals(broker_mqtt))
        return;
    Serial.println("[MQTT] Disconnetto e cambio broker");
    broker_mqtt = new_broker;
    mqtt_client.publish(topic_will_message_g.c_str(), "0"); // Comunico la disconnessione, andrebbe messo con qos 2
    mqtt_client.disconnect();
    mqtt_ble_setup(device_name_g, lista_topic_subscription_g, num_topic_subscription_g, topic_will_message_g);    
}


void cambia_reset_board_mqtt_disconnect(String payload_str){
    max_tentativi_reconnect_prima_del_restart = atoi(payload_str.c_str());
    num_tentativi_connessione_mqtt = 0 ;    // Resetto per evitare di (potenzialmente) resettare subito lo board
    num_tentativi_connessione_wifi = 0 ;
    Serial.println("[WIFI-MQTT] Cambio max tentativi prima del reset:" + (String) max_tentativi_reconnect_prima_del_restart);
}

// ****************** INVIO MESSAGGI ******************

void inviaBigMessaggio(String &topic, String &messaggio, int size_messaggio){
    Serial.print("(long mess) " );

    if (mqtt_client.connected()){

        Serial.print(mqtt_client.beginPublish(topic.c_str(), messaggio.length(), false));
        Serial.print("a ");

        // Con MQTT "splitto" le write del messaggio, per aiutare la pubblicazione. Il messaggio inviato sara' comunque solo uno
        int length = messaggio.length();
        int chunkSize = BIG_MESSAGE_SIZE_CHUNK_BYTE;

        for (int i = 0; i < length; i += chunkSize) {

            Serial.print(" "+(String) i);
            String chunk = messaggio.substring(i, min(i + chunkSize, length));
            Serial.print("i");
            
            if (!mqtt_client.connected()){
                Serial.println("\n[MQTT] ERRORE: disconnesso durante la pubblicazione! ");
                num_tentativi_falliti_send_messaggio++;
                Serial.print(mqtt_client.endPublish());
                return;

            }
            mqtt_client.write((uint8_t *)chunk.c_str(), chunk.length());
            Serial.print("f");
            // mqtt_client.loop();
            delay(10); 
        }
        // Serial.print(mqtt_client.write((uint8_t *)messaggio.c_str(), messaggio.length()));
        Serial.print("b");
        int res = mqtt_client.endPublish();
        if (res == 1)
            num_tentativi_falliti_send_messaggio = 0;
        else 
            num_tentativi_falliti_send_messaggio++;
        
        Serial.print(res);
    
    
    } else if (SerialBT.hasClient()) {
        // Serial.println("bt big mess"+ (String) msg.length());
        // String msg = "["+topic+"] " + messaggio;
        // Serial.print(SerialBT._spp_queue_packet_img((uint8_t *) msg.c_str(), msg.length()));
        // return;
        size_t risultato_tot = 0; 
        risultato_tot += SerialBT.print("[");
        risultato_tot += SerialBT.print(topic);
        risultato_tot += SerialBT.print("] ");
        
        risultato_tot += SerialBT.println(messaggio);

        /*int chunkSize = 100;

        for (int i = 0; i < size_messaggio; i += chunkSize) {

            Serial.print(" "+(String) i);
            String chunk = messaggio.substring(i, min(i + chunkSize, size_messaggio));
            Serial.print("i");
            risultato_tot += SerialBT.print(chunk);
            Serial.print("f");
            delay(10); 
        }

        risultato_tot += SerialBT.println();*/

        // Size del messaggio + size del topic + 2 parentesi + 1 spazio + 1 new line + \0
        if (risultato_tot == size_messaggio+topic.length() + 5)
            num_tentativi_falliti_send_messaggio = 0;
        else 
            num_tentativi_falliti_send_messaggio++;

        Serial.print((String) risultato_tot ); //+ "Ideale: " + (String)size_messaggio+topic.length());

    } else 
        Serial.println("board disconnessa");
    
   
}


void inviaMessaggio(String topic, String messaggio, int size_messaggio, bool retain){
    
    if (mqtt_client.connected()){
        Serial.print("(MQTT) " );
        Serial.print(mqtt_client.publish(topic.c_str(), messaggio.c_str(), retain));
    
    } else if (SerialBT.hasClient()) {  
        Serial.print("(BT ) " );
        String msg = "["+topic+"] " + messaggio;
        size_t risultato_invio = SerialBT.println(msg);
        Serial.println(risultato_invio);
    } else 
        Serial.println("board disconnessa");
    
}
void inviaMessaggio(String topic, String messaggio){
    inviaMessaggio(topic, messaggio, messaggio.length(), false);
}
void inviaMessaggio(String topic, String messaggio, bool retain){
    inviaMessaggio(topic, messaggio, messaggio.length(), retain);
}

// ********************* BLUETOOTH ******************

void callback_bluetooth(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
    if(event == ESP_SPP_INIT_EVT){
        wait_for_init = true;
        Serial.print("[BLUETOOTH] Indirizzo Bluetooth della esp e'\t" + SerialBT.getBtAddressString());

    } else if(event == ESP_SPP_SRV_OPEN_EVT){
        Serial.print("[BLUETOOTH] Client connected to ");
        
        char* address = (char*)param->srv_open.rem_bda;
        for (int i=0; i<ESP_BD_ADDR_LEN; i++){
            Serial.print(address[i], HEX);
            Serial.print(":");
        }
        Serial.println("");        

    } else if (event == ESP_SPP_CLOSE_EVT){
        Serial.println("[BLUETOOTH] Client disonnected");

    } else if (event == ESP_SPP_DATA_IND_EVT){
        String data = (char*) param->data_ind.data;
        int len = param->data_ind.len;
        
        Serial.println("[BLUETOOTH] Nuovo messaggio arrivato: len=" + (String) len  + "\tMess:" + (String) data );
        
        int fine_topic = data.indexOf("]");
        if (fine_topic == -1)
            return;

        String topic = data.substring(1, fine_topic);
        String payload = data.substring(fine_topic + 2, len); 
        messaggio_arrivato2(topic, payload);

    
    } /*else 
        Serial.println("Altro evento:" + (String)((int) event));*/
}

void accendi_blt(){
    Serial.print("[BLUETOOTH] Accendo bluetooth ...") ; 
    if(SerialBT.begin(device_name_g)){
        Serial.println("OK");
        bluetooth_is_acceso = true;
    } else{
        Serial.println("ERROR");
        bluetooth_is_acceso = false;
    }
    SerialBT.register_callback(callback_bluetooth);
}


void spegni_blt(){
    Serial.println("[BLUETOOTH] Spengo il bluetooth");
    SerialBT.end();
    bluetooth_is_acceso = false;
}


void usa_bluetooth_changed(int nuovo_usa_bt){
    
    if (usa_bluetooth == nuovo_usa_bt)
        return;

    Serial.println("[BLUETOOTH] Usa bluetooth changed");
    switch (nuovo_usa_bt){
        case 0:
            spegni_blt();
            break;
        case 1:
            if (bluetooth_is_acceso && mqtt_client.connected())
                spegni_blt();
            break;
        case 2: 
            accendi_blt();
            break;
    }
    usa_bluetooth = nuovo_usa_bt;
    

}

// ************ CAMBIO DEEP SLEEP ************ 
// void change_deep_sleep(bool new_deep_sleep){
//     deep_sleep = new_deep_sleep;
// }