/*
 * Author: Pierre-Yves Taunay
 * Date: November 2018
 * MAE 412 ACIA/Arduino communication
 * This is the Arduino part of the ACIA/Arduino Serial communication
 */

const int LEDPIN = 3;
const int NBLINK = 5;
int counter = 0;
int data = 0;

/*
 * Function: setup
 * Description: 
 * 1. sets up the baud rate, 
 * 2. initializes an LED pin for debugging 
 * 3. resets the initial counter to 0
 */
void setup() {
  Serial.begin(9600);
  pinMode(LEDPIN,OUTPUT);
  counter = 0;
}

/* 
 *  Function: send_counter
 *  Description: sends the variable "counter" away through serial comm.
 *  The data is passed as a reference; const is used to avoid modifying counter.
 *  Input: 
 *  - counter: data to send
 */
void send_counter(const int &counter) {
      // Indicate we are sending something
      for(int cnt = 0; cnt < NBLINK; ++cnt) {
        digitalWrite(LEDPIN,LOW);
        delay(100);
        digitalWrite(LEDPIN,HIGH);
      }     
      // Send the data away    
      Serial.write(counter);   
}

/*
 * Function: loop
 * Description: waits for data to be available on the serial bus
 * If the data received is 0x89, increment the counter
 * If the data received is 0x88, reset the counter
 * Otherwise, ignore
 * If the counter gets to 10, reset it to 0
 */
void loop() {
  // Read data from serial.read
  if(Serial.available() > 0) {
    // Indicate we received something, then read it
    digitalWrite(LEDPIN,HIGH); 
    data = Serial.read();

    // If we receceive 0x89 or 0x88 from the ACIA...
    if (data == 0x88) {
      // Otherwise we reset the counter
      counter = 0;
      send_counter(counter);
    } else if(data == 0x89) { 
      // Then we increment the counter
      counter++;
      if(counter == 10)
        counter = 0;

      send_counter(counter); 
    }
    digitalWrite(LEDPIN,LOW);
  }
  delay(10);
}

