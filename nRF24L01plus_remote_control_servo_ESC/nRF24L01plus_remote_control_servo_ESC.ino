
/*
* nRF24L01plus_remote_control_servo_ESC
* 
* Example sketch for using nRF24L01+ radios to act as a RC transmitter and receiver for controlling a servo or ESC.
* Required Libraries: Servo (included) and RF24: https://github.com/tmrh20/RF24/ .  
* 
* For tranceiver pin setup go here:   http://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/ .  
* This tutorial is great and does the same thing, but I couldn't get it working.
* 
* Potentiometer was setup using Analog 0 / A0
* 
* Required components: 
* - 2 arduino boards
* - 2 wireless tranceivers.  Tested with these: https://www.amazon.com/gp/product/B01BWCYYKG  
* - 1 potentiometer 
* - 1 servo or ESC.  Note the some code works for servos and ESCs.  Servos are easier to start with for testing as some ESCs require intial calibration.
* - numerous jumper pins and a breadboard to share 5v 
*  
* IMPORTANT!!!!  You can upload the same code to both arduinos, with one exception - YOU MUST set the role to 0/receiver on one board and 1(transmitter on the other.  
* 
* IMPORTANT !!!  This code has only been bench tested and is BETA!!!  Use at your own risk.
*/
#include <Servo.h>
#include <SPI.h>
#include "RF24.h"

bool role = 0; // transmitter 1, receiver 0
byte addresses[][6] = {"1Node","2Node"};
bool radioNumber = 0;
RF24 radio(7,8);

Servo esc; //Creating a servo class with name as esc

struct dataStruct{
  unsigned long _micros;
  int value;}myData;

void setup() {

  Serial.begin(115200);

  esc.attach(9); //Specify the esc signal pin, D9
  esc.writeMicroseconds(1000); //initialize the signal to 1000
  
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  
  myData.value = 0;
  // Start the radio listening for data
  radio.startListening();
}




void loop() {
  
  
/****************** START Transmitter Role ***************************/  
if (role == 1)  
{

    int val= analogRead(A0); //Read input from analog pin a0 potentiometer
    
    //Serial.print("Analog: ");
    //Serial.print(val);        
    
    
    radio.stopListening();                                    // First, stop listening so we can talk.    
    
    Serial.println(F("Now sending"));

    myData._micros = micros();                      //  we are sending the microseconds here.  Future use will be for error checking
    myData.value = val;                             //  value from the potentiometer 
     if (!radio.write( &myData, sizeof(myData) ))
     {
       Serial.println(F("failed"));
     }
        
    radio.startListening();                                    // Now, continue listening
    
    unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
    boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not
    
    while ( ! radio.available() ){                             // While nothing is received
      if (micros() - started_waiting_at > 200000 ){            // If waited longer than 200ms, indicate timeout and exit while loop
          timeout = true;
          break;
      }      
    }
        
    if ( timeout ){                                             // Describe the results
        Serial.println(F("Failed, response timed out."));
    }else{
                                                                // Grab the response, compare, and send to debugging spew
        radio.read( &myData, sizeof(myData) );
        unsigned long time = micros();
        
        // Spew it
        Serial.print(F("Sent "));
        Serial.print(time);
        Serial.print(F(", Got response "));
        Serial.print(myData._micros);
        Serial.print(F(", Round-trip delay "));
        Serial.print(time-myData._micros);
        Serial.print(F(" microseconds Value "));
        Serial.println(myData.value);
    }

    //delay(5);  // no delay
  }
/****************** END Receiver Role ***************************/  



/****************** START Receiver Role ***************************/  

  if ( role == 0 ) // 
  {
    
    if( radio.available()){
                                                           // Variable for the received timestamp
      while (radio.available()) 
      {                          // While there is data ready
        radio.read( &myData, sizeof(myData) );             // Get the payload
      }
     
        radio.stopListening();                               // First, stop listening so we can talk  
        radio.write( &myData, sizeof(myData) );              // Send the final one back.      
        radio.startListening();                              // Now, resume listening so we catch the next packets.     

        esc.writeMicroseconds(map(myData.value, 0, 900,1200,2000)); 
        //mapping val to minimum and maximum(Change if needed)  
        //using val as the signal to esc 
        
        Serial.print(F("Sent response "));
        Serial.print(myData._micros);  
        Serial.print(F(" : "));
        Serial.println(myData.value);
      }
   }

/****************** START Receiver Role ***************************/  

} // Loop


