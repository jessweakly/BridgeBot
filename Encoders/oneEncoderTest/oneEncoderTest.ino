
 /*     Adapted From
  *      Arduino Rotary Encoder Tutorial
  *      Reads the encoder counts as the motor runs
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
 
 #define outputA 5 //5
 #define outputB 11//11
 int counter = 0; 
MiniMoto motor0(0x60);
MiniMoto motor1(0x61);

 int aState;
 int aLastState;
 int bState;  
 void setup() { 
   Wire.begin();
   pinMode(outputA,INPUT);
   pinMode(outputB,INPUT);
   Serial.begin(9600);
   motor0.drive(80);
   //motor1.drive(80);
   delay(1000);
   while (!Serial); 
   Serial.println("Starting to Run");
   // Reads the initial state of the outputA
   aLastState = digitalRead(outputA);   
 } 
 void loop() { 
    //analogWrite(motorPin,255);
    //analogWrite(5,255);
    
    aState = digitalRead(outputA); // Reads the "current" state of the outputA
    bState= digitalRead(outputB);
   // Serial.println(aState);
    //Serial.println(bState);
    //Serial.println(" ");
    //delay(1000);
    // If the previous and the current state of the outputA are different, that means a Pulse has occured
    if (aState != aLastState){     
     // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
     if (bState != aState) { 
       counter ++;
     } else {
       counter --;
     }
     Serial.println(counter);
    } else {
      //Serial.println("no");
    }
      
   aLastState = aState; // Updates the previous state of the outputA with the current state
 }
