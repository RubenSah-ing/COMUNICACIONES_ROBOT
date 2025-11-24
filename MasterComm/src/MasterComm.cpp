#include "MasterComm.h"

MasterComm *MasterGlobal = nullptr;       // Puntero global para acceder a la instancia del maestro


// CALLBACK: Envío
void Master_OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    Serial.println(sendStatus == 0 ? "ESPNOW enviado" : "Error envío");  
    // Muestra si el envío por ESP-NOW fue exitoso
}


// CALLBACK: Recepción ESPNOW
void Master_OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
    if (!MasterGlobal) return;             // Si no existe instancia, no procesa

    char msg[200];                         // Buffer para almacenar mensaje
    memcpy(msg, incomingData, len);        // Copia los datos recibidos
    msg[len] = 0;                          // Termina la cadena

    Serial.print("Respuesta robot → ");    // Imprime mensaje recibido del esclavo
    Serial.println(msg);

    MasterGlobal->processRobotResponse(msg); // Procesa la respuesta mediante la clase
}


// CONSTRUCTOR
MasterComm::MasterComm() {
    _robotCount = 0;                     // No hay robots registrados aún
    _udpPort = 0;                        // Puerto por defecto
    _broadcastIP = "255.255.255.255";    // IP broadcast por defecto
}


// INICIO
bool MasterComm::begin(const char *ssid, const char *password, unsigned int udpPort) {

    MasterGlobal = this;                 // Guarda la instancia en la variable global

    WiFi.mode(WIFI_STA);                 // Modo estación
    WiFi.begin(ssid, password);          // Conexión WiFi
    while (WiFi.status() != WL_CONNECTED) delay(100); // Espera conexión

    Serial.print("MAC Maestro: ");       // Imprime MAC del maestro
    Serial.println(WiFi.macAddress());

    _udpPort = udpPort;                  // Guarda puerto UDP
    _udp.begin(_udpPort);                // Inicia servidor UDP

    if (esp_now_init() != 0) return false; // Inicializa ESP-NOW y verifica errores

    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);     // Maestro como controlador
    esp_now_register_send_cb(Master_OnDataSent);        // Callback para envíos
    esp_now_register_recv_cb(Master_OnDataRecv);        // Callback para recepción

    for (int i = 0; i < _robotCount; i++) {             // Registra todas las MAC de robots
        esp_now_add_peer(_robotMACs[i], ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
    }

    return true;                                        // Inicialización correcta
}


// Registrar MAC de robots esclavos
void MasterComm::addRobotMAC(uint8_t mac[6]) {
    if (_robotCount >= MAX_ROBOTS) return; // Evita desbordar la lista
    memcpy(_robotMACs[_robotCount], mac, 6); // Copia la MAC
    _robotCount++;                           // Incrementa número de robots (una vez)
    Serial.print("numero robots: ");
    Serial.println(_robotCount);
}


// Enviar al robot correspondiente por ID
void MasterComm::sendToRobot(int id, const char *msg) {
    int idx = id - 1; // IDs de protocolo empiezan en 1; índices internos en 0
    if (idx < 0 || idx >= _robotCount) {
        Serial.printf("ID robot inválido: %d\n", id);
        return;
    }
    esp_now_send(_robotMACs[idx], (uint8_t*)msg, strlen(msg)); // Envía mensaje por ESP-NOW
    Serial.printf("Enviado al robot %d (idx %d)\n", id, idx);
}


// Procesar respuestas de los robots
void MasterComm::processRobotResponse(const char *msg) {
    _udp.beginPacket(_broadcastIP.c_str(), _udpPort); // Prepara envío UDP
    _udp.print(msg);                                  // Escribe mensaje
    _udp.endPacket();                                 // Envía paquete
}


// Leer mensajes UDP y mandarlos al robot
void MasterComm::readUDP() {
    char buffer[256];                                  // Buffer para datagramas UDP

    int packetSize = _udp.parsePacket();               // Verifica si llegó mensaje
    if (!packetSize) return;

    int len = _udp.read(buffer, sizeof(buffer) - 1);   // Lee contenido
    if (len > 0) buffer[len] = '\0';                   // Termina cadena

    int targetID;
    if (sscanf(buffer, "id=%d", &targetID) == 1) {      // Extrae ID del mensaje
        Serial.printf("id: %d\n", targetID);
        if (targetID != 0) {
            sendToRobot(targetID, buffer);                  // Envía orden al robot
            Serial.printf("Enviado por ESPNOW al robot %d\n", targetID);
        } else {
            Serial.print("UDP recibido para maestro: ");
            Serial.println(buffer);
            float ang, dist;
            int out;

            if (sscanf(buffer, "id=0, ang=%f, dist=%f, Out=%d", &ang, &dist, &out) == 3) {
                _angle = ang;
                _distance = dist;
                _out = out;
                Serial.printf("Datos maestro → ang: %.2f, dist: %.2f, out: %d\n",
                       _angle, _distance, _out);
            }

            processRobotResponse("OK id=0");
            Serial.println("OK MAESTRO");
        }
    }
}

bool MasterComm::dataChanged() {
    if (_firstData) {
        _firstData = false;
        _lastAngle = _angle;
        _lastDistance = _distance;
        _lastOut = _out;
        return true;     // Primer dato siempre se considera "cambiado"
    }

    bool changed = false;

    if (_angle != _lastAngle ||
        _distance != _lastDistance ||
        _out != _lastOut) 
    {
        changed = true;
    }

    // Actualiza los valores anteriores
    _lastAngle = _angle;
    _lastDistance = _distance;
    _lastOut = _out;

    return changed;
}
