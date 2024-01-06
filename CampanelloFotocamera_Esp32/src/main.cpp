#include <Arduino.h>
#include <connessione_wifi.h>
#include "camera_ov7670.h"  
// #include <WiFi.h>
// #include "OV7670.h"

#include <PubSubClient.h>
#include <modalita_comunicazione.h>

// Variabili per fare foto ogni MILLISECONDS_SEND_PIC millisecondi
unsigned long previousMillis = 0;
long MILLISECONDS_SEND_PIC = 5*1000; // TODO Configurazione

// Variabili per il websocket
IPAddress ip_address_esp ;

// Variabili per mqtt
const int NUM_SUB = 2;
const char* TOPIC_CONFIGURAZIONE = "door/esp_cam/config";
const char* TOPIC_CAMPANELLO_PREMUTO = "door/esp_nfc/button" ;


const char* TOPIC_WILL_MESSAGE = "door/esp_cam/state";
const char* TOPIC_PUBLISH_IMMAGINE = "immagine";  // TODO cambia topic
// char* hexArray;


// Variabili per BLE



void messageReceived2(String &topic, String &payload) {
  // NB: NON USARE mqtt_client QUI -> se devi usarlo, cambia una variabile globale
  Serial.println("incoming: " + topic + " - " + payload);
}

// PROVA:
// const int SIOD = 21; //SDA
// const int SIOC = 22; //SCL

// const int VSYNC = 34;
// const int HREF = 35;

// const int XCLK = 32;
// const int PCLK = 33;

// const int D0 = 27;
// const int D1 = 17;
// const int D2 = 16;
// const int D3 = 15;
// const int D4 = 14;
// const int D5 = 13;
// const int D6 = 12;
// const int D7 = 4;
// OV7670 *camera2;


void setup() {
  Serial.begin(115200); // TODO serial forse e' considerata quella del BLE ?

  

  Serial.println("--- Fotocamera --- ");
  
  
  // Inizializzo Wifi e MQTT 
  connessione_wifi(); // TODO questo va fatto non blocante
    
  // Controllo MQTT e Bluetooth
  const String elenco_subscription[NUM_SUB] = {TOPIC_CONFIGURAZIONE, TOPIC_CAMPANELLO_PREMUTO};
  mqtt_ble_setup("cam", elenco_subscription, NUM_SUB, TOPIC_WILL_MESSAGE);
  gestisciComunicazioneIdle();

  
  // Inizializzo la camera facendo una foto
  // if (camera2 == NULL) {
  //     Serial.print("Inizializzo camera...");
  //     while (camera2 == NULL){ // TODO non bloccante
  //         Serial.print(".");
  //         camera2 = new OV7670(OV7670::Mode::QQQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
  //     }
  //     Serial.println("OK");
  // }

  
}

void take_pic_and_send(){
  Serial.print("Faccio foto...") ;
   /*Serial.print(mqtt_is_connected());
  camera2->oneFrame();
  Serial.print(mqtt_is_connected());

  
  unsigned char* foto =camera2->frame;// take_picture(size);
  size_t size = camera2->xres * camera2->yres * 2;*/ 

  size_t size;
  unsigned char* foto = take_picture(size);
  Serial.print("ok\t") ;
  
  // Serial.print("Converto...") ;
  String string_to_send = convert_to_mqtt_string(foto, size);
  // Serial.print("ok\t") ;

  Serial.print("Invio...Risulato: ") ;
  unsigned int tempo_prec = millis();
  // string_to_send[2000] = '\0';
  inviaMessaggio(TOPIC_PUBLISH_IMMAGINE, string_to_send);
  
  Serial.println("\tTempo impiegato: " + (String) (millis() - tempo_prec) + " ms");
}

void Bluetooth_handle();

unsigned int last_mqtt_loop_called = -1;
void loop() {
  // Serial.print(xPortGetCoreID());
  // Serial.print("***");
  
  // return;


  gestisciComunicazioneIdle();

  /*if (millis() - previousMillis >= MILLISECONDS_SEND_PIC) {

    
    previousMillis = millis();

  } */ 

  delay(20);


  
  
/*
  // Chiamo i loop di web socket o mqtt 
  if (modalita_usata == IMMAGINE_CON_MQTT){
    mqtt_clientLoop(mqtt_client);
    last_mqtt_loop_called = millis();
  } else if (modalita_usata == IMMAGINE_CON_BLE){
    bleSerial.poll();

    while (bleSerial.available() > 0) {
      // Leggo i dati dalla ble serial
      Serial.write(bleSerial.read());
    }
  }
  
  // Controllo se e' ora di mandare la foto
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= MILLISECONDS_SEND_PIC) {
    
    
   if (modalita_usata == IMMAGINE_CON_MQTT){
      // Se uso mqtt, pubblico la foto sul topic

      Serial.print("Faccio foto...") ;
      size_t size;
      unsigned char* foto = take_picture(size);
      Serial.print("ok\t") ;
      
      Serial.print("Converto...") ;
      String string_to_send = convert_to_mqtt_string(foto, size);
      Serial.print("ok\t") ;

      Serial.print("Pubblico...Risulato: ") ;
      unsigned int tempo_prec = millis();
      Serial.print(mqtt_client.publish(TOPIC_PUBLISH_IMMAGINE, string_to_send.c_str()));
      Serial.println("\tTempo impiegato: " + (String) (millis() - tempo_prec));

    } else if (modalita_usata == IMMAGINE_CON_BLE) {
      size_t size;
      unsigned char* foto = take_picture(size);
      String string_to_send = convert_to_mqtt_string(foto, size);

      
      Serial.println("mando fotos");
      bleSerial.print(string_to_send.c_str());
        
      

      // SerialBT.write((uint8_t *) string_to_send.c_str(), (size_t) string_to_send.length());
    }


    previousMillis = currentMillis;

  }
*/
  
}

void messaggio_arrivato(char* topic, byte* payload, unsigned int length) {
  String payload_str = "" ;
  for (size_t i = 0; i < length; i++)  {
    payload_str += (char) payload[i];
  }
  Serial.println("[" + (String) topic + "] " + payload_str);


  if (((String) TOPIC_CAMPANELLO_PREMUTO).equals(topic)){
    if (payload_str.equals("1")){
        take_pic_and_send();
    }
  }
}