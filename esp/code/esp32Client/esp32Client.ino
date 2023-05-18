// #include <stdio.h>
#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>

#include <WebSocketsClient.h>
#include <SocketIOclient.h>

WiFiMulti WiFiMulti;
SocketIOclient socketIO;

#define USE_SERIAL Serial

//Điều khiển led
const int ledPin = LED_BUILTIN;

String globalValue;

bool receivedValueFromServer = false;


void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case sIOtype_DISCONNECT:
            //in ra khi ngắt kết nối server
            USE_SERIAL.printf("[IOc] Disconnected!\n");
            break;
        case sIOtype_CONNECT:
            //Kết nối tới server
            USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);

            socketIO.send(sIOtype_CONNECT, "/");
            break;

        //Nhận dữ liệu từ server    
        case sIOtype_EVENT:
        {
            USE_SERIAL.printf("Recive from server");
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

            //Lấy giá trị của button
            globalValue = value;

            //Kiểm tra có chưa ID ACK từ server hay không, nếu có thì phản hổi về server
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

bool getValueLed(String value){
  if(value == "true")
    {return 1;}
  else {
    return 0;
  }
}


void setup() {
    //controll Led  
    pinMode(ledPin, OUTPUT);

    USE_SERIAL.begin(115200);

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

    //Kết nối tới Wifi
    String ip = WiFi.localIP().toString();
    USE_SERIAL.printf("[SETUP] WiFi Connected %s\n", ip.c_str());

    // server address, port and URL:
    //103.163.119.124:4200
    //192.168.43.160 local

    //Kết nối tới server
    socketIO.begin("103.163.119.124", 4200, "/socket.io/?EIO=4");

    // Xử lý sự kiện 
    socketIO.onEvent(socketIOEvent);
}

unsigned long messageTimestamp = 0;

bool ledState = 0;
void loop() {
  //loop để giữ kết nối
    socketIO.loop();

    ledState = getValueLed(globalValue);
    //led
    digitalWrite(ledPin, ledState);


    // Gủi lời chào tới server mỗi 5s/1lần
    uint64_t now = millis();

    if(now - messageTimestamp > 5000) {

        USE_SERIAL.println(temp_ledState);
        USE_SERIAL.println(ledState);
        messageTimestamp = now;

        //Tạo gói tin Json cho Socket.io
        DynamicJsonDocument doc(1024);
        JsonArray array = doc.to<JsonArray>();

        //Thêm kênh
        array.add("user-chat");
        //Thêm nội dung gửi tới kênh
        array.add("Hello from ESP32");

        
        String output;
        //chuyển đổi đối tượng JSON thành chuỗi JSON - output 
        serializeJson(doc, output);

        // Gửi gói tin
        socketIO.sendEVENT(output);

        USE_SERIAL.println(output);
    }
}