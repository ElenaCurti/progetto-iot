#include "OV7670.h"
// #include "mia_camera_OV7670.h"

const int SIOD = 21; //SDA
const int SIOC = 22; //SCL

const int VSYNC = 34;
const int HREF = 35;

const int XCLK = 32;
const int PCLK = 33;

const int D0 = 27;
const int D1 = 17;
const int D2 = 16;
const int D3 = 15;
const int D4 = 14;
const int D5 = 13;
const int D6 = 12;
const int D7 = 4;

OV7670 *mia_camera;


// Tempo di attesa prima di segnalare errore nella fotocamera
const int SECONDI_ATTESA_CAMERA = 5;


/*void printHexValues(const unsigned char *frame, size_t size) {
  for (size_t i = 0; i < size; i++) {
    // Print the hex values to the Serial Monitor
    Serial.print(frame[i], DEC);
    Serial.print(' '); // Add a space between each byte
  }
  Serial.println(); // Move to the next line after printing all bytes
}

int*  convertToDecimal(unsigned char* foto,  size_t size) {
    int foto_dec[size];

    if (foto_dec == NULL) {
        Serial.println("Errore nell allocazione dell array coi valori decimali delle foto!");
        return NULL;
    }

    for (size_t i = 0; i < size; ++i) 
        foto_dec[i] = (int)foto[i];
    
    return foto_dec;
}


char* convertToHexString(unsigned char* array, size_t size) {
    char* hexString = (char*)malloc((2 * size + 1) * sizeof(char));
    // char hexString [2 * size + 1];

    if (hexString == NULL) {
        // Handle memory allocation failure
        Serial.println("Memory allocation failed!");
        return NULL;
    }

    for (size_t i = 0; i < size; ++i) {
        sprintf(hexString + 2 * i, "%02X", array[i]);
    }

    hexString[2 * size] = '\0'; // Null-terminate the string

    return hexString;
}*/

void convertToHexString(unsigned char* array, size_t size, char* hexArray) {
    for (size_t i = 0; i < size; ++i) {
        snprintf(hexArray + i * 2, 3, "%02X", array[i]);
    }
}

String convert_to_mqtt_string(const unsigned char* array, size_t size, const char* formato) {
    // Il formato può essere: %c -> per ritornare il carattere ASCII    %d -> decimale      %02X -> hex

    if (strcmp(formato, "%d") != 0 && strcmp(formato, "%c") != 0 && strcmp(formato, "%02X") != 0) {
        Serial.println("Formato non valido!");
        return "";
    }

    char buffer[3];  // Buffer for each byte in hexadecimal representation
    String ret_str;

    for (size_t i = 0; i < size; i++) {
        sprintf(buffer, "%02X", array[i]);
        ret_str += buffer;
    }

    ret_str.toUpperCase(); // Convert to uppercase

    return ret_str;
}


String old_convert_to_mqtt_string(unsigned char* array, size_t size, char* formato){
    // Il formato puo' essere: 
    // %c    -> per ritornare il carattere ASCII
    // %d    -> decimale
    // %02X  -> hex


    if (formato!="%d" && formato!="%c" && formato!="%02X"){
        Serial.println("Formato non valido!");
        return "";
    }
    

    // char* ret_str = (char*)malloc(size);
    // if (ret_str == NULL) {
        // Handle memory allocation failure
        // Serial.println("Memory allocation failed!");
        // return NULL;
    // }
    String ret_str = "";
    for (size_t i = 0; i < size; i++) {
        String tmp = String(array[i], HEX); 
        if (tmp.length() < 2)
            tmp = "0" + tmp;
        ret_str += tmp;        // HEX
        ret_str.toUpperCase(); // Convert to uppercase
        
        // ret_str += char(array[i]);
    }
    return ret_str;
}


String convert_to_mqtt_string(unsigned char* array, size_t size){
    return convert_to_mqtt_string(array, size, "%02X");
}


unsigned char* take_picture_with_camera(OV7670*& fotocamera, size_t &size){
    if (fotocamera == NULL) {
        Serial.print("Inizializzo camera...");
        unsigned long primo_tentativo_allocazione = millis();
        bool risultato = false;
        fotocamera = new OV7670(risultato,OV7670::Mode::QQQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
        /*
        OV7670* tmp;
        while (!risultato && millis() - primo_tentativo_allocazione <= SECONDI_ATTESA_CAMERA*1000){ // TODO sistema (non bloccante)
            Serial.print(".");
            tmp = new OV7670(risultato,OV7670::Mode::QQQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
            if (!risultato){
                Serial.print("Delete");
                // delete tmp;
            }
            delay(100);
        }
        if (!risultato){
            size = -1;
            Serial.println( "Errore allocazione camera. Controllare cavo alimentazione");
            // char* errore = "Errore allocazione camera";
            return (unsigned char* ) "Errore allocazione camera. Controllare cavo alimentazione";
        }

        Serial.print("O");
        fotocamera = tmp;
        Serial.println("K");
        */
        Serial.println("OK");

    }


    String foto = "";
    unsigned char* to_ret = NULL;

    /* // Libreria con Web sockets -> messa in cartella prova solo per non linkarla
    int blk_count = fotocamera->yres/I2SCamera::blockSlice;//30, 60, 120
    
    for (int i=0; i<blk_count; i++) {

        if (i == 0) {
            fotocamera->startBlock = 1;
            fotocamera->endBlock = I2SCamera::blockSlice;
            // webSocket.sendBIN(0, &start_flag, 1);
        }

        if (i == blk_count-1) {
            // webSocket.sendBIN(0, &end_flag, 1);
        }
    
        fotocamera->oneFrame();
        size_t frameSize = fotocamera->xres * I2SCamera::blockSlice * 2 ;

        size = frameSize;
        to_ret = fotocamera->frame;
        // return (fotocamera->frame);


        // for (int j=0; j< frameSize; j++)
        //     foto += (fotocamera->frame)[j];
        // printHexValues(fotocamera->frame, 100);
        // webSocket.sendBIN(0, camera->frame, camera->xres * I2SCamera::blockSlice * 2);
        fotocamera->startBlock += I2SCamera::blockSlice;
        fotocamera->endBlock   += I2SCamera::blockSlice;
    }*/

    // Libreria ESP32_I2S_Camera
    fotocamera->oneFrame();
    size = fotocamera->xres * fotocamera->yres * 2;
    Serial.println("Size: " + (String) size);
    // size = 9600;
    to_ret = fotocamera->frame;


    return to_ret;
    // return foto;
      
}



unsigned char* take_picture(size_t &size){
    return take_picture_with_camera(mia_camera, size);
}
