#include "MasterComm.h"

MasterComm master;

// MAC del robot de prueba
uint8_t robotPrueba[] = {0x5C,0xCF,0x7F,0xB5,0x64,0x10};

// Variables para los datos a enviar
int targetID;
float ang, dist;
int out;
char msg[100];

void setup() {
    Serial.begin(115200);
    while (!Serial);  // Esperar Serial listo

    // Agregar robot
    master.addRobotMAC(robotPrueba);

    // Iniciar maestro
    master.begin("OPPO A53", "611b10a883c5", 8888);

    // Activar broadcast
    master.enableBroadcastIP("255.255.255.255");

    Serial.println("MAESTRO listo para enviar datos a los robots");
}

void loop() {
    Serial.println("\nIntroduce ID del robot:");
    while (Serial.available() == 0) delay(10);
    targetID = Serial.parseInt();
    Serial.readStringUntil('\n');

    Serial.println("Introduce ANG (ángulo):");
    while (Serial.available() == 0) delay(10);
    ang = Serial.parseFloat();
    Serial.readStringUntil('\n');

    Serial.println("Introduce DIST (distancia):");
    while (Serial.available() == 0) delay(10);
    dist = Serial.parseFloat();
    Serial.readStringUntil('\n');

    Serial.println("Introduce OUT (salida, entero):");
    while (Serial.available() == 0) delay(10);
    out = Serial.parseInt();
    Serial.readStringUntil('\n');

    // Construir mensaje en formato protocolario
    snprintf(msg, sizeof(msg), "id=%d, ang=%.2f, dist=%.2f, Out=%d", targetID, ang, dist, out);

    // Enviar al robot
    master.sendToRobot(targetID, msg);

    Serial.println("\nMensaje enviado:");
    Serial.println(msg);

    delay(50);
}
