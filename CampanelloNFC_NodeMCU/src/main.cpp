#include <Arduino.h>
#include "nfc_nodemcu.h"

#define USA_BLUETOOTH
#include <modalita_comunicazione.h>

#include "configurazione_board.h"

// Lista dei vari topic 
// TODO se broker remoto: aggiungi un livello root per filtrare i "miei" messaggi
const int NUM_SUB = 3;
const char* TOPIC_CONFIGURAZIONE = "door/esp_nfc/config";
const char* TOPIC_STATO_PORTA = "door/esp_nfc/led";
const char* TOPIC_STATO_LETTORE_NFC = "door/esp_nfc/nfc_reader_state";

const char* TOPIC_WILL_MESSAGE = "door/esp_nfc/state";
const char* TOPIC_BUTTON_PREMUTO = "door/esp_nfc/button" ;
const char* TOPIC_TAG_STRISCIATO = "door/esp_nfc/tag_swiped";
const char* TOPIC_INTRUSO = "door/esp_nfc/intruder"; 
const char* TOPIC_ERRORE = "door/esp_nfc/errors"; 

// Lista dei parametri configurabili, inizialmente settati con valori di default
int num_tag_autorizzati = 1 ;
String* tag_autorizzati = {new String("209.53.34.217")} ; // Inizialmente solo tag nero autorizzato
int config_num_tentativi_errati_permessi[2] = {3,5};
bool check_tag_locally = true;
bool deep_sleep = false;
 

// Varibili per controllo campanello premuto
#define PIN_CAMPANELLO 35
bool stato_campanello = false;              // True se campanello premuto; false altrimenti
bool old_stato_campanello_premuto = false;  // Come stato_campanello, ma per lo stato "precedente"
void check_campanello_premuto(void* pvParameters);

// Variabili/funzioni usati per il controllo del lettore nfc
bool isTagAuthorized(const String& tag);
int num_tentativi_errati_fatti = 0;
unsigned long ultimo_tentativo_errato = -1 ;
const int SECONDI_BLOCCO_READER = 30;


// Simulatore apertura porta
#define PIN_LED_VERDE 32
void apertura_porta_led_verde();



void setup() {
  Serial.begin(115200);
  Serial.println("--- NFC --- ");
  
  // Setup lettore NFC
  nfc_setup();

  // Setup comunicazione
  const String elenco_subscription[NUM_SUB] = {TOPIC_CONFIGURAZIONE, TOPIC_STATO_PORTA, TOPIC_STATO_LETTORE_NFC};
  mqtt_ble_setup("nfc", elenco_subscription, NUM_SUB, TOPIC_WILL_MESSAGE);

  // Setup campanello + Creo un task per la lettura del button del campanello, per evitare ritardi
  pinMode(PIN_CAMPANELLO, INPUT);
  xTaskCreatePinnedToCore(check_campanello_premuto,"check_campanello_premuto",1000,NULL,0,NULL,0);

  // Setup "porta" (led)
  pinMode(PIN_LED_VERDE, OUTPUT);


}

bool isNFCReaderBloccato(){

  if (num_tentativi_errati_fatti >= config_num_tentativi_errati_permessi[1]){
    // Serial.print("if ");
    return true;
  }

  if (num_tentativi_errati_fatti >= config_num_tentativi_errati_permessi[0]) {
    // Serial.print("else:" + (String) (millis()-ultimo_tentativo_errato) + " ");
    bool isBlocked = millis()-ultimo_tentativo_errato <= SECONDI_BLOCCO_READER*1000;
    if (isBlocked)
      Serial.print("b");
        
    return isBlocked;
  }

  return false;

}

void tentativoErrato(String tag_letto){
  num_tentativi_errati_fatti++;

  if (num_tentativi_errati_fatti == config_num_tentativi_errati_permessi[0]){
    ultimo_tentativo_errato = millis();
    Serial.println("Blocco nfc reader per " + (String) SECONDI_BLOCCO_READER + " secondi"); 
  } else if (num_tentativi_errati_fatti == config_num_tentativi_errati_permessi[1]){
    Serial.println("Blocco nfc reader. Sbloccare da pc."); 
    inviaMessaggio(TOPIC_INTRUSO, tag_letto);

  } else 
    ultimo_tentativo_errato = -1;


}

void loop() {

  gestisciComunicazioneIdle();

  // Lettura NFC
  if (!isNFCReaderBloccato()) {
    String tag_letto = readNFC() ;
    if (!tag_letto.equals("")) {
      Serial.print("Tag letto: " + tag_letto + "\tRisultato dell invio: ");
      inviaMessaggio(TOPIC_TAG_STRISCIATO, tag_letto);  // TODO aggiungi tipo tag_letto + "e' entrato"/ non e' entrato
      Serial.println("");

      if (check_tag_locally){
        if (isTagAuthorized(tag_letto)) {
          apertura_porta_led_verde();
          num_tentativi_errati_fatti = 0;
        } else {
          tentativoErrato(tag_letto); 
          Serial.println("NON autorizzato! Tentitivi: " + (String) num_tentativi_errati_fatti);

          
        }
      }

    }
  }
  // Lettura del click del campanello
  if (stato_campanello){
      Serial.println("Campanello premuto. Invio: ");
      inviaMessaggio(TOPIC_BUTTON_PREMUTO, "1");
      stato_campanello = false;      
  }
  // Serial.print( (String) digitalRead(PIN_CAMPANELLO) + " " );

  


  delay(20);

}



void check_campanello_premuto(void* pvParameters){
  old_stato_campanello_premuto = true;
  while (1) {
    if (digitalRead(PIN_CAMPANELLO) == HIGH && !old_stato_campanello_premuto){
        // Serial.println("[Loop 2] Campanello premuto");
        stato_campanello = true;
    }
    old_stato_campanello_premuto = (digitalRead(PIN_CAMPANELLO) == HIGH);

    delay(20);
  }
}

void apertura_porta_led_verde(){
    Serial.println("Apro porta (led verde)");

    digitalWrite(PIN_LED_VERDE, HIGH);
    delay(2000);
    digitalWrite(PIN_LED_VERDE, LOW);

}


bool isTagAuthorized(const String& tag) {
    for (int i = 0; i < num_tag_autorizzati; i++) {
        if (tag == tag_autorizzati[i]) {
            return true; // Tag found in the authorized list
        }
    }
    return false; // Tag not found in the authorized list
}


void messaggio_arrivato2(String topic, String payload_str) {
  // Controllo se il messaggio e' una configurazione
  Serial.println("[" + (String) topic + "] " + payload_str);
    
  if(((String) TOPIC_CONFIGURAZIONE).equals((String) topic)){
    int usa_bluetooth = -1;
    String err = configurazioneBoardJSON(payload_str, tag_autorizzati,num_tag_autorizzati, config_num_tentativi_errati_permessi, check_tag_locally, deep_sleep, usa_bluetooth) ;
    if (usa_bluetooth!=-1)
      usa_bluetooth_changed(usa_bluetooth);
    
    if (!err.equals(""))
      inviaMessaggio(TOPIC_ERRORE, err.c_str());
    
  }

  if(((String) TOPIC_STATO_LETTORE_NFC).equals((String) topic)){
    if (payload_str.equals("1")){
      num_tentativi_errati_fatti = 0;
      Serial.println("Sblocco reader nfc");
    } else {
      num_tentativi_errati_fatti = config_num_tentativi_errati_permessi[1]-1;
      tentativoErrato("Lettore bloccato manualmente");
    }
  }

  if(((String) TOPIC_STATO_PORTA).equals((String) topic)){
    if (payload_str.equals("1")){
      apertura_porta_led_verde();
    }
  }
  
}


void messaggio_arrivato(char* topic, byte* payload, unsigned int length) {
  String payload_str = "" ;
  for (size_t i = 0; i < length; i++)  {
    payload_str += (char) payload[i];
  }
  
  messaggio_arrivato2((String)topic, payload_str);

}
