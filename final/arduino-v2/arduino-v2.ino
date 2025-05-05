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
const int HALL_PIN = 10;

const int PICKUPSERVOPIN = 6;
const int DROPOFFSERVOPIN = 5;

const int TRICKLE_ENTRANCE_DIR_PIN = A2;
const int TRICKLE_DROP1_DIR_PIN = A1;
const int TRICKLE_DROP2_DIR_PIN = A0;

const int TRICKLE_ENTRANCE_TRIG_PIN = 8;
const int TRICKLE_DROP_TRIG_PIN = 7;

const int TRACK_RELAY_PIN = A3;

// Response and Request Bytes for ACIA
byte req;
byte res;

// states
char color;

// Tunable Parameters in program
const int RELAY_DELAY = 3000; // after hall is triggered, relay turns off track power for how long?
const int COLOR_THRESH = 10; // Light range, from 0-1024.
const int LEAVE_HALL_DELAY = 2000; // after marble dropped off, how long after hall 1 do we wait before setting tracks straight?

const float pickupServoStart = 4.9;
const float pickupServoStop = 172.5;
const float pickupServoMid = (pickupServoStart + pickupServoStop) / 2;

const float dropoffServoStart = 90;
const float dropoffServoStop = 172.5;

const bool isTesting = true;

//??OUIhgevpoiaerhbgvohriegvpihrepriuh
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
  pickupServo.attach(PICKUPSERVOPIN, 500, 2700); 
  dropoffServo.attach(DROPOFFSERVOPIN, 500, 2500);

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

  // make sure trickles are not triggering
  digitalWrite(TRICKLE_ENTRANCE_TRIG_PIN, HIGH);
  digitalWrite(TRICKLE_DROP_TRIG_PIN, HIGH);
  digitalWrite(TRACK_RELAY_PIN, HIGH);

  pickupServo.write(pickupServoStart);
  dropoffServo.write(dropoffServoStart);
  Serial.println("Ready");
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
    Serial.println("Waiting for hall..");
      return waitHall();
      break;
    case 0x22:
      Serial.println("Pickup Routine Called!");
      return pickupRoutine();
      break;
    case 0x23:
      Serial.println("Dropoff Routine Called!");
      return dropoffRoutine();
      break;
    case 0x24:
      Serial.println("Setting outer tracks straight.");
      handleEntranceRamp(true);
      return 0x01;
      break;
    case 0x25:
    Serial.println("cutting track power.");
      return cutTrackPower();
      break;
    default:
      return 0x00;
  }
}

// -----------------------------------------------
//    Handle Color Sensing and track switching
// -----------------------------------------------

byte pickupRoutine() {


  // Move Servo to start position, wiggle.
  pickupServo.write(pickupServoStart);
  delay(1000);
  for (int i = 0; i < 10; i++) {
    pickupServo.write(pickupServoStart-1);
    delay(50);
    pickupServo.write(pickupServoStart+1);
    delay(50);
  }
  for (int i = 0; i < 10; i++) {
    pickupServo.write(pickupServoStart-2);
    delay(50);
    pickupServo.write(pickupServoStart+2);
    delay(50);
  }

  // Move Servo to color detector
  for (int i = pickupServoStart; i <= pickupServoMid; i++) {
    pickupServo.write(i);
    delay(20);
  }
  pickupServo.write(pickupServoMid);
  delay(500);

  // Detect Color, trigger the right track.
  if (isBlackBall()) {
    handleDropRamp(false);
  } else {
    handleDropRamp(true);
  }

  // Move Servo to drop, wiggle.
  for (int i = pickupServoMid; i <= pickupServoStop; i++) {
    pickupServo.write(i);
    delay(20);
  }
  pickupServo.write(pickupServoStop);
  delay(500);
  for (int i = 0; i < 10; i++) {
    pickupServo.write(pickupServoStop-1);
    delay(50);
    pickupServo.write(pickupServoStop+1);
    delay(50);
  }
  for (int i = 0; i < 10; i++) {
    pickupServo.write(pickupServoStop-2);
    delay(50);
    pickupServo.write(pickupServoStop+2);
    delay(50);
  }

  // Move Servo to normal
  for (int i = pickupServoStop; i >= pickupServoStart; i--) {
    pickupServo.write(i);
    delay(15);
  }
  pickupServo.write(pickupServoStart);

  
  // Close the tracks
  handleEntranceRamp(false);

  return (color == 'B' ? 0xC0 : 0xCF);
}

byte dropoffRoutine() {
  // servo control thingies needed here.
  for (int i = dropoffServoStart; i <= dropoffServoStop; i++) {
    dropoffServo.write(i);
    delay(10);
  }
  for (int i = dropoffServoStop; i >= dropoffServoStart; i--) {
    dropoffServo.write(i);
    delay(10);
  }

  // After a delay. make tracks straight.
  waitHall();
  delay(LEAVE_HALL_DELAY);
  handleEntranceRamp(false);

  return 0x01;
}

// --------------------------------------------
//            TRACK SWITCHING LOGIC
//          Logic for trickle charges
// --------------------------------------------

byte handleEntranceRamp(bool isOpen) {
  digitalWrite(TRICKLE_ENTRANCE_TRIG_PIN, HIGH);

  // set the directions
  digitalWrite(TRICKLE_ENTRANCE_DIR_PIN, isOpen ? LOW : HIGH);
  delay(200);

  // trigger the trickle charge.
  digitalWrite(TRICKLE_ENTRANCE_TRIG_PIN, LOW);
  delay(10);
  digitalWrite(TRICKLE_ENTRANCE_TRIG_PIN, HIGH);

  return 0x01;
}

byte handleDropRamp(bool isNorth) {
  digitalWrite(TRICKLE_DROP_TRIG_PIN, HIGH);

  // set the directions
  digitalWrite(TRICKLE_DROP1_DIR_PIN, isNorth ? HIGH : LOW);
  digitalWrite(TRICKLE_DROP2_DIR_PIN, isNorth ? LOW : HIGH);
  delay(200);
  
  // trigger the trickle charge.
  digitalWrite(TRICKLE_DROP_TRIG_PIN, LOW);
  delay(10);
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
byte waitHall() {
  while (digitalRead(HALL_PIN) == HIGH); // kill time, wait for hall 1 to trig
  Serial.println("magnet detected!");
  return 0x01;
}

byte cutTrackPower() {
  digitalWrite(TRACK_RELAY_PIN, LOW);
  delay(RELAY_DELAY);
  digitalWrite(TRACK_RELAY_PIN, HIGH);
  return 0x01;
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
