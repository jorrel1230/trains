#include <Servo.h>

const int PICKUPSERVOPIN = 6;
const int DROPOFFSERVOPIN = 5;

//.Initialize Servos
Servo pickupServo;
Servo dropoffServo;

const int start = 5;
const int stop = 171;

void setup() {
  Serial.begin(9600);
  // Set Up Servo Motos with Arduino Digital Pins
  pickupServo.attach(PICKUPSERVOPIN, 500, 2700); 
  dropoffServo.attach(DROPOFFSERVOPIN, 500, 2500);
}

void loop() {
  Serial.println("Testing Servo Pickup");
  dropoffServo.write(start);
  pickupServo.write(start);
  delay(500);

   for (int i = 0; i < 3; i++) {
    pickupServo.write(start-1);
    delay(100);
    pickupServo.write(start+1);
    delay(100);
  }

  for (int pos = start; pos <= stop; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    dropoffServo.write(pos);              // tell servo to go to position in variable 'pos'
    pickupServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }

  dropoffServo.write(stop);
  pickupServo.write(stop);
  delay(500);

  for (int i = 0; i < 3; i++) {
    pickupServo.write(stop-1);
    delay(100);
    pickupServo.write(stop+1);
    delay(100);
  }


  for (int pos = stop; pos >= start; pos -= 1) { // goes from 180 degrees to 0 degrees
    dropoffServo.write(pos);              // tell servo to go to position in variable 'pos'
    pickupServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  
}