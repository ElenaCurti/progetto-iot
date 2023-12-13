//	knolleary/PubSubClient@^2.8

#include <connessione_wifi.h>
#include <PubSubClient.h>

void mqtt_inizializzaServerAndCallback(const char* server, const String clientId, int port, boolean is_ip);
// void mqtt_inizializzaServerAndCallback(const IPAddress ip_server, const String clientId);
void mqtt_inizializzaServerAndCallback(const char* server, const String clientId);
// void mqtt_inizializzaServerAndCallback(const char* server, const String clientId, int port);
void mqtt_clientLoop();

void mqtt_callback(char* topic, byte* payload, unsigned int length) ;