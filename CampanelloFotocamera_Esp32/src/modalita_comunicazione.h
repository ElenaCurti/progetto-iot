#include <MQTTClient.h>
#include <HardwareBLESerial.h>

enum modalita_comunicazione {
  IMMAGINE_CON_MQTT, 
  IMMAGINE_CON_BLE
} ; 

modalita_comunicazione getModalitaComunicazioneUsata();
void gestisciComunicazioneIdle(MQTTClient &mqtt_client, HardwareBLESerial &bleSerial);
