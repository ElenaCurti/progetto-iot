// #include <Arduino.h>
// #include "OV7670.h"

unsigned char* take_picture(size_t &);
// unsigned char* take_picture_with_camera(OV7670*, size_t &) ;
void printHexValues(const unsigned char *frame, size_t size);
// int* convertToDecimal(unsigned char* foto, size_t size) ;
// char* convertToHexString(unsigned char* array, size_t size) ;
String convert_to_mqtt_string(unsigned char* array, size_t size);
void convertToHexString(unsigned char* array, size_t size, char* hexArray) ;