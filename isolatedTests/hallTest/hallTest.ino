const int HALL_PIN = 9;

void setup() {
  Serial.begin(9600);
  pinMode(HALL_PIN, INPUT);
}

void loop() {
  Serial.println(digitalRead(HALL_PIN));
}

