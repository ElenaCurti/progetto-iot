# Esempi di messaggi




## Camera

### Configurazioni
`
mosquitto_pub -t "my_devices/esp_cam/config" -m '{ "freq_send_img": 500, "timeout_send_img": 30, "use_bluetooth": 1 }' 
`


### Deep sleep
`
mosquitto_pub -t "my_devices/esp_cam/deep_sleep" -m "30 s"
`

### Campanello premuto
`
mosquitto_pub -t "my_devices/esp_nfc/button" -m "1"
`

### Intruso rilevato 
`
mosquitto_pub -t  "my_devices/esp_nfc/intruder" -m "intruso"
`

### Manda foto/video
`
mosquitto_pub -t  "my_devices/esp_cam/request_send_img" -m "1"
`

### Reset camera
`
mosquitto_pub -t  "my_devices/esp_cam/reset" -m "1"
`


### Vedi stato connessione MQTT
`
mosquitto_sub -t  "my_devices/esp_cam/state" 
`

### Vedi foto 
`
mosquitto_sub -t "my_devices/esp_cam/image" 
`


## Nfc

### Configurazioni
Abilita tag nero e bianco:

`
mosquitto_pub -t "my_devices/esp_nfc/config" -m "{ 'tag_autorizzati': [ '122.48.29.217', '209.53.34.217' ] }"
`


Blocca tag bianco:

`
mosquitto_pub -t "my_devices/esp_nfc/config" -m "{ 'tag_autorizzati': [ '209.53.34.217' ] }"
`


Cambia numero di tentativi errati:
     
`
mosquitto_pub -t "my_devices/esp_nfc/config" -m '{ "num_tentativi_errati": [1, 2]  }'
`


Check tag su Node Red:

`
mosquitto_pub -t "my_devices/esp_nfc/config" -m '{ "check_tag_localmente": false }'
`


Bluetooth sempre acceso:

`
mosquitto_pub -t "my_devices/esp_nfc/config" -m '{ "use_bluetooth": 2 }'
`


Deep sleep dopo 60 sec di inattivita':

`
mosquitto_pub -t "my_devices/esp_nfc/config" -m '{ "secondi_board_inattiva": 30 }'
`


### Apri porta
`
mosquitto_pub -t "my_devices/esp_nfc/led" -m "1"
`

### Lettore NFC
Blocca

`
mosquitto_pub -t "my_devices/esp_nfc/nfc_reader_state" -m "0"
`


Sblocca
`
mosquitto_pub -t "my_devices/esp_nfc/nfc_reader_state" -m "1"
`

Chiedi stato del lettore NFC

`
mosquitto_sub -t "my_devices/esp_nfc/nfc_attempts"

mosquitto_pub -t "my_devices/esp_nfc/nfc_reader_state" -m "2"
`



### Deep sleep
`
mosquitto_pub -t "my_devices/esp_nfc/deep_sleep" -m '30 s'
`


### Controlla stato connessione MQTT
`
mosquitto_sub -t "my_devices/esp_nfc/state"
`




### Reset 
`
mosquitto_pub -t "my_devices/esp_nfc/reset" -m '1'
`

## Configurazioni globali
`
mosquitto_pub -t  "my_devices/global_config/change_broker" -m "broker.mqtt.cool"
` 

`
mosquitto_pub -t  "my_devices/global_config/rst_disconnect" -m "-1"
`