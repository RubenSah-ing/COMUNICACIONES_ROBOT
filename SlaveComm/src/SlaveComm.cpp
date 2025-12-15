#include "SlaveComm.h"

SlaveComm *globalSlave = nullptr;   


void Slave_OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    if (sendStatus == 0) {
        Serial.println("ESPNOW enviado correctamente");
    }
    else {
        Serial.println("ESPNOW no enviado: error");
    }
}


// Callback de ESP-NOW cuando se reciben datos
void Slave_OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
    if (!globalSlave) return;

    Serial.printf("Paquete recibido ESP-NOW", len);

    globalSlave->processIncoming(incomingData, len);
}


// CONSTRUCTOR
SlaveComm::SlaveComm() {
}


// INICIALIZACIÓN
bool SlaveComm::begin(const char *ssid, const char *password) {

  globalSlave = this;       

  WiFi.mode(WIFI_STA);        
  WiFi.disconnect();
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED){
    delay(100); 
    Serial.println(".");
  }

  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != 0) {
    Serial.print("Error iniciando ESPNOW");
    return false;  
  }
  Serial.println("ESPNOW iniciado");

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);           
  esp_now_register_recv_cb(Slave_OnDataRecv);     

  if (_masterMACSet)                                  
    esp_now_add_peer(_masterMACAddress, ESP_NOW_ROLE_CONTROLLER, 1, NULL, 0);

  return true;                                         
}


// GUARDAR MAC DEL MAESTRO
void SlaveComm::setMasterMACAddress(uint8_t mac[6]) {
  memcpy(_masterMACAddress, mac, 6);
  _masterMACSet = true;                
}


// PROCESAR MENSAJE RECIBIDO
void SlaveComm::processIncoming(const uint8_t *data, uint8_t len){
    if (len != 13) {
        Serial.printf("Paquete descartado, len=%d\n", len);
        return;
    }

    uint32_t id;
    float ang, dist;
    bool out;

    memcpy(&id,   data,      4);
    memcpy(&ang,  data + 4,  4);
    memcpy(&dist, data + 8,  4);
    memcpy(&out,  data + 12, 1);

    Serial.printf(
        "RX BIN → ID:%u ANG:%.2f DIST:%.2f OUT:%d\n",
        id, ang, dist, out
    );

    _angle = ang;
    _distance = dist;
    _out = out;

    sendOK();
}


void SlaveComm::sendOK() {
    char msg[32];
    snprintf(msg, sizeof(msg), "OK id=%d", _id);
    esp_now_send(_masterMACAddress, (uint8_t *)msg, strlen(msg) + 1);
}



bool SlaveComm::dataChanged() {
    if (_firstData) {
        _firstData = false;
        _lastAngle = _angle;
        _lastDistance = _distance;
        _lastOut = _out;
        return true; 
    }

    bool changed = false;

    if (_angle != _lastAngle ||
        _distance != _lastDistance ||
        _out != _lastOut) 
    {
        changed = true;
    }

    _lastAngle = _angle;
    _lastDistance = _distance;
    _lastOut = _out;

    return changed;
}
