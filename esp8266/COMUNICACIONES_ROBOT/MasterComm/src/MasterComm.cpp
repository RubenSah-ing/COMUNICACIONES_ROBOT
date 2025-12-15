//.h
#include "MasterComm.h"


// Puntero global para acceder a la instancia del maestro
MasterComm *MasterGlobal = nullptr;


// Callback de ESP-NOW cuando se envían datos
void Master_OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    if (sendStatus == 0) {
        Serial.println("ESPNOW enviado correctamente");
    }
    else {
        Serial.println("ESPNOW no enviado: error");
    }
}


// Callback de ESP-NOW cuando se reciben datos
void Master_OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
    if (!MasterGlobal) {
        return;
    }

    char msg[200];
    if (len >= sizeof(msg)) len = sizeof(msg) - 1;
    memcpy(msg, incomingData, len);
    msg[len] = '\0';


    Serial.print("Respuesta robot -> ");    // Imprime mensaje recibido del esclavo
    Serial.println(msg);

    MasterGlobal->processRobotResponse(msg); // Procesa la respuesta
}


// Constructor
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

    
// Conectarse a red Wifi y configurar ESP NOW
bool MasterComm::begin(const char *ssid, const char *password) {
    MasterGlobal = this;

    //Configuracion Wifi
    WiFi.mode(WIFI_STA);                                // Modo estación
    WiFi.begin(ssid, password);                         // Conexión WiFi
    while (WiFi.status() != WL_CONNECTED){              // Espera conexión
        delay(100);
        Serial.print(".");
    }
    Serial.print("MAC Maestro: ");       
    Serial.println(WiFi.macAddress());      // Imprime MAC del maestro
    Serial.print("IP Maestro: ");       
    Serial.println(WiFi.localIP());         // Imprime IP del maestro
    delay(3000);
    //Configuracion Servidor TCP
    _server.begin();
    Serial.print("Servidor TCP iniciado. IP: ");
    Serial.print(WiFi.localIP());
    Serial.print(" , Puerto: ");
    Serial.println(this->_port);

    //Configuracion ESP NOW
    if (esp_now_init() != 0){
        return false;               // Inicializa ESP-NOW y verifica errores
    }

    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);     // Maestro como controlador
    esp_now_register_send_cb(Master_OnDataSent);        // Callback para envíos
    esp_now_register_recv_cb(Master_OnDataRecv);        // Callback para recepción

    for (int i = 0; i < _robotCount; i++) {             // Bucle para registrar todas las MAC de robots
        esp_now_add_peer(_robotMACs[i], ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
    }

    return true;
}


// Registrar MAC de robots esclavos
void MasterComm::addRobotMAC(uint8_t mac[6]) {
    // Control de tamaño de array
    if (_robotCount >= _maxRobots){
        Serial.println("Numero maximo de robots registrados");
        return; 
    }
    else {
        // Copiar MAC en memoria
        memcpy(_robotMACs[_robotCount], mac, 6);
        _robotCount++;
        Serial.print("Robot añadido correctamente. Total de robots: ");
        Serial.println(_robotCount);
    }
}


// Mantener servidor
void MasterComm::handleServer(){
    if (!_client || !_client.connected()) {
        _client = _server.available();

        if (_client){
            Serial.println("Cliente TCP conectado");
        }
        else{
            Serial.println("Cliente no conectado");
        }
    }
}


// Enviar al robot correspondiente por ID
void MasterComm::sendToRobot(int id, float ang, float dist, bool out) {
    int idx = id - 1;
    if (idx < 0 || idx >= _robotCount) return;

    uint8_t buf[13];
    memcpy(buf,      &id,  4);
    memcpy(buf + 4,  &ang, 4);
    memcpy(buf + 8,  &dist,4);
    memcpy(buf + 12, &out, 1);

    esp_now_send(_robotMACs[idx], buf, 13);
    Serial.printf("Enviado ID:%d", id);
}

// Getters
float MasterComm::getAngle() {
    return _angle;
}

float MasterComm::getDistance() {
    return _distance;
}

bool MasterComm::getOut() {
    return _out;
}


// Recibir respuestas de los robots y devolver por TCP
void MasterComm::processRobotResponse(const char *msg) {
    if (_client && _client.connected()) {
        _client.println(msg);
    }
}

// Leer mensajes TCP y reenviar
void MasterComm::readTCP() {
    if (!_client || !_client.connected()) return;

    const size_t PACKET_SIZE = 13;

    if (_client.available() < PACKET_SIZE) return;

    uint8_t buf[PACKET_SIZE];
    size_t readBytes = _client.read(buf, PACKET_SIZE);
    if (readBytes != PACKET_SIZE) return;

    uint32_t id;
    float ang, dist;
    bool out;

    memcpy(&id,   buf,      4);
    memcpy(&ang,  buf + 4,  4);
    memcpy(&dist, buf + 8,  4);
    memcpy(&out,  buf + 12, 1);

    if (id != 0) {
        Serial.printf(
            "Mensaje esclavo -> ID:%u ANG:%.2f DIST:%.2f OUT:%d\n",
            id, ang, dist, out
        );

        char msg[80];
        snprintf(msg, sizeof(msg),"ID:%u ANG:%.2f DIST:%.2f OUT:%d",id, ang, dist, out);
        sendToRobot(id, ang, dist, out);
    }
    else {
        Serial.printf(
            "Mensaje maestro -> ANG:%.2f DIST:%.2f OUT:%d\n",
            ang, dist, out
        );

        _angle = ang;
        _distance = dist;
        _out = out;

        processRobotResponse("OK id=0");
    }
}



bool MasterComm::dataChanged() {
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