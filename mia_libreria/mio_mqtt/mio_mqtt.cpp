
#include "mio_mqtt.h"

WiFiClient espClient;
PubSubClient mqtt_client(espClient);
String clientId_salvato = "";
String server_salvato = "";
const int SECONDI_TENTATIVO_RICONNESSIONE_MQTT = 2;
const int PORTA_DEFAULT = 1883;

void mqtt_inizializzaServerAndCallback(const char* server, const String clientId){
  mqtt_inizializzaServerAndCallback(server, clientId, PORTA_DEFAULT, false);
}

void mqtt_inizializzaServerAndCallback(const char* server, const String clientId, boolean is_ip){
  mqtt_inizializzaServerAndCallback(server, clientId, PORTA_DEFAULT, is_ip);
}
void mqtt_inizializzaServerAndCallback(const char* server, const String clientId, int port, boolean is_ip){

  if (WiFi.status() != WL_CONNECTED)
      connessione_wifi();
    // Serial.println("[mqtt_inizializzaServerAndCallback] indirizzo: " + (String)((long long int)&mqtt_client) );

    if (is_ip){
      int ipValues[4];
      int bytesRead = sscanf(server, "%d.%d.%d.%d", &ipValues[0], &ipValues[1], &ipValues[2], &ipValues[3]);
      if (bytesRead != 4) {
        Serial.println("Ip invalido!");
        return ;
      }
      mqtt_client.setServer(IPAddress(ipValues[0], ipValues[1], ipValues[2], ipValues[3]), port);
    } else
      mqtt_client.setServer(server, port);

    mqtt_client.setCallback(mqtt_callback);
    clientId_salvato = clientId;
    server_salvato = String(server);
    mqtt_clientLoop();
}



void mqtt_clientLoop() {
  int num_tentativi_falliti = 0 ;

  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    if (WiFi.status() != WL_CONNECTED)
      connessione_wifi();
    
    Serial.print("Attempting MQTT connection...");
    
    // Attempt to connect
    if (mqtt_client.connect(clientId_salvato.c_str())) {
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
      
      /*if (num_tentativi_falliti >= 2){     // Dopo 2 tentativi, resetto MQTT
        Serial.println("Reset MQTT connection...Tentativi: " + (String) num_tentativi_falliti);
        mqtt_client = PubSubClient(espClient);
        mqtt_inizializzaServerAndCallback(server_salvato.c_str(), clientId_salvato);
      }*/

      Serial.println("connesso?");
      Serial.println(mqtt_client.connected());


    }
  }

  mqtt_client.loop();
}
