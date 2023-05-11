/*
 * WebSocketClientSocketIOack.ino
 *
 *  Created on: 20.07.2019
 *
 */

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>

#include <WebSocketsClient.h>
#include <SocketIOclient.h>
// SocketIOclient


// WiFiClientSecure client;


WiFiMulti WiFiMulti;
SocketIOclient socketIO;

#define USE_SERIAL Serial

// const char* rootCACertificate =
//   "-----BEGIN CERTIFICATE-----\n"
//   "MIIDajCCAlICCQD7y6Uay6f2JzANBgkqhkiG9w0BAQsFADBqMQswCQYDVQQGEwJV\n"
//   "UzELMAkGA1UECAwCQ0ExEjAQBgNVBAcMCVN0YXRlIFN1YnN0b24xDTALBgNVBAoM\n"
//   "BENvbXBhbnkxHjAcBgNVBAMMFU15IFNlcnZlciBDb21wYW55IENBMB4XDTIxMDcx\n"
//   "NjAwMTA0MloXDTIyMDcxNTAwMTA0MlowajELMAkGA1UEBhMCVVMxCzAJBgNVBAgM\n"
//   "AkNBMRIwEAYDVQQHDAlTdGF0ZSBTdWJzdG9uMQ0wCwYDVQQKDARDb21wYW55MR4w\n"
//   "HAYDVQQDDBVNeSBTZXJ2ZXIgQ29tcGFueTCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"
//   "ADCCAQoCggEBANHuyRpT2fc7Db9UmyqJXsk+0TtQFfgzClyW8UzIh3vebe1izCCs\n"
//   "MORi24ZIDo6Ym3n9cvugUKPLHdAEx4LULuglTQZjFJhvWhM8jdyMg5T/h+/eR9ss\n"
//   "GgIzBrWuByp/zAmZ0nV8w6DShfOhuSp2/SmSd2zLNGg9gF4yDaFO8lb+1qDqjtD2\n"
//   "cd8u2aTbM6AlK1mHsDqJzj12o+zSpwVw3JqGFSzUJSpJiAyIyUDle2zqvpGrrci2\n"
//   "S1bI7r7bZRhFgUzhBd2l/VlOHv5dxS6FFMtv0mFGWtpduyXuY\n"
//   "-----END CERTIFICATE-----\n";


/////////////
//LED/////////
//////////////
const int ledPin = LED_BUILTIN;
int ledState = 0;



void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case sIOtype_DISCONNECT:
            //in ra khi ngắt kết nối server
            USE_SERIAL.printf("[IOc] Disconnected!\n");
            break;
        case sIOtype_CONNECT:
            //Kết nối tới server
            USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);

            // join default namespace (no auto join in Socket.IO V3)
            socketIO.send(sIOtype_CONNECT, "/");
            break;

        //Nhận dữ liệu từ server    
        case sIOtype_EVENT:
        {
            char * sptr = NULL;
            int id = strtol((char *)payload, &sptr, 10);
            USE_SERIAL.printf("[IOc] get event: %s id: %d\n", payload, id);
            if(id) {
                payload = (uint8_t *)sptr;
            }
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload, length);
            if(error) {
                USE_SERIAL.print(F("deserializeJson() failed: "));
                USE_SERIAL.println(error.c_str());
                return;
            }

            String eventName = doc[0];
            USE_SERIAL.printf("[IOc] event name: %s\n", eventName.c_str());
            String value = doc[1];
            USE_SERIAL.printf("[IOc] value: %s\n", value.c_str());

            //led
            if(value == "true"){
                ledState = 1;
            } 
            else {
                ledState = 0;
            }


            // Message Includes a ID for a ACK (callback)
            if(id) {
                // creat JSON message for Socket.IO (ack)
                DynamicJsonDocument docOut(1024);
                JsonArray array = docOut.to<JsonArray>();

                // add payload (parameters) for the ack (callback function)
                JsonObject param1 = array.createNestedObject();
                param1["now"] = millis();

                // JSON to String (serializion)
                String output;
                output += id;
                serializeJson(docOut, output);

                // Send event
                socketIO.send(sIOtype_ACK, output);
            }
        }
            break;
        case sIOtype_ACK:
            USE_SERIAL.printf("[IOc] get ack: %u\n", length);
            break;
        case sIOtype_ERROR:
            USE_SERIAL.printf("[IOc] get error: %u\n", length);
            break;
        case sIOtype_BINARY_EVENT:
            USE_SERIAL.printf("[IOc] get binary: %u\n", length);
            break;
        case sIOtype_BINARY_ACK:
            USE_SERIAL.printf("[IOc] get binary ack: %u\n", length);
            break;
    }
}



void setup() {
    //controll Led
    pinMode(ledPin, OUTPUT);

    // client.setCACert(rootCACertificate);


    //USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    //Serial.setDebugOutput(true);
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

      for(uint8_t t = 4; t > 0; t--) {
          USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
          USE_SERIAL.flush();
          delay(1000);
      }

    WiFiMulti.addAP("HT", "Thiet123");

    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    String ip = WiFi.localIP().toString();
    USE_SERIAL.printf("[SETUP] WiFi Connected %s\n", ip.c_str());

    // server address, port and URL: http://103.163.119.63:4200/
    //https://test-esp-server-1.onrender.com
    //216.24.57.253
    //http://216.24.57.253:443/
    //216.24.57.253:443
    //Connect to serveroutput
    //13.228.225.19
    //18.142.128.26
    //54.254.162.138
    // if (client.connect("test-esp-server-1.onrender.com", 443)) {
    //     socketIO.beginSSL(client, "test-esp-server-1.onrender");
    // }

    socketIO.begin("216.24.57.253", 443, "/socket.io/?EIO=4");

    // event handler
    socketIO.onEvent(socketIOEvent);

}

unsigned long messageTimestamp = 0;

void loop() {
  //loop để giữ kết nối
    socketIO.loop();

    //led
    digitalWrite(ledPin, ledState);


    //Gủi lời chào tới server
    uint64_t now = millis();

    if(now - messageTimestamp > 2000) {
        messageTimestamp = now;

        // creat JSON message for Socket.IO (event)
        DynamicJsonDocument doc(1024);
        JsonArray array = doc.to<JsonArray>();

        // add evnet name
        // Hint: socket.on('event_name', ....
        array.add("on-chat");
        array.add("Hello from ESP32");


        // JsonObject objectEvent = array.createNestedObject();
        // objectEvent["event_name"] = "Hello from ESP32";
        
        

        // JSON to String (serializion)
        String output;
        serializeJson(doc, output);

        // Send event
        socketIO.sendEVENT(output);

        // Print output to serial
        USE_SERIAL.println(output);
    }

}