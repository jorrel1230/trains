#include <Arduino_APDS9960.h>

const int COLOR_THRESH = 150;
void setup() {
  Serial.begin(9600);

 // Set Up Color Sensor with I2C.
  if (!APDS.begin()) {
    Serial.println("Error initializing APDS-9960 sensor.");
  } else {
    Serial.println("Color Sensor Initialized!");
  }

}

void loop() {
  while (!APDS.colorAvailable());
  int r, g, b;
  // read the color
  APDS.readColor(r, g, b);
  float grayscale = (0.2126*r + 0.7152*g + 0.0722*b);

  Serial.println(grayscale);
}