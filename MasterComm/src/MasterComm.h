#ifndef MASTERCOMM_H
#define MASTERCOMM_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

class MasterComm {
    private:
        WiFiServer _server;
        WiFiClient _client;
        unsigned int _port;

        static const int _maxRobots = 2;
        uint8_t _robotMACs[_maxRobots][6];
        int _robotCount;

        float _angle;
        float _distance;
        bool _out;

        float _lastAngle;
        float _lastDistance;
        bool _lastOut;
        bool _firstData;

    public:
        MasterComm(unsigned int port);

        bool begin(const char *ssid, const char *password);

        void addRobotMAC(uint8_t mac[6]);

        void handleServer();
        void readTCP();

        void sendToRobot(int id, float ang, float dist, bool out);
        void processRobotResponse(const char *msg);

        float getAngle();
        float getDistance();
        bool getOut();

        bool dataChanged();
};

// Callback de ESP-NOW cuando se envían datos
void Master_OnDataSent(uint8_t *mac_addr, uint8_t sendStatus);

// Callback de ESP-NOW cuando se reciben datos
void Master_OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len);

// Puntero global para acceder a la instancia del maestro desde los callbacks
extern MasterComm *MasterGlobal;

#endif