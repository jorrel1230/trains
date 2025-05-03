// arduinoFinal.ino
// Author: Jorrel Rajan

// ------------------------------------------------------------
// Arduino Program that acts an an 'API' for the ACIA to query.
// Maintains internal states that can be requested at any time.
// ------------------------------------------------------------

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
const int HALL1PIN = 9;
const int HALL2PIN = 10;
const int HALL3PIN = 11;
const int HALL4PIN = 12;
const int HALL5PIN = 13;

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
byte color;

// Byte Constants for Communication Protocol 
const byte whiteByte = 0xCA;
const byte blackByte = 0xCB;

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
  pinMode(HALL1PIN, INPUT);
  pinMode(HALL2PIN, INPUT);
  pinMode(HALL3PIN, INPUT);
  pinMode(HALL4PIN, INPUT);
  pinMode(HALL5PIN, INPUT);

  // Set Up Servo Motos with Arduino Digital Pins
  pickupServo.attach(PICKUPSERVOPIN); 
  dropoffServo.attach(DROPOFFSERVOPIN);

  // Set up trickle pins
  pinMode(TRICKLE_ENTRANCE_DIR_PIN, OUTPUT);
  pinMode(TRICKLE_DROP1_DIR_PIN, OUTPUT);
  pinMode(TRICKLE_DROP2_DIR_PIN, OUTPUT);
  pinMode(TRICKLE_ENTRANCE_TRIG_PIN, OUTPUT);
  pinMode(TRICKLE_DROP_TRIG_PIN, OUTPUT);
}

void loop() {

  // Update Color Sensor State
  color = updateColor();

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
  byte returnData;
  
  switch (data) {

    case 0x21:
      returnData = handleEntranceRamp();
      break;
    case 0x22:
      returnData = trickleEntranceTrigger();
    case 0x23:
      returnData = waitHall1();
      break;
    case 0x87:
      returnData = handleDropoffServo();
      break;
    case 0x89:
      returnData = handleColorSensor();
      break;
    default:
      returnData = 0x00;
  }

  return returnData;
}

byte handleColorSensor() {
  return color;
}

byte handlePickupServo() {
  pickupServo.write(0);
  delay(250);
  pickupServo.write(180);
  delay(250);
  pickupServo.write(0);
  delay(250);
  pickupServo.write(180);
  delay(250);

  return 0x01;
}

byte handleDropoffServo() {
  dropoffServo.write(0);
  delay(250);
  dropoffServo.write(180);
  delay(250);
  dropoffServo.write(0);
  delay(250);
  dropoffServo.write(180);
  delay(250);

  return 0x01;
}

byte openEntranceRamp() {
  // Logic for trickle charges
  Serial.println("Entrance Ramp Logic Not implemented yet.");
  return 0x00;
}

byte closeEntranceRamp() {
  // Logic for trickle charges
  Serial.println("Entrance Ramp Logic Not implemented yet.");
  return 0x00;
}

byte handleNorthDrop() {
  // Logic for trickle charges
  Serial.println("North Dropoff Logic Not implemented yet.");
  return 0x00;
}

byte handleSouthDrop() {
  // Logic for trickle charges
  Serial.println("South Dropoff Logic Not implemented yet.");
  return 0x00;
}


byte waitHall1() {
  while (digitalRead(HALL1PIN) == HIGH); // kill time, wait for hall 1 to trigger
  triggerRelay();
  return 0x01;
}

byte triggerRelay() {
  // NOTE: Not implemented. We want to turn off track power for a good 3-5 seconds.
  
  return 0x00
}

// ------------------------------------------------------------
// UPDATER FUNCTIONS: 
// Main purpose of these functions is to updates the Arduino's 
// INTERNAL STATE of a sensor 
// ------------------------------------------------------------

// Updates currently held color in arduino variable
byte updateColor() {
  if (APDS.colorAvailable()) {
    int r, g, b;

    // read the color
    APDS.readColor(r, g, b);

    float grayscale = (0.2126*r + 0.7152*g + 0.0722*b);

    if (grayscale < 150) {
      return blackByte;
    } else {
      return whiteByte;
    }
  }
}

// Updates all hall sensors
void updateHalls() {
  hall1 = (digitalRead(HALL1PIN) == HIGH) ? hallOn : hallOff;
  hall2 = (digitalRead(HALL2PIN) == HIGH) ? hallOn : hallOff;
  hall3 = (digitalRead(HALL3PIN) == HIGH) ? hallOn : hallOff;
  hall4 = (digitalRead(HALL4PIN) == HIGH) ? hallOn : hallOff;
  hall5 = (digitalRead(HALL5PIN) == HIGH) ? hallOn : hallOff;
}





