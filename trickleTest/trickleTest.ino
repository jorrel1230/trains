// arduinoFinal.ino
// Author: Jorrel Rajan

// Include all necessary libraries
#include <SoftwareSerial.h>
#include <Arduino_APDS9960.h>

// Initialize Software Serial for ACIA communication
SoftwareSerial mySerial(2, 3); // RX = 2, TX = 3

// Pin Definitions
const int TRICKLE_ENTRANCE_DIR_PIN = A2;
const int TRICKLE_DROP1_DIR_PIN = A1;
const int TRICKLE_DROP2_DIR_PIN = A0;

const int TRICKLE_ENTRANCE_TRIG_PIN = 8;
const int TRICKLE_DROP_TRIG_PIN = 7;

void setup() {
  // Initialize both Hardware and Software Serial.
  // Keep Hardware Serial plugged in for debugging purposes.
  Serial.begin(9600);
  mySerial.begin(9600);

  // Set up trickle pins
  pinMode(TRICKLE_ENTRANCE_DIR_PIN, OUTPUT);
  pinMode(TRICKLE_DROP1_DIR_PIN, OUTPUT);
  pinMode(TRICKLE_DROP2_DIR_PIN, OUTPUT);
  pinMode(TRICKLE_ENTRANCE_TRIG_PIN, OUTPUT);
  pinMode(TRICKLE_DROP_TRIG_PIN, OUTPUT);

  Serial.println("Starting Up");
  digitalWrite(TRICKLE_ENTRANCE_TRIG_PIN, HIGH);
  digitalWrite(TRICKLE_DROP_TRIG_PIN, HIGH);
  delay(10000);
}

void loop() {
  Serial.println("Testing...");


  digitalWrite(TRICKLE_ENTRANCE_DIR_PIN, LOW);
  digitalWrite(TRICKLE_DROP1_DIR_PIN, LOW);
  digitalWrite(TRICKLE_DROP2_DIR_PIN, LOW);
  delay(200);

  digitalWrite(TRICKLE_ENTRANCE_TRIG_PIN, LOW);
  digitalWrite(TRICKLE_DROP_TRIG_PIN, LOW);
  delay(10);

  digitalWrite(TRICKLE_ENTRANCE_TRIG_PIN, HIGH);
  digitalWrite(TRICKLE_DROP_TRIG_PIN, HIGH);

  delay(15000);




  digitalWrite(TRICKLE_ENTRANCE_DIR_PIN, HIGH);
  digitalWrite(TRICKLE_DROP1_DIR_PIN, HIGH);
  digitalWrite(TRICKLE_DROP2_DIR_PIN, HIGH);
  delay(200);

  digitalWrite(TRICKLE_ENTRANCE_TRIG_PIN, LOW);
  digitalWrite(TRICKLE_DROP_TRIG_PIN, LOW);
  delay(10);

  digitalWrite(TRICKLE_ENTRANCE_TRIG_PIN, HIGH);
  digitalWrite(TRICKLE_DROP_TRIG_PIN, HIGH);

  delay(15000);

}

// --------------------------------------------
//            TRACK SWITCHING LOGIC
//          Logic for trickle charges
// --------------------------------------------

byte handleEntranceRamp(bool isOpen) {
  // set the directions
  digitalWrite(TRICKLE_ENTRANCE_DIR_PIN, isOpen ? HIGH : LOW);
  delay(100);

  // trigger the trickle charge.
  digitalWrite(TRICKLE_DROP_TRIG_PIN, LOW);
  delay(100);
  digitalWrite(TRICKLE_DROP_TRIG_PIN, HIGH);

  return 0x01;
}

byte handleDropRamp(bool isNorth) {
  // set the directions
  digitalWrite(TRICKLE_DROP1_DIR_PIN, isNorth ? HIGH : LOW);
  digitalWrite(TRICKLE_DROP2_DIR_PIN, isNorth ? LOW : HIGH);
  delay(100);
  
  // trigger the trickle charge.
  digitalWrite(TRICKLE_DROP_TRIG_PIN, LOW);
  delay(100);
  digitalWrite(TRICKLE_DROP_TRIG_PIN, HIGH);

  return 0x01;
}