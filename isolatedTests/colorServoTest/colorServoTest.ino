#include <Arduino_APDS9960.h>
#include <Servo.h>

const int PICKUPSERVOPIN = 6;
const int COLOR_THRESH = 10;

//.Initialize Servos
Servo pickupServo;
Servo dropoffServo;

const float start = 5;
const float stop = 172.5;
const float mid = (start + stop) / 2;

void setup() {
  Serial.begin(9600);

 // Set Up Color Sensor with I2C.
  if (!APDS.begin()) {
    Serial.println("Error initializing APDS-9960 sensor.");
  } else {
    Serial.println("Color Sensor Initialized!");
  }

  pickupServo.attach(PICKUPSERVOPIN, 500, 2700); 
}

void loop() {
  pickupServo.write(start);
  delay(500);

  for (int i = 0; i < 10; i++) {
    pickupServo.write(start-1);
    delay(50);
    pickupServo.write(start+1);
    delay(50);
  }

  for (int pos = start; pos <= mid; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    pickupServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(20);                       // waits 15ms for the servo to reach the position
  }

  pickupServo.write(mid);
  delay(500);

  
  while (!APDS.colorAvailable());
  int r, g, b;
  // read the color
  APDS.readColor(r, g, b);
  float grayscale = (0.2126*r + 0.7152*g + 0.0722*b);

  Serial.println("Reading: ");
  for (int i = 0; i < 5; i++) {
    Serial.println(grayscale);
    delay(100);
  }

   for (int pos = mid; pos <= stop; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    pickupServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(20);                       // waits 15ms for the servo to reach the position
  }

  pickupServo.write(stop);
  delay(500);

  for (int i = 0; i < 10; i++) {
    pickupServo.write(stop-1);
    delay(50);
    pickupServo.write(stop+1);
    delay(50);
  }
  for (int i = 0; i < 10; i++) {
    pickupServo.write(stop-2);
    delay(50);
    pickupServo.write(stop+2);
    delay(50);
  }


  for (int pos = stop; pos >= start; pos -= 1) { // goes from 180 degrees to 0 degrees
    pickupServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  


}