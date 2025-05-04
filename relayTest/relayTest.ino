#include <SoftwareSerial.h>

const int TRACK_RELAY_PIN = A3;

const int RELAY_DELAY = 3000; // after hall is triggered, relay turns off track power for how long?

SoftwareSerial mySerial(2, 3); // RX = 2, TX = 3

void setup() {
  // Initialize both Hardware and Software Serial.
  // Keep Hardware Serial plugged in for debugging purposes.
  Serial.begin(9600);
  mySerial.begin(9600);

  // Set up relay trigger
  pinMode(TRACK_RELAY_PIN, OUTPUT);
}

void loop() {
  cutTrackPower();
  delay(5000);
}


void cutTrackPower() {
  digitalWrite(TRACK_RELAY_PIN, LOW);
  Serial.println("off");
  delay(RELAY_DELAY);
  digitalWrite(TRACK_RELAY_PIN, HIGH);
  Serial.println("on");
}
