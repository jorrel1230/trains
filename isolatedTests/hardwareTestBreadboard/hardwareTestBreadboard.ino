#include <Servo.h>

const int PICKUPSERVOPIN = 5;
const int DROPOFFSERVOPIN = 6;

// Pin Definitions
const int HALL1PIN = 9;
const int HALL2PIN = 10;
const int HALL3PIN = 11;
const int HALL4PIN = 12;
const int HALL5PIN = 13;


const int TRICKLE_ENTRANCE_DIR_PIN = A0;
const int TRICKLE_DROP1_DIR_PIN = A1;
const int TRICKLE_DROP2_DIR_PIN = A2;

const int TRICKLE_ENTRANCE_TRIG_PIN = 7;
const int TRICKLE_DROP_TRIG_PIN = 8;

//.Initialize Servos
Servo pickupServo;
Servo dropoffServo;

const int start = 0;
const int stop = 180;

void setup() {
  Serial.begin(9600);
  // Set Up Servo Motos with Arduino Digital Pins
  pickupServo.attach(PICKUPSERVOPIN); 
  dropoffServo.attach(DROPOFFSERVOPIN);

  pickupServo.write(start);
  dropoffServo.write(start);

  // Set up Hall Effect Sensors with Arduino Digital Pins.
  pinMode(HALL1PIN, INPUT);
  pinMode(HALL2PIN, INPUT);
  pinMode(HALL3PIN, INPUT);
  pinMode(HALL4PIN, INPUT);
  pinMode(HALL5PIN, INPUT);

  // Set up trickle pins
  pinMode(TRICKLE_ENTRANCE_DIR_PIN, OUTPUT);
  pinMode(TRICKLE_DROP1_DIR_PIN, OUTPUT);
  pinMode(TRICKLE_DROP2_DIR_PIN, OUTPUT);
  pinMode(TRICKLE_ENTRANCE_TRIG_PIN, OUTPUT);
  pinMode(TRICKLE_DROP_TRIG_PIN, OUTPUT);
}

void printUpdate() {
  Serial.print(digitalRead(HALL1PIN));
  Serial.print(",");
  Serial.print(digitalRead(HALL2PIN));
  Serial.print(",");
  Serial.print(digitalRead(HALL3PIN));
  Serial.print(",");
  Serial.print(digitalRead(HALL4PIN));
  Serial.print(",");
  Serial.print(digitalRead(HALL5PIN));
  Serial.println();
}

void setPinsHigh() {
  digitalWrite(TRICKLE_ENTRANCE_DIR_PIN, HIGH);
  digitalWrite(TRICKLE_DROP1_DIR_PIN, HIGH);
  digitalWrite(TRICKLE_DROP2_DIR_PIN, HIGH);
  digitalWrite(TRICKLE_ENTRANCE_TRIG_PIN, HIGH);
  digitalWrite(TRICKLE_DROP_TRIG_PIN, HIGH);
}

void setPinsLow() {
  digitalWrite(TRICKLE_ENTRANCE_DIR_PIN, LOW);
  digitalWrite(TRICKLE_DROP1_DIR_PIN, LOW);
  digitalWrite(TRICKLE_DROP2_DIR_PIN, LOW);
  digitalWrite(TRICKLE_ENTRANCE_TRIG_PIN, LOW);
  digitalWrite(TRICKLE_DROP_TRIG_PIN, LOW);
}

void loop() {
  setPinsHigh();
  // Serial.println("Testing Servo Pickup");
  for (int pos = start; pos <= stop; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    pickupServo.write(pos);              // tell servo to go to position in variable 'pos'
    printUpdate();
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (int pos = stop; pos >= start; pos -= 1) { // goes from 180 degrees to 0 degrees
    pickupServo.write(pos);              // tell servo to go to position in variable 'pos'
    printUpdate();
    delay(15);                       // waits 15ms for the servo to reach the position
  }

  setPinsLow();
  // Serial.println("Testing Servo Dropoff");
  for (int pos = start; pos <= stop; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    dropoffServo.write(pos);              // tell servo to go to position in variable 'pos'
    printUpdate();
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (int pos = stop; pos >= start; pos -= 1) { // goes from 180 degrees to 0 degrees
    dropoffServo.write(pos);              // tell servo to go to position in variable 'pos'
    printUpdate();
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  
}