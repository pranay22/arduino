// -----------------------------------------------------------------------------
/*
  Altair 101 Memory Definitions and Management.

  Reference > Language > Variables > Data types > Array
    https://www.arduino.cc/reference/en/language/variables/data-types/array/
  A byte stores an 8-bit unsigned number, from 0 to 255.
    https://www.arduino.cc/reference/en/language/variables/data-types/byte/
  Array size, sizeof(): Reference > Language > Variables > Utilities > Sizeof
    https://www.arduino.cc/reference/en/language/variables/utilities/sizeof/
  Octal,  Reference > Language > Variables > Constants > Integerconstants
    https://www.arduino.cc/reference/en/language/variables/constants/integerconstants/
    leading "0" characters 0-7 valid, for example: 0303 is octal 303.


  -------------
  void setup()
  {
  Serial.begin(9600);
  byte myByte = B11000011;
  //byte myByte = 0xC3;
  //byte myByte = 195;
  Serial.println(myByte, DEC);
  Serial.println(myByte, BIN);
  Serial.println(myByte, HEX);
  shiftOut(1,1,LSBFIRST, myByte); // <<<< will shift out the same no matter which of the three assignments you choose above.
  }

  --------------------

  int latchPin = 8;   //Pin connected to ST_CP of 74HC595
  int clockPin = 12;  //Pin connected to SH_CP of 74HC595
  int dataPin = 11;   //Pin connected to DS of 74HC595
  byte myArray[] = {
    B00000000, B10000000, B11100000,
    B01100000, B01000000, B01110000,
    B00110000, B00100000, B00111000,
    B00011000, B00010000, B00011100,
    B00001100, B00001000, B00001110,
    B00000110, B00000100, B00000111,
    B00000011, B00000010, B1000011,
    B10000001, B00000001, B11000001,
    B11000000
  };
  void setup() {
  Serial.begin(9600);
  //set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  }
  void loop() {
  for (int i = 0; i < sizeof(myArray)/sizeof(myArray[0]); i++) {
    // "/sizeof(myArray[0]" not needed when array items are byte size.
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, myArray[i]);
    digitalWrite(latchPin, HIGH);
    delay(100);
  }
  }

  // -----------------------------------------------------------------------------
  Following from:
    https://www.altairduino.com/wp-content/uploads/2017/10/Documentation.pdf

*/
// -----------------------------------------------------------------------------
// Front Panel LEDs

const int WAIT_PIN = A2;

// -----------------------------------------------------------------------------
// Front Panel Control Buttons

const int STOP_BUTTON_PIN = 4;   // Nano D4
const int RUN_BUTTON_PIN = 5;
const int STEP_BUTTON_PIN = 6;
const int EXAMINE_BUTTON_PIN = 7;
const int EXAMINENEXT_BUTTON_PIN = 8;

boolean runProgram = false;

// Only do the action once, don't repeat if the button is held down.
// Don't repeat action if the button is not pressed.
boolean stopButtonState = true;
boolean runButtonState = true;
boolean stepButtonState = true;

void checkStopButton() {
  if (digitalRead(STOP_BUTTON_PIN) == HIGH) {
    if (!stopButtonState) {
      Serial.println("+ Stop process.");
      runProgram = false;
      digitalWrite(WAIT_PIN, HIGH);
      stopButtonState = false;
    }
    stopButtonState = true;
  } else {
    if (stopButtonState) {
      stopButtonState = false;
    }
  }
}

void checkRunButton() {
  if (digitalRead(RUN_BUTTON_PIN) == HIGH) {
    if (!runButtonState) {
      Serial.println("+ Run process.");
      runProgram = true;
      digitalWrite(WAIT_PIN, LOW);
      runButtonState = false;
    }
    runButtonState = true;
  } else {
    if (runButtonState) {
      runButtonState = false;
    }
  }
}

void checkStepButton() {
  // If the button is pressed (circuit closed), the button status is HIGH.
  if (digitalRead(STEP_BUTTON_PIN) == HIGH) {
    if (!stepButtonState) {
      // digitalWrite(STEP_BUTTON_PIN, HIGH);
      // Serial.println("+ checkButton(), turn LED on.");
      processInstruction();
      stepButtonState = false;
    }
    stepButtonState = true;
  } else {
    if (stepButtonState) {
      // digitalWrite(STEP_BUTTON_PIN, LOW);
      // Serial.println("+ checkButton(), turn LED off.");
      stepButtonState = false;
    }
  }
}

// -----------------------------------------------------------------------------
// Memory definitions

int memoryBytes = 1024;
byte memoryData[1024];

// Define a jump loop program byte array.
byte jumpLoopProgram[] = {
  0303, 0006, 0000,
  0000, 0000, 0000,
  0303, 0000, 0000
};

// Define a jump loop program with NOP instructions.
byte jumpLoopNopProgram[] = {
  0303, 0004, 0000,
  0000, 0000, 0000,
  0303, 0000, 0000
};

// Define a jump loop program
//    with a halt(HLT binary 01 110 110 = octal 166) instruction,
//    and NOP instructions.
byte jumpHaltLoopProgram[] = {
  0303, 0006, 0000,
  0000, 0000, 0000,
  0166, 0000, 0000,
  0303, 0000, 0000
};

// -----------------------------------------------------------------------------
// Print byte data

byte zeroByte = B00000000;
char charBuffer[16];

void printByte(byte b) {
  for (int i = 7; i >= 0; i--)
    Serial.print(bitRead(b, i));
}
void printWord(int theValue) {
  String sValue = String(theValue, BIN);
  for (int i = 1; i <= 16 - sValue.length(); i++) {
    Serial.print("0");
  }
  Serial.print(sValue);
}

void printOctal(byte b) {
  String sOctal = String(b, OCT);
  for (int i = 1; i <= 3 - sOctal.length(); i++) {
    Serial.print("0");
  }
  Serial.print(sOctal);
}

void printData(byte theByte) {
  printByte(theByte);
  // Serial.print(memoryData[i], BIN);
  Serial.print(" :  ");
  printOctal(theByte);
  // Serial.print(memoryData[i], OCT);
  Serial.print(" : ");
  // Serial.println(memoryData[i], DEC);
  sprintf(charBuffer, "%3d", theByte);
  Serial.print(charBuffer);
}

// -----------------------------------------------------------------------------
// Memory Functions

void initMemoryToZero() {
  Serial.println("+ Initialize all memory bytes to zero.");
  for (int i = 0; i < memoryBytes; i++) {
    memoryData[i] = zeroByte;
  }
}

void copyByteArrayToMemory(byte btyeArray[], int arraySize) {
  Serial.println("+ Copy the program into the computer's memory array.");
  for (int i = 0; i < arraySize; i++) {
    memoryData[i] = btyeArray[i];
  }
  Serial.println("+ Copied.");
}

void listByteArray(byte btyeArray[], int arraySize) {
  Serial.println("+ List the jump loop program.");
  for (int i = 0; i < arraySize; i++) {
    Serial.print("++ ");
    sprintf(charBuffer, "%3d", i);
    Serial.print(charBuffer);
    Serial.print(": ");
    printData(btyeArray[i]);
    Serial.println("");
  }
  Serial.println("+ End of listing.");
}

// -----------------------------------------------------------------------------
// Instruction Set

int programCounter = 0;
// int nextAddress = 0;

/*
  Destination and Source register fields:
    111=A   (Accumulator)
    000=B
    001=C
    010=D
    011=E
    100=H
    101=L
    110=M   (Memory reference through address in H:L)

  Register pair 'RP' fields:
    00=BC   (B:C as 16 bit register)
    01=DE   (D:E as 16 bit register)
    10=HL   (H:L as 16 bit register)
    11=SP   (Stack pointer, refers to PSW (FLAGS:A) for PUSH/POP)

  Instruction parameters:
    db = Data byte (8 bit)
    lb = Low byte of 16 bit value
    hb = High byte of 16 bit value
    pa = Port address (8 bit)
    p  = 8 bit port address
*/

// Following from, section: - 26 - 8080 Instruction Set
//    https://www.altairduino.com/wp-content/uploads/2017/10/Documentation.pdf
// Octals stored as a bytes.
//
//                               Inst      Encoding          Flags   Description
// Programmed and tested:
const byte HLT = 0166;        // HLT       01110110          -       Halt processor
const byte JMP = 0303;        // JMP a     11000011 lb hb    -       Unconditional jump*
const byte NOP = 0000;        // NOP       00000000          -       No operation
//
// To be programmed:
const byte IN =  B11011011;   // IN p      11011011 pa       -       Read input port into A
const byte STA = B00110010;   // STA a     00110010 lb hb    -       Store A to memory
//
// More program instructions for the Kill the Bit program,
// DAD RP    00RP1001          C       Add register pair to HL (16 bit add)*
// JNC ?
// LDAX RP   00RP1010 *1       -       Load indirect through BC or DE
//                    *1 = Only RP=00(BC) and 01(DE) are allowed for LDAX/STAX
// LXI RP,#  00RP0001 lb hb    -       Load register pair immediate*
// MOV D,S   01DDDSSS          -       Move register to register*
// MVI D,#   00DDD110 db       -       Move immediate to register*
// RRC       00001111          C       Rotate A right
// XRA S     10101SSS          ZSPCA   Exclusive OR register with A

void processByte(byte theByte) {
  // Serial.print(":theByte,");
  // Serial.print(theByte);
  // Serial.print(":");
  switch (theByte) {
    case HLT:
      Serial.print(" > HLT Instruction, Halt the processor.");
      runProgram = false;
      digitalWrite(WAIT_PIN, HIGH);
      programCounter++;
      break;
    case JMP:
      Serial.print(" > JMP Instruction, jump to address :");
      // Serial.print(programCounter);
      int lowOrder = ++programCounter;
      int highOrder = ++programCounter;
      // word(high order byte, low order byte)
      programCounter = word(memoryData[highOrder], memoryData[lowOrder]);
      printWord(programCounter);  // Example: 00 000 000 00 000 110
      Serial.print(":DEC,");
      Serial.print(programCounter);
      Serial.print(":");
      break;
    default:
      Serial.print(" - Error, unknow instruction.");
  }
  if (theByte == NOP) {
    // Note, NOP does not work in the switch statement.
    Serial.print(" > NOP Instruction, No operation.");
    // Can add a delay, for example, to wait for an interrupt proccess.
    delay(10);
    programCounter++;
  }
}

// -----------------------------------------------------------------------------
void processInstruction() {

  Serial.print("+ ");
  sprintf(charBuffer, "%3d", programCounter);
  Serial.print(charBuffer);
  Serial.print(" > ");
  printData(memoryData[programCounter]);
  processByte(memoryData[programCounter]);
  Serial.println("");

}

// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);        // Give the serial connection time to start before the first print.
  Serial.println(""); // Newline after garbage characters.
  Serial.println("+++ Setup.");

  pinMode(WAIT_PIN, OUTPUT);
  digitalWrite(WAIT_PIN, HIGH);
  Serial.println("+ LEDs configured for output.");

  // List and load a program.
  // Possible programs:
  //    jumpLoopProgram
  //    jumpLoopNopProgram
  //    jumpHaltLoopProgram
  int programSize = sizeof(jumpHaltLoopProgram);
  listByteArray(jumpHaltLoopProgram, programSize);
  copyByteArrayToMemory(jumpHaltLoopProgram, programSize);

  Serial.println("+++ Start program loop.");
}

// -----------------------------------------------------------------------------
// Device Loop for processing machine code.

static unsigned long timer = millis();
void loop() {

  if (runProgram) {
    // Execute a step based on this timer.
    // Example, 3000 : once every 3 seconds.
    if (millis() - timer >= 500) {
      processInstruction();
      timer = millis();
    }
    checkStopButton();
  } else {
    checkRunButton();
    checkStepButton();
  }

  delay(60);
}
// -----------------------------------------------------------------------------
