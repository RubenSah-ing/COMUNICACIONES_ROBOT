#ifndef MASTERCOMM_H
#define MASTERCOMM_H

// --- LIBRERIAS --- //
#include <Arduino.h>                // Biblioteca base de Arduino
#include <ESP8266WiFi.h>            // Manejo de WiFi en ESP8266
#include <espnow.h>                 // Biblioteca para comunicación ESP-NOW

// --- CLASE --- //
class MasterComm {
    private:
        WiFiServer _server;                     // Servidor TCP
        WiFiClient _client;                     // Cliente TCP
        unsigned int _port;                     // Puerto del servidor TCP

        static const int _maxRobots = 2;        // Máximo número de robots esclavos
        uint8_t _robotMACs[_maxRobots][6];      // Array con MACs de robots
        int _robotCount;                        // Contador de robots

        float _angle;                           // Valor recibido: ángulo
        float _distance;                        // Valor recibido: distancia
        bool _out;                              // Valor recibido: salida o estado

        float _lastAngle;
        float _lastDistance;
        bool _lastOut;
        bool _firstData;

    public:
        MasterComm(unsigned int port);                              // Constructor de la clase

        bool begin(const char *ssid, const char *password);         // Inicializa WiFi, ESP-NOW y servidor TCP

        void addRobotMAC(uint8_t mac[6]);                           // Añadir MAC de robot esclavo

        void handleServer();                                        // Mantener servidor TCP    
        void readTCP();

        void sendToRobot(int id, float ang, float dist, bool out);  // Enviar datos a robot por ID
        void processRobotResponse(const char *msg);                 // Procesar respuesta de robot

        float getAngle();                                           // Obtener ángulo recibido
        float getDistance();                                        // Obtener distancia recibida
        bool getOut();                                              // Obtener valor de salida           

        bool dataChanged();                                       // Comprobar si los datos han cambiado respecto a los recibidos anteriormente                                       
};

// Callback de ESP-NOW cuando se envían datos
void Master_OnDataSent(uint8_t *mac_addr, uint8_t sendStatus);

// Callback de ESP-NOW cuando se reciben datos
void Master_OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len);

// Puntero global para acceder a la instancia del maestro desde los callbacks
extern MasterComm *MasterGlobal;

#endif