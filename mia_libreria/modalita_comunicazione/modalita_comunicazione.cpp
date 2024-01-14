#include <WiFiClient.h>
#include "modalita_comunicazione.h"
#include <PubSubClient.h>
#include <connessione_wifi.h>
#include "BluetoothSerial.h"


String device_name_g ="";



// TODO: usa BT solo se e' nelle configurazioni e solo se non raggiungibile MQTT
BluetoothSerial SerialBT;
bool client_bluetooth_connected = false;
String clients_connected[3];
int attualmente_connesso = -1;
int usa_bluetooth = 1;  // 0->non usarlo, 1->usalo solo se MQTT non va, 2->sempre acceso
bool wait_for_init = false;
bool bluetooth_is_acceso = false;
const uint8_t remoteAddressPC[] = {0x90, 0xE8, 0x68, 0xEF, 0x95, 0x1A};
const uint8_t remoteAddressCell[] = {0xF4, 0x7D, 0xEF, 0x70, 0x90, 0xEE};

void callback_bluetooth(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
    // Callback function implementation
    Serial.println("");
    
    if(event == ESP_SPP_SRV_OPEN_EVT){
        

        Serial.print("[BLUETOOTH] Client Connected to ");
        
        char* address = (char*)param->srv_open.rem_bda;
        for (int i=0; i<ESP_BD_ADDR_LEN; i++){
            Serial.print(address[i], HEX);
            Serial.print(":");
        }
        Serial.println("");

        client_bluetooth_connected = true;
        
    } else if (event == ESP_SPP_CLOSE_EVT){
        Serial.println("[BLUETOOTH] Client disonnected");
        client_bluetooth_connected = false;

    } else if (event == ESP_SPP_DATA_IND_EVT){
        String data = (char*) param->data_ind.data;
        int len = param->data_ind.len;
        
        Serial.println("[BLUETOOTH] Nuovo messaggio arrivato: len=" + (String) len  + "\tMess:" + (String) data );
        
        int fine_topic = data.indexOf("]");
        if (fine_topic == -1)
            return;

        String topic = data.substring(1, fine_topic);
        String payload = data.substring(fine_topic + 2, len); 
        // Serial.println("Topic: " + topic  + "\t Payload:" + payload );
        messaggio_arrivato2(topic, payload);

    
    } else if(event == ESP_SPP_INIT_EVT){
        wait_for_init = true;
        Serial.print("[BLUETOOTH] Indirizzo Bluetooth della esp e'\t" + SerialBT.getBtAddressString());
    } /*else 
        Serial.println("Altro evento:" + (String)((int) event));*/
}

void accendi_blt(){
    // Serial.println("esp_bluedroid_init. Error:" + (String) ((int) esp_bluedroid_init()));
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


// const size_t size_foto = 9600;
// WiFiClient net;
// MQTTClient mqtt_client(size_foto*2+100);
WiFiClient espClient;
PubSubClient mqtt_client(espClient);
String topic_will_message_g ;
String* lista_topic_subscription_g;
int num_topic_subscription_g;


// void messageReceived2(String &topic, String &payload) ;







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

    #ifdef WIFI_CASA
    IPAddress ip_broker = IPAddress(192,168,1,23);    // TODO mettere broker come configurazione
    #else
    IPAddress ip_broker = IPAddress(192,168,43,252);    // TODO mettere broker come configurazione
    #endif

    mqtt_client.setServer(ip_broker, 1883);
    mqtt_client.setCallback(messaggio_arrivato);
    /*if (device_name.equals("cam")){
        mqtt_client.setBufferSize(9700);
        mqtt_client.setKeepAlive(20);
        // mqtt_client.setSocketTimeout(60);
    }*/



    // Inizialzzo Bluetooth solo se usa_bluetooth e' 2
    if (usa_bluetooth == 2){
        accendi_blt();
    }
        
}


void gestisciComunicazioneIdle(){

    // Serial.print("gc ");
    
    if (WiFi.status() != WL_CONNECTED) {
        //  il Wifi
        if (millis() - last_wifi_reconnection_attempt >= MILLISECONDI_DISCONNESSIONE_WIFI_AMMESSA){
            Serial.println("[WIFI] Disconnetto e riconnetto Wifi...");
            WiFi.disconnect();  // TODO forse disconnect e reconnect vanno tolti
            setup_wifi();
            // WiFi.reconnect();

            last_wifi_reconnection_attempt = millis();
        }
        

    }

        
    if (mqtt_client.connected()){ 
        // Client MQTT connesso -> tutto ok, chiamo il loop
        last_stable_mqtt_connection = millis();

        
        if (bluetooth_is_acceso && usa_bluetooth == 1){
            spegni_blt();
        }
        
        // return;
        // TODO eventualmente vai in deep sleep
    } else if (WiFi.status() == WL_CONNECTED){
        
        // client non comunica con MQTT, ma e' connesso al Wifi -> se sono passati piu' di SECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT, ritento la connessione
        if (millis() - last_mqtt_reconnection_attempt >= MILLISECONDI_TRA_TENTATIVI_RICONNESSIONE_MQTT){
            // String clientId =  device_name_g + "-"+  String(random(0xffff), HEX); 
            String clientId =  device_name_g ; 
            Serial.print("[MQTT] Wifi OK. Attempting MQTT re-connection as " + clientId + " ...");
            
            // Provo a riconnettere il client e specifico il will message da mandare in caso di disconnessione
            String will_messaggio = "0" ;//+ millis(); 
            if (mqtt_client.connect(clientId.c_str(), "", "", topic_will_message_g.c_str(), 2, true, will_messaggio.c_str(), false)) {
                Serial.println("reconnected as " + clientId );

                for (int i=0; i<num_topic_subscription_g; i++)
                    mqtt_client.subscribe(lista_topic_subscription_g[i].c_str());
                mqtt_client.publish(topic_will_message_g.c_str(), "1", true);   

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


    // Serial.print("l ");
    mqtt_client.loop();



    // TODO -> se BLE rotto, risolvi

    
    // Se bluetooth e' spendo, eventualmente lo accendo
    if (!bluetooth_is_acceso && ((usa_bluetooth == 1 && !mqtt_client.connected())  || usa_bluetooth == 2 ) ){
        accendi_blt();
    }
    
            
    /*if (SerialBT.available()) {
        Serial.write(SerialBT.read());
    }*/
}



bool mqtt_is_connected(){
    return mqtt_client.connected();
}


const int BIG_MESSAGE_SIZE_CHUNK_BYTE = 2400;

void inviaBigMessaggio(String &topic, String &messaggio, int size_messaggio){
    Serial.print("(long mess) " );

    if (mqtt_client.connected()){

        Serial.print(mqtt_client.beginPublish(topic.c_str(), messaggio.length(), false));
        Serial.print("a ");

        // "Splitto" le write del messaggio, per aiutare la pubblicazione. Il messaggio inviato sara' comunque solo 1
        int length = messaggio.length();
        int chunkSize = BIG_MESSAGE_SIZE_CHUNK_BYTE;

        for (int i = 0; i < length; i += chunkSize) {

            Serial.print(" "+(String) i);
            String chunk = messaggio.substring(i, min(i + chunkSize, length));
            Serial.print("i");
            if (!mqtt_client.connected()){
                Serial.println("\n[MQTT] ERRORE: disconnesso durante la pubblicazione! ");
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
        Serial.print(mqtt_client.endPublish());
    
    
    } else if (SerialBT.hasClient()) {

        size_t risultato_tot = 0; 
        risultato_tot += SerialBT.print("[");
        risultato_tot += SerialBT.print(topic);
        risultato_tot += SerialBT.print("] ");
        risultato_tot += SerialBT.println(messaggio);
        Serial.print(risultato_tot);

    } else 
        Serial.println("board disconnessa");
    
   
}


void inviaMessaggio(String topic, String messaggio, int size_messaggio){
    
    // if (WiFi.status() == WL_CONNECTED && msqtt_client.connected()){
    if (mqtt_client.connected()){
        Serial.print("(MQTT) " );
        Serial.print(mqtt_client.publish(topic.c_str(), messaggio.c_str()));

    
    } else if (SerialBT.hasClient()) {  
        Serial.print("(BT ) " );
        String msg = "["+topic+"] " + messaggio;
        size_t risultato_invio = SerialBT.println(msg);
        Serial.println(risultato_invio);
    } else 
        Serial.println("board disconnessa");
    
}
void inviaMessaggio(String topic, String messaggio){
    inviaMessaggio(topic, messaggio, messaggio.length());
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
            spegni_blt();
            break;
        case 2: 
            accendi_blt();
            break;
    
    }
    usa_bluetooth = nuovo_usa_bt;
    

}