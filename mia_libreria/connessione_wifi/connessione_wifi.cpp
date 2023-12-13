#include "connessione_wifi.h"

void connessione_wifi(){
    // TODO se la esp non si connette (es. entro 5 sec) -> prova a fare restart dell'esp

    // #define WIFI_PROF
    // #define WIFI_CASA


    #ifdef WIFI_PROF
        const char* ssid = "MORE-IOT";
        const char* password = "MORE-IOT-PWD";
    #else
        #ifdef WIFI_CASA
            #include "credenziali_wifi_casa.h"
        #else
            const char* ssid = "Wifi Ele";
            const char* password = "ElenaCurti00";
        #endif
    #endif

    WiFi.mode(WIFI_STA);

    WiFi.begin(ssid, password);
  
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
        Serial.println(WiFi.localIP());

    }

    Serial.print("Connected to WiFi. IP: ");
    Serial.println(WiFi.localIP());

    WiFi.setAutoReconnect(true);    // to reconnect to Wi-Fi after a connection is lost
    WiFi.persistent(true);          // to automatically reconnect to the previously connected access point

}
