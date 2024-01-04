#include <Arduino.h>
#include <connessione_wifi.h>
#include "camera_ov7670.h"  // TODO vedi se conveniva usare questa: https://github.com/bitluni/ESP32CameraI2S
#include <WiFi.h>
#include <PubSubClient.h>
#include <modalita_comunicazione.h>

#include <HardwareBLESerial.h>

// Variabili per fare foto ogni MILLISECONDS_SEND_PIC millisecondi
unsigned long previousMillis = 0;
long MILLISECONDS_SEND_PIC = 50; // TODO Configurazione

// Variabili per il websocket
IPAddress ip_address_esp ;

// Variabili per mqtt
const int NUM_SUB = 2;
const char* TOPIC_CONFIGURAZIONE = "door/esp_cam/config";
const char* TOPIC_SEND_IMG = "door/esp_cam/send_img";

char* TOPIC_PUBLISH_IMMAGINE = "immagine";
// char* hexArray;


// Variabili per BLE



void messageReceived2(String &topic, String &payload) {
  // NB: NON USARE mqtt_client QUI -> se devi usarlo, cambia una variabile globale
  Serial.println("incoming: " + topic + " - " + payload);

}


void setup() {
  Serial.begin(115200); // TODO serial forse e' considerata quella del BLE ?

  Serial.println("--- Fotocamera --- ");

  // Inizializzo Wifi e MQTT 
  connessione_wifi(); // TODO questo va fatto non blocante
    
  // Controllo MQTT e Bluetooth
  const String elenco_subscription[NUM_SUB] = {TOPIC_CONFIGURAZIONE, TOPIC_SEND_IMG};
  mqtt_ble_setup("cam", elenco_subscription, NUM_SUB);
  gestisciComunicazioneIdle();

  // Inizializzo la camera facendo una foto
  size_t size;
  take_picture(size);

  
}



void Bluetooth_handle();

unsigned int last_mqtt_loop_called = -1;
void loop() {
  // Serial.print(xPortGetCoreID());
  // Serial.print("***");
  
  // return;


  gestisciComunicazioneIdle();

  if (millis() - previousMillis >= MILLISECONDS_SEND_PIC) {

    // Serial.print("Faccio foto...") ;
    size_t size;
    unsigned char* foto = take_picture(size);
    // Serial.print("ok\t") ;
    
    // Serial.print("Converto...") ;
    String string_to_send = convert_to_mqtt_string(foto, size);
    // Serial.print("ok\t") ;

    Serial.print("Invio...Risulato: ") ;
    unsigned int tempo_prec = millis();
    // string_to_send[2000] = '\0';
    inviaMessaggio(TOPIC_PUBLISH_IMMAGINE, string_to_send);
    
    Serial.println("\tTempo impiegato: " + (String) (millis() - tempo_prec) + " ms");

    previousMillis = millis();

  }  

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
}