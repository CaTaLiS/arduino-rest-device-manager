#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>

#define BUZZER_PIN 7
#define LIGHT_PIN 8

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 0, 0, 50);
IPAddress subnet(255, 255, 255, 128);
IPAddress dnServer(87, 204, 204, 204);
IPAddress gateway(10, 0, 0, 1);

EthernetServer server(80);

void initPins() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LIGHT_PIN, LOW);
}

void initSerial() {
  Serial.begin(9600);
  while (!Serial) {}
}

void initServer() {
  Ethernet.begin(mac, ip, dnServer, gateway, subnet);
  server.begin();
  Serial.print("Server started at ");
  Serial.println(Ethernet.localIP());
}

void handleClientRequest() {
  EthernetClient client = server.available();
  if (client) {
    Serial.println("Incoming request");
    String request = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
      } else {
        StaticJsonBuffer<200> jsonBuffer;
        String jsonFromRequest = request.substring('{');
        Serial.println(jsonFromRequest);
        JsonObject& root = jsonBuffer.parseObject(jsonFromRequest);

        JsonObject& answerRoot = jsonBuffer.createObject();
        String jsonResponse = "";
        if (!root.success()) {
          answerRoot["status"] = "INCORRECT CONTENT";
        } else {
          const char* service = root["serviceName"];
          Serial.println(service);
          if (strcmp(service, "buzzer") == 0 || strcmp(service, "light") == 0) {
            answerRoot["service"] =  root["serviceName"];
            if (strcmp(service, "buzzer") == 0) {
                const char* buzzerState = root["state"];
                if (strcmp(buzzerState, "ON") == 0) {
                  digitalWrite(7, HIGH);
                  answerRoot["status"] =  "OK";
                } else if (strcmp(buzzerState, "OFF") == 0) {
                  digitalWrite(7, LOW);
                  answerRoot["status"] =  "OK";
                } else {
                  answerRoot["status"] =  "UNKNOWN COMMAND";
                }
            }
          } else {
            answerRoot["status"] = "SERVICE FOT FOUND";
          }
        }

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.println();
        answerRoot.prettyPrintTo(client);
        break;
      }
    }
    delay(1);
    client.stop();
    Serial.println("Client disconnected");
  }
}

void setup() {
  initSerial();
  initServer();
  initPins();
}

void loop() {
  handleClientRequest();
}
