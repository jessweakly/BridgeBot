 /*    Uses interrupts to read two encoders
 *     Can set each encoder to a target
 *  by Jessica McWilliams
 */

#include <Wire.h>
#include "ARM_MiniMoto.h"


 #if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
// Required for Serial on Zero based boards
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

//Back board
//8 9 gives 0 0
// 3 4 gives 11

//Front board
// 5 11 gives 01
// 10 6 gives 0 0 or 1 0

 // Define motor encoder pins for interrupts
 #define output0A 5 //5
 #define output0B 11//11
 #define output1A 10
 #define output1B 6
 #define numMotors 2
 #define thresh 4    //the tolerance for encoder values. If withing +/- thresh its "good"

// Define motor encoder pins as array for easier manipulation
 int outA[numMotors] = {output0A, output1A};
 int outB[numMotors] = {output0B, output1B};


//initialize counters
int counter[numMotors] = {0, 0}; 

//Create motors
MiniMoto motor0(0x67);
MiniMoto motor1(0x64);
MiniMoto motor[] = {motor0, motor1};

int speed[2] = {-80, -80};
int targets[2] = {-1200, 1000};

void setup() { 
   Wire.begin();
   Serial.begin(9600);
   // Define encoder pins as input
   pinMode(output0A,INPUT);
   pinMode(output0B,INPUT);

   // Set up interrupt pins
   attachInterrupt(digitalPinToInterrupt(output0A), count0, RISING);
   attachInterrupt(digitalPinToInterrupt(output1A), count1, RISING);

   while (!Serial); 
   Serial.println("Starting to Run");
 } 
void loop() { 
  toTargets();
   
  delay(5000);
  
 }

// ISR for encoder 0
void count0() {
  // Check direction of rotation
  if (digitalRead(output0B)) {
    counter[0]++;
  } else {
    counter[0]--;
  }
 }

//ISR for encoder 1
void count1() {
  if (digitalRead(output1B)) {
    counter[1]++;
  } else {
    counter[1]--;
  }
 }

 void resetCounts() {
  for (int i = 0; i < numMotors; i++) {
    counter[i] = 0;
  }
 }
 void toTargets() {
  resetCounts();
  Serial.println("In function");
  int i=numMotors; // subtract from i each time a target is reached
  int done[numMotors] = {0,0};
  while (i > 0) {
    for (int j = 0; j < numMotors; j++) {
      if (counter[j] < (targets[j]-thresh)) {
        motor[j].drive(speed[j]);
      } else if (counter[j] > (targets[j] + thresh)) {
        motor[j].drive(-speed[j]);
      } else if (!done[j]){
        done[j] =1;  // don't count twice
        motor[j].drive(0);
        i--;
      }
    }
   }
 }
 


