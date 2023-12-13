// https://how2electronics.com/interfacing-pn532-nfc-rfid-module-with-arduino/

#include "mia_nfc_library.h"
#include <SoftwareSerial.h>
#include <PN532.h>
#include <PN532_SWHSU.h>


#ifdef ESP8266
      
  #define PIN_SDA D6  // Filo bianco Miso
  #define PIN_SCL D5  // Filo viola clk
#else 
    #ifdef ESP32
      #define PIN_SDA 19  // Filo bianco Miso
      #define PIN_SCL 18  // Filo viola clk
    #else 
        #error "Errore"
    #endif
#endif



/* 
Valori letti:
  122.48.29.217   -> quello con elastico bianco
  209.53.34.217   -> quello con elastico nero  
*/

SoftwareSerial SWSerial( PIN_SDA, PIN_SCL ); // RX, TX 
PN532_SWHSU pn532swhsu( SWSerial );
PN532 nfc( pn532swhsu );
const int NUMERO_BYTE_UID = 4;


void nfc_setup(){
    nfc.begin();
    Serial.print("Searching NFC module...");
    uint32_t versiondata;
    do {    
      // TODO aggiungi tipo max 5 sec di ricerca
      versiondata = nfc.getFirmwareVersion();
      Serial.print(".");
      // if (! versiondata) {
      //     Serial.print("Didn't Find PN53x Module");
      //     // while (1); // Halt
      // }
    } while(! versiondata);

    // Got valid data, print it out!
    Serial.print("Found chip PN5");
    Serial.println((versiondata >> 24) & 0xFF, HEX);
    Serial.print("Firmware ver. ");
    Serial.print((versiondata >> 16) & 0xFF, DEC);
    Serial.print('.'); 
    Serial.println((versiondata >> 8) & 0xFF, DEC);

    // Configure board to read RFID tags
    nfc.SAMConfig();
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

  boolean success;
  uint8_t uid[NUMERO_BYTE_UID] = { 0, 0, 0, 0};   // Buffer to store the returned UID
  uint8_t uidLength;                              // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  
  if (success) {
    String valore_letto  = tagToString(uid);
    // Serial.print("UID Value: " + valore_letto);
    
    return valore_letto;
  }

  return "";
  
}

 