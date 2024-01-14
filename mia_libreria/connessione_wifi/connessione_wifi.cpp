#include "connessione_wifi.h"
// #include <ESP32Ping.h>

const int SECONDI_ATTESA_WIFI = 10;

void setup_wifi(){

    // Definiti in connessione_wifi.h
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

bool connessione_wifi(){
    // TODO se la esp non si connette (es. entro 5 sec) -> prova a fare restart dell'esp
    // TODO fai uno script su pc per connettersi sempre allo stesso wifi e per resettare mosquitto ogni volta che si cambia wifi

    if (WiFi.status() == WL_CONNECTED)
        return true;
    
    unsigned long primo_tentativo_connessione_wifi = millis(); 

    Serial.print("[WIFI] Connecting to WiFi...");
    setup_wifi();
    while (WiFi.status() != WL_CONNECTED && millis() - primo_tentativo_connessione_wifi <= SECONDI_ATTESA_WIFI*1000) {
        Serial.print(".");
        // Serial.print("Ping 192.168.1.23: " );
        // Serial.print(Ping.ping(IPAddress(192,168,1,23), 1));
        // Serial.print("\tPing 8.8.8.8: " );
        // Serial.println(Ping.ping(IPAddress(8,8,8,8), 1));

        delay(1000);

        // Serial.println(WiFi.localIP());

    }

    if (WiFi.status() == WL_CONNECTED){
        Serial.print("OK. IP: ");
        Serial.println(WiFi.localIP());
        return true;
    } else
        Serial.println("ERROR");
    return false;

}





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