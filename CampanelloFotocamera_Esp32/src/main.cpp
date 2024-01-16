#include <Arduino.h>
#include <connessione_wifi.h>
#include "camera_ov7670.h"  
#include <PubSubClient.h>
#include <modalita_comunicazione.h>

// Variabili per mqtt
const int NUM_SUB = 9;
const char* TOPIC_FREQUENZA_INVIO_IMMAGINI = "my_devices/door/esp_cam/config/freq_send_img";
const char* TOPIC_DEEP_SLEEP = "my_devices/door/esp_cam/config/deep_sleep";
const char* TOPIC_CONFIGURAZIONI_GLOBALI = "my_devices/global_config/change_broker";
const char* TOPIC_CONFIGURAZIONI_GLOBALI_RESET_MQTT_DISCONNECT = "my_devices/global_config/rst_disconnect";
const char* TOPIC_TIMEOUT_INVIO_IMMAGINI = "my_devices/door/esp_cam/config/timeout_send_img";

const char* TOPIC_CAMPANELLO_PREMUTO = "my_devices/door/esp_nfc/button" ;
const char* TOPIC_INTRUSO = "my_devices/door/esp_nfc/intruder" ;
const char* TOPIC_RESET_ESP = "my_devices/door/esp_cam/reset";
const char* TOPIC_RICHIESTA_INVIO_IMMAGINI = "my_devices/door/esp_cam/request_send_img";


const char* TOPIC_WILL_MESSAGE = "my_devices/door/esp_cam/state";  // TODO migliora con orario
const char* TOPIC_PUBLISH_IMMAGINE = "my_devices/door/esp_cam/image"; 
// char* hexArray;

// Variabili per BLE

// Variabili per video in streaming
unsigned long primaImmagineMandata = -1;
unsigned long previousMillisStreamingVideo = 0;
bool streaming_video_in_corso = false;

// Configurazioni
long frequenza_invio_immagini = -1;  
long timeout_invio_immagini = 30*1000;  //Dopo 30 sec da quando e' stato premuto il campanello smetto di inviare il video ; -1 per continuare "per sempre" 
bool deep_sleep = false;  // TODO


void setup() {
  Serial.begin(115200); // TODO serial forse e' considerata quella del BLE ?

  

  Serial.println("--- Fotocamera --- ");
  
    
  // Controllo MQTT e Bluetooth
  const String elenco_subscription[NUM_SUB] = {
    TOPIC_FREQUENZA_INVIO_IMMAGINI,
    TOPIC_DEEP_SLEEP,
    TOPIC_CAMPANELLO_PREMUTO, 
    TOPIC_RESET_ESP, 
    TOPIC_RICHIESTA_INVIO_IMMAGINI, 
    TOPIC_TIMEOUT_INVIO_IMMAGINI, 
    TOPIC_CONFIGURAZIONI_GLOBALI, 
    TOPIC_CONFIGURAZIONI_GLOBALI_RESET_MQTT_DISCONNECT, 
    TOPIC_INTRUSO};
  mqtt_ble_setup("cam", elenco_subscription, NUM_SUB, TOPIC_WILL_MESSAGE);
  gestisciComunicazioneIdle();

}
bool first_pic = true;
void take_pic_and_send(){
  Serial.print("Faccio foto...") ;
  size_t size;
  unsigned char* foto = take_picture(size);

  if (first_pic) {
    foto = take_picture(size);
    first_pic = false;
  }
  // if (size == -1){
  //   Serial.println((char*)foto);
  //   inviaMessaggio(TOPIC_PUBLISH_IMMAGINE, (String) foto);
    
  //   return;
  // }
  // Serial.print("ok\t") ;
  
  Serial.print("Converto...") ;
  String string_to_send = convert_to_mqtt_string(foto, size);
  Serial.print("ok\t") ;

  Serial.print("Invio...Risulato: ") ;
  unsigned int tempo_prec = millis();
  // string_to_send[2000] = '\0';
  // Serial.print("prima:" + (String) string_to_send.length() + " ");
  String tmp_topic = "";
  for (int i =0; i<((String)TOPIC_PUBLISH_IMMAGINE).length(); i++)
    tmp_topic += TOPIC_PUBLISH_IMMAGINE[i];

  inviaBigMessaggio(tmp_topic, string_to_send, string_to_send.length());
  
  Serial.println("\tTempo impiegato: " + (String) (millis() - tempo_prec) + " ms");
}

void Bluetooth_handle();

unsigned int last_mqtt_loop_called = -1;
void loop() {
  
  gestisciComunicazioneIdle();

  // Se devo trasmettere il video in streaming, non e' scaduto il timeout e sono 
  // passati frequenza_invio_immagini millisec dall'ultimo invio dell'immaigne, allroa ne mando un'altra
  if (streaming_video_in_corso) {
    if (timeout_invio_immagini != -1 && millis() - primaImmagineMandata >= timeout_invio_immagini*1000){
      Serial.println("Timeout scaduto");
      streaming_video_in_corso = false;
    }
  
    if (streaming_video_in_corso &&  millis() - previousMillisStreamingVideo >= frequenza_invio_immagini) {
      take_pic_and_send();
      previousMillisStreamingVideo = millis();
    } 
  }

  delay(20);

}

void messaggio_arrivato2(String topic, String payload_str){

  Serial.println("[" + (String) topic + "] " + payload_str);

  // Configurazione "globale" (per tutte le board): broker_mqtt, abilita/disabilita reset board se sconnessa da broker mqtt
  if (((String) TOPIC_CONFIGURAZIONI_GLOBALI).equals(topic)){
    cambia_broker_mqtt(payload_str);
  
  }

  if (((String) TOPIC_CONFIGURAZIONI_GLOBALI_RESET_MQTT_DISCONNECT).equals(topic)){
    cambia_reset_board_mqtt_disconnect(payload_str);  
  }

  
  
  // Configurazioni "locali": frequenza invio immagini, deep sleep, timeout invio immagini
  if (((String) TOPIC_FREQUENZA_INVIO_IMMAGINI).equals(topic)){
    frequenza_invio_immagini = atoi(payload_str.c_str());
    Serial.println("Distanza invio immagini: " + (String) frequenza_invio_immagini);
  
  } else if (((String) TOPIC_TIMEOUT_INVIO_IMMAGINI).equals(topic)){
    timeout_invio_immagini = atoi(payload_str.c_str());
    Serial.println("Timeout video streaming: " + (String) timeout_invio_immagini);
  
  } else if (((String) TOPIC_DEEP_SLEEP).equals(topic)){
    if (payload_str.equals("1"))
        deep_sleep = true;
    else 
        deep_sleep = false;
    Serial.println("Deep sleep: " + (String) deep_sleep);
  
  
  } else if (  ((String) TOPIC_CAMPANELLO_PREMUTO).equals(topic) || ((String) TOPIC_RICHIESTA_INVIO_IMMAGINI).equals(topic)){
    // Se campanello premuto o richiesta invio immagine -> mando foto o video
    if (payload_str.equals("1")){
        primaImmagineMandata = millis();
        take_pic_and_send();
        if (frequenza_invio_immagini != -1) {
          Serial.println("Inizio video ");
          streaming_video_in_corso = true;
        }
    } else {
      Serial.println("Stop video/foto");
      streaming_video_in_corso = false;
    }

  } else if (((String) TOPIC_INTRUSO).equals(topic)){
    Serial.println("Intruso, mando foto");
    take_pic_and_send();
  } else if (((String) TOPIC_RESET_ESP).equals(topic)){
    
    if (payload_str.equals("1")){
        ESP.restart();
    }
  }
  
}


void messaggio_arrivato(char* topic, byte* payload, unsigned int length) {
  String payload_str = "" ;
  for (size_t i = 0; i < length; i++)  {
    payload_str += (char) payload[i];
  }

  messaggio_arrivato2((String) topic, payload_str);
}
