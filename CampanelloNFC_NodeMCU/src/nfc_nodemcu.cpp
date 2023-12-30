#include <PN532_HSU.h>
#include <PN532.h>
#include "nfc_nodemcu.h"

/*
-------------------------------------------------
Wiring:
  SDA (filo bianco) -> SD2 (Serial1 RX)
  SCL (filo blu)    -> SD3 (Serial1 TX) 
-------------------------------------------------
Valori letti:
  122.48.29.217   -> con elastico bianco
  209.53.34.217   -> con elastico nero  
*/

PN532_HSU pn532hsu(Serial1);
PN532 nfc(pn532hsu);
volatile bool connected = false;
const int NUMERO_BYTE_UID = 4;
String old_valore = "";

boolean success;
uint8_t uid[NUMERO_BYTE_UID] = { 0, 0, 0, 0};   // Buffer to store the returned UID
uint8_t uidLength;                              // Length of the UID (4 or 7 bytes depending on ISO14443A card type)


void nfc_setup(){
  nfc.begin();
  Serial.print("Searching NFC module...");
  uint32_t versiondata;
  do {    
    // TODO aggiungi tipo max 5 sec di ricerca
    versiondata = nfc.getFirmwareVersion();
    Serial.print(".");
  } while(! versiondata);

  Serial.println("OK");
  nfc.SAMConfig();

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  // nfc.setPassiveActivationRetries(0xFF);

  return;

  // Got valid data, print it out!
  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); 
  Serial.println((versiondata >> 8) & 0xFF, DEC);

}


String tagToString(const uint8_t* id) {
  String tagId = "";
  for (byte i = 0; i < NUMERO_BYTE_UID; i++)
  {
    if (i < NUMERO_BYTE_UID-1) 
      tagId += String(id[i]) + ".";
    else 
      tagId += String(id[i]);
  }
  return tagId;
}
 
String readNFC() {
  Serial.print(" Funzione1");
  // if (!nfc.getFirmwareVersion()) {
  //   Serial.println("NFC non trovato!");
  //   nfc_setup();
  //   return "";
  // }

  // Board che si resetta: VEDI
  // https://arduino.stackexchange.com/questions/86274/serial-comm-uart1-works-on-arduino-but-makes-esp32-continuously-reboot
  Serial.print(" Funzione2");

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  Serial.print(" Funzione3");
  
  if (success) {
    String valore_letto  = tagToString(uid);
    
    if (!old_valore.equals(valore_letto)){
      // Serial.print("+");
      old_valore = valore_letto ;
      return valore_letto;
    }

    // Serial.print(".");

  } else {
    // Serial.print("-");
    old_valore ="";
  }

  Serial.print(" Funzione4");


  return "";
  
}

 