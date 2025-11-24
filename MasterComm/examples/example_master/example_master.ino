#include "MasterComm.h"

MasterComm master;

uint8_t robotPrueba[] = {0x5C,0xCF,0x7F,0xB5,0x64,0x10};

void setup() {
  Serial.begin(115200);

  master.addRobotMAC(robotPrueba);

  master.begin("OPPO A53", "611b10a883c5", 8888);

  master.enableBroadcastIP("255.255.255.255");

  Serial.println("MAESTRO listo");
}

void loop() {
  master.readUDP();
  if (master.dataChanged()){
    Serial.print("Ángulo: ");   Serial.println(master.getAngle());
    Serial.print("Distancia: ");Serial.println(master.getDistance());
    Serial.print("Out: ");      Serial.println(master.getOut());
  }
}
