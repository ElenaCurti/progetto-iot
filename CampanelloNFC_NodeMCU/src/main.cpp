#include <Arduino.h>
#include "nfc_nodemcu.h"

// #define USA_BLUETOOTH
#include <modalita_comunicazione.h>

#include "configurazione_board.h"

#include "driver/rtc_io.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc.h"
#include "esp_sleep.h"



// Lista dei vari topic 
const int NUM_SUB = 7;  // Aggiorna anche array
const char* TOPIC_CONFIGURAZIONE = "my_devices/esp_nfc/config";
const char* TOPIC_CONFIGURAZIONI_GLOBALI = "my_devices/global_config/change_broker";
const char* TOPIC_CONFIGURAZIONI_GLOBALI_RESET_MQTT_DISCONNECT = "my_devices/global_config/rst_disconnect";
const char* TOPIC_RESET_ESP = "my_devices/esp_nfc/reset";

const char* TOPIC_DEEP_SLEEP = "my_devices/esp_nfc/deep_sleep"; 
const char* TOPIC_STATO_PORTA = "my_devices/esp_nfc/led";
const char* TOPIC_STATO_LETTORE_NFC = "my_devices/esp_nfc/nfc_reader_state";


const char* TOPIC_WILL_MESSAGE = "my_devices/esp_nfc/state";
const char* TOPIC_BUTTON_PREMUTO = "my_devices/esp_nfc/button" ;
const char* TOPIC_TAG_STRISCIATO = "my_devices/esp_nfc/tag_swiped";
const char* TOPIC_INTRUSO = "my_devices/esp_nfc/intruder"; 
// const char* TOPIC_PUBLISH_STATO_NFC = "my_devices/esp_nfc/reader_is_blocked";
const char* TOPIC_ERRORE = "my_devices/esp_nfc/errors"; 
const char* TOPIC_PUB_STATO_NFC_E_TENTATIVI = "my_devices/esp_nfc/nfc_attempts";


// Lista dei parametri configurabili, inizialmente settati con valori di default
int num_tag_autorizzati = 1 ;
String* tag_autorizzati = {new String("209.53.34.217")} ; // Inizialmente solo tag nero autorizzato
int config_num_tentativi_errati_permessi[2] = {3,5}; // Dopo 3 tentativi errati bloccati per 30 sec. Dopo 5 lo sbocco deve essere fatto da pc
bool check_tag_locally = true;  // Se true il controllo del tag viene fatto dalla board
int secondi_board_inattiva = -1;  // La board e' considerata "inattiva" dopo un tot di secondi, settabili qui e configurabili. Se =-1 la board non va in deep sleep

unsigned long last_button_premuto_or_tag_letto = millis();  // Per inattivita della board

// Varibili per controllo campanello premuto
#define PIN_CAMPANELLO GPIO_NUM_25
bool stato_campanello = false;              // True se campanello premuto; false altrimenti
bool old_stato_campanello_premuto = false;  // Come stato_campanello, ma per lo stato "precedente"
void check_campanello_premuto(void* pvParameters);

// Variabili/funzioni usati per il controllo del lettore nfc
bool isTagAuthorized(const String& tag);
/* 
    Se num_tentativi_errati_fatti=0 e c'e' il deep sleep (secondi_board_inattiva!=-1) -> supponiamo 
    che l'utente non autorizzato strisci per tante volte un tag non autorizzato -> il lettore si blocca.
    L'utente non autorizzato potrebbe aspettare che la board vada in deep sleep e poi "svegliarla" 
    premendo il campanello -> al risveglio della board, i tentativi verrebbero resettati e il lettore 
    sbloccato.
    Quindi ho disattivato il deep sleep (secondi_board_inattiva=-1) e ho attivato il lettore(
    num_tentativi_errati_fatti = 0). 
    Soluzione alternativa (se si vuole il deep sleep) -> settare num_tentativi_errati_fatti uguale a
    config_num_tentativi_errati_permessi[1]+1  e aspettare che il lettore venga sbloccato dal pc
  */
RTC_DATA_ATTR int num_tentativi_errati_fatti = 0; 
  
unsigned long ultimo_tentativo_errato = -1 ;  // Orario in cui l'utente ha strisciato l'ultimo tag sbagliato
const int SECONDI_BLOCCO_READER = 30;
bool isNFCReaderBloccato();

 



// Simulatore apertura porta
#define PIN_LED_VERDE 32
void apertura_porta_led_verde();

#define uS_TO_S_FACTOR 1000000
void check_wakeup_reason(); 

void setup() {
  Serial.begin(115200);
  Serial.println("--- NFC --- ");
  
  // Setup lettore NFC
  nfc_setup();

  // Setup comunicazione
  const String elenco_subscription[NUM_SUB] = {
    TOPIC_CONFIGURAZIONE, 
    TOPIC_STATO_PORTA, 
    TOPIC_STATO_LETTORE_NFC,
    TOPIC_CONFIGURAZIONI_GLOBALI, 
    TOPIC_CONFIGURAZIONI_GLOBALI_RESET_MQTT_DISCONNECT, 
    TOPIC_RESET_ESP, 
    TOPIC_DEEP_SLEEP
  };
  mqtt_ble_setup("nfc", elenco_subscription, NUM_SUB, TOPIC_WILL_MESSAGE);
  mqtt_bt_loop();


  // Setup campanello + Creo un task per la lettura del button del campanello, per evitare ritardi
  pinMode(PIN_CAMPANELLO, INPUT);
  xTaskCreatePinnedToCore(check_campanello_premuto,"check_campanello_premuto",1000,NULL,0,NULL,0);
  // rtc_gpio_hold_dis(PIN_CAMPANELLO);


  // Setup "porta" (led)
  pinMode(PIN_LED_VERDE, OUTPUT);

  
  check_wakeup_reason();





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

  mqtt_bt_loop();

  // Lettura NFC
  bool stato_attuale_nfc_reader = isNFCReaderBloccato();
  // if (nfc_reader_is_bloccato != stato_attuale_nfc_reader){
  //   inviaMessaggio(TOPIC_PUBLISH_STATO_NFC, (String)((int)stato_attuale_nfc_reader), true);
  //   nfc_reader_is_bloccato = stato_attuale_nfc_reader;
  // }
  if (!stato_attuale_nfc_reader) {
    String tag_letto = readNFC() ;
    if (!tag_letto.equals("")) {
      Serial.println("Tag letto: " + tag_letto );
      
      String result = "";
      if (check_tag_locally){
        if (isTagAuthorized(tag_letto)) {
          apertura_porta_led_verde();
          num_tentativi_errati_fatti = 0;
          result = " - Autorizzato";
        } else {
          tentativoErrato(tag_letto); 
          Serial.println("NON autorizzato! Tentitivi: " + (String) num_tentativi_errati_fatti);
          result = " - Non autorizzato" ; 
        }
      } else 
        result = " - Non controllato" ; 
      
      inviaMessaggio(TOPIC_TAG_STRISCIATO, tag_letto + result);


      last_button_premuto_or_tag_letto = millis();

    }
  }

  // Lettura del click del campanello
  if (stato_campanello){
      Serial.print("Campanello premuto. Invio: ");
      inviaMessaggio(TOPIC_BUTTON_PREMUTO, "1");
      Serial.println();
      stato_campanello = false;   
      last_button_premuto_or_tag_letto = millis();   
  }

  if (secondi_board_inattiva!=-1 && millis() - last_button_premuto_or_tag_letto >= secondi_board_inattiva*1000){
    Serial.print("[Deep sleep] Attivo come wake up source il campanello: ");
    
    Serial.println(esp_sleep_enable_ext0_wakeup(PIN_CAMPANELLO, LOW));

    Serial.print("Vado in deep sleep ");
    esp_deep_sleep_start();

  }

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


void check_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0){
    Serial.println("Board risvegliata al click del campanello");
    // La board si e' svegliata per il click del campanello -> segno il campanello come premuto, in modo da inviare il messaggio "campanello premuto"
    delay(2000);
    stato_campanello = true;
  }
  
}



void messaggio_arrivato2(String topic, String payload_str) {
  // Messaggio MQTT o BT

  Serial.println("[" + (String) topic + "] " + payload_str);

  // Configurazione "globale" (per tutte le board): broker_mqtt, reset board se disconnessa
  if (((String) TOPIC_CONFIGURAZIONI_GLOBALI).equals(topic)){
    cambia_broker_mqtt(payload_str);
  }

  if (((String) TOPIC_CONFIGURAZIONI_GLOBALI_RESET_MQTT_DISCONNECT).equals(topic)){
    cambia_reset_board_mqtt_disconnect(payload_str);  
  }
  
  // Configurazione "locale": tag autorizzati, deep sleep, ecc
  if(((String) TOPIC_CONFIGURAZIONE).equals((String) topic)){
    int usa_bluetooth = -1;
    String err = configurazioneBoardJSON(payload_str, tag_autorizzati,num_tag_autorizzati, config_num_tentativi_errati_permessi, check_tag_locally, secondi_board_inattiva, usa_bluetooth) ;
    if (usa_bluetooth!=-1)
      usa_bluetooth_changed(usa_bluetooth);
    
    if (!err.equals(""))
      inviaMessaggio(TOPIC_ERRORE, err.c_str());
    else 
      last_button_premuto_or_tag_letto = millis();

  }

  // Blocco / sblocco lettore nfc;
  if(((String) TOPIC_STATO_LETTORE_NFC).equals((String) topic)){
    if (payload_str.equals("1")){
      num_tentativi_errati_fatti = 0;
      Serial.println("Sblocco reader nfc");
    } else if (payload_str.equals("0")){
      num_tentativi_errati_fatti = config_num_tentativi_errati_permessi[1]+1;
      Serial.println("Lettore bloccato dal pc");
      // tentativoErrato("Lettore bloccato manualmente");
    } else {
      String msg =  (String)((int) isNFCReaderBloccato()) + " " + (String) num_tentativi_errati_fatti;
      inviaMessaggio(TOPIC_PUB_STATO_NFC_E_TENTATIVI,msg);
    }
  }

  // Apro porta
  if(((String) TOPIC_STATO_PORTA).equals((String) topic)){
    if (payload_str.equals("1")){
      apertura_porta_led_verde();
    }
  }

  // Reset board
  if (((String) TOPIC_RESET_ESP).equals(topic)){
    
    if (payload_str.equals("1")){
        ESP.restart();
    }
  }

  // Deep sleep
  if (((String) TOPIC_DEEP_SLEEP).equals(topic)){
    // Setto la wake up source del deep 
    Serial.print("[Deep sleep] Abilito timer wake up...");
    bool risultato = false;
    int spaceIndex = payload_str.indexOf(' ');
    if (spaceIndex == -1 ){
      Serial.println("Tempo specificato NON valido!");
      return;
    }
    int num = payload_str.substring(0, spaceIndex).toInt();
    char unita_tempo = payload_str.charAt(spaceIndex + 1);

    switch (unita_tempo) {
      case 's':
          risultato = esp_sleep_enable_timer_wakeup(num*uS_TO_S_FACTOR) == ESP_OK;
          break;
      case 'm':
          risultato = esp_sleep_enable_timer_wakeup(60*num*uS_TO_S_FACTOR) == ESP_OK;
          break;
      case 'h':
          risultato = esp_sleep_enable_timer_wakeup(60*60*num*uS_TO_S_FACTOR) == ESP_OK;
          break;
    }

    if (!risultato){
      Serial.println("Tempo specificato NON valido!");
      return;
    }
    Serial.println("ok");

    Serial.print("[Deep sleep] Abilito campanello wake up...");    
    Serial.println(esp_sleep_enable_ext0_wakeup(PIN_CAMPANELLO, LOW)==ESP_OK ? "ok" : "error");

    Serial.println("Deep sleep attivo");
    esp_deep_sleep_start();

  } 
  
}


void messaggio_arrivato(char* topic, byte* payload, unsigned int length) {
  // Messaggio MQTT
  String payload_str = "" ;
  for (size_t i = 0; i < length; i++)  {
    payload_str += (char) payload[i];
  }
  
  messaggio_arrivato2((String)topic, payload_str);

}


