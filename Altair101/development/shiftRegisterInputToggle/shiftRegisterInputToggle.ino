// -----------------------------------------------------------------------------
/*
  Shifting to Expand I/O.

  Using a 74HC595 Shift Register, serial to parallel chip, to read multiple pin inputs.
  Uses 4 pins on a microcontroller.

  Wiring,
  + 8 buttons, one side of each are connected to the Nano data input pin (dataInputPin).
  + Attach a 10K pull down resister to the data input pin: 10K to ground from the data input pin.
  + The other side of each button goes through a diode to a sift register output pin.
  ++ Diodes prevent shorts when multiple key pressed at the same time.

  Program works as follows,
  + Shift out 255, to turn the all shift register outputs high: Pins: 15, 1-7 are high.
  ++ Since no button is pressed. the Nano data input pin is LOW.
  + When a button is pressed, the data input pin goes HIGH.
  ++ Note, as an alternative, can use an interupt to be notified that a key was pressed.
  + Then scan each button by shifting one HIGH bit through each shift register output pin.
  ++ A HIGH value indicates which button was pressed.
    Shift out B10000000. If HIGH on the Nano pin, button connected to pin 15 was pressed.
    Shift out B01000000. If HIGH on the Nano pin, button connected to pin 1 was pressed.
    Shift out B00100000. If HIGH on the Nano pin, button connected to pin 2 was pressed.
    Shift out B00010000. If HIGH on the Nano pin, button connected to pin 3 was pressed.
    Shift out B00001000. If HIGH on the Nano pin, button connected to pin 4 was pressed.
    Shift out B00000100. If HIGH on the Nano pin, button connected to pin 5 was pressed.
    Shift out B00000010. If HIGH on the Nano pin, button connected to pin 6 was pressed.
    Shift out B00000001. If HIGH on the Nano pin, button connected to pin 7 was pressed.
  + A complete scan will indicate which buttons ar pressed.

  + The above technique works when using 74HC595 for inputs. The basic idea is from the following:
  https://www.youtube.com/watch?v=nXl4fb_LbcI
  https://www.youtube.com/watch?v=1Vb2BpxPnjU
  + Link where I found the video links:
  https://forum.arduino.cc/index.php?topic=163813.0

  + Another method is to use 74HC165 for inputs, which is parallel to serial:
  https://www.youtube.com/watch?v=hR6qOhUeKMc
  https://www.theengineeringprojects.com/2018/11/arduino-74hc165-interfacing-increase-input-pins.html
*/
// -----------------------------------------------------------------------------
void printByte(byte b) {
  for (int i = 7; i >= 0; i--)
    Serial.print(bitRead(b, i));
}

// -----------------------------------------------------------------------------
// Shift Register

const int dataPin = 7;            // 74HC595 Data  pin 14 is connected to Digital pin 7
const int latchPin = 8;           // 74HC595 Latch pin 12 is connected to Digital pin 8
const int clockPin = 9;           // 74HC595 Clock pin 11 is connected to Digital pin 9
const int dataInputPin = A0;      // Nano data input check pin. Digital pins work and some analog pins work.

// -----------------------------------------------------------------------------
// Only do the action once, don't repeat if the button is held down.
// Don't repeat action if the button is not pressed.

const int numberOfToogles = 8;
byte toggleAddressByte = B00000000;
void getToogleAddress() {
  toggleAddressByte = B00000000;
  byte toggleAddressBit = B10000000;
  for (int i = 0; i < numberOfToogles; i++) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, toggleAddressBit);
    digitalWrite(latchPin, HIGH);
    if (digitalRead(dataInputPin) == HIGH) {
      toggleAddressByte = toggleAddressByte | toggleAddressBit;
    }
    toggleAddressBit = toggleAddressBit >> 1;
  }
}

// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);        // Give the serial connection time to start before the first print.
  Serial.println(""); // Newline after garbage characters.
  Serial.println("+++ Setup.");

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(dataInputPin, INPUT);
  Serial.println("+ Ready to check toogles.");

  Serial.println("+++ Start program loop.");
}

// -----------------------------------------------------------------------------
// Device Loop.

void loop() {
  Serial.print("+ getToogleAddress = ");
  getToogleAddress();
  printByte(toggleAddressByte);
  Serial.println("");
  delay(3000);
}
// -----------------------------------------------------------------------------
