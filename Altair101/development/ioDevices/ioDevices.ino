// -----------------------------------------------------------------------------
/*
  Use a Micro SD Card Module to save and load binary 8080 machine language programs.

  Connect DS3231 Clock, and the LCD display, pins to the Nano:
  + VCC to Nano 5v, note, also works with 3.3v, example: NodeMCU.
  + GND to Nano ground.
  + SDA to Nano D4 (pin 4), same on Uno.
  + SCL to Nano D5 (pin 5), same on Uno.
*/

#define INCLUDE_LCD 1
#define INCLUDE_CLOCK 1
#define CLOCK_MESSAGES 1
// #define INCLUDE_PCF8574 1
// #define INCLUDE_SDCARD 1
// #define SDCARD_MESSAGES 1

#ifdef INCLUDE_PCF8574
// -----------------------------------------------------------------------------
#define SWITCH_MESSAGES 1

bool runProgram = false;

// -----------------------------------------------------------------------------
#include <PCF8574.h>
#include <Wire.h>

PCF8574 pcf20(0x020);

// Set switch flag for on/off.
const int INTERRUPT_PIN = 2;
boolean switchSetOn = false;
// Interrupt setup: I2C address, interrupt pin to use, interrupt handler routine.
void pcf20interrupt() {
  switchSetOn = true;
}

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
  for (int pinGet = 7; pinGet >= 0; pinGet--) {
    int pinValue = pcf20.readButton(pinGet);  // Read each PCF8574 input
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
  for (int pinGet = 7; pinGet >= 0; pinGet--) {
    int pinValue = pcf20.readButton(pinGet);  // Read each PCF8574 input
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
#endif

// -----------------------------------------------------------------------
// For the infrared receiver.
#include <IRremote.h>
int IR_PIN = A1;
IRrecv irrecv(IR_PIN);
decode_results results;

// -----------------------------------------------------------------------------
// For the clock board.
#ifdef INCLUDE_CLOCK

#include "RTClib.h"

RTC_DS3231 rtc;
DateTime now;

#endif

// -----------------------------------------------------------------------------
// SD Card module uses SPI.
#ifdef INCLUDE_SDCARD

#include <SPI.h>
#include <SD.h>

// SD Card Module is an SPI slave device.
//  CS (chip/slave select)     to Nano pin 10 (SS pin). Can be any digital pin on the master(Nano) to enable and disable this device on the SPI bus.
// SCK, MOSI, MISO are the SPI hardware pins are declared in the SPI library.
//  SCK (serial clock)         to Nano pin 13 SPI: accepts clock pulses which synchronize data transmission generated by Arduino
//  MOSI (master out slave in) to Nano pin 11 SPI: input to the Micro SD Card Module.
//  MISO (master in slave Out) to Nano pin 12 SPI: output from the Micro SD Card Module.
//  VCC (3.3V or 5V)           to Nano pin 5V+
//  GND (ground)               to Nano pin ground

// The CS pin is the only one that is not really fixed as any of the Arduino digital pin.
const int csPin = 10;  // SD Card module is connected to Nano pin 10.
File myFile;

// Test filename.
// Files are created using uppercase: F1.TXT.
String theFilename = "f1.txt";

#endif

// -----------------------------------------------------------------------------
// Test program to use in this program. The program tests the 16 bit add opcode, dad.

byte theProgram[] = {
  //                //            ; --------------------------------------
  //                //            ; Test opcode dad.
  //                //            ; --------------------------------------
  //                // Start:     ; Intialize register values.
  B00111110, 6,     // mvi a,6    ; Move numbers to registers.
  B00000110, 0,     // mvi b,0
  B00001110, 1,     // mvi c,1
  B00010110, 2,     // mvi d,2
  B00011110, 3,     // mvi e,3
  B00100110, 4,     // mvi h,0
  B00101110, 5,     // mvi l,0
  0343, 38,         // out 38     ; Print the Intialized register values.
  //                //            ; --------------------------------------
  //0RP1001
  B00001001,        // dad b      ; Add register pair B:C to H:L.
  0343, 36,         // out 36     ; Print register pair, H:L.
  B00011001,        // dad d      ; Add register pair D:E to H:L.
  0343, 36,         // out 36     ; Print register pair, H:L.
  //                //            ; --------------------------------------
  0166,             // hlt
  0303, 0000, 0000, // jmp Start  ; Jump back to beginning.
  0000              //            ; End.
};

// -----------------------------------------------------------------------------
// Memory definitions

const int memoryBytes = 256;
byte memoryData[memoryBytes];
unsigned int programCounter = 0;     // Program address value

// -----------------------------------------------------------------------------
// Memory Functions

char charBuffer[17];
byte zeroByte = B00000000;

void initMemoryToZero() {
  Serial.println(F("+ Initialize all memory bytes to zero."));
  for (int i = 0; i < memoryBytes; i++) {
    memoryData[i] = zeroByte;
  }
}

void copyByteArrayToMemory(byte btyeArray[], int arraySize) {
  Serial.println(F("+ Copy the program into the computer's memory array."));
  for (int i = 0; i < arraySize; i++) {
    memoryData[i] = btyeArray[i];
  }
  Serial.println(F("+ Copied."));
}

void listByteArray(byte btyeArray[], int arraySize) {
  Serial.println(F("+ List the program."));
  Serial.println(F("++   Address:       Data value"));
  for (int i = 0; i < arraySize; i++) {
    Serial.print(F("+ Addr: "));
    sprintf(charBuffer, "%4d:", i);
    Serial.print(charBuffer);
    Serial.print(F(" Data: "));
    printData(btyeArray[i]);
    Serial.println("");
  }
  Serial.println(F("+ End of listing."));
}

// -----------------------------------------------------------------------------
// Output: log messages and Front Panel LED data lights.

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
  sprintf(charBuffer, "%3d = ", theByte);
  Serial.print(charBuffer);
  printOctal(theByte);
  Serial.print(F(" = "));
  printByte(theByte);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// Write Program memory to a file.
#ifdef INCLUDE_SDCARD

// -----------------------------
// Write Program Memory To File

void writeProgramMemoryToFile(String theFilename) {
#ifdef SDCARD_MESSAGES
  Serial.println(F("+ Write program memory to a new file named: "));
  Serial.print(theFilename);
  Serial.println(F("+ Check if file exists. "));
#endif
  if (SD.exists(theFilename)) {
    SD.remove(theFilename);
    // Serial.println("++ Exists, so it was deleted.");
  } else {
    // Serial.println("++ Doesn't exist.");
  }
  myFile = SD.open(theFilename, FILE_WRITE);
  if (!myFile) {
    Serial.print(F("- Error opening file: "));
    Serial.println(theFilename);
    return; // When used in setup(), causes jump to loop().
  }
#ifdef SDCARD_MESSAGES
  Serial.println(F("++ New file opened."));
  Serial.println(F("++ Write binary memory to the file."));
#endif
  for (int i = 0; i < memoryBytes; i++) {
    myFile.write(memoryData[i]);
  }
  myFile.close();
#ifdef SDCARD_MESSAGES
  Serial.println(F("+ Completed, file closed."));
#endif
}

// -----------------------------
// Read Program File Into Memory

void readProgramFileIntoMemory(String theFilename) {
#ifdef SDCARD_MESSAGES
  Serial.println(F("+ Read a file into program memory, file named: "));
  Serial.print(theFilename);
  Serial.println(F("+ Check if file exists. "));
#endif
  if (SD.exists(theFilename)) {
#ifdef SDCARD_MESSAGES
    Serial.println(F("++ Exists, so it can be read."));
#endif
  } else {
    Serial.println(F("-- Doesn't exist, cannot read."));
    return;
  }
  myFile = SD.open(theFilename);
  if (!myFile) {
#ifdef SDCARD_MESSAGES
    Serial.print(F("- Error opening file: "));
    Serial.println(theFilename);
#endif
    return;
  }
  int i = 0;
  while (myFile.available()) {
    // Reads one character at a time.
    memoryData[i] = myFile.read();
#ifdef SDCARD_MESSAGES
    // Print Binary:Octal:Decimal values.
    Serial.print("B");
    printByte(memoryData[i]);
    Serial.print(":");
    printOctal (memoryData[i]);
    Serial.print(":");
    Serial.println(memoryData[i], DEC);
#endif
    i++;
    if (i > memoryBytes) {
      Serial.println(F("-+ Warning, file contains more data bytes than abailable memory."));
    }
  }
  myFile.close();
#ifdef SDCARD_MESSAGES
  Serial.println(F("+ File closed."));
#endif
}

#endif

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// 1602 LCD
#ifdef INCLUDE_LCD

#include<Wire.h>
#include<LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

int displayColumns = 16;  // LCD display column length
void displayPrintln(int theRow, String theString) {
  String printString = theString;
  int theRest = displayColumns - theString.length();
  if (theRest < 0) {
    // Since longer that the display column length, shorten the string for printing.
    printString = theString.substring(0, displayColumns);
  } else {
    // Buffer with spaces to the column length.
    while (theRest > 0) {
      printString = printString + " ";
      theRest--;
    }
  }
  lcd.setCursor(0, theRow);
  lcd.print(printString);
}

#endif

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// DS3231 Clock
#ifdef INCLUDE_CLOCK

int theCursor;
const int printRowClockDate = 0;
const int printColClockDate = 0;
const int printRowClockPulse = 0;
const int thePrintColHour = 8;
const int thePrintColMin = thePrintColHour + 3;
const int thePrintColSec = thePrintColMin + 3;

int theCounterYear = 0;
int theCounterMonth = 0;
int theCounterDay = 0;
int theCounterHours = 0;
int theCounterMinutes = 0;
int theCounterSeconds = 0;

void syncCountWithClock() {
  now = rtc.now();
  theCounterHours = now.hour();
  theCounterMinutes = now.minute();
  theCounterSeconds = now.second();
  printClockDate();

#ifdef INCLUDE_LCD
  //
  theCursor = thePrintColHour;
  printClockInt(theCursor, printRowClockPulse, theCounterHours);  // Column, Row
  theCursor = theCursor + 3;
  lcd.print(":");
  printClockInt(theCursor, printRowClockPulse, theCounterMinutes);
  theCursor = theCursor + 3;
  lcd.print(":");
  printClockInt(theCursor, printRowClockPulse, theCounterSeconds);
#endif

#ifdef CLOCK_MESSAGES
  Serial.println(F("+ syncCountWithClock, current time:"));
  Serial.print(F(" theCounterHours="));
  Serial.println(theCounterHours);
  Serial.print(F(" theCounterMinutes="));
  Serial.println(theCounterMinutes);
  Serial.print(F(" theCounterSeconds="));
  Serial.println(theCounterSeconds);
#endif
}

void processClockNow() {
  now = rtc.now();
  if (now.second() != theCounterSeconds) {
    // When the clock second value changes, that's a clock second pulse.
    theCounterSeconds = now.second();
    clockPulseSecond();
    if (theCounterSeconds == 0) {
      // When the clock second value changes to zero, that's a clock minute pulse.
      theCounterMinutes = now.minute();
      clockPulseMinute();
      if (theCounterMinutes == 0) {
        // When the clock minute value changes to zero, that's a clock hour pulse.
        theCounterHours = now.hour();
        clockPulseHour();
        // -------------------------
        // Date pulses.
        if (now.hour() == 0) {
          // When the clock hour value changes to zero, that's a clock day pulse.
          printClockDate(); // Prints and sets the values for day, month, and year.
          clockPulseDay();
          if (theCounterDay == 1) {
            // When the clock day value changes to one, that's a clock month pulse.
            clockPulseMonth();
            if (theCounterMonth == 1) {
              // When the clock Month value changes to one, that's a clock year pulse.
              clockPulseYear();
            }
          }
        }
        // -------------------------
      }
    }
  }
}

void clockPulseYear() {
  Serial.print(F("+++++ clockPulseYear(), theCounterYear= "));
  Serial.println(theCounterYear);
}
void clockPulseMonth() {
  Serial.print(F("++++ clockPulseMonth(), theCounterMonth= "));
  Serial.println(theCounterMonth);
}
void clockPulseDay() {
  Serial.print(F("+++ clockPulseDay(), theCounterDay= "));
  Serial.println(theCounterDay);
}
int theHour = 0;
void clockPulseHour() {
  // 12 hour AM/PM clock. Use 3, rather than 15.
  if (theCounterHours > 12) {
    theHour = theCounterHours - 12;
  } else if (theCounterHours == 0) {
    theHour = 12; // 12 midnight, 12am
  } else {
    theHour = theCounterHours;
  }
  printClockInt(thePrintColHour, printRowClockPulse, theHour);
  // sendByte2nano(theHour);
#ifdef CLOCK_MESSAGES
  Serial.print(F("++ clockPulseHour(), theCounterHours= "));
  Serial.print(theCounterHours);
  Serial.print(F(", theHour= "));
  Serial.println(theHour);
#endif
}
void clockPulseMinute() {
  printClockInt(thePrintColMin, printRowClockPulse, theCounterMinutes);
  // sevseg.setNumber(theCounterMinutes);
#ifdef CLOCK_MESSAGES
  Serial.println("");
  Serial.print(F("+ clockPulseMinute(), theCounterMinutes= "));
  Serial.println(theCounterMinutes);
#endif
}
void clockPulseSecond() {
  printClockInt(thePrintColSec, printRowClockPulse, theCounterSeconds);  // Column, Row
  // sevseg.setNumber(theCounterSeconds);
#ifdef CLOCK_MESSAGES
  if (theCounterSeconds == 1) {
    Serial.print(F("+ theCounterSeconds > "));
  } else if (theCounterSeconds == 21 || theCounterSeconds == 41) {
    Serial.println("");
    Serial.print(F("+ theCounterSeconds > "));
  }
  if (theCounterSeconds == 0) {
    Serial.print("60");
  } else {
    if (theCounterSeconds < 10) {
      Serial.print("0");
    }
    Serial.print(theCounterSeconds);
  }
  Serial.print(".");
#endif
}

#endif

// -----------------------------------------------------------------------------
char dayOfTheWeek[7][1] = {"S", "M", "T", "W", "T", "F", "S"};
#ifdef INCLUDE_CLOCK
#ifdef INCLUDE_LCD

void printClockInt(int theColumn, int theRow, int theInt) {
  lcd.setCursor(theColumn, theRow);    // Column, Row
  if (theInt < 10) {
    lcd.print("0");
    lcd.setCursor(theColumn + 1, theRow);
  }
  lcd.print(theInt);
}

void printClockDate() {
  now = rtc.now();
  theCounterYear = now.year();
  theCounterMonth = now.month();
  theCounterDay = now.day();
  //
  theCursor = printColClockDate;
  lcd.setCursor(theCursor, printRowClockDate);    // Column, Row
  lcd.print(dayOfTheWeek[now.dayOfTheWeek()]);
  // ---
  lcd.setCursor(++theCursor, printRowClockDate);    // Column, Row
  lcd.print(":");
  printClockByte(++theCursor, printRowClockDate, now.month());
  // ---
  theCursor = theCursor + 2;
  lcd.print("/");
  printClockByte(++theCursor, printRowClockDate, now.day());
}

void printClockByte(int theColumn, int theRow, char theByte) {
  int iByte = (char)theByte;
  lcd.setCursor(theColumn, theRow);    // Column, Row
  if (iByte < 10) {
    lcd.print("0");
    lcd.setCursor(theColumn + 1, theRow);
  }
  lcd.print(iByte);
}

#endif
#endif

// -----------------------------------------------------------------------
// Menu options and menu processing.

#ifdef INCLUDE_LCD
// Toggle the LCD backlight on/off.
boolean theLcdBacklightOn = true;
void toggleLcdBacklight() {
  if (theLcdBacklightOn) {
    theLcdBacklightOn = false;
    // Serial.println("+ Toggle: off.");
    lcd.noBacklight();
  } else {
    theLcdBacklightOn = true;
    // Serial.println("+ Toggle: on.");
    lcd.backlight();
  }
}
#endif

int theSetRow = 1;
int theSetCol = 0;
int theSetMin = 0;
int theSetMax = 59;
int setValue = 0;

int setClockValue = 0;
void cancelSet() {
  if (setClockValue) {
    // Serial.println("Cancel set.");
    displayPrintln(theSetRow, "");
    setClockValue = false;
  }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------
// Menu items to set the clock date and time values.

void setClockMenuItems() {
  if (!theLcdBacklightOn) {
    // Don't make clock setting changes when the LCD is off.
    return;
  }
  switch (setClockValue) {
    case 0:
      // Serial.print("Cancel set");
      displayPrintln(theSetRow, "");
      break;
    case 1:
      // Serial.print("seconds");
      displayPrintln(theSetRow, "Set:");
      theSetMax = 59;
      theSetMin = 0;
      theSetCol = thePrintColSec;
      setValue = theCounterSeconds;
      printClockInt(theSetCol, theSetRow, setValue);
      break;
    case 2:
      // Serial.print("minutes");
      displayPrintln(theSetRow, "Set:");
      theSetMax = 59;
      theSetMin = 0;
      theSetCol = thePrintColMin;
      setValue = theCounterMinutes;
      printClockInt(theSetCol, theSetRow, setValue);
      break;
    case 3:
      // Serial.print("hours");
      displayPrintln(theSetRow, "Set:");
      theSetMax = 24;
      theSetMin = 0;
      theSetCol = thePrintColHour;
      setValue = theCounterHours;
      printClockInt(theSetCol, theSetRow, setValue);
      break;
    case 4:
      // Serial.print("day");
      displayPrintln(theSetRow, "Set day:");
      theSetMax = 31;
      theSetMin = 1;
      theSetCol = thePrintColMin;
      setValue = theCounterDay;
      printClockInt(theSetCol, theSetRow, setValue);
      break;
    case 5:
      // Serial.print("month");
      displayPrintln(theSetRow, "Set month:");
      theSetMax = 12;
      theSetMin = 1;
      theSetCol = thePrintColMin;
      setValue = theCounterMonth;
      printClockInt(theSetCol, theSetRow, setValue);
      break;
    case 6:
      // Serial.print("year");
      displayPrintln(theSetRow, "Set year:");
      theSetMax = 2525; // In the year 2525, If man is still alive, If woman can survive...
      theSetMin = 1795; // Year John Keats the poet was born.
      theSetCol = thePrintColMin;
      setValue = theCounterYear;
      printClockInt(theSetCol, theSetRow, setValue);
      break;
  }
}

void infraredSwitch() {
  switch (results.value) {
    case 0xFFFFFFFF:
      // Ignore. This is from holding the key down.
      break;
    // -----------------------------------
    case 0xFF5AA5:
    case 0xE0E046B9:
      // Serial.print("+ Key > - next");
      // Serial.print(", set clock menu option");
      setClockValue--;
      if (setClockValue < 0) {
        setClockValue = 6;
      }
      setClockMenuItems();
      // Serial.println(".");
      break;
    case 0xFF10EF:
    case 0xE0E0A659:
      // Serial.print("+ Key < - previous");
      // Serial.print(", set clock menu option");
      setClockValue++;
      if (setClockValue > 6) {
        setClockValue = 0;
      }
      setClockMenuItems();
      // Serial.println(".");
      break;
    case 0xFF18E7:
    case 0xE0E006F9:
      // Serial.print("+ Key up");
      if (setClockValue) {
        // Serial.print(", increment");
        setValue++;
        if (setValue > theSetMax) {
          setValue = theSetMin;
        }
        printClockInt(theSetCol, theSetRow, setValue);
      }
      // Serial.println(".");
      break;
    case 0xFF4AB5:
    case 0xE0E08679:
      // Serial.print("+ Key down");
      if (setClockValue) {
        // Serial.print(", decrement");
        setValue--;
        if (setValue < theSetMin) {
          setValue = theSetMax;
        }
        printClockInt(theSetCol, theSetRow, setValue);
      }
      // Serial.println(".");
      break;
    case 0xFF38C7:
    case 0xE0E016E9:
      // Serial.print("+ Key OK");
      if (setClockValue) {
        // Serial.print(", set ");
        switch (setClockValue) {
          case 1:
            // Serial.print("seconds");
            theCounterSeconds = setValue;
            printClockInt(theSetCol, printRowClockPulse, setValue);
            break;
          case 2:
            // Serial.print("minutes");
            theCounterMinutes = setValue;
            printClockInt(theSetCol, printRowClockPulse, setValue);
            break;
          case 3:
            // Serial.print("hours");
            theCounterHours = setValue;
            printClockInt(theSetCol, printRowClockPulse, setValue);
            break;
          case 4:
            // Serial.print("day");
            theCounterDay = setValue;
            break;
          case 5:
            // Serial.print("month");
            theCounterMonth = setValue;
            break;
          case 6:
            // Serial.print("year");
            theCounterYear = setValue;
            break;
        }
        // The following offsets the time to make the change.
        // Else, the clock looses about second each time a setting is made.
        theCounterSeconds ++;
        delay(100);
        //
        rtc.adjust(DateTime(theCounterYear, theCounterMonth, theCounterDay, theCounterHours, theCounterMinutes, theCounterSeconds));
        displayPrintln(theSetRow, "Value is set.");
        printClockDate();
        delay(2000);
        displayPrintln(theSetRow, "");
      }
      // Serial.println(".");
      //
      setClockValue = false;
      delay(200);   // To block the double press.
      break;
    // -----------------------------------
    case 0xFF6897:
    case 0xE0E01AE5:
      // Serial.print("+ Key * (Return): ");
      // Serial.println("Cancel set.");
      cancelSet();
      break;
    case 0xFFB04F:
    case 0xE0E0B44B:
      // Serial.print("+ Key # (Exit): ");
      // Serial.println("Cancel set and Toggle display on/off.");
      cancelSet();
      toggleLcdBacklight();
      break;
    // -----------------------------------
    case 0xFF9867:
    case 0xE0E08877:
      Serial.print("+ Key 0:");
      Serial.println("");
      break;
    case 0xFFA25D:
    case 0xE0E020DF:
      Serial.print("+ Key 1: ");
      Serial.println("");
      break;
    case 0xFF629D:
      Serial.print("+ Key 2: ");
      Serial.println("");
      break;
    case 0xFFE21D:
      Serial.print("+ Key 3: ");
      Serial.println("");
      break;
    case 0xFF22DD:
      Serial.print("+ Key 4: ");
      Serial.println("");
      break;
    case 0xFF02FD:
      Serial.print("+ Key 5: ");
      Serial.println("");
      break;
    case 0xFFC23D:
      Serial.print("+ Key 6: ");
      Serial.println("");
      break;
    case 0xFFE01F:
      Serial.print("+ Key 7: ");
      Serial.println("");
      break;
    case 0xFFA857:
      Serial.print("+ Key 8: ");
      Serial.println("");
      break;
    case 0xFF906F:
      Serial.print("+ Key 9: ");
      Serial.println("");
      break;
    // -----------------------------------
    default:
      Serial.print("+ Result value: ");
      Serial.println(results.value, HEX);
      // -----------------------------------
  } // end switch

  irrecv.resume();

}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  // Give the serial connection time to start before the first print.
  delay(1000);
  Serial.println(""); // Newline after garbage characters.
  Serial.println("+++ Setup.");

  // ----------------------------------------------------
  irrecv.enableIRIn();
  Serial.println(F("+ Infrared receiver enabled."));

  // ----------------------------------------------------
#ifdef INCLUDE_LCD
  lcd.init();
  lcd.backlight();
  //                1234567890123456
  displayPrintln(0, "Ready...");
  displayPrintln(1, "Altair 101");
  delay(3000);
  displayPrintln(0, "        ");
  Serial.println(F("+ LCD ready for output."));
#endif

  // ----------------------------------------------------
#ifdef INCLUDE_CLOCK
  // RTC: Real Time Clock
  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }
  //
  // Set for testing.
  // rtc.adjust(DateTime(2019, 10, 22, 23, 59, 56));
  // delay(100);
  //
  syncCountWithClock();
  Serial.println(F("+ Clock set and synched with program variables."));
#endif

#ifdef INCLUDE_PCF8574
  // ------------------------------
  // I2C Two Wire + interrupt initialization
  pcf20.begin();
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), pcf20interrupt, CHANGE);
  Serial.println("+ PCF module initialized.");
#endif

#ifdef INCLUDE_SDCARD
  // ----------------------------------------------------
  int programSize = sizeof(theProgram);
  // List a program.
  listByteArray(theProgram, programSize);
  // Load a program.
  copyByteArrayToMemory(theProgram, programSize);

  // ----------------------------------------------------
  // Note, csPin is optional. The default is the hardware SS line (pin 10) of the SPI bus.
  // If using pin, other than 10, add: pinMode(otherPin, OUTPUT);
  // The pin connected to the chip select pin (CS) of the SD card.
  if (!SD.begin(csPin)) {
    Serial.println(F("- Error initializing SD card."));
    return; // When used in setup(), causes jump to loop().
  }
  Serial.println(F("+ SD card initialized."));
  //
  // Write the file to the SD card.
  // writeProgramMemoryToFile("dad.asm");
  //
  // Read the SD card file data into an array.
  readProgramFileIntoMemory("dad.asm");
#endif

  // -------------------------------
  Serial.println(F("+++ Go to loop."));

#ifdef INCLUDE_CLOCK
  Serial.print(F("+ theCounterSeconds > "));
#endif
}

// -----------------------------------------------------------------------------
// Device Loop

#ifdef INCLUDE_CLOCK
static unsigned long clockTimer = millis();
#endif
void loop() {

  // Process infrared key presses.
  if (irrecv.decode(&results)) {
    infraredSwitch();
  }

#ifdef INCLUDE_CLOCK
  // Check the clock and pulse when the clock's second value changes.
  if (millis() - clockTimer >= 200) {
    processClockNow();
    clockTimer = millis();
  }
#else
  Serial.println("+ Looping");
  delay(10000);
#endif

#ifdef INCLUDE_PCF8574
  if (runProgram) {
    if (switchSetOn) {
      // Serial.println("+ runProgram = true, switchSetOn is true.");
      runningSwitches();
      delay(30);           // Handle switch debounce.
      switchSetOn = false;
    }
    // ----------------------------
  } else {
    if (switchSetOn) {
      // Serial.println("+ runProgram = false, switchSetOn is true.");
      controlSwitches();
      delay(30);           // Handle switch debounce.
      switchSetOn = false;
    }
    // ----------------------------
      delay(30);
  }
#endif

}
// -----------------------------------------------------------------------------
