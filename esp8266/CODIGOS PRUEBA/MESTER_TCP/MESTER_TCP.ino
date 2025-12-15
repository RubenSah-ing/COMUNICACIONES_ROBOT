#include <ESP8266WiFi.h>

const char *ssid = "OPPO A53";
const char *password = "611b10a883c5"; 

int port = 8888;
WiFiServer server(port);

void configWifi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {   
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  
  
}

void configServer(){
  server.begin();
  Serial.print("Open Server IP:");
  Serial.print(WiFi.localIP());
  Serial.print(" on port ");
  Serial.println(port);
}

void setup() {
  Serial.begin(115200);
  while(!Serial){;}
  configWifi();
  configServer();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected");

    while (client.connected()) {
      if (client.available() >= 13) {
        uint8_t buf[13];
        client.read(buf, 13);

        uint32_t id = *(uint32_t*)(buf);
        float ang = *(float*)(buf + 4);
        float dist = *(float*)(buf + 8);
        bool out = *(bool*)(buf + 12);

        Serial.printf("ID: %u, ANG: %.2f, DIST: %.2f, OUT: %d\n", id, ang, dist, out);

        client.println("ESP received struct");
      }
    }

    client.stop();
    Serial.println("Client disconnected");
  }
}