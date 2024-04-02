// Start Pins
#define buttonMotorForwardX 21//
#define buttonMotorForwardY 22//
#define buttonMotorForwardW 23//
#define buttonMotorForwardE 24//
#define buttonMotorBackwardX 25//
#define buttonMotorBackwardY 26//
#define buttonMotorBackwardW 27//
#define buttonMotorBackwardE 28//
#define buttonResetPosition 29//
#define driverReadyToMove 20//
#define SerialForPLC 15//
#define sensorMaxX 16//
#define sensorMaxY 18//
#define sensorMaxW 19//
#define dataPLC 17//
#define motorFrequency 3000
int pinsMotors[4][4] = {
  { 33, 34, 35, 36 },
  { 37, 38, 39, 40 },
  { 1, 2, 3, 4 },
  { 5, 6, 7, 8 }
};
// End Pins
// Start Settings
int manipulatorPosition[4][4] = {
  { 790, 400, 0, 500 },
  { 254, 985, 999, 0 },
  { 443, 645, 1, 954 },
  { 223, 543, 432, 123 }
};
bool motorsValues[4][4]={
  {1,1,0,0},
  {0,1,1,0},
  {0,0,1,1},
  {1,0,0,1}
};
// End Settings
// Start variables function move
int countStep[4] = { 0, 0, 0, 0 };
int moveCounter[4] = {0,0,0,0};
uint32_t  moveTimer = 0;
// End variables function move
// Start variables automation
int position = 0;
bool startMove = false;
int counterAuto = 0;
bool autoReverse = false;
// End variables automation
// Start variables function parsePLC
uint32_t  timerParsePLC = 0; 
int counterParsePLC = 5;   
bool resivingPLCData = false;
// End variables function parsePLC



void setup() {
 timerParsePLC = millis();
 moveTimer = micros();
 Serial.begin(9600);
}

void loop() {
  transferDriverPosition();
  if (micros() - moveTimer < 0){
    moveTimer = micros();
  }
  parsePLC();
  if (startMove) {
    digitalWrite(driverReadyToMove, LOW);
    if (manipulatorPosition[position][counterAuto] < countStep[counterAuto]) {
      autoReverse = true;
    } else {
      autoReverse = false;
    }
    move(counterAuto, autoReverse);
    if (manipulatorPosition[position][counterAuto] == countStep[counterAuto]) {
      counterAuto++;
    }
    if (counterAuto == 4) {
      startMove = false;
      counterAuto = 0;
    }
    
  } else {
    digitalWrite(driverReadyToMove, HIGH); 
    int buttonsActive = (digitalRead(buttonMotorForwardX) == HIGH) ? 1 : 0
      + (digitalRead(buttonMotorForwardY) == HIGH) ? 1 : 0
      + (digitalRead(buttonMotorForwardW) == HIGH) ? 1 : 0
      + (digitalRead(buttonMotorForwardE) == HIGH) ? 1 : 0 
      + (digitalRead(buttonMotorBackwardX) == HIGH) ? 1 : 0
      + (digitalRead(buttonMotorBackwardY) == HIGH) ? 1 : 0
      + (digitalRead(buttonMotorBackwardW) == HIGH) ? 1 : 0
      + (digitalRead(buttonMotorBackwardE) == HIGH) ? 1 : 0
      + (digitalRead(buttonResetPosition) == HIGH)  ? 1 : 0;
    if (buttonsActive > 1) {
      return;
    }
    if (digitalRead(buttonMotorForwardX) == HIGH) {
      move(0, false);
    }
    if (digitalRead(buttonMotorForwardY) == HIGH) {
      move(1, false);
    }
    if (digitalRead(buttonMotorForwardW) == HIGH) {
      move(2, false);
    }
    if (digitalRead(buttonMotorForwardE) == HIGH) {
      move(3, false);
    }
    if (digitalRead(buttonMotorBackwardX) == HIGH) {
      move(0, true);
    }
    if (digitalRead(buttonMotorBackwardY) == HIGH) {
      move(1, true);
    }
    if (digitalRead(buttonMotorBackwardW) == HIGH) {
      move(2, true);
    }
    if (digitalRead(buttonMotorBackwardE) == HIGH) {
      move(3, true);
    }
    if (digitalRead(buttonResetPosition) == HIGH) {
      for (int i = 1; i < 4; i++) {
        countStep[i] *= 0;
      }
    }
    
  }
}

void move(int indexMotor, bool reverse) {
  if (countStep[indexMotor] >= 0) {
    if (micros() - moveTimer > 250000/motorFrequency){
      digitalWrite(pinsMotors[indexMotor][3], motorsValues[moveCounter[indexMotor]][0]);
      digitalWrite(pinsMotors[indexMotor][2], motorsValues[moveCounter[indexMotor]][1]);
      digitalWrite(pinsMotors[indexMotor][1], motorsValues[moveCounter[indexMotor]][2]);
      digitalWrite(pinsMotors[indexMotor][0], motorsValues[moveCounter[indexMotor]][3]);
      moveTimer = micros();
      if (reverse){
        moveCounter[indexMotor]--;
        if (moveCounter[indexMotor] < 0){
        moveCounter[indexMotor] = 3;
        countStep[indexMotor]--;
        }
      }
      else {
        moveCounter[indexMotor]++;
        if (moveCounter[indexMotor] > 3){
          moveCounter[indexMotor] = 0;
          countStep[indexMotor]++;
        }
      }
    }
  }
}

void moveToHome() {
  digitalWrite(driverReadyToMove, LOW);
  while (true) {
    if (digitalRead(sensorMaxX) == 0) {
      move(0, true);
    }
    else if (digitalRead(sensorMaxY) == 0) {
      move(1, true);
    }
    else if (digitalRead(sensorMaxW) == 0) {
      move(2, true);
    }
    else if (digitalRead(sensorMaxX) == 1 && digitalRead(sensorMaxY) == 1 && digitalRead(sensorMaxW) == 1) {
      for (int i = 1; i < 4; i++) {
        countStep[i] *= 0;
      }
      break;
    }
  }
  digitalWrite(driverReadyToMove, HIGH);
}

int booferPLCData = 0;

void parsePLC() {
  if (digitalRead(dataPLC) == HIGH || resivingPLCData == true){
    if (resivingPLCData == true){
      if (millis() - timerParsePLC >= 100){
        if (digitalRead(dataPLC) == HIGH){
          booferPLCData += 2 ^ counterParsePLC;
        }
        timerParsePLC = millis();
        counterParsePLC--;
      }
    }
    else{
      resivingPLCData = true;
      booferPLCData = 0;
      timerParsePLC = millis();
      counterParsePLC--;
    }
    if (counterParsePLC < 0 ) {
      counterParsePLC = 5;
      if (booferPLCData == 0) {
        moveToHome();
        startMove = false;
      } 
      else if (booferPLCData == 1 ) {
        startMove = false;
      }
      else if (!startMove){
        startMove = true;
        position = booferPLCData - 2;
        if (sizeof(manipulatorPosition) / sizeof(manipulatorPosition[0]) >= position){
          startMove = false;
        }
      }
      resivingPLCData = false;
    }
  }
}


bool transferingDriverData = false;
int counterMotorsPositions = 0;
int counterDecToBin = 0;
int bin = 65536;
unsigned int dec = 0;
int transferTimer = 0 ;
int transferDelay = 1;

void transferDriverPosition() {
  if (transferingDriverData == false){
    transferingDriverData = true;
    digitalWrite(SerialForPLC, 1);
    dec = countStep[counterMotorsPositions];
    transferTimer = millis();
  }
  if(transferingDriverData == true){
    if(millis() - transferTimer >= transferDelay/9600){
      if(dec >= bin){
        digitalWrite(SerialForPLC,1);
        dec -= bin;
      }
      else {
        digitalWrite(SerialForPLC,0);
      }
      bin/=2;
      counterDecToBin++;
      transferTimer = millis();
    }
    if (counterDecToBin == 16){
      counterMotorsPositions++;
      dec = countStep[counterMotorsPositions];
      bin = 65536;
    }
    if (counterMotorsPositions == 4){
      transferingDriverData = false;
      counterDecToBin = 0;
      counterMotorsPositions = 0;
      bin = 65536;
    }
  }
}