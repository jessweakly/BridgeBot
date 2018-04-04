
 /*     Adapted From
  *      Arduino Rotary Encoder Tutorial
  *      Reads the encoder counts as the motor runs
  *      
  *      Uses interrupts to read two encoders
 *      
 *  by Dejan Nedelkovski, www.HowToMechatronics.com
 *  
 */

#include <Wire.h>
#include "MyMiniMoto.h"


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

// Define motor encoder pins as array for easier manipulation
 int outA[2] = {output0A, output1A};
 int outB[2] = {output0B, output1B};


//initialize counters
int counter[2] = {0, 0}; 

//Create motors
MiniMoto motor0(0x60);
MiniMoto motor1(0x64);
MiniMoto motor[] = {motor0, motor1};

int speed = 80;

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
   Serial.print(counter[0]);
   Serial.print("\t");
   Serial.println(counter[1]);
   delay(1000);
   motor[0].drive(speed);
   motor[1].drive(-speed);
   speed = -speed;
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

 


