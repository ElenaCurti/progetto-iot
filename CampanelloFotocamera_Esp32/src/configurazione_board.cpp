#include "configurazione_board.h"
#include <ArduinoJson.h>

// Chiavi del json
const char* LEVEL_FREQ_SEND_IG = "freq_send_img";
const char* LEVEL_TIMEOUT_SEND_IMG = "timeout_send_img";
const char* LEVEL_USA_BLUETOOTH = "use_bluetooth";

String configurazioneCamJSON(const String jsonString, long& frequenza_invio_immagini, long& timeout_invio_immagini, int &usa_bluetooth) {

  Serial.println("------ Nuova configurazione -----");

  const size_t bufferSize = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(5) + jsonString.length(); 
  DynamicJsonDocument doc(bufferSize);

  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
    Serial.print(F("Failed to parse JSON: "));
    Serial.println(error.c_str());
    return (String) error.c_str();
  }
  
  if (doc.containsKey(LEVEL_FREQ_SEND_IG)) {
    frequenza_invio_immagini = doc[LEVEL_FREQ_SEND_IG];
    Serial.println((String) LEVEL_FREQ_SEND_IG + ": " + (String) frequenza_invio_immagini);
  }

  if (doc.containsKey(LEVEL_TIMEOUT_SEND_IMG)) {
    timeout_invio_immagini = (int) doc[LEVEL_TIMEOUT_SEND_IMG];
    Serial.println((String) LEVEL_TIMEOUT_SEND_IMG + ": " + (String) timeout_invio_immagini);
  }

  if (doc.containsKey(LEVEL_USA_BLUETOOTH)) {
    usa_bluetooth = (int) doc[LEVEL_USA_BLUETOOTH];
    Serial.println((String) LEVEL_USA_BLUETOOTH + ": " + (String) usa_bluetooth);
  }
  
  Serial.println("----------------");
  return "";
}

/*
// NON JSON:

const String TOPIC_CONFIGURAZIONE = "door/esp_nfc/config/#";

IN MESSAGGIO ARRIVATO:

 String sub_topic = (String) topic;
  int pos = sub_topic.indexOf(TOPIC_CONFIGURAZIONE.substring(0, TOPIC_CONFIGURAZIONE.length() - 1));
  if ( pos != -1) {
    sub_topic.remove(pos, TOPIC_CONFIGURAZIONE.length() - 1);
    Serial.println(sub_topic);
    String err = configurazioneBoard((String) sub_topic, payload_str, tag_autorizzati,num_tag_autorizzati, config_num_tentativi_errati_permessi, check_tag_locally, deep_sleep) ;
    if (!err.equals(""))
      inviaMessaggio(TOPIC_ERRORE, err.c_str());
  }


*/


/*int parseStringList(String inputString, String*& stringArray) {
    // Count the number of strings in the list
    int count = 1;  // At least one string (even if the input is empty)
    for (int i = 0; inputString[i] != '\0'; i++) {
        if (inputString[i] == ',') {
            count++;
        }
    }
    Serial.println("count:" + (String) count);

    if (count == 0){
      stringArray = nullptr;
    } else {
      count++;
      stringArray = new String[count];

      String buffer="";  
      int currentIndex = 0;
      for (int i = 0; i <= inputString.length(); i++) {
          if ( i == inputString.length() || inputString[i]==',' ) {
            stringArray[currentIndex] = buffer;
            Serial.println("tag letto:" + (String) stringArray[currentIndex]);
            
            buffer="";
            currentIndex++;
          } else
            buffer += inputString[i];
      }
    
  }
  return count;
  
}


String configurazioneBoard(String sub_topic, const String payload, String*& tag_autorizzati, int &num_tag_autorizzati, int* num_tentativi_errati, bool &check_tag_locally, bool &deep_sleep) {
  Serial.println("------ Nuova configurazione -----");

  if (sub_topic.endsWith("/")) 
    sub_topic = sub_topic.substring(0, sub_topic.length() - 1);

  if (sub_topic.equals(LEVEL_TAG_AUTORIZZATI)){
//        return "Formato non valido! Esempio di formato valido: 122.48.29.217,10.11.12.217";
      num_tag_autorizzati = parseStringList(payload, tag_autorizzati);
      
      Serial.print("Nuovi tag autorizzati: \t");
      for (int i=0; i<num_tag_autorizzati; i++)
        Serial.print(tag_autorizzati[i] + "\t");
      Serial.println("");

  }

  return "";
}
*/