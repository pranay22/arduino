// -----------------------------------------------------------------------------
/*
  PCF8574 I2C Module

  I2C to 8-bit Parallel-Port Expander

  Module adjustable pin address settings:
   --------------
  |              |
  |              | 0 is up   (-)
  | A2 A1 A0     | 1 is down (+)
  | 0  0  0      |  = 0x20
  | 1  0  0      |  = 0x21, A2 is connected down, the others up.
  | 0  1  0      |  = 0x22
  | 1  1  0      |  = 0x23
  | 0  0  1      |  = 0x24
  | 1  0  1   P0 |  = 0x25
  | 0  1  1   P1 |  = 0x26
  | 1  1  1   P2 |  = 0x27
  |           P3 |
  |           P4 |
  |           P5 |
  |           P6 |
  |  V G S S  P7 |
  |  C N D C INT | For multiple, I used a diode to each.
  |  C D A L     |
   --------------
     | | | |
     
  Wiring:
  + SDA to Nano A4.
  + SCL to Nano A5.
  + GND to Nano GND
  + VCC to Nano 5V
  + INT to interrupt pin, pin 2 on Nano, in this sample program.
  + P0 ... O7 to switches. Other side of the switch to ground.

  Library:
    https://github.com/RobTillaart/Arduino/tree/master/libraries/PCF8574
  Sample switch/button program:
    https://github.com/RobTillaart/Arduino/blob/master/libraries/PCF8574/examples/buttonRead/buttonRead.ino
  Reference:
    https://forum.arduino.cc/index.php?topic=204596.msg1506639#msg1506639
  Information and code sample:
    https://protosupplies.com/product/pcf8574-i2c-i-o-expansion-module/

  Example statements:
    uint8_t value = pcf20.read8();
    Serial.println(pcf20.read8(), BIN);

  Note, uint8_t is the same as a byte. it's shorthand for an unsigned integer of length 8 bits.
    byte aByteBit;
    --- or ---
    uint8_t aByteBit;
*/
// -----------------------------------------------------------------------------
#define SWITCH_MESSAGES 1
bool runProgram = false;
byte dataByte;
void printByte(byte b) {
  for (int i = 7; i >= 0; i--)
    Serial.print(bitRead(b, i));
}

// -----------------------------------------------------------------------------
#include <PCF8574.h>
#include <Wire.h>

PCF8574 pcf20(0x020);

// Interrupt setup: interrupt pin to use, interrupt handler routine.
//                   Mega pin for control toggle interrupt. Same pin for Nano.
const int INTERRUPT_PIN = 2;
boolean switchSetOn = false;
void pcfinterrupt() {
  switchSetOn = true;
}

// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  // Give the serial connection time to start before the first print.
  delay(1000);
  Serial.println(""); // Newline after garbage characters.
  Serial.println("+++ Setup.");

  // ------------------------------
  // I2C Two Wire + interrupt initialization
  pcf20.begin();
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), pcfinterrupt, CHANGE);
  Serial.println("+ PCF module initialized.");

  // ------------------------------
  Serial.println("+++ Go to loop.");
}

// -----------------------------------------------------------------------------
// Device Loop
void loop() {

  if (switchSetOn) {
    // ----------------------
    Serial.println("+ Interrupt call, switchSetOn is true.");
    dataByte = pcf20.read8();                   // Read all PCF8574 inputs
    Serial.print("+ PCF8574 0x20 byte, read8      = ");
    printByte(dataByte);
    Serial.println("");
    // ----------------------
    Serial.print("+ PCF8574 0x20 byte, readButton = ");
    for (int pinGet = 7; pinGet >= 0; pinGet--) {
      int pinValue = pcf20.readButton(pinGet);  // Read each PCF8574 input
      Serial.print(pinValue);
    }
    // ----------------------
    Serial.println("");
    switchSetOn = false;                        // Reset for the next switch event.
  }

  delay (60);
}
// -----------------------------------------------------------------------------