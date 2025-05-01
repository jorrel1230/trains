#include <Servo.h>

const int PICKUPSERVOPIN = 5;
const int DROPOFFSERVOPIN = 6;

//.Initialize Servos
Servo pickupServo;
Servo dropoffServo;

const int start = -15;
const int stop = 195;

void setup() {
  Serial.begin(9600);
  // Set Up Servo Motos with Arduino Digital Pins
  pickupServo.attach(PICKUPSERVOPIN); 
  dropoffServo.attach(DROPOFFSERVOPIN);
}

void loop() {
  Serial.println("Testing Servo Pickup");
  pickupServo.write(start);
  delay(500);

  for (int pos = start; pos <= stop; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    pickupServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }

  pickupServo.write(stop);
  delay(500);

  for (int pos = stop; pos >= start; pos -= 1) { // goes from 180 degrees to 0 degrees
    pickupServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  
}