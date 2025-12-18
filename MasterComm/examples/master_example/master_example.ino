#include <Arduino.h>
#include "MasterComm.h"

// ===== CONFIGURACIÓN WIFI =====
const char* WIFI_SSID = "OPPO A53";
const char* WIFI_PASS = "611b10a883c5";

// Puerto TCP del servidor
const uint16_t TCP_PORT = 8888;

// MACs de robots esclavos
uint8_t robot1MAC[6] = {0x5C,0xCF,0x7F,0xB5,0x64,0x10};
uint8_t robot2MAC[6] = {0xCC,0x50,0xE3,0x55,0x09,0x6A};

// Instancia del maestro
MasterComm master(TCP_PORT);

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== MASTER ESP8266 INICIANDO ===");

    // Iniciar maestro
    if (!master.begin(WIFI_SSID, WIFI_PASS)) {
        Serial.println("Error iniciando MasterComm");
        while (true);
    }

    // Registrar robots
    master.addRobotMAC(robot1MAC);
    master.addRobotMAC(robot2MAC);

    Serial.println("Maestro listo");
}

void loop() {
    // Mantener conexión TCP
    master.handleServer();

    // Leer comandos TCP (13 bytes)
    master.readTCP();

    // Ejemplo: detectar cambios recibidos
    if (master.dataChanged()) {
        Serial.println("Datos del maestro cambiaron:");
        Serial.printf(" ANG: %.2f\n", master.getAngle());
        Serial.printf(" DIST: %.2f\n", master.getDistance());
        Serial.printf(" OUT: %d\n", master.getOut());
    }

    delay(5);
}
