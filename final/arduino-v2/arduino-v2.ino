// arduinoFinal.ino
// Author: Jorrel Rajan

// Include all necessary libraries
#include <SoftwareSerial.h>
#include <Arduino_APDS9960.h>
#include <Servo.h>

// Initialize Software Serial for ACIA communication
SoftwareSerial mySerial(2, 3); // RX = 2, TX = 3

//.Initialize Servos
Servo pickupServo;
Servo dropoffServo;

// Pin Definitions
const int HALL_PIN = 9;

const int PICKUPSERVOPIN = 5;
const int DROPOFFSERVOPIN = 6;

const int TRICKLE_ENTRANCE_DIR_PIN = A0;
const int TRICKLE_DROP1_DIR_PIN = A1;
const int TRICKLE_DROP2_DIR_PIN = A2;

const int TRICKLE_ENTRANCE_TRIG_PIN = 7;
const int TRICKLE_DROP_TRIG_PIN = 8;

const int TRACK_RELAY_PIN = A3;

// Response and Request Bytes for ACIA
byte req;
byte res;

// states
char color;

// Tunable Parameters in program
const int RELAY_DELAY = 3000; // after hall is triggered, relay turns off track power for how long?
const int COLOR_THRESH = 150; // Light range, from 0-1024.
const int LEAVE_HALL_DELAY = 2000; // after marble dropped off, how long after hall 1 do we wait before setting tracks straight?


void setup() {
  // Initialize both Hardware and Software Serial.
  // Keep Hardware Serial plugged in for debugging purposes.
  Serial.begin(9600);
  mySerial.begin(9600);

  // Set Up Color Sensor with I2C.
  if (!APDS.begin()) {
    Serial.println("Error initializing APDS-9960 sensor.");
  } else {
    Serial.println("Color Sensor Initialized!");
  }

  // Set up Hall Effect Sensors with Arduino Digital Pins.
  pinMode(HALL_PIN, INPUT);

  // Set Up Servo Motos with Arduino Digital Pins
  pickupServo.attach(PICKUPSERVOPIN, 1000, 2000); 
  dropoffServo.attach(DROPOFFSERVOPIN, 1000, 2000);

  // Set up trickle pins
  pinMode(TRICKLE_ENTRANCE_DIR_PIN, OUTPUT);
  pinMode(TRICKLE_DROP1_DIR_PIN, OUTPUT);
  pinMode(TRICKLE_DROP2_DIR_PIN, OUTPUT);
  pinMode(TRICKLE_ENTRANCE_TRIG_PIN, OUTPUT);
  pinMode(TRICKLE_DROP_TRIG_PIN, OUTPUT);

  // Set up relay trigger
  pinMode(TRACK_RELAY_PIN, OUTPUT);

  // Set all initial states
  color = 'N'; // not found yet
  handleEntranceRamp(true); // make entrance ramp tracks initially straight
  pickupServo.write(0);
  dropoffServo.write(90);
}

void loop() {
  // Read data from serial.read
  if(mySerial.available() > 0) {
    // Indicate we received something, then read it
    req = mySerial.read();
    res = handleACIA(req);
    mySerial.write(res);
    Serial.print("Req: ");
    Serial.print(int(req));
    Serial.print(" | Res: ");
    Serial.println(int(res));
  }
  delay(10);
}

// ------------------------------------------------------------
// Handler Functions
// Manages ACIA Requests
// ------------------------------------------------------------

byte handleACIA(byte data) {
  switch (data) {
    case 0x21:
      return waitAndCut();
      break;
    case 0x22:
      return pickupRoutine();
      break;
    case 0x23:
      return dropoffRoutine();
      break;
    default:
      return 0x00;
  }
}

// -----------------------------------------------
//    Handle Color Sensing and track switching
// -----------------------------------------------

byte pickupRoutine() {
  // Close the tracks
  handleEntranceRamp(false);

  // Move Servo to start position
  pickupServo.write(0);
  delay(1500);

  // Move Servo to color detector
  for (int i = 0; i <= 90; i++) {
    pickupServo.write(i);
    delay(10);
  }
  pickupServo.write(90);

  // Detect Color, trigger the right track.
  if (isBlackBall()) {
    handleDropRamp(false);
  } else {
    handleDropRamp(true);
  }

  // Move Servo to drop
  for (int i = 90; i <= 180; i++) {
    pickupServo.write(i);
    delay(10);
  }
  pickupServo.write(180);

  delay(1000); // experimental delay
  //
  // Move Servo to normal
  for (int i = 180; i >= 0; i++) {
    pickupServo.write(i);
    delay(5);
  }
  pickupServo.write(0);

  return (color == 'B' ? 0xC0 : 0xCF);
}

byte dropoffRoutine() {
  // servo control thingies needed here.
  if (color == 'B') {
    for (int i = 90; i <= 180; i++) {
      dropoffServo.write(i);
      delay(10);
    }
    for (int i = 180; i >= 90; i--) {
      dropoffServo.write(i);
      delay(10);
    }
  } else {
    for (int i = 90; i >= 0; i--) {
      dropoffServo.write(i);
      delay(10);
    }
    for (int i = 0; i <= 90; i++) {
      dropoffServo.write(i);
      delay(10);
    }
  }

  // After a delay. make tracks straight.
  waitHall();
  delay(LEAVE_HALL_DELAY);
  handleEntranceRamp(true);

  return 0x01;
}

// --------------------------------------------
//            TRACK SWITCHING LOGIC
//          Logic for trickle charges
// --------------------------------------------

byte handleEntranceRamp(bool isOpen) {
  // set the directions
  digitalWrite(TRICKLE_ENTRANCE_DIR_PIN, isOpen ? HIGH : LOW);
  delay(25);

  // trigger the trickle charge.
  digitalWrite(TRICKLE_DROP_TRIG_PIN, LOW);
  delay(25);
  digitalWrite(TRICKLE_DROP_TRIG_PIN, HIGH);

  return 0x01;
}

byte handleDropRamp(bool isNorth) {
  // set the directions
  digitalWrite(TRICKLE_DROP1_DIR_PIN, isNorth ? HIGH : LOW);
  digitalWrite(TRICKLE_DROP2_DIR_PIN, isNorth ? LOW : HIGH);
  delay(25);
  
  // trigger the trickle charge.
  digitalWrite(TRICKLE_DROP_TRIG_PIN, LOW);
  delay(25);
  digitalWrite(TRICKLE_DROP_TRIG_PIN, HIGH);

  return 0x01;
}

// -------------------------------------------------
// HANDLERS FOR HALL EFFECT AND TRACK RELAY:
// -------------------------------------------------
byte waitAndCut() {
  waitHall();
  cutTrackPower();
  return 0x01;
}

// Stands by until a hall effect sensor goes low.
void waitHall() {
  while (digitalRead(HALL_PIN) == HIGH); // kill time, wait for hall 1 to trig
}

void cutTrackPower() {
  digitalWrite(TRACK_RELAY_PIN, LOW);
  delay(RELAY_DELAY);
  digitalWrite(TRACK_RELAY_PIN, HIGH);
}

// ------------------------------------------------------------
// UPDATER FUNCTIONS: 
// Main purpose of these functions is to updates the Arduino's 
// INTERNAL STATE of a sensor 
// ------------------------------------------------------------

// Updates currently held color in arduino variable
bool sensorIsBlackBall() {
  while (!APDS.colorAvailable());
  
  int r, g, b;

  // read the color
  APDS.readColor(r, g, b);

  float grayscale = (0.2126*r + 0.7152*g + 0.0722*b);

  return grayscale < COLOR_THRESH;
}

bool isBlackBall() {
  if (color == 'N') {
    color = sensorIsBlackBall() ? 'B' : 'W';
  }
  return color == 'B';
}

byte clearColorState() {
  color = 'N';
  return 0x01;
}
