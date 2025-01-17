/*
  Blink
  http://www.arduino.cc/en/Tutorial/Blink
*/

#define LED_PIN 10

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pins for output.
  //pinMode(LED_PIN + 0, OUTPUT);
  pinMode(LED_PIN + 3, OUTPUT);
  pinMode(LED_PIN + 2, OUTPUT);
  pinMode(LED_PIN + 1, OUTPUT);
}

// Loop runs continually
void loop() {
  digitalWrite(LED_PIN + 3, HIGH);   // On
  delay(500);                       // wait a second
  digitalWrite(LED_PIN + 3, LOW);    // Off
  digitalWrite(LED_PIN + 2, HIGH);
  delay(1000);
  digitalWrite(LED_PIN + 2, LOW);
  digitalWrite(LED_PIN + 1, HIGH);
  delay(500);
  digitalWrite(LED_PIN + 1, LOW);
  digitalWrite(LED_PIN + 0, HIGH);
  delay(1000);
  digitalWrite(LED_PIN + 0, LOW);
  
}
