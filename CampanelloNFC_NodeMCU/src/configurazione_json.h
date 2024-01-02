#include <Arduino.h>

String deserializeJsonConfiguration(const String jsonString, String*& tag_autorizzati, int &num_tag_autorizzati, int* num_tentativi_errati, bool &check_tag_locally, bool &deep_sleep) ;