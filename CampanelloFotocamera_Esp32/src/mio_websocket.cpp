#include <WebSockets.h>
#include <WebSocketsServer.h>
#include "pagina_web.h"

const int PORTA_WEB_SOCKET = 3001;

WebSocketsServer mio_webSocket(PORTA_WEB_SOCKET); // create a websocket server on port 81
WiFiServer server(PORTA_WEB_SOCKET - 1);
IPAddress ip_esp_glob;
int id_host_connesso;

void print_type_mess(WStype_t type){
     switch (type){
    case WStype_ERROR:
      Serial.println("WStype_ERROR");
      break;
    case WStype_DISCONNECTED:
        Serial.println("WStype_DISCONNECTED");
        break;
    case WStype_CONNECTED:
        Serial.println("WStype_CONNECTED");
        break;
    case WStype_TEXT:
        Serial.println("WStype_TEXT");
        break;
    case WStype_BIN:
        Serial.println("WStype_BIN");
        break;
    case WStype_FRAGMENT_TEXT_START:
        Serial.println("WStype_FRAGMENT_TEXT_START");
        break;
    case WStype_FRAGMENT_BIN_START:
        Serial.println("WStype_FRAGMENT_BIN_START");
        break;
    case WStype_FRAGMENT:
        Serial.println("WStype_FRAGMENT");
        break;
    case WStype_FRAGMENT_FIN:
        Serial.println("WStype_FRAGMENT_FIN");
        break;
    case WStype_PING:
        Serial.println("WStype_PING");
        break;
    case WStype_PONG:
        Serial.println("WStype_PONG");
        break;
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t payloadlength)
{
    Serial.println("---mess---");
    print_type_mess(type);
    String payload_str = "";

    switch(type){
        case WStype_DISCONNECTED:
            Serial.printf("[%d] Disconnesso\n", num);
            id_host_connesso = -1;
            break;
        case WStype_CONNECTED:
            Serial.printf("[%d] Connesso\n", num);
            break;
        case WStype_TEXT:
            for (int i=0; i<payloadlength; i++)
                payload_str += (char) payload[i];
            
            if (payload_str.equals("ready"))    
                id_host_connesso = num;
            break;
        

    }
}

int get_id_host_connesso(){
    return id_host_connesso;
}

void ws_setup(IPAddress ip_esp)
{
    ip_esp_glob = ip_esp;
    id_host_connesso = -1;

    // Web socket
    mio_webSocket.begin();                 // start the websocket server
    mio_webSocket.onEvent(webSocketEvent); // if there's an incomming websocket message, go to function 'webSocketEvent'
    Serial.println("WebSocket server started.");

    // Web server
    server.begin();
    Serial.println("Http web server started.");
}

void ws_loop()
{
    // Web socket
    mio_webSocket.loop();

    // Web server
    WiFiClient client = server.available();
    if (client)
    {
        // Serial.println("New Client.");
        String currentLine = "";
        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();
                // Serial.write(c);
                if (c == '\n')
                {
                    if (currentLine.length() == 0)
                    {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();
                        client.print(get_pagina_web(ip_esp_glob.toString(), PORTA_WEB_SOCKET));
                        client.println();
                        break;
                    }
                    else
                    {
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {
                    currentLine += c;
                }
            }
        }
        // close the connection:
        client.stop();
    }
}