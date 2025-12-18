// --- LIBRERIAS --- //
#include "MasterComm.h"


// --- Puntero al objeto --- //
MasterComm *MasterGlobal = nullptr;


// --- CALBACKS ESP NOW --- //
// Callback de envio por ESP - NOW
void Master_OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    if (sendStatus == 0) {
        Serial.println("ESPNOW enviado correctamente");
    }
    else {
        Serial.println("ESPNOW no enviado: error");
    }
}

// Callback de recepcion por ESP - NOW
void Master_OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
    if (!MasterGlobal) {
        return;
    }
    // Copiar datos recibidos a un buffer temporal
    char msg[200];
    if (len >= sizeof(msg)) len = sizeof(msg) - 1;
    memcpy(msg, incomingData, len);
    msg[len] = '\0';
    Serial.print("Respuesta robot -> ");

    // Apuntar al metodo de la clase para poder usar el callback
    MasterGlobal->processRobotResponse(msg);
}


// --- CONSTRUCTOR --- //
// Inicializa variables
MasterComm::MasterComm(unsigned int port = 8888)
    : _server(port),
      _port(port),
      _robotCount(0),
      _angle(0),
      _distance(0),
      _out(false),
      _lastAngle(0),
      _lastDistance(0),
      _lastOut(false),
      _firstData(true)
{}

    
// --- INICICAR WIFI Y ESP - NOW --- //
bool MasterComm::begin(const char *ssid, const char *password) {
    MasterGlobal = this;

    // Iniciar WIFI modo estacion y conectar a la red
    WiFi.mode(WIFI_STA);     
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED){
        delay(100);
        Serial.print(".");
    }
    // Devolver MAC e IP
    Serial.print("MAC Maestro: ");       
    Serial.println(WiFi.macAddress());
    Serial.print("IP Maestro: ");       
    Serial.println(WiFi.localIP());
  
    //Configuracion Servidor TCP
    _server.begin();
    Serial.print("Servidor TCP iniciado. IP: ");
    Serial.print(WiFi.localIP());
    Serial.print(" , Puerto: ");
    Serial.println(this->_port);

    //Configuracion ESP NOW
    if (esp_now_init() != 0){
        return false;
    }

    // Rol de maestro y registro callbacks
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(Master_OnDataSent);
    esp_now_register_recv_cb(Master_OnDataRecv);

    // Añadir peers (robots esclavos)
    for (int i = 0; i < _robotCount; i++) {
        esp_now_add_peer(_robotMACs[i], ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
    }
    return true;
}


// --- AÑADIR MAC ROBOT ESCLAVO --- //
void MasterComm::addRobotMAC(uint8_t mac[6]) {
    // Comprobar si hay espacio para más robots
    if (_robotCount >= _maxRobots){
        Serial.println("Numero maximo de robots registrados");
        return; 
    }
    else {
        // Copiar MAC en memoria 
        memcpy(_robotMACs[_robotCount], mac, 6);
        // Incrementar contador de robots
        _robotCount++;
        Serial.print("Robot añadido correctamente. Total de robots: ");
        Serial.println(_robotCount);
    }
}


// --- MANTENER SERVIDOR TCP --- //
void MasterComm::handleServer(){
    // Comprobar si hay un cliente conectado
    if (!_client || !_client.connected()) {
        _client = _server.available();
        // Si hay un cliente nuevo, mostrar mensaje
        if (_client){
            Serial.println("Cliente TCP conectado");
        }
        // Si no hay cliente, mostrar mensaje
        else{
            Serial.println("Cliente no conectado");
        }
    }
}


// --- ENVIAR DATOS A ROBOT ESCLAVO POR ID --- //
void MasterComm::sendToRobot(int id, float ang, float dist, bool out) {
    // Comprobar ID valido
    int idx = id - 1;
    if (idx < 0 || idx >= _robotCount) {
        return;
    }

    // Preparar y enviar datos por ESP-NOW
    uint8_t buf[13];
    memcpy(buf,      &id,  4);
    memcpy(buf + 4,  &ang, 4);
    memcpy(buf + 8,  &dist,4);
    memcpy(buf + 12, &out, 1);

    // Enviar datos
    esp_now_send(_robotMACs[idx], buf, 13);
    Serial.printf("Enviado ID:%d", id);
}

// --- GETTERS Y SETTERS --- //
float MasterComm::getAngle() {
    return _angle;
}

float MasterComm::getDistance() {
    return _distance;
}

bool MasterComm::getOut() {
    return _out;
}


// --- PROCESAR RESPUESTA ROBOT ESCLAVO --- //
void MasterComm::processRobotResponse(const char *msg) {
    // Enviar respuesta al cliente TCP si está conectado
    if (_client && _client.connected()) {
        _client.println(msg);
    }
}

// --- LEER DATOS TCP --- //
void MasterComm::readTCP() {
    // Comprobar cliente conectado
    if (!_client || !_client.connected()) {
        return;
    }

    // Comprobar si hay datos disponibles
    const size_t PACKET_SIZE = 13;
    if (_client.available() < PACKET_SIZE){
        return;
    }

    // Leer datos del cliente TCP
    uint8_t buf[PACKET_SIZE];
    size_t readBytes = _client.read(buf, PACKET_SIZE);
    if (readBytes != PACKET_SIZE){
        return;
    }

    // Extraer datos del buffer
    uint32_t id;
    float ang, dist;
    bool out;
    memcpy(&id,   buf,      4);
    memcpy(&ang,  buf + 4,  4);
    memcpy(&dist, buf + 8,  4);
    memcpy(&out,  buf + 12, 1);

    // Si el ID es distinto de 0, enviar a robot esclavo
    if (id != 0) {
        Serial.printf("Mensaje esclavo -> ID:%u ANG:%.2f DIST:%.2f OUT:%d\n",id, ang, dist, out);
        // Enviar datos al robot por ID
        char msg[80];
        snprintf(msg, sizeof(msg),"ID:%u ANG:%.2f DIST:%.2f OUT:%d",id, ang, dist, out);
        sendToRobot(id, ang, dist, out);
    }
    // Si el ID es 0, actualizar datos del maestro
    else {
        Serial.printf("Mensaje maestro -> ANG:%.2f DIST:%.2f OUT:%d\n",ang, dist, out);
        _angle = ang;
        _distance = dist;
        _out = out;
        // Simular respuesta OK al cliente TCP
        processRobotResponse("OK id=0");
    }
}


// --- COMPROBAR SI LOS DATOS HAN CAMBIADO --- //
bool MasterComm::dataChanged() {
    if (_firstData) {
        _firstData = false;
        _lastAngle = _angle;
        _lastDistance = _distance;
        _lastOut = _out;
        return true;
    }

    bool changed = false;

    if (_angle != _lastAngle ||_distance != _lastDistance ||_out != _lastOut) {
        changed = true;
    }

    _lastAngle = _angle;
    _lastDistance = _distance;
    _lastOut = _out;

    return changed;
}