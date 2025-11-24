#ifndef MASTERCOMM_H
#define MASTERCOMM_H

#include <Arduino.h>        // Funciones básicas de Arduino
#include <ESP8266WiFi.h>    // Manejo de WiFi en ESP8266
#include <espnow.h>         // Biblioteca para comunicación ESP-NOW
#include <WiFiUdp.h>        // Biblioteca para comunicación UDP

class MasterComm {
public:
    MasterComm();           // Constructor

    bool begin(const char *ssid, const char *password, unsigned int udpPort);
    // Inicializa WiFi, ESP-NOW y el puerto UDP

    void addRobotMAC(uint8_t mac[6]);   // Agrega una MAC de robot (esclavo)
    void enableBroadcastIP(const char *ip) { _broadcastIP = ip; }
    // Guarda la IP de broadcast para enviar mensajes UDP

    void readUDP();        // Lee mensajes entrantes por UDP
    void sendToRobot(int id, const char *msg);  // Envía mensaje a un robot por ESP-NOW
    void processRobotResponse(const char *msg); // Procesa respuesta recibida de un robot

    float getAngle() { return _angle; }
    float getDistance() { return _distance; }
    int getOut() { return _out; }

    bool dataChanged();
    

private:
    WiFiUDP _udp;                // Objeto UDP para enviar/recibir datagramas
    unsigned int _udpPort;       // Puerto UDP en uso
    String _broadcastIP;         // IP de broadcast configurada

    static const int MAX_ROBOTS = 10;          // Máximo de robots permitidos
    uint8_t _robotMACs[MAX_ROBOTS][6];         // Lista de direcciones MAC de robots
    int _robotCount;                           // Cantidad actual de robots registrados
    
    int _id;                      // ID del dispositivo esclavo
    float _angle;                 // Valor recibido: ángulo
    float _distance;              // Valor recibido: distancia
    int _out;                     // Valor recibido: salida o estado

    float _lastAngle = 0;
    float _lastDistance = 0;
    int _lastOut = 0;
    bool _firstData = true;


};

void Master_OnDataSent(uint8_t *mac_addr, uint8_t sendStatus);
//// Callback de ESP-NOW cuando se envían datos

void Master_OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len);
//// Callback de ESP-NOW cuando se reciben datos

extern MasterComm *MasterGlobal;
//// Puntero global para acceder a la instancia del maestro desde los callbacks

#endif
