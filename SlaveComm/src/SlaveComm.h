#ifndef SLAVECOMM_h
#define SLAVECOMM_h

// --- LIBRERIAS --- //
#include <Arduino.h>              // Biblioteca base de Arduino
#include <ESP8266WiFi.h>          // Manejo de WiFi en ESP8266
#include <espnow.h>               // Biblioteca para comunicación ESP-NOW


// --- CLASE --- //
class SlaveComm {
  private:
    int _id;                          // ID del dispositivo esclavo
    float _angle;                     // Valor recibido: ángulo
    float _distance;                  // Valor recibido: distancia
    bool _out;                        // Valor recibido: salida o estado

    uint8_t _masterMACAddress[6];     // Dirección MAC del maestro
    uint8_t _myMACAddress[6];         // Dirección MAC propia del esclavo
    bool _masterMACSet;               // Indicador de si la MAC del maestro ya fue configurada

    float _lastAngle = 0;
    float _lastDistance = 0;
    int _lastOut = 0;
    bool _firstData = true;

  public:
    SlaveComm();                                              // Constructor de la clase

    bool begin(const char *ssid, const char *password);       // Inicializa WiFi y ESP-NOW

    int getID();                                              // Obtener ID del esclavo
    float getAngle();                                         // Obtener ángulo recibido
    float getDistance();                                      // Obtener distancia recibida
    bool getOut();                                            // Obtener valor de salida
    void setID(int id);                                       // Establecer ID del esclavo
    void setMasterMACAddress(uint8_t mac[6]);                 // Configurar dirección MAC del maestro

    void sendOK();                                            // Enviar mensaje de confirmación al maestro

    void processIncoming(const uint8_t *data, uint8_t len);   // Procesar mensaje recibido

    bool dataChanged();                                       // Verificar si los datos han cambiado respecto  los recibidos anteriormente
};

// Callback de recepcion por ESP - NOW
void Slave_OnDataRecv(uint8_t *mac, uint8_t *incomingMsg, uint8_t len);

// Callback de envio por ESP - NOW
void Slave_OnDataSent(uint8_t *mac_addr, uint8_t sendStatus);

// Puntero global a la instancia del esclavo
extern SlaveComm *globalSlave;               

#endif