#include <Arduino.h>
#include "nfc_nodemcu.h"
#include <modalita_comunicazione.h>

const char* TOPIC_TAG_NFC = "tag_nfc";

void setup() {
  Serial.begin(115200);
  Serial.println("--- NFC --- ");
  nfc_setup();
  mqtt_ble_setup("nfc");
}

unsigned long last_invio = -1;

void loop() {
  gestisciComunicazioneIdle();

  Serial.print("Leggo tag...");
  String tag_letto = readNFC() ;
  Serial.println("OK\t");

  if (!tag_letto.equals("")) {

  // if (millis() - last_invio >= 5*1000) {
    // String tag_letto = "123";

    Serial.println("Invio: " + tag_letto + "\tRisultato: ");
    inviaMessaggio(TOPIC_TAG_NFC, tag_letto);
    last_invio = millis();
  }

  delay(20);

}

