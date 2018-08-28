#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <XLR8Servo.h>

class ServoSystemClass {

private:

  // SD communication
  File dataFile;
  int csPin = 10;

  // Servo profile
  int numServos;
  int positions[10];
  int defaultPos[10];
  Servo servos[10];
  int increment = 50;

  // Servo command stream
  char * cmd = malloc(sizeof(char) * 3); // servo command
  char * arg = malloc(sizeof(char) * 4); // value to be sent
  bool haveCmd = false;
  char func;
  int target;
  int val;

public:

  bool sdBegin();
  void readServoProfile();
  void printFiles();
  void sendServoProfile();
  void loadFile(int);

  void bleBegin();
  void monitorBle();
  int bleReady();
  void processBle();

  void servosBegin();
  void servosNeutral();
  void moveServo(int, int);

  void processCommands(Stream*);

};