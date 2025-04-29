#include <SoftwareSerial.h>
#include <Arduino_APDS9960.h>


SoftwareSerial mySerial(2, 3);

// Response and Request Bytes for ACIA
byte req;
byte res;
byte color;

const byte whiteByte = 0xCA;
const byte blackByte = 0xCB;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  if (!APDS.begin()) {
    Serial.println("Error initializing APDS-9960 sensor.");
  } else {
    Serial.println("Color Sensor Initialized!");
  }
}

void loop() {

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

byte handleACIA(byte data) {
  byte returnData;
  
  switch (data) {
    case 0x00: 
      // Designated Clear Command
      returnData = 0x01;
      break;
    case 0x89:
      returnData = handleColorSensor();
      break;
    case 0x81:
      returnData = handleHallEffect1();
      break;
    case 0x82:
      returnData = handleHallEffect2();
      break;
    case 0x83:
      returnData = handleHallEffect3();
      break;
    case 0x84:
      returnData = handleHallEffect4();
      break;
    case 0x85:
      returnData = handleHallEffect5();
      break;
    default:
      returnData = 0x00;
  }

  return returnData;
}

byte handleColorSensor() {
  return color;
}

byte handleHallEffect1() {
  return 0x11;
}

byte handleHallEffect2() {
  return 0x22;
}

byte handleHallEffect3() {
  return 0x33;
}

byte handleHallEffect4() {
  return 0x44;
}

byte handleHallEffect5() {
  return 0x55;
}

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
