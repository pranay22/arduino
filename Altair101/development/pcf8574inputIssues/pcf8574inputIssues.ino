// -----------------------------------------------------------------------------
/*
  PCF8574 I2C Module
  I2C to 8-bit Parallel-Port Expander

  Issues with this example:
  1) Issue: when switching too many times, the interrupt stop working.
  2) Issue description:
  + After starting, control switches worked fine.
  + Switch Run (runProgram = true).
  ++ Switch Reset works fine.
  ++ Switch Stop, the interrupt happens, but the read fails (pcf8574.digitalRead(pinGet)) the first time.
  ++ Switch Stop, the interrupt happens, the read succeeds.
  ++ Returns to Control switches (runProgram = false).
  
  Module adjustable pin address settings:
  A0 A1 A2
   0  0  0 = 0x20
   0  0  1 = 0x21
   0  1  0 = 0x22
    ...
   1  1  1 = 0x27
   
  Wiring:
  + SDA to Nano A4.
  + SCL to Nano A5.
  + GND to Nano GND
  + VCC to Nano 5V
  + INT to Nano interrupt pin, pin 2 in this sample program.
  + P0 ... O7 to switches. Other side of the switch to ground.
  
  Library:
    https://github.com/xreef/PCF8574_library
  Information and code sample:
    https://protosupplies.com/product/pcf8574-i2c-i-o-expansion-module/
*/
#include "Arduino.h"
// -----------------------------------------------------------------------------
#define SWITCH_MESSAGES 1

bool runProgram = false;

// -----------------------------------------------------------------------------
#include "PCF8574.h"

// Button press flag.
boolean switchSetOn = false;
//
void PCF8574_Interrupt();
//
// Interrupt setup: I2C address, interrupt pin to use, interrupt handler routine.
int PCF_INTERRUPT_ADDRESS = 0x020;
const int INTERRUPT_PIN = 2;
void pcf01interrupt() {
  switchSetOn = true;
}
PCF8574 pcf8574(PCF_INTERRUPT_ADDRESS, INTERRUPT_PIN, pcf01interrupt);

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Front Panel Control Switches

const int pinStop = 0;
const int pinRun = 1;
const int pinStep = 2;
const int pinExamine = 3;
const int pinExamineNext = 4;
const int pinDeposit = 5;
const int pinDepositNext = 6;
const int pinReset = 7;

boolean switchStop = false;
boolean switchRun = false;
boolean switchStep = false;
boolean switchExamine = false;
boolean switchExamineNext = false;
boolean switchDeposit = false;;
boolean switchDepositNext = false;;
boolean switchReset = false;

// ----------------------------------------------
// When a program is not running.

// void controlSwitches(int pinGet, int pinValue) {
void controlSwitches() {
  for (int pinGet = 0; pinGet < 8; pinGet++) {
    int pinValue = pcf8574.digitalRead(pinGet);  // Read each PCF8574 input
    switch (pinGet) {
      // -------------------
      case pinRun:
        if (pinValue == 0) {    // 0 : switch is on.
          if (!switchRun) {
            switchRun = true;
          }
        } else if (switchRun) {
          switchRun = false;
#ifdef SWITCH_MESSAGES
          Serial.println("+ Control, Run > run the program.");
#endif
          // ...
          runProgram = true;
        }
        break;
      // -------------------
      case pinStep:
        if (pinValue == 0) {
          if (!switchStep) {
            switchStep = true;
          }
        } else if (switchStep) {
          switchStep = false;
#ifdef SWITCH_MESSAGES
          Serial.println("+ Control, Step.");
#endif
          // ...
        }
        break;
      // -------------------
      case pinExamine:
        if (pinValue == 0) {
          if (!switchExamine) {
            switchExamine = true;
          }
        } else if (switchExamine) {
          switchExamine = false;
#ifdef SWITCH_MESSAGES
          Serial.println("+ Control, Examine.");
#endif
          // ...
        }
        break;
      // -------------------
      case pinExamineNext:
        if (pinValue == 0) {
          if (!switchExamineNext) {
            switchExamineNext = true;
          }
        } else if (switchExamineNext) {
          switchExamineNext = false;
#ifdef SWITCH_MESSAGES
          Serial.println("+ Control, Examine Next.");
#endif
          // ...
        }
        break;
      // -------------------
      case pinDeposit:
        if (pinValue == 0) {
          if (!switchDeposit) {
            switchDeposit = true;
          }
        } else if (switchDeposit) {
          switchDeposit = false;
#ifdef SWITCH_MESSAGES
          Serial.println("+ Control, Deposit.");
#endif
          // ...
        }
        break;
      // -------------------
      case pinDepositNext:
        if (pinValue == 0) {
          if (!switchDepositNext) {
            switchDepositNext = true;
          }
        } else if (switchDepositNext) {
          switchDepositNext = false;
#ifdef SWITCH_MESSAGES
          Serial.println("+ Control, Deposit Next.");
#endif
          // ...
        }
        break;
      // -------------------
      case pinReset:
        if (pinValue == 0) {
          if (!switchReset) {
            switchReset = true;
          }
        } else if (switchReset) {
          switchReset = false;
#ifdef SWITCH_MESSAGES
          Serial.println("+ Control, Reset.");
#endif
          // ...
        }
        break;
    }
    // -------------------
  }
}

// ----------------------------------------------
// When a program is running.

void runningSwitches() {
  for (int pinGet = 0; pinGet < 8; pinGet++) {
    int pinValue = pcf8574.digitalRead(pinGet);  // Read each PCF8574 input
    switch (pinGet) {
      case pinStop:
        if (pinValue == 0) {    // 0 : switch is on.
          if (!switchStop) {
            switchStop = true;
          }
        } else if (switchStop) {
          switchStop = false;
#ifdef SWITCH_MESSAGES
          Serial.println("+ Running, Stop > hlt, halt the processor.");
#endif
          // ...
          runProgram = false;
        }
        break;
      // -------------------
      case pinReset:
        if (pinValue == 0) {
          if (!switchReset) {
            switchReset = true;
          }
        } else if (switchReset) {
          switchReset = false;
#ifdef SWITCH_MESSAGES
          Serial.println("+ Running, Reset.");
#endif
          // ...
        }
        break;
      // -------------------
      default:
        delay(3);
    }
  }
  // Serial.println(":");
}

// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  // Give the serial connection time to start before the first print.
  delay(1000);
  Serial.println(""); // Newline after garbage characters.
  Serial.println("+++ Setup.");

  // ------------------------------
  // I2C based switch initialization
  // pinMode(INTERRUPT_PIN, INPUT_PULLUP); // Enable pullup on interrupt pin of Uno
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), pcf01interrupt, CHANGE);
  // Set all pins as inputs on the PCF8574 module.
  pcf8574.pinMode(P0, INPUT);
  pcf8574.pinMode(P1, INPUT);
  pcf8574.pinMode(P2, INPUT);
  pcf8574.pinMode(P3, INPUT);
  pcf8574.pinMode(P4, INPUT);
  pcf8574.pinMode(P5, INPUT);
  pcf8574.pinMode(P6, INPUT);
  pcf8574.pinMode(P7, INPUT);
  // Set an intial state.
  pcf8574.digitalWrite(P0, HIGH);
  pcf8574.digitalWrite(P1, HIGH);
  pcf8574.digitalWrite(P2, HIGH);
  pcf8574.digitalWrite(P3, HIGH);
  pcf8574.digitalWrite(P4, HIGH);
  pcf8574.digitalWrite(P5, HIGH);
  pcf8574.digitalWrite(P6, HIGH);
  pcf8574.digitalWrite(P7, HIGH);
  pcf8574.begin();
  delay(300); // required to give startup time for the PCF8574 module.
  Serial.println("+ I2C PCF input module initialized.");

  // ------------------------------
  if (runProgram) {
    Serial.println("+ runProgram is true.");
  } else {
    Serial.println("+ runProgram is false.");
  }
  // ------------------------------
  Serial.println("+++ Go to loop.");
}

// -----------------------------------------------------------------------------
// Device Loop

void loop() {

  if (runProgram) {
    if (switchSetOn) {
      Serial.println("+ runProgram = true, switchSetOn is true.");
      runningSwitches();
      delay(30);           // Handle switch debounce.
      switchSetOn = false;
    } else {
      delay(30);
    }
    // ----------------------------
  } else {
    if (switchSetOn) {
      // Serial.println("+ runProgram = false, switchSetOn is true.");
      controlSwitches();
      delay(30);           // Handle switch debounce.
      switchSetOn = false;
    } else {
      delay(30);
    }
    // ----------------------------
  }

}
// -----------------------------------------------------------------------------