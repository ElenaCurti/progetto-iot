
#include <HardwareBLESerial.h>

enum modalita_comunicazione {
  IMMAGINE_CON_MQTT, 
  IMMAGINE_CON_BLE
} ; 

modalita_comunicazione getModalitaComunicazioneUsata();
void gestisciComunicazioneIdle();
void inviaMessaggio(String topic, String messaggio);
void init_mqtt_ble();