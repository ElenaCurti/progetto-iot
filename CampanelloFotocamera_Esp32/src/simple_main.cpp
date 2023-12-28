/*#include <Wifi.h>
#include "BluetoothSerial.h" 
#include <ESP32Ping.h>



BluetoothSerial SerialBT;


int bluedata; // variable for storing bluetooth data 


const char* ssid = "Wifi Ele";
const char* pass = "ElenaCurti00";


void setup() 
{
  Serial.begin(115200);
  
  btStart();  Serial.println("Bluetooth On");
  
  SerialBT.begin("ESP32_Bluetooth"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  delay(10000);

 
  Serial.println("Connecting to Internet");
  delay(2000);

  WiFi.begin(ssid, pass); Serial.println("WiFi On");

  
  delay(2000);

}

void Bluetooth_handle();


void loop() 
{

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Not Connected");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    WiFi.setAutoReconnect(true);    // to reconnect to Wi-Fi after a connection is lost
    WiFi.persistent(true);       


    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("Connecting to WiFi...\t");
        Serial.print("Ping 192.168.1.23: " );
        Serial.print(Ping.ping(IPAddress(192,168,1,23), 1));
        Serial.print("\tPing 8.8.8.8: " );
        Serial.println(Ping.ping(IPAddress(8,8,8,8), 1));

        delay(1000);

        // Serial.println(WiFi.localIP());

    }

    Serial.print("Connected to WiFi. IP: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    // Serial.println(" Connected");
    // Blynk.run();
  }

  if (SerialBT.available())
  {

    Bluetooth_handle();

  }

}

void Bluetooth_handle()
{
//   char bluedata;
//   bluedata = SerialBT.parseInt();
//   Serial.println("VAL:");
//   Serial.println(bluedata);
  delay(20);
    if (SerialBT.available()) {
        String receivedData = SerialBT.readString();
        Serial.print("Received from PC: ");
        Serial.println(receivedData);

        String sendData = "mio messaggio";
        SerialBT.print("Sent from ESP32: ");
        SerialBT.println(sendData);
    }

} */