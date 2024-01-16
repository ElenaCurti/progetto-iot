
void mqtt_ble_setup(String device_name, const String lista_topic_subscription[], int num_topic_subscription, const String topic_will_message);

void gestisciComunicazioneIdle();

void inviaMessaggio(String topic, String messaggio, int size_messaggio);
void inviaMessaggio(String topic, String messaggio, bool retain);
void inviaMessaggio(String topic, String messaggio);

void inviaBigMessaggio(String& topic, String& messaggio, int size_messaggio);

void messaggio_arrivato2(String topic, String payload_str) ;
void messaggio_arrivato(char* topic, byte* payload, unsigned int length) ;

bool mqtt_is_connected();
void cambia_broker_mqtt(String new_broker);
void cambia_reset_board_mqtt_disconnect(String payload_str);

void usa_bluetooth_changed(int nuovo_usa_bt);

