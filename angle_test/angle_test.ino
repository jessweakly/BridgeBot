/*
 * BRIDGE Bot angle test
 * This code reads the data from one IMU and transmits it over radio.
 * The bot can be in several states, which may be manually sent over radio
 * or automatically determined when IMU data indicates a plane transition
 * has occurred.
 * 
 *
 */


/*
 * 
 Credit for the MPU-6050 functions
 *
 *
  ===Contact & Support===
  Website: http://eeenthusiast.com/
  Youtube: https://www.youtube.com/EEEnthusiast
  Facebook: https://www.facebook.com/EEEnthusiast/
  Patreon: https://www.patreon.com/EE_Enthusiast
  Revision: 1.0 (July 13th, 2016)

  ===Hardware===
  - Arduino Uno R3
  - MPU-6050 (Available from: http://eeenthusiast.com/product/6dof-mpu-6050-accelerometer-gyroscope-temperature/)

  ===Software===
  - Latest Software: https://github.com/VRomanov89/EEEnthusiast/tree/master/MPU-6050%20Implementation/MPU6050_Implementation
  - Arduino IDE v1.6.9
  - Arduino Wire library

  ===Terms of use===
  The software is provided by EEEnthusiast without warranty of any kind. In no event shall the authors or
  copyright holders be liable for any claim, damages or other liability, whether in an action of contract,
  tort or otherwise, arising from, out of or in connection with the software or the use or other dealings in
  the software.
*/

#include <Wire.h>

int whichMPU = 1;

long accelX, accelY, accelZ;
float gForceX, gForceY, gForceZ;
float average = 0;
int LED = 13;
int FL[2] = {10, 6}; //First is forward, second is back
int FR[2] = {3, 4};
int RL[2] = {8, 9};
int RR[2] = {11, 5};
int ten = 0;

int incomingByte;
float AvgA[8];
int count = 0;
char state = 's';

int motors[][2] = {{10, 6}, {3, 4}, {8, 9}, {5, 11}}; //NOT Flipped

int speedVariants[] = {180, 255, 180, 255}; //FL,FR,RL,RR
float transGain = 5;

long gyroX, gyroY, gyroZ;
float rotX, rotY, rotZ;

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
// Required for Serial on Zero based boards
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

void setup() {
  Serial1.begin(115200);
  Wire.begin();
  setupMPU();
  Serial1.println("Start");
}


void loop() {
  incomingByte = Serial1.read();
  recordAccelRegisters();
  recordGyroRegisters();
  printData();

  while (count <= 7) {
    recordAccelRegisters() ;
    AvgA[count] = gForceZ;
    //Serial.print(gForceZ);
    count++;
  }
  if (count == 8) {
    float total = 0;
    for (int i = 0; i <= 7; i++) {
      total += AvgA[i];
    }
    average = total / 8.0;
  }

  if (incomingByte != -1) {
    state = incomingByte;
  }

//  Serial.print(state);
//  Serial.print("\n ");
  switch (state) {
    case 's':
      ten++;
      if (ten >= 19) {
        ten = 0;
        count = 0;
        //Serial.print(" REFRESH \n");
      }
      driveForward(0);
      //Serial.print(" Stopped \n");
      analogWrite(LED, 0);
      break;

    
    case 'r':
      turnRight(200);
    case 'l':
      turnLeft(200);
    
    case 'f':
      driveForward(200);
      ten++;
      analogWrite(LED, ten*10);
      recordAccelRegisters();
      if (ten >= 19) {
        ten = 0;
        count = 0;
        //Serial.print(" REFRESH \n");
      }
      if (gForceZ < average - .09) {
        state = 't';
      }
      if ((gForceZ > 3) && (gForceZ <3.6)) {
        state = 'u';
      }
      //Serial.print("Driving Forward \n");
      break;
    case 'a':
//      Serial.print("Average ");
//      Serial.print(average);
//      Serial.print("\n ");
      count = 0;
      state = 's';
      break;
    case 't':
      //Serial.print("Plane Transition \n");
      driveForward(0);
      driveFront(200);
      delay(100);
      driveForward(0);
      count = 0;
      state = 'f';
      analogWrite(LED, 0);
      delay(50);
      analogWrite(LED, 255);
      delay(50);
      analogWrite(LED, 0); 
      delay(50);
      analogWrite(LED, 255);
      break;
    case 'u':
      //Serial.print("Going Up! \n");
      driveForward(200);
      if (gForceZ < 3) {
        state = 'h';
      }
      break;
    case 'h':
      //Serial.print("Hanging out! \n");
      driveForward(200);
//      if ((gForceZ < 2) && (gForceZ > .7)) {
//        state = 'd';
//      }
//      if ((gForceZ < 3.7) && (gForceZ > 3)) {
//        state = 'f';
//        delay(500);
//      }
      break;

    case 'd':
      //Serial.print("Going DOWN! \n");
      driveForward(200);
      if (gForceZ < .5) {
        state = 'f';
      }
      break;
  }

  delay(75);
}


void turnLeft(int speed) {
  for (int i = 0; i < 4; i +=2) { //left motors back
    analogWrite(motors[i][0], 0);
    analogWrite(motors[i][1], speed * speedVariants[i]/255);
  }
  for (int i = 1; i < 4; i +=2) {  //ight motors forward
    analogWrite(motors[i][1], 0);
    analogWrite(motors[i][0], speed * speedVariants[i]/255);
  }
}

void turnRight(int speed) {
    for (int i = 1; i < 4; i +=2) { //right motors back
    analogWrite(motors[i][0], 0);
    analogWrite(motors[i][1], speed * speedVariants[i]/255);
  }
  for (int i = 0; i < 4; i +=2) {  //left motors forward
    analogWrite(motors[i][1], 0);
    analogWrite(motors[i][0], speed * speedVariants[i]/255);
  }
}

void driveForward(int speed) {
  for (int i = 0; i < 4; i++) {
    analogWrite(motors[i][0], speed * speedVariants[i] / 255); //Activate forward motors
    analogWrite(motors[i][1], 0);     //Deactivate reverse motors
  }
  //  digitalWrite(motors[i][0],HIGH);  //Activate forward motors
  //  digitalWrite(motors[i][1],LOW);      //Deactivate reverse motors

}

void driveFront(int speed) {
  for (int i = 2; i < 4; i++) {
    analogWrite(motors[i][0], speed * speedVariants[i] / 255); //Activate forward motors
    analogWrite(motors[i][1], 0);     //Deactivate reverse motors
  }
  //  digitalWrite(motors[i][0],HIGH);  //Activate forward motors
  //  digitalWrite(motors[i][1],LOW);      //Deactivate reverse motors

}

void setupMPU() {
  Wire.beginTransmission(0b1101000 + whichMPU); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000000); //Setting SLEEP register to 0. (Required; see Note on p. 9)
  Wire.endTransmission();
  Wire.beginTransmission(0b1101000 + whichMPU); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4)
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg./s
  Wire.endTransmission();
  Wire.beginTransmission(0b1101000 + whichMPU); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5)
  Wire.write(0b00000000); //Setting the accel to +/- 2g
  Wire.endTransmission();
}

void recordAccelRegisters() {
  Wire.beginTransmission(0b1101000 + whichMPU); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000 + whichMPU, 6); //Request Accel Registers (3B - 40)
  while (Wire.available() < 6);
  accelX = Wire.read() << 8 | Wire.read(); //Store first two bytes into accelX
  accelY = Wire.read() << 8 | Wire.read(); //Store middle two bytes into accelY
  accelZ = Wire.read() << 8 | Wire.read(); //Store last two bytes into accelZ
  processAccelData();
}

void processAccelData() {
  gForceX = accelX / 16384.0;
  gForceY = accelY / 16384.0;
  gForceZ = accelZ / 16384.0;
}

void recordGyroRegisters() {
  Wire.beginTransmission(0b1101000 + whichMPU); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000 + whichMPU, 6); //Request Gyro Registers (43 - 48)
  while (Wire.available() < 6);
  gyroX = Wire.read() << 8 | Wire.read(); //Store first two bytes into accelX
  gyroY = Wire.read() << 8 | Wire.read(); //Store middle two bytes into accelY
  gyroZ = Wire.read() << 8 | Wire.read(); //Store last two bytes into accelZ
  processGyroData();
}

void processGyroData() {
  rotX = gyroX / 131.0;
  rotY = gyroY / 131.0;
  rotZ = gyroZ / 131.0;
}

void printData() {
  Serial1.print("");
  Serial1.print(" ");
  Serial1.print(rotX);
  Serial1.print(" ");
  Serial1.print(rotY);
  Serial1.print(" ");
  Serial1.print(rotZ);
  Serial1.print(" ");
  Serial1.print(" ");
  Serial1.print(gForceX);
  Serial1.print(" ");
  Serial1.print(gForceY);
  Serial1.print(" ");
  Serial1.println(gForceZ);
//  Serial.print("");
//  Serial.print(" ");
//  Serial.print(rotX);
//  Serial.print(" ");
//  Serial.print(rotY);
//  Serial.print(" ");
//  Serial.print(rotZ);
//  Serial.print(" ");
//  Serial.print(" ");
//  Serial.print(gForceX);
//  Serial.print(" ");
//  Serial.print(gForceY);
//  Serial.print(" ");
//  Serial.println(gForceZ);
////}
}


