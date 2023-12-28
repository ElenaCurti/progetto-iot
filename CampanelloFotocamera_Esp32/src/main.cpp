#include <Arduino.h>
#include <connessione_wifi.h>
#include "camera_ov7670.h"  // TODO vedi se conveniva usare questa: https://github.com/bitluni/ESP32CameraI2S
// #include "mio_websocket.h"
#include <WiFi.h>
// #include <WebSocketsServer.h>
// #include "mio_mqtt.h"
#include <MQTTClient.h>
#include "modalita_comunicazione.h"

#include <HardwareBLESerial.h>



// Variabili per fare foto ogni MILLISECONDS_SEND_PIC millisecondi
unsigned long previousMillis = 0;
long MILLISECONDS_SEND_PIC = -1;

// Variabili per il websocket
IPAddress ip_address_esp ;

// Variabili per mqtt
char* TOPIC_PUBLISH_IMMAGINE = "immagine";
const size_t size_foto = 9600;
char hexArray[size_foto*2+1];
WiFiClient net;
MQTTClient mqtt_client(size_foto*2+100);

// Variabili per BLE
HardwareBLESerial &bleSerial = HardwareBLESerial::getInstance();
bool stato_ble = false;


void messageReceived2(String &topic, String &payload) {
  // NB: NON USARE mqtt_client QUI -> se devi usarlo, cambia una variabile globale
  Serial.println("incoming: " + topic + " - " + payload);

}


void setup() {
  Serial.begin(115200); // TODO serial forse e' considerata quella del BLE ?

  // // Bluetooth
  // btStart();  Serial.println("Bluetooth On");
  // SerialBT.begin("ESP32_Bluetooth"); //Bluetooth device name
  // Serial.println("The device started, now you can pair it with bluetooth!");

  modalita_comunicazione modalita_usata = getModalitaComunicazioneUsata();

  // if (modalita_usata == IMMAGINE_CON_MQTT){
    Serial.println("MQTT per mandare immagine.");

    connessione_wifi();
    ip_address_esp = WiFi.localIP();
    
    IPAddress ip_broker = IPAddress(192,168,43,252);    // TODO mettere broker come configurazione
    mqtt_client.begin(ip_broker, net);
    mqtt_client.onMessage(messageReceived2);
    // mqtt_client.subscribe("topic");
    // mqtt_client.setKeepAlive(60);
    // mqtt_client.setCleanSession(false);
    // mqtt_client.setTimeout(7000);

    // mqtt_clientLoop_old(mqtt_client, false);
    // SerialBT.begin("myName");
    // if (!bleSerial.beginAndSetupBLE("Esp32prova2")) {
      // Serial.println("failed to initialize HardwareBLESerial!");
      // while (true) {
      //   Serial.println("failed to initialize HardwareBLESerial!");
      //   delay(1000);
      // }
    // }

    MILLISECONDS_SEND_PIC = 100;

    // Creo l'array che conterra' i bytes della foto 
    // Serial.print("Inizializzo array foto: ");
    // while(hexArray == NULL) {
    //   Serial.print(".");
    //   // hexArray = (char*) malloc(size_foto * 2 + 1);
      
    // }
    // Serial.println("OK");
    if (hexArray == NULL) 
      Serial.println("errore hex array foto");


    // Inizializzo la camera facendo una foto
    size_t size;
    take_picture(size);

   
    
  // } else if (modalita_usata == IMMAGINE_CON_BLE) {
    MILLISECONDS_SEND_PIC = 30 * 1000;

    if (!bleSerial.beginAndSetupBLE("Esp32provaa")) {
      Serial.println("failed to initialize HardwareBLESerial!");
      return;
    } else {
      Serial.println(" HardwareBLESerial inizializzata correttamente.");
    }
    
    // Serial.println(bleSerial);


  // } else 
    // Serial.println("---Modalita per comunicare immagine sconosciuta!---");

    gestisciComunicazioneIdle(mqtt_client, &bleSerial);

}



void Bluetooth_handle();

unsigned int last_mqtt_loop_called = -1;
void loop() {

  // Serial.print(WiFi.isConnected());
  // Serial.println("a");
  // Serial.println(WiFi.status() != WL_CONNECTED);
  

  // Modalita di comunicazione
  // mqtt_clientLoop_old(mqtt_client, true);
  gestisciComunicazioneIdle(mqtt_client, &bleSerial);

  // Serial.print(";");
  // Serial.print(bleSerial == NULL);
  
  return ;

  bleSerial.poll();
    // Serial.print("b");

    if (stato_ble != bleSerial){
        // bleSerial.connect()
        Serial.print("Nuovo stato BLE: ");
        Serial.println(bleSerial ? "Connesso" : "NON connesso");
        stato_ble=bleSerial;
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
        // FIXME quando wifi si scollega, la print del bluetooth non funziona. la ricezione pero' funziona ancora
        bleSerial.println("ciao");    
    }




  

  

  // if (SerialBT.available()){

    // Bluetooth_handle();

  // }


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


// void Bluetooth_handle()
// {
// //   char bluedata;
// //   bluedata = SerialBT.parseInt();
// //   Serial.println("VAL:");
// //   Serial.println(bluedata);
//   delay(20);
//   Serial.print("-");
//   if (SerialBT.available()) {
//       String receivedData = SerialBT.readString();
//       Serial.print("Received from PC: ");
//       Serial.println(receivedData);

//       String sendData = "mio messaggio";
//       SerialBT.print("Sent from ESP32: ");
//       SerialBT.println(sendData);
//   }

// }