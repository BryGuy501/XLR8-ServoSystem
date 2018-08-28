#include <ServoSystem.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(14, 15);

ServoSystemClass arm;

void setup() {

  Serial.begin(57600); // debugging
  while (!Serial) {
    ; // wait
  }

  Serial.println("Initializing SD...");
  if (!arm.sdBegin()) {
    Serial.println(" failed!");
  }
  Serial.println(" card initialized.");

  Serial.println("Starting BLE connection...");
  arm.bleBegin();

  Serial.println("Starting Servos");
  arm.servosBegin();

  Serial.println("Ready");
}

void loop() {
  /*if (mySerial.available()) {
    arm.processCommands(&mySerial);
  }*/
  if (arm.bleReady()) {
    arm.processBle();
  }
}

