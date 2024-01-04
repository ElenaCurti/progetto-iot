#include "configurazione_board.h"
#include <ArduinoJson.h>

// Chiavi del json
const char* LEVEL_TAG_AUTORIZZATI = "tag_autorizzati";
const char* LEVEL_NUM_TENTATIVI_ERRATI = "num_tentativi_errati";
const char* LEVEL_CHECK_TAG_LOCALLY = "check_tag_localmente";
const char* LEVEL_DEEP_SLEEP = "deep_sleep";

String configurazioneBoardJSON(const String jsonString, String*& tag_autorizzati, int &num_tag_autorizzati, int* num_tentativi_errati, bool &check_tag_locally, bool &deep_sleep) 
{
  Serial.println("------ Nuova configurazione -----");

  const size_t bufferSize = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(5) + jsonString.length(); 
  DynamicJsonDocument doc(bufferSize);

  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
    Serial.print(F("Failed to parse JSON: "));
    Serial.println(error.c_str());
    return (String) error.c_str();
  }

  
  if (doc.containsKey(LEVEL_TAG_AUTORIZZATI)) {
    JsonArray tagsArray = doc[LEVEL_TAG_AUTORIZZATI].as<JsonArray>();
    int numTags = tagsArray.size();
    // TODO check con array vuoto
   
    Serial.println("Nuovi tag autorizzati: ");
    // if (tag_autorizzati != nullptr) {
    //   delete tag_autorizzati;
    //   tag_autorizzati = nullptr;  
    // }

    num_tag_autorizzati = numTags;
    tag_autorizzati = new String[numTags];

    for (int i = 0; i < numTags; i++) {
      tag_autorizzati[i] = tagsArray[i].as<const char*>();
      Serial.print(tag_autorizzati[i] + "\t");
    }
    Serial.println("");
    
  }

  if (doc.containsKey(LEVEL_NUM_TENTATIVI_ERRATI)) {
    JsonArray tentArray = doc[LEVEL_NUM_TENTATIVI_ERRATI].as<JsonArray>();
    int numTentArray = tentArray.size();
    if(numTentArray!=2)
      return "Errore! Sono necessari i 2 tentativi errati di configurazione!";

    // TODO eventualemente metterlo nella forma:
    // "num_tentativi_errati": [ [3,30], [4,60], [5, -1]], 
    // cioe' dopo 3 tentativi blocca 30 sec, dopo 4 blocca per 60 sec, dopo 5 richiedi sblocco manuale
    num_tentativi_errati[0] = tentArray[0].as<int>();
    num_tentativi_errati[1] = tentArray[1].as<int>();
    
    Serial.println((String) LEVEL_NUM_TENTATIVI_ERRATI + ": " + (String) num_tentativi_errati[0] + "\t" + (String) num_tentativi_errati[1] );
  }

  if (doc.containsKey(LEVEL_CHECK_TAG_LOCALLY)) {
    check_tag_locally = doc[LEVEL_CHECK_TAG_LOCALLY];
    Serial.println((String) LEVEL_CHECK_TAG_LOCALLY + ": " + (String) check_tag_locally);
  }

  if (doc.containsKey(LEVEL_DEEP_SLEEP)) {
    deep_sleep = doc[LEVEL_DEEP_SLEEP];
    Serial.println((String) LEVEL_DEEP_SLEEP + ": " + (String) deep_sleep);
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