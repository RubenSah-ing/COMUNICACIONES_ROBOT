// --- LIBRERIAS --- //
#include "SlaveComm.h"


// --- Puntero al objeto --- //
SlaveComm *globalSlave = nullptr;   


// --- CALBACKS ESP NOW --- //
// Callback de envio por ESP - NOW
void Slave_OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    if (sendStatus == 0) {
        Serial.println("ESPNOW enviado correctamente");
    }
    else {
        Serial.println("ESPNOW no enviado: error");
    }
}

// Callback de recepcion por ESP - NOW
void Slave_OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
    if (!globalSlave) {return;}
    // Apuntar al metodo de la clase para poder usar el callback
    Serial.printf("Paquete recibido ESP-NOW", len);
    globalSlave->processIncoming(incomingData, len);
}


// --- CONSTRUCTOR --- //
SlaveComm::SlaveComm() {}


// --- INICICAR WIFI Y ESP - NOW --- //
bool SlaveComm::begin(const char *ssid, const char *password) {
    globalSlave = this;       

    // Iniciar WIFI modo estacion y conectar a la red
    WiFi.mode(WIFI_STA);        
    WiFi.disconnect();
    WiFi.begin(ssid, password); 
    while (WiFi.status() != WL_CONNECTED){
        delay(100); 
        Serial.print(".");
    }
    // Devolver MAC
    Serial.print("MAC Esclavo: ");
    Serial.println(WiFi.macAddress());

    //Inicicar ESP - NOW
    if (esp_now_init() != 0) {
        Serial.print("Error iniciando ESPNOW");
        return false;  
    }
    Serial.println("ESPNOW iniciado");

    // Rol de esclavo y registro de callbacks
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);           
    esp_now_register_recv_cb(Slave_OnDataRecv);     
    esp_now_register_send_cb(Slave_OnDataSent);

    // Agregar maestro como peer
    if (_masterMACSet){
        esp_now_add_peer(_masterMACAddress, ESP_NOW_ROLE_CONTROLLER, 1, NULL, 0);
    }                                
    return true;                                         
}


// --- PROCESAR MENSAJE RECIBIDO POR CALLBACK--- //
void SlaveComm::processIncoming(const uint8_t *data, uint8_t len){
    // Esperar un paquete de 13 bytes: 4 (ID) + 4 (ángulo) + 4 (distancia) + 1 (out)
    if (len != 13) {
        Serial.printf("Paquete descartado, len=%d\n", len);
        return;
    }

    // Extraer datos del paquete
    uint32_t id;
    float ang, dist;
    bool out;
    memcpy(&id,   data,      4);
    memcpy(&ang,  data + 4,  4);
    memcpy(&dist, data + 8,  4);
    memcpy(&out,  data + 12, 1);
    //Serial.printf("RX BIN → ID:%u ANG:%.2f DIST:%.2f OUT:%d\n",id, ang, dist, out);

    _angle = ang;
    _distance = dist;
    _out = out;

    // Enviar confirmación de recepción al maestro
    sendOK();
}


// --- ENVIAR MENSAJE DE CONFIRMACIÓN AL MAESTRO --- //
void SlaveComm::sendOK() {
    // Construir y enviar mensaje de confirmación por ESP-NOW
    char msg[32];
    snprintf(msg, sizeof(msg), "OK id=%d", _id);
    esp_now_send(_masterMACAddress, (uint8_t *)msg, strlen(msg) + 1);
}


// --- VERIFICAR SI LOS DATOS HAN CAMBIADO --- //
bool SlaveComm::dataChanged() {
    if (_firstData) {
        _firstData = false;
        _lastAngle = _angle;
        _lastDistance = _distance;
        _lastOut = _out;
        return true; 
    }

    bool changed = false;

    if (_angle != _lastAngle || _distance != _lastDistance || _out != _lastOut) {
        changed = true;
    }

    _lastAngle = _angle;
    _lastDistance = _distance;
    _lastOut = _out;

    return changed;
}


// --- GETTERS Y SETTERS --- //
int SlaveComm::getID() {
    return _id;
}                  

float SlaveComm::getAngle() {
    return _angle;
}

float SlaveComm::getDistance() {
    return _distance;
}

bool SlaveComm::getOut() {
    return _out;
}

void SlaveComm::setID(int id) {
    _id = id;
}    

void SlaveComm::setMasterMACAddress(uint8_t mac[6]) {
    // Copiar la MAC del maestro
    memcpy(_masterMACAddress, mac, 6);
    // Indicar que la MAC del maestro ha sido configurada
    _masterMACSet = true;                
}