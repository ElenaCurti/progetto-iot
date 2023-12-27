/*#include <connessione_wifi.h>
#include "camera_ov7670.h"  // TODO vedi se conveniva usare questa: https://github.com/bitluni/ESP32CameraI2S
#include "mio_websocket.h"
#include <WiFi.h>
#include <WebSocketsServer.h>
#include "mio_mqtt.h"
#include <MQTTClient.h>
#include <HardwareBLESerial.h>


// Modalita per mandare l'immagine:
enum modalita_comunicazione_immagine {
  IMMAGINE_CON_WEBSOCKET, 
  IMMAGINE_CON_MQTT, 
  IMMAGINE_CON_BLE
} ; 
modalita_comunicazione_immagine modalita_usata = IMMAGINE_CON_BLE;


// Variabili per fare foto ogni MILLISECONDS_SEND_PIC millisecondi
unsigned long previousMillis = 0;
long MILLISECONDS_SEND_PIC = -1;

// Variabili per il websocket
IPAddress ip_address_esp ;
extern WebSocketsServer mio_webSocket;

// Variabili per mqtt
const int SECONDI_TENTATIVO_RICONNESSIONE_MQTT = 2;
char* TOPIC_PUBLISH_IMMAGINE = "immagine";
char* hexArray;
size_t size_foto = 9600;
WiFiClient net;
MQTTClient mqtt_client(size_foto*2+100);

// Variabili per BLE
HardwareBLESerial &bleSerial = HardwareBLESerial::getInstance();


void setup() {
  Serial.begin(115200);

  if (modalita_usata == IMMAGINE_CON_WEBSOCKET){
    Serial.println("Web socket per mandare immagine.");
    ip_address_esp = esp_as_AP();
    ws_setup(ip_address_esp); 
    MILLISECONDS_SEND_PIC = 100;
    

  } else if (modalita_usata == IMMAGINE_CON_MQTT){
    Serial.println("MQTT per mandare immagine.");

    connessione_wifi();
    ip_address_esp = WiFi.localIP();
    
    IPAddress ip_broker = IPAddress(192,168,1,53);    // TODO mettere broker come configurazione
    mqtt_client.begin(ip_broker, net);
    mqtt_client.onMessage(messageReceived);
    mqtt_clientLoop(mqtt_client);

    MILLISECONDS_SEND_PIC = 100;

    // Creo l'array che conterra' i bytes  
    while(hexArray == NULL)
      hexArray = (char*)malloc(size_foto * 2 + 1);

    // Inizializzo la camera facendo una foto
    size_t size;
    take_picture(size);

   
    
  } else if (modalita_usata == IMMAGINE_CON_BLE) {
    MILLISECONDS_SEND_PIC = 30 * 1000;

    if (!bleSerial.beginAndSetupBLE("Esp32prova")) {
      while (true) {
        Serial.println("failed to initialize HardwareBLESerial!");
        delay(1000);
      }
    }
    Serial.println("BLE OK");

  } else 
    Serial.println("---Modalita per comunicare immagine sconosciuta!---");


}





unsigned int last_mqtt_loop_called = -1;
void loop() {


  // Chiamo i loop di web socket o mqtt 
  if (modalita_usata == IMMAGINE_CON_WEBSOCKET) {
    ws_loop(); 
  } else if (modalita_usata == IMMAGINE_CON_MQTT){
    mqtt_clientLoop(mqtt_client);
    last_mqtt_loop_called = millis();
  } else if (modalita_usata == IMMAGINE_CON_BLE){
    bleSerial.poll();

    // whatever is written to BLE UART appears in the Serial Monitor
    while (bleSerial.available() > 0) {
      Serial.write(bleSerial.read());
    }
  }
  
  // Controllo se e' ora di mandare la foto
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= MILLISECONDS_SEND_PIC) {
    
    
    if (modalita_usata == IMMAGINE_CON_WEBSOCKET &&  get_id_host_connesso() != -1 ) {
      // Se uso i web sockets e c'e un client connesso -> mando la foto 
      size_t size;
      unsigned char* foto = take_picture(size);
      mio_webSocket.sendBIN(get_id_host_connesso(), foto, size);  // -> ottimo se uso la esp32 per fare da AP
      
    } else if (modalita_usata == IMMAGINE_CON_MQTT){
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

  
}

*/