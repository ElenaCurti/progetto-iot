#include <connessione_wifi.h>
#include "camera_ov7670.h"
#include "mio_websocket.h"
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <PubSubClient.h>
// #include <Esp32Ping.h>

// #include <mio_mqtt.h>
// #include "OV7670.h"

// Modalita per mandare l'immagine:
enum modalita_comunicazione_immagine {
  IMMAGINE_CON_WEBSOCKET, 
  IMMAGINE_CON_MQTT
} ; 
modalita_comunicazione_immagine modalita_usata = IMMAGINE_CON_MQTT;


// Variabili per fare foto ogni MILLISECONDS_SEND_PIC millisecondi
unsigned long previousMillis = 0;
long MILLISECONDS_SEND_PIC = -1;

// Variabili per il websocket
IPAddress ip_address_esp ;
extern WebSocketsServer mio_webSocket;

// Variabili per mqtt
// extern PubSubClient mqtt_client ;
WiFiClient espClient;
PubSubClient mqtt_client(espClient);
const int SECONDI_TENTATIVO_RICONNESSIONE_MQTT = 2;


char* TOPIC_PUBLISH_IMMAGINE = "immagine";
char* hexArray;
size_t size_foto = 9600;

void initWifiAP() { 
  // TODO sposta questa funzione  in connessione_wifi.cpp

  const char *ap_ssid     = "Wifi Esp32";
  const char *ap_password = "Esp32Password";

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, ap_password);
  ip_address_esp = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(ip_address_esp);
}


void setup() {
  Serial.begin(115200);
  // delay(1000);
  // Serial.println("prova");

  if (modalita_usata == IMMAGINE_CON_WEBSOCKET){
    Serial.println("Web socket per mandare immagine.");
    initWifiAP();
    ws_setup(ip_address_esp); 
    MILLISECONDS_SEND_PIC = 100;
    

  } else if (modalita_usata == IMMAGINE_CON_MQTT){

    
    Serial.println("MQTT per mandare immagine.");
    connessione_wifi();
    ip_address_esp = WiFi.localIP();
    MILLISECONDS_SEND_PIC = 100;//1*1000;
    IPAddress ip_broker = IPAddress(192,168,1,53);    // TODO mettere configurazione
    // bool ris_ping = Ping.ping(ip_broker, 3);
    // if(ris_ping)
    //     Serial.println("Ping pc succesful.");
    // else 
    //     Serial.println("Ping pc failed");

    // ris_ping = Ping.ping("www.google.it", 3);
    // if(ris_ping)
    //     Serial.println("Ping google succesful.");
    // else 
    //     Serial.println("Ping google failed");

    // mqtt_inizializzaServerAndCallback("192.168.1.53", "esp32", 1883, true);
    
    // mqtt_client.setKeepAlive(60); // TODO se non da problemi togli questa riga
    
    mqtt_client.setServer(ip_broker, 1883);
    // mqtt_client.setCallback(mqtt_callback);


    while(hexArray == NULL)
      hexArray = (char*)malloc(size_foto * 2 + 1);

    // Serial.println("get buffer size:" );
    // Serial.println(mqtt_client.getBufferSize());
    
  } else 
    Serial.println("---Modalita per comunicare immagine sconosciuta!---");


}


void mqtt_clientLoop(){
  int num_tentativi_falliti = 0 ;
  String  clientId = "elenaId-esp32-";

    while (!mqtt_client.connected()) {
    if (WiFi.status() != WL_CONNECTED)
      connessione_wifi();
    
    Serial.print("Attempting MQTT connection...");
    
    // Attempt to connect
    clientId += String(random(0xffff), HEX);

    if (mqtt_client.connect(clientId.c_str())) {
      Serial.println("connected");  
    } else {
      num_tentativi_falliti ++;

      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in "+(String)SECONDI_TENTATIVO_RICONNESSIONE_MQTT+" seconds");
      delay(SECONDI_TENTATIVO_RICONNESSIONE_MQTT*1000);

      if (num_tentativi_falliti >= 6){
        // TODO es. se per 6 volte fallisce, reset esp
        Serial.println("Reset Esp...Tentativi: " + (String) num_tentativi_falliti);
        ESP.restart();
      }

      // Dopo 4 tentativi, resetto il Wifi
      // if (num_tentativi_falliti >= 2){
      //   Serial.println("Reset Wifi connection...Tentativi: " + (String) num_tentativi_falliti);
      //   WiFi.reconnect();

      //   continue; 
      // } 
      
      // if (num_tentativi_falliti >= 2){     // Dopo 2 tentativi, resetto MQTT
      //   Serial.println("Reset MQTT connection...Tentativi: " + (String) num_tentativi_falliti);
      //   mqtt_client = PubSubClient(espClient);
      //   mqtt_inizializzaServerAndCallback(server_salvato.c_str(), clientId_salvato);
      // }

      Serial.println("connesso?");
      Serial.println(mqtt_client.connected());


    }
  }

  mqtt_client.loop();
}

unsigned int last_mqtt_loop_called = -1;
void loop() {


  // Chiamo i loop di web socket o mqtt 
  if (modalita_usata == IMMAGINE_CON_WEBSOCKET) {
    ws_loop(); 
  } else if (modalita_usata == IMMAGINE_CON_MQTT){
    mqtt_clientLoop();
    last_mqtt_loop_called = millis();
    // Serial.println("Chiamo il loop..." + (String) millis());
  }
  // delay(30*1000);
  // return ;
  // Controllo se e' ora di mandare la foto
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= MILLISECONDS_SEND_PIC) {
    
    // Se uso i web sockets e c'e un client connesso -> mando la foto 
    if (modalita_usata == IMMAGINE_CON_WEBSOCKET &&  get_id_host_connesso() != -1 ) {
      size_t size;
      unsigned char* foto = take_picture(size);
      mio_webSocket.sendBIN(get_id_host_connesso(), foto, size);  // -> ottimo se uso la esp32 per fare da AP
      
    } else if (modalita_usata == IMMAGINE_CON_MQTT){
      // Se uso mqtt, pubblico la foto sul topic
      Serial.print("Faccio foto...") ;
      size_t size;
      unsigned char* foto = take_picture(size);
      Serial.print("ok\t") ;
      
      // if (size!=size_foto){
      //   Serial.println("size cambiata!");
      //   return;
      // }
      // Serial.print("Pubblico foto: \tSize:");
      // Serial.print(2*size+1);
      // Serial.print("\tRisultato:");
      Serial.print("Converto...") ;

      String string_to_send = convert_to_mqtt_string(foto, size);
      Serial.print("ok\t") ;


      // for (size_t i = 0; i < 10; i++) {
      //   Serial.print(foto[i]);
      //   Serial.print(" ");
      //   Serial.printf("%d ", string_to_send[i]);
      // }

      // Serial.println("primaaa");
      // convertToHexString(foto, size, hexArray);
      // Serial.println("dopooo");



      // Serial.print("Size: "+ (String) size_foto );
      // Serial.print("\tBuffer size: "+ (String) mqtt_client.getBufferSize()+ "\tResult: " );
      Serial.print("Pubblico...Risulato: ") ;

      unsigned int tempo_prec = millis();
      // String prova = "";
      // for (int i = 0 ; i<120; i++){
      //   prova += "a";
      // }
      // for (int i = 0 ; i<75; i++){
      //   Serial.print((String) i + " " );
      //   Serial.println(mqtt_client.publish("immagine2", prova.c_str()));
      // }

      // Serial.println(mqtt_client.publish(TOPIC_PUBLISH_IMMAGINE, (uint8_t*) string_to_send.c_str(), string_to_send.length(), false));
      Serial.println(mqtt_client.publish(TOPIC_PUBLISH_IMMAGINE, string_to_send.c_str()));
      Serial.println("Lunghezza stringa:" + (String) string_to_send.length());
      Serial.println("Tempo impiegato: " + (String) (millis() - tempo_prec));
//      Serial.println("Differenza di chiamata del client loop: " + (String) (millis() - last_mqtt_loop_called));
      // if (string_to_send != NULL)
      //   free(string_to_send);

    }


    previousMillis = currentMillis;

  }

  
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {}
