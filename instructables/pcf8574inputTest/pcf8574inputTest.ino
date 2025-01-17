// -----------------------------------------------------------------------------
/*
  PCF8574 I2C Module, I2C to 8-bit Parallel-Port Expander

  Program to test switch controls for PCF8574 module.
  Not testing with interrupt option.

  Module with adjustable pin address settings:
   --------------
  |  V G S S     | Above are female pins
  |  C N D C     |
  |  C D A L     |
  |              | 0 is up   (-)
  |     A2 A1 A0 | 1 is down (+), toward the male pins at the bottom.
  |     0  0  0  |  = 0x20
  |     0  0  1  |  = 0x21, A0 is connected down, the others up.
  |     0  1  0  |  = 0x22
  |     0  1  1  |  = 0x23
  |     1  0  0  |  = 0x24
  | P0  1  0  1  |  = 0x25
  | P1  1  1  0  |  = 0x26
  | P2  1  1  1  |  = 0x27
  | P3           |
  | P4           |
  | P5           |
  | P6           |
  | P7           |
  | INT          | Interrupt pin is not used in this program.
  |              |
  |  V G S S     | Below are male pins
  |  C N D C     |
  |  C D A L     |
   --------------
     | | | |

  Wiring to an Arduino such as a Nano or Mega:
  + SDA to Arduino A4.
  + SCL to Arduino A5.
  + GND to Arduino GND
  + VCC to Arduino 5V
  + P0 ... O7 to switches. Other side of the switch to ground.

*/
// -----------------------------------------------------------------------------
// Nicely formated print of a byte.

// Instruction parameters:
byte dataByte = 0;           // db = Data byte (8 bit)

void printByte(byte b) {
  for (int i = 7; i >= 0; i--)
    Serial.print(bitRead(b, i));
}

// -----------------------------------------------------------------------------
#include <PCF8574.h>
#include <Wire.h>

// pcfSwitches has interupt enabled.
// Implement: pcfSwitches interupt handling. Likely use the same pin, pin 2.

// -------------------------
// Address for the PCF8574 module being tested.
PCF8574 pcfSwitches(0x020);    // Control: STOP, RUN, SINGLE STEP, EXAMINE, EXAMINE NEXT, DEPOSIT, DEPOSIT NEXT, REST
// PCF8574 pcfSwitches(0x021);    // Low bytes
// PCF8574 pcfSwitches(0x022);    // High bytes
// PCF8574 pcfSwitches(0x023);    // AUX switches and others: Step down, CLR, Protect, Unprotect, AUX1 up, AUX1 down,  AUX2 up, AUX2 down

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Front Panel Switches

// -------------------------
// Getting a byte of toggle values.

int toggleDataByte() {
  // Invert byte bits using bitwise not operator: "~";
  // Bitwise "not" operator to invert bits:
  //  int a = 103;  // binary:  0000000001100111
  //  int b = ~a;   // binary:  1111111110011000 = -104
  // byte toggleByte = ~pcfSwitches.read8();
  return ~pcfSwitches.read8();
}

// -----------------------------------------------------------------------------
void echoSwitchData() {
  // ----------------------
  Serial.print("+ PCF8574 byte, read8      = ");
  dataByte = pcfSwitches.read8();                   // Read all PCF8574 inputs
  printByte(dataByte);
  Serial.println("");
  // ----------------------
  Serial.print("+ PCF8574 byte, readSwitch = ");
  for (int pinGet = 7; pinGet >= 0; pinGet--) {
    int pinValue = pcfSwitches.readButton(pinGet);  // Read each PCF8574 input
    Serial.print(pinValue);
  }
  Serial.println("");
  // ----------------------
  Serial.print("+ Toggle Data Byte         = ");
  dataByte = toggleDataByte();                      // Read all PCF8574 inputs using toggleDataByte
  printByte(dataByte);
  Serial.println("");
  // ----------------------
}

// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  // Give the serial connection time to start before the first print.
  delay(1000);
  Serial.println(""); // Newline after garbage characters.
  Serial.println("+++ Setup.");

  // ------------------------------
  pcfSwitches.begin();
  Serial.println("+ PCF PCF8574 I2C Two Wire module initialized.");

  // ------------------------------
  Serial.println("+++ Go to loop.");
}

// -----------------------------------------------------------------------------
// Device Loop

int counter = 0;
void loop() {
  delay (50);
  counter++;
  // 20 is 1 second (20 x 50 = 1000). 40 is every 2 seconds.
  if (counter == 40) {
    Serial.println("---------------------------");
    echoSwitchData();
    counter = 0;
  }
}
// -----------------------------------------------------------------------------
