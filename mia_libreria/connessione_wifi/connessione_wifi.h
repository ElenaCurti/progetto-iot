#ifdef ESP8266
    #include <ESP8266WiFi.h>
#else 
    #ifdef ESP32
        #include <WiFi.h>
    #else 
        #error "Errore"
    #endif
#endif

void setup_wifi();
void connessione_wifi();
IPAddress esp_as_AP() ;
