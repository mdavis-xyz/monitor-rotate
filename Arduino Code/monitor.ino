// This Arduino sketch uses the MMA7455 accelerometer to tell
// a computer to rotate the display digitally
// when the monitor is physically rotated
//
// This uses I2C to talk to the MMA7455 accelerometer
//
// Written by Matthew Davis
// December 2015
//
// Distributed under the GNU Public Licence
//
// Wiring:
// Connect GND to GND, and VCC (on the sensor) to 5V (on the Arduino)
// Connect CS to VCC = 5V
// Connect SCL on the sensor to A5 on the Uno
// Connect SDA on the sensor to A4 on the Uno
//
// this example code works out which way the device is pointing
// and prints it through serial
// The computer decides which way to rotate, based on that information
//
// This code assumes the device starts with the Y axis up
// (the device callibrates itself in the setup phase).
// Change START_ORIENTATION if it starts in some other orientation.
//
//


#include <Wire.h>
#include "MMA7455.h" // must be in the same directory as this file

// The monitor is always 'vertical'
// So the sensor should never be Z_POS, Z_NEG or NOT_SURE
#define VALID_ORIENTATION(o) ((o==Y_POS) || (o==Y_NEG) || (o==X_POS) || (o==X_NEG))

// this is the orientation it starts in
// for callibration purposes
#define START_ORIENTATION X_NEG

// We're going to poll the accelerometer, since the MMA7455 doesn't
// Do axis-specific interupts.
// We just check each loop what the orientation is, and see if that
// differs from the previous loop
orientation previous_orientation = START_ORIENTATION;


// There is a button you press, to manually send
// A command to the computer even if the rotation hasn't
// changed recently
#define BUTTON_PIN 12

// Since we're polling the accelerometer, we need not bother using interrupts
// for the button. Instead we just read the button state each loop and compare
// it to the previous loop
boolean previous_button_state = 1; // active low, with a 100k pull up resistor

// We send some diagnostics to the computer, e.g. when starting up.
//
// If we want to tell the computer we've detected a change, we
// send a line starting with LINE_START and ending with LINE_END
// e.g. "Rotate Monitor <X_POS>", tells the computer that the x axis is now
// pointing up
#define LINE_START "Rotate Monitor <"
#define LINE_END ">"

// Writes something to Serial to tell the computer to rotate
void sendOrientation(orientation o);

// Gets a string to describe the orientation enum
char * textFromOrientation(orientation o);

void setup()
{

    pinMode(BUTTON_PIN,INPUT);

    int error;
    uint8_t c;

    Serial.begin(9600);
    Serial.println("Freescale MMA7455 accelerometer");
    Serial.println("May 2012");

    // Initialize the 'Wire' class for I2C-bus communication.
    Wire.begin();


    // Initialize the MMA7455, and set the offset.
    error = MMA7455_init(START_ORIENTATION);

    if (error == 0){
        Serial.println("The MMA7455 is okay");
    }else{
        Serial.println("Check your wiring !");
    }

    // Read the Status Register
    MMA7455_status(&c);
    Serial.print("STATUS : ");
    Serial.println(c,HEX);

    // Read the "Who am I" value
    MMA7455_who_am_I(&c);
    Serial.print("WHOAMI : ");
    Serial.println(c,HEX);

    Serial.print("Assuming the device is ");
    Serial.println(textFromOrientation(START_ORIENTATION));
    
    previous_orientation = START_ORIENTATION;
}



void loop()
{
    // What's the orientation this time around?
    orientation current_orientation = MMA7455_orientation();

    // If the orientation has changed to a new valid orientation
    if((current_orientation != previous_orientation) &&
       VALID_ORIENTATION(current_orientation)){
        previous_orientation = current_orientation;
        Serial.println("change detected");
        sendOrientation(current_orientation);
    }else if(!digitalRead(BUTTON_PIN) // button has been pressed
          && previous_button_state)    // and this is the first time it's been
                                      // pressed
    {
       // send even if orientation hasn't changed
       setup(); //re-calibrate
       sendOrientation(START_ORIENTATION);
       delay(100); //software debounce
       previous_button_state = 0;
    }else{
       previous_button_state = digitalRead(BUTTON_PIN);
    }

    delay(200); // there's no need to poll every millisecond, let's
                // add a delay
}

void sendOrientation(orientation o){
    Serial.print(LINE_START);
    Serial.print(textFromOrientation(o));
    Serial.println(LINE_END);
}

char * textFromOrientation(orientation o){
  switch(o){
      case Y_POS:
          return "Y_POS";
          break;
      case Y_NEG:
          return "Y_NEG";
          break;
      case X_POS:
          return "X_POS";
          break;
      case X_NEG:
          return "X_NEG";
          break;
      default:
          return "NOT SURE";
          break;
  }
}
