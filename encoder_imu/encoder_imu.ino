/*
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
#include "ARM_MiniMoto.h"
#include "My_MPU9250.h"
#define DEBUG 1

#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.print(x)
 #define DEBUG_PRINTLN(x) Serial.println(x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTLN(x)
#endif

// Define encoder pins
#define FL1 8
#define FL2 9
#define FR1 3
#define FR2 4
#define BL1 5
#define BL2 11
#define BR1 10
#define BR2 6

#define NUM_ENCODERS 4
#define MOVING_AVERAGE 8

int out1[] = {FL1, FR1, BL1, BR1};
int out2[] = {FL2, FR2, BL2, BL2};

//counter for encoders
int counter[4]= {0,0,0,0};

//Create motors
MiniMoto FL(0x60);
MiniMoto FR(0x61);
MiniMoto BL(0x64);
MiniMoto BR(0x67);
MiniMoto motor[] = {FL, FR, BL, BR};

// Create IMU
MPU9250 imu1(0x69);

int ten = 0;

//keyboard
int incomingByte;
char state = 'h';

//Averaging accelZ data
float sum = 0;
float average=0;
float AvgA[MOVING_AVERAGE];
int count = 0;

int motorPower[] = {80,80,80,80}; //FL,FR,RL,RR
int targetSpeed[] = {90, 90, 90, 90}; //percentage of full speed we want to go
float topSpeed = 7.50; // multiply by 100 for counts/second

float transGain = 5;

float kp = 1;
float LOOP_TIME = .1; //time in seconds

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
// Required for Serial on Zero based boards
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

void setup() {
  Serial.begin(9600);
  Wire.begin();
  imu1.initMPU();

  // Initialize the encoders
  for ( int i = 0; i < NUM_ENCODERS; i++) {
    pinMode(out1[i], INPUT);
    pinMode(out2[i], INPUT);
  }

  //Attach interrupts to one pin on each encoder
  attachInterrupt(digitalPinToInterrupt(out1[0]), count0, RISING);
  attachInterrupt(digitalPinToInterrupt(out1[1]), count1, RISING);
  attachInterrupt(digitalPinToInterrupt(out1[2]), count2, RISING);
  attachInterrupt(digitalPinToInterrupt(out1[3]), count3, RISING);

  Serial.println("Starting"); 
  // Reads the initial state of the accel to fill averages array
  accel_init();
  delay(3000);
}


void loop() {
  incomingByte = Serial.read();

  //Record accel with a moving average
  imu1.recordAccel();
  imu1.recordGyro();
  sum -= AvgA[count];    //remove old value from sum
  AvgA[count] = imu1.accelZ;
  sum += imu1.accelZ;        //add new value to sum
  average = sum/8;       //Find the new average
  
  DEBUG_PRINTLN(imu1.accelZ);

  // Maintain moving average window
  if (count < MOVING_AVERAGE-1) {
    count++;
  } else {
    count = 0;
  }
 
  printMPU(imu1);
  printEncoders();
  
 
}


//assign speed a percentage (value from -100 to 100);
void setForward(int speed) {
  for (int i = 0; i < 4; i++) {
    targetSpeed[i]= speed; //Set target speed for all motors to be equal
  }
}

//Rewritten
void drive() {
  for (int i = 0; i < 4; i++) {
     motor[i].drive(motorPower[i]);    
  }
}

//antiquated
//void stopAll() {
//  for (int i = 0; i < 4; i++) {
//      analogWrite(motors[i][0], 0);
//      analogWrite(motors[i][1], 0);
//  }
//}
//
//void driveFront(int speed) {
//  for (int i = 2; i < 4; i++) {
//    analogWrite(motors[i][0], speed); //Activate forward motors
//    analogWrite(motors[i][1], 0);     //Deactivate reverse motors
//  }
//}





//Fill the accel array with values, calculate the average, and reset the count
void accel_init(){
  count = 0;
  sum = 0;
 while (count < MOVING_AVERAGE) {
    recordAccelRegisters();
    AvgA[count] = accelZ;
    sum += accelZ;
    DEBUG_PRINTLN(accelZ);
    count++;
  }
 count = 0;
 average = sum / (float) MOVING_AVERAGE;
 }


///////////////////////////////////////////////////////////////////////

////// Serial Output //////////////////
void printMPU(MPU9250 mpu) {
  Serial.print("IMU");
  Serial.print(" ");
  Serial.print(mpu.gyroX);
  Serial.print(" ");
  Serial.print(mpu.gyroY);
  Serial.print(" ");
  Serial.print(mpu.gyroZ);
  Serial.print(" ");
  Serial.print(" ");
  Serial.print(mpu.accelX);
  Serial.print(" ");
  Serial.print(mpu.accelY);
  Serial.print(" ");
  Serial.println(mpu.accelZ);
}

void printEncoders() {
  Serial.print("Encoders");
  Serial.print(" ");
  for (int i = 0; i < 4; i++) {
    Serial.print(" ");
    Serial.print(counter[i]);
  }
  Serial.println("");
}
/////////////////////////////////////////////////////////////////////////

////////////// Encoder Interrupt Service Routines ////////////////////////
void count0() {
  // Check direction of rotation
  if (digitalRead(out2[0])) {
    counter[0]++;
  } else {
    counter[0]--;
  }
}

void count1() {
  if (digitalRead(out2[1])) {
    counter[1]++;
  } else {
    counter[1]--;
  }
}

void count2() {
  // Check direction of rotation
  if (digitalRead(out2[2])) {
    counter[2]++;
  } else {
    counter[2]--;
  }
}

void count3() {
  // Check direction of rotation
  if (digitalRead(out2[3])) {
    counter[3]++;
  } else {
    counter[3]--;
  }
}
////////////////////////////////////////////////////////////////////////

//  //Calculate speeds and adjust motor power
//  int currSpeed[NUM_ENCODERS];
//  int error[NUM_ENCODERS];
//  for (int i = 0; i < NUM_ENCODERS; i++) {
//    currSpeed[i] = (counter[i] - lastCounters[i]) * (1.0 / LOOP_TIME);
//    
//    error[i] = kp * ((targetSpeed[i] * topSpeed) - currSpeed[i]);
//
//    if (error[i] != 0) { //counter 1 is ahead
//      motorPower[i] = motorPower[i] + error[i];
//      if (motorPower[i] > 255) {
//        motorPower[i] = 255;
//      } else if (motorPower[i] < 0) {
//        motorPower[i] = 0;
//      }
//    }
//    
//    DEBUG_PRINTLN((String) i + "\t" + (String)currSpeed[i] + "\t" +
//          (String)error[i] + "\t" + (String)motorPower[i]);
//  } 
//  
//}

