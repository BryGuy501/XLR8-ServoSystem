#include "ServoSystem.h"

SoftwareSerial bleSerial(14, 15);

bool ServoSystemClass::sdBegin() {
  if (!SD.begin(csPin)) {
    return false;
  }
  readServoProfile();
  return true;
}

void ServoSystemClass::bleBegin() {
  bleSerial.begin(9600);
  //bleSerial.println("Online");
  bleSerial.flush();
}

void ServoSystemClass::monitorBle() {
  while (true) {
    if (bleSerial.available()) {
      Serial.println("BLE");
      processCommands(&bleSerial);
    }
  }
}

int ServoSystemClass::bleReady() {
  return bleSerial.available();
}

void ServoSystemClass::processBle() {
  processCommands(&bleSerial);
}

void ServoSystemClass::readServoProfile() {
  dataFile = SD.open("/servos.dat");
  char * entryName = malloc(sizeof(char) * 20);
  char * defTxt = malloc(sizeof(char) * 3);
  char c;
  int def;
  bool haveNumServos = false;
  bool haveName = false;
  int servoNum = 0;
  // look at the file for a name
  if (dataFile.size() > 0) {
    while (dataFile.available()) {
      c = dataFile.read();
      if (c == '#') {
        sprintf(entryName, "");
        sprintf(defTxt, "");
        while (c != '\r') {
          c = dataFile.read();
          if ((strlen(entryName) > 0) || (c != ' ')) {
            sprintf(entryName, "%s%c", entryName, c);
          }
        }
      }
      else if (c == '\n') {
        if (!haveNumServos) {
          numServos = atoi(entryName);
          if (numServos > 0) {
            Serial.print("NumServos: ");
            Serial.println(numServos);
            haveNumServos = true;
          }
        }
        else if (haveName) {
          def = atoi(defTxt);
          Serial.print(entryName);
          Serial.print(" ");
          Serial.println(def);
          defaultPos[servoNum] = def;
          servoNum++;
        }
        sprintf(entryName, "");
        sprintf(defTxt, "");
        haveName = false;
      }
      else {
        while (c != '\r') {
          if (c == ' ') {
            haveName = true;
          }
          else if (haveName) {
            sprintf(defTxt, "%s%c", defTxt, c);
          }
          else {
            sprintf(entryName, "%s%c", entryName, c);
          }
          c = dataFile.read();
        }
      }
    }
  }
  dataFile.close();
  //bleSerial.println("LF");
}

void ServoSystemClass::printFiles() {
  int fileNum = 0;
  dataFile = SD.open("/scripts/");
  while (true) {
    File entry = dataFile.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    if (entry.isDirectory()) {
      // skip
    } else {
      char * entryName = malloc(sizeof(char) * 20);
      char c;
      sprintf(entryName, "%s", entry.name());
      // look at the file for a name
      if (entry.size() > 0) {
        c = entry.read();
        if (c == '#') {
          sprintf(entryName, "");
          while (c != '\r') {
            c = entry.read();
            if ((strlen(entryName) > 0) || (c != ' ')) {
              sprintf(entryName, "%s%c", entryName, c);
            }
          }
        }
      }
      // files have sizes, directories do not
      //bleSerial.print(fileNum);
      //bleSerial.print(" ");
      bleSerial.println(entryName);
      bleSerial.flush();
      delay(100);
      fileNum++;
    }
    entry.close();
  }
  bleSerial.println("LF");
  bleSerial.flush();
}

void ServoSystemClass::sendServoProfile() {
  for (int idx = 0; idx < numServos; idx++) {
    char * sendStr = malloc(sizeof(char) * 10);
    sprintf(sendStr, "%c%d %d", 'S', idx, positions[idx]);
    /*bleSerial.print("S");
    bleSerial.print(idx);
    bleSerial.print(" ");
    bleSerial.println(positions[idx]);*/
    Serial.println(sendStr);
    bleSerial.println(sendStr);
    bleSerial.flush();
    delay(100);
  }
  bleSerial.println("LF");
  bleSerial.flush();
}

void ServoSystemClass::loadFile(int fileNum) {
  char fp;
  File root = SD.open("/scripts/");
  for (int idx = 0; idx <= fileNum; idx++) {
    dataFile.close();
    dataFile = root.openNextFile();
    if (dataFile.isDirectory()) idx--;
  }
  processCommands(&dataFile);
  dataFile.close();
}

// Move the servo in increments, not too quickly
void ServoSystemClass::moveServo(int servo, int pos) {
  int delta = (pos > positions[servo]) ? 1 : -1;
  while (positions[servo] != pos) {
    positions[servo] += delta;
    servos[servo].write(positions[servo]);
    delay(increment);
  }
}

// Put arm in relaxed position
void ServoSystemClass::servosBegin() {
  for (int idx = 0; idx < numServos; idx++) {
    positions[idx] = defaultPos[idx];
  }
  for (int idx = 0; idx < numServos; idx++) {
    servos[idx].attach(idx + 2);
    servos[idx].write(positions[idx]);
    delay(300);
  }
}

// Put arm in relaxed position
void ServoSystemClass::servosNeutral() {
  for (int idx = 0; idx < numServos; idx++) {
    moveServo(idx, defaultPos[idx]);
  }
}

void ServoSystemClass::processCommands(Stream* stream) {
  char c;
  sprintf(cmd, "");
  sprintf(arg, "");
  while (stream->available()) {
    c = stream->read();
    if (c == '#') { // comment, skip the line
      while (c != '\n') {
        c = stream->read();
      }
    } else if (c == ' ') {
      haveCmd = true;
      func = cmd[0];
      target = cmd[1] - '0';
    } else if (c == '\n') {
      val = atoi(arg);
      haveCmd = false;
      sprintf(cmd, "");
      sprintf(arg, "");
      // manual servo move
      if (func == 'S') {
        if (val == -1) {
          sendServoProfile();
        }
        else {
          moveServo(target, val);
        }
      }
      // special command
      else if (func == 'C') {
        if (val == 0) {
          servosNeutral();
        }
      }
      // file command
      else if (func == 'F') {
        if (val == -1) {
          // file list
          printFiles();
        } else {
          loadFile(val);
        }
      }
    } else if (c != '\r') {
      if (haveCmd) {
        sprintf(arg, "%s%c", arg, c);
      } else {
        sprintf(cmd, "%s%c", cmd, c);
      }
    }
    delay(10);
  }
}
