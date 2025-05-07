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

int droppedCount = 0;


// states
char color;

// Tunable Parameters in program
const int RELAY_DELAY = 3000; // after hall is triggered, relay turns off track power for how long?
const int INTERMAGNET_RELAY_DELAY = 350; 
const int RAPID_RELAY_DELAY = 0; 

const int COLOR_THRESH = 20; // Light range, from 0-1024.
const int LEAVE_HALL_DELAY = 2000; // after marble dropped off, how long after hall 1 do we wait before setting tracks straight?

const float pickupServoStart = 14.5;
const float pickupServoStop = 177;
const float pickupServoMid = 95;

const float dropoffServoStart = 90;
const float dropoffServoStop = 175;

const bool isTesting = true;

void setup() {
  // Initialize both Hardware and Software Serial.
  // Keep Hardware Serial plugged in for debugging purposes.
  Serial.begin(9600);
  mySerial.begin(9600);

  // Set up Hall Effect Sensors with Arduino Digital Pins.
  pinMode(HALL_PIN, INPUT);

  // Set Up Servo Motos with Arduino Digital Pins
  pickupServo.attach(PICKUPSERVOPIN, 400, 2600); 
  dropoffServo.attach(DROPOFFSERVOPIN, 500, 2500);

  // Set up trickle pins
  pinMode(TRICKLE_ENTRANCE_DIR_PIN, OUTPUT);
  pinMode(TRICKLE_DROP1_DIR_PIN, OUTPUT);
  pinMode(TRICKLE_DROP2_DIR_PIN, OUTPUT);
  pinMode(TRICKLE_ENTRANCE_TRIG_PIN, OUTPUT);
  pinMode(TRICKLE_DROP_TRIG_PIN, OUTPUT);

  // Set up relay trigger
  pinMode(TRACK_RELAY_PIN, OUTPUT);

  // make sure trickles are not triggering
  digitalWrite(TRICKLE_ENTRANCE_TRIG_PIN, HIGH);
  digitalWrite(TRICKLE_DROP_TRIG_PIN, HIGH);
  digitalWrite(TRACK_RELAY_PIN, HIGH);

  Serial.println("Ready");
}

void loop() {
  // Read data from serial.read
  if(mySerial.available() > 0) {
    // Indicate we received something, then read it
    req = mySerial.read();
    res = handleACIA(req);
    if (res != 0x00) {
      mySerial.write(res);
    }
    // Serial.print("Req: ");
    // Serial.print(int(req));
    // Serial.print(" | Res: ");
    // Serial.println(int(res));
  }
  delay(5);
}

// ------------------------------------------------------------
// Handler Functions
// Manages ACIA Requests
// ------------------------------------------------------------

byte handleACIA(byte data) {
  switch (data) {
    case 0x20:
      Serial.println("Init called");
      return initArduino();
      break;
    case 0x21:
      Serial.println("Wait Hall called.");
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
      Serial.println("Wait Hall, with precise stopping");
      return waitHallStop();
    case 0x26:
      Serial.println("cutting track power.");
      return cutTrackPower();
      break;
    case 0x27:
      Serial.println("starting track power");
      return startTrackPower();
      break;
    default:
      return 0x00;
  }
}

byte initArduino() {
  // Set Up Color Sensor with I2C.
  if (!APDS.begin()) {
    Serial.println("Error initializing APDS-9960 sensor.");
  } else {
    Serial.println("Color Sensor Initialized!");
  }

  // Set all initial states
  color = 'N'; // not found yet

  pickupServo.write(pickupServoStart);
  dropoffServo.write(dropoffServoStart);

  delay(10000);
  handleEntranceRamp(true); // make entrance ramp tracks initially straight

  return 0x01;
}

// -----------------------------------------------
//    Handle Color Sensing and track switching
// -----------------------------------------------

byte pickupRoutine() {


  // Move Servo to start position, wiggle.
  pickupServo.write(pickupServoStart);
  delay(1000);
  for (int i = 0; i < 10; i++) {
    pickupServo.write(pickupServoStart-3);
    delay(100);
    pickupServo.write(pickupServoStart+3);
    delay(100);
  }
  for (int i = 0; i < 10; i++) {
    pickupServo.write(pickupServoStart-1);
    delay(100);
    pickupServo.write(pickupServoStart+1);
    delay(100);
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
    Serial.println("Black");
  } else {
    handleDropRamp(true);
    Serial.println("white");
  }

  // Move Servo to drop, wiggle.
  for (int i = pickupServoMid; i <= pickupServoStop; i++) {
    pickupServo.write(i);
    delay(20);
  }
  pickupServo.write(pickupServoStop);
  delay(500);

  for (int i = 0; i < 10; i++) {
    pickupServo.write(pickupServoStop-3);
    delay(100);
    pickupServo.write(pickupServoStop+3);
    delay(100);
  }

  for (int i = 0; i < 10; i++) {
    pickupServo.write(pickupServoStop-1);
    delay(100);
    pickupServo.write(pickupServoStop+1);
    delay(100);
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
  for (int i = dropoffServoStart; i <= dropoffServoStop; i += (dropoffServoStop - dropoffServoStart) / 8) {
    dropoffServo.write(i);
    delay(500);
  }

  for (int i = 0; i < 10; i++) {
    dropoffServo.write(dropoffServoStop-5);
    delay(15);
    dropoffServo.write(dropoffServoStop+5);
    delay(15);
  }

  for (int i = dropoffServoStop; i >= dropoffServoStart; i--) {
    dropoffServo.write(i);
    delay(10);
  }

  droppedCount += 1;
  clearColorState(); 

  // dropped count greater than 1 at this point for sure.
  if (droppedCount < 10) return byte(droppedCount);
  else return 0x00;
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

// Stands by until a hall effect sensor goes low.
byte waitHall() {
  while (digitalRead(HALL_PIN) == HIGH); // kill time, wait for hall 1 to trig
  delay(RAPID_RELAY_DELAY);
  digitalWrite(TRACK_RELAY_PIN, LOW);
  delay(INTERMAGNET_RELAY_DELAY);
  digitalWrite(TRACK_RELAY_PIN, HIGH);

  while (digitalRead(HALL_PIN) == HIGH); // kill time, wait for hall 1 to trig
  Serial.println("2nd magnet detected!");
  return 0x01;
}

// Stands by until a hall effect sensor goes low.
byte waitHallStop() {
  while (digitalRead(HALL_PIN) == HIGH); // kill time, wait for hall 1 to trig

  delay(RAPID_RELAY_DELAY);
  digitalWrite(TRACK_RELAY_PIN, LOW);
  delay(INTERMAGNET_RELAY_DELAY);

  unsigned long lastToggleTime;
  const unsigned long interval = 500;

  bool isRelayCurrentlyLow = true; // State variable: true if relay is LOW, false if HIGH
  lastToggleTime = millis();       // Record the time of this first action

  // Loop as long as the HALL_PIN is HIGH.
  while (digitalRead(HALL_PIN) == HIGH) {
    unsigned long currentMillis = millis(); // Get the current time

    // Check if the 'interval' has passed since the last toggle
    if (currentMillis - lastToggleTime >= interval) {
      if (isRelayCurrentlyLow) {
        // If it was LOW, switch to HIGH (this was the second action in your original loop segment)
        digitalWrite(TRACK_RELAY_PIN, HIGH);
        isRelayCurrentlyLow = false;
      } else {
        // If it was HIGH, switch to LOW (this would be the start of the next cycle's first action)
        digitalWrite(TRACK_RELAY_PIN, LOW);
        isRelayCurrentlyLow = true;
      }
      lastToggleTime = currentMillis; // Reset the timer for the new state
    }
  }
  
  return cutTrackPower();
}

byte cutTrackPower() {
  digitalWrite(TRACK_RELAY_PIN, LOW);
  return 0x01;
}

byte startTrackPower() {
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
  float avg_grayscale = 0;
  const int READINGS = 10;
  for (int i = 0; i < READINGS; i++) {
    APDS.readColor(r, g, b);
    avg_grayscale += (0.2126*r + 0.7152*g + 0.0722*b) / READINGS;
    delay(100);
  }
  Serial.println(avg_grayscale);
  return avg_grayscale < COLOR_THRESH;
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