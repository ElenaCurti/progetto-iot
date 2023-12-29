#include "connessione_wifi.h"
#include <ESP32Ping.h>


IPAddress esp_as_AP() { 
  const char *ap_ssid     = "Wifi Esp32";
  const char *ap_password = "Esp32Password";

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress ip_address_esp = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(ip_address_esp);

  return ip_address_esp;
}


void setup_wifi(){
    // #define WIFI_PROF
    // #define WIFI_CASA


    #ifdef WIFI_PROF
        const char* ssid = "MORE-IOT";
        const char* password = "MORE-IOT-PWD";
    #else
        #ifdef WIFI_CASA
            #include "credenziali_wifi_casa.h"
        #else
            const char* ssid = "Wifi Ele2";
            const char* password = "ElenaaCurti00";
        #endif
    #endif

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    // WiFi.setAutoReconnect(true);    // to reconnect to Wi-Fi after a connection is lost
    // WiFi.persistent(true);          // to automatically reconnect to the previously connected access point
}

void connessione_wifi(){
    // TODO se la esp non si connette (es. entro 5 sec) -> prova a fare restart dell'esp
    // TODO fai uno script su pc per connettersi sempre allo stesso wifi e per resettare mosquitto ogni volta che si cambia wifi

    if (WiFi.status() == WL_CONNECTED)
        return;

    setup_wifi();
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("Connecting to WiFi...\t");
        Serial.print("Ping 192.168.1.23: " );
        Serial.print(Ping.ping(IPAddress(192,168,1,23), 1));
        Serial.print("\tPing 8.8.8.8: " );
        Serial.println(Ping.ping(IPAddress(8,8,8,8), 1));

        delay(1000);

        // Serial.println(WiFi.localIP());

    }

    Serial.print("Connected to WiFi. IP: ");
    Serial.println(WiFi.localIP());
}



