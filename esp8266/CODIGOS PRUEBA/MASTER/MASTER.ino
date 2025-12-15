#include <ESP8266WiFi.h>
#include <espnow.h>


const char *ssid = "OPPO A53";
const char *password = "611b10a883c5";
uint8_t my_id0 = 0;
uint8_t robot_id1[] = {0x5C,0xCF,0x7F,0x01,0x65,0x6B};
uint8_t robot_id2[] = {0x5C,0xCF,0x7F,0x01,0x65,0x6B};

int port = 8888;
WiFiServer server(port);

void Master_OnDataSent(uint8_t *mac_addr, uint8_t sendStatus){
  Serial.println("Enviado a esclavo");
}

void Master_OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
    char msg[200];
    memcpy(msg, incomingData, len);
    msg[len] = 0;
    Serial.print("Respuesta robot → ");
    Serial.println(msg);
}

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
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
}

void configServer(){
  server.begin();
  Serial.print("Open Server IP:");
  Serial.print(WiFi.localIP());
  Serial.print(" on port ");
  Serial.println(port);
}

void configESPNOW(){
  if (esp_now_init() != 0){
    Serial.println("No iniciado");
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(Master_OnDataSent);
  esp_now_register_recv_cb(Master_OnDataRecv);
  esp_now_add_peer(esclavo, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void setup() {
  Serial.begin(115200);
  while(!Serial){;}
  configWifi();
  configServer();
  configESPNOW();
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
        if (id != 0) {
          char msg[100];
          sprintf(msg, "ID:%u ANG:%.2f DIST:%.2f OUT:%d", id, ang, dist, out);
          esp_now_send(esclavo, (uint8_t*)msg, strlen(msg));
        }
        client.println("ESP received struct");
      }
    }

    client.stop();
    Serial.println("Client disconnected");
  }
}








