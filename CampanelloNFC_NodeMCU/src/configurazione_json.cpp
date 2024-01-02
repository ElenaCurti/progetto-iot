#include <ArduinoJson.h>
#include "configurazione_json.h"

// Chiavi del json
const char* CHIAVE_TAG_AUTORIZZATI = "tag_autorizzati";
const char* CHIAVE_NUM_TENTATIVI_ERRATI = "num_tentativi_errati";
const char* CHIAVE_CHECK_TAG_LOCALLY = "check_tag_localmente";
const char* CHIAVE_DEEP_SLEEP = "deep_sleep";


String deserializeJsonConfiguration(const String jsonString, String*& tag_autorizzati, int &num_tag_autorizzati, int* num_tentativi_errati, bool &check_tag_locally, bool &deep_sleep) {
  Serial.println("------ Nuova configurazione -----");

  const size_t bufferSize = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(5) + jsonString.length(); 
  DynamicJsonDocument doc(bufferSize);

  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
    Serial.print(F("Failed to parse JSON: "));
    Serial.println(error.c_str());
    return (String) error.c_str();
  }

  
  if (doc.containsKey(CHIAVE_TAG_AUTORIZZATI)) {
    JsonArray tagsArray = doc[CHIAVE_TAG_AUTORIZZATI].as<JsonArray>();
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

  if (doc.containsKey(CHIAVE_NUM_TENTATIVI_ERRATI)) {
    JsonArray tentArray = doc[CHIAVE_NUM_TENTATIVI_ERRATI].as<JsonArray>();
    int numTentArray = tentArray.size();
    if(numTentArray!=2)
      return "Errore! Sono necessari i 2 tentativi errati di configurazione!";

    // TODO eventualemente metterlo nella forma:
    // "num_tentativi_errati": [ [3,30], [4,60], [5, -1]], 
    // cioe' dopo 3 tentativi blocca 30 sec, dopo 4 blocca per 60 sec, dopo 5 richiedi sblocco manuale
    num_tentativi_errati[0] = tentArray[0].as<int>();
    num_tentativi_errati[1] = tentArray[1].as<int>();
    
    Serial.println((String) CHIAVE_NUM_TENTATIVI_ERRATI + ": " + (String) num_tentativi_errati[0] + "\t" + (String) num_tentativi_errati[1] );
  }

  if (doc.containsKey(CHIAVE_CHECK_TAG_LOCALLY)) {
    check_tag_locally = doc[CHIAVE_CHECK_TAG_LOCALLY];
    Serial.println((String) CHIAVE_CHECK_TAG_LOCALLY + ": " + (String) check_tag_locally);
  }

  if (doc.containsKey(CHIAVE_DEEP_SLEEP)) {
    deep_sleep = doc[CHIAVE_DEEP_SLEEP];
    Serial.println((String) CHIAVE_DEEP_SLEEP + ": " + (String) deep_sleep);
  }
  
  Serial.println("----------------");
  return "";
}