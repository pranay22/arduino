// -----------------------------------------------------------------------------
/*
  PCF8574 I2C Module, I2C to 8-bit Parallel-Port Expander

  Program to test switch controls for PCF8574 module.
  Test interrupt aware option.
  
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
  | INT          | For multiple, test using a diode to the Arduino interrupt pin.
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
  + INT to Arduino interrupt pin such as pin 2 in this sample program.
  + P0 ... O7 to switches. Other side of the switch to ground.

  When using multiple modules to the same Arduino interrupt pin,
  + Test with a diode from the modules to the Arduino interrupt pin.

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

// -------------------------
// Interrupt handler routine.
//                   Mega pin for control toggle interrupt. Same pin for Nano.
const int INTERRUPT_PIN = 2;

boolean pcfSwitchesinterrupted = false;
void pcfSwitchesinterrupt() {
  pcfSwitchesinterrupted = true;
}

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
// -----------------------------------------------------------------------------
// Front Panel Switches

// --------------------------------------------------------
// Front Panel Control Switches, when a program is not running.
// Control:   STOP, RUN, SINGLE STEP, EXAMINE, EXAMINE NEXT, DEPOSIT, DEPOSIT NEXT, REST
// Switches:  AUX switches and others: Step down, CLR, Protect, Unprotect, AUX1 up, AUX1 down,  AUX2 up, AUX2 down

const int pcfSwitch0 = 0; // RESET            AUX2 down
const int pcfSwitch1 = 1; // DEPOSIT NEXT     AUX2 up
const int pcfSwitch2 = 2; // DEPOSIT          AUX1 down
const int pcfSwitch3 = 3; // EXAMINE NEXT     AUX1 up
const int pcfSwitch4 = 4; // EXAMINE          UNPROTECT
const int pcfSwitch5 = 5; // SINGLE STEP UP   PROTECT
const int pcfSwitch6 = 6; // RUN              CLR
const int pcfSwitch7 = 7; // STOP             SINGLE STEP down

boolean switchLogic0 = false;
boolean switchLogic1 = false;
boolean switchLogic2 = false;
boolean switchLogic3 = false;

void checkSwitches() {
  // -------------------
  if (pcfSwitches.readButton(pcfSwitch0) == 0) {
    if (!switchLogic0) {
      switchLogic0 = true;
      Serial.print(F("+ Control, pcfSwitch0: "));
      printByte(toggleDataByte());
    }
  } else if (switchLogic0) {
    switchLogic0 = false;
    // Switch logic.
    Serial.println(F(", released."));
  }
  // -------------------
  if (pcfSwitches.readButton(pcfSwitch1) == 0) {
    if (!switchLogic1) {
      switchLogic1 = true;
      Serial.print(F("+ Control, pcfSwitch1: "));
      printByte(toggleDataByte());
    }
  } else if (switchLogic1) {
    switchLogic1 = false;
    // Switch logic.
    Serial.println(F(", released."));
  }
  // -------------------
  if (pcfSwitches.readButton(pcfSwitch2) == 0) {
    if (!switchLogic2) {
      switchLogic2 = true;
      Serial.print(F("+ Control, pcfSwitch2: "));
      printByte(toggleDataByte());
    }
  } else if (switchLogic2) {
    switchLogic2 = false;
    // Switch logic.
    Serial.println(F(", released."));
  }
  // -------------------
  if (pcfSwitches.readButton(pcfSwitch3) == 0) {
    if (!switchLogic3) {
      switchLogic3 = true;
      Serial.print(F("+ Control, pcfSwitch3: "));
      printByte(toggleDataByte());
    }
  } else if (switchLogic3) {
    switchLogic3 = false;
    // Switch logic.
    Serial.println(F(", released."));
  }
  // -------------------
  if (pcfSwitches.readButton(pcfSwitch4) == 0) {
    Serial.print(F("+ Control, pcfSwitch4: "));
    printByte(toggleDataByte());
    Serial.println("");
  }
  if (pcfSwitches.readButton(pcfSwitch5) == 0) {
    Serial.print(F("+ Control, pcfSwitch5: "));
    printByte(toggleDataByte());
    Serial.println("");
  }
  if (pcfSwitches.readButton(pcfSwitch6) == 0) {
    Serial.print(F("+ Control, pcfSwitch6: "));
    printByte(toggleDataByte());
    Serial.println("");
  }
  if (pcfSwitches.readButton(pcfSwitch7) == 0) {
    Serial.print(F("+ Control, pcfSwitch7: "));
    printByte(toggleDataByte());
    Serial.println("");
  }
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
  pcfSwitches.begin();
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), pcfSwitchesinterrupt, CHANGE);
  Serial.println("+ PCF module initialized.");

  // ------------------------------
  Serial.println("+++ Go to loop.");
}

// -----------------------------------------------------------------------------
// Device Loop

void loop() {
  if (pcfSwitchesinterrupted) {
    // Serial.println("---------------------------");
    // Serial.println("+ Interrupt call, switchSetOn is true.");
    checkSwitches();
    pcfSwitchesinterrupted = false; // Reset for next interrupt.
  }
  delay (50);
}
// -----------------------------------------------------------------------------
