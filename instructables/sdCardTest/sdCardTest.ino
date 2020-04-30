// -----------------------------------------------------------------------------
/*
  Micro SD Card Module

  Connections,
      Mega Nano - SPI module pins
  Pin 53   10   - CS   : chip/slave select (SS pin). Can be any master(Nano) digital pin to enable/disable this device on the SPI bus.
  Pin 52   13   - SCK  : serial clock, SPI: accepts clock pulses which synchronize data transmission generated by Arduino.
  Pin 51   11   - MOSI : master out slave in, SPI: input to the Micro SD Card Module.
  Pin 50   12   - MISO : master in slave Out, SPI: output from the Micro SD Card Module.
  Pin 5V+  5V+  - VCC  : can use 3.3V or 5V
  Pin GND  GND  - GND  : ground
  Notes,
  + This program was tested successfully with a Mega and a Nano.
  + Pins are declared in the SPI library for SCK, MOSI, and MISO.

  Other sample programs from the Arduino IDE menu: File/Examples/SD/ReadWrite.

  Reference documentation,
    https://www.arduino.cc/en/reference/SD
  Get SD card information,
    https://www.arduino.cc/en/Tutorial/CardInfo
  List files,
    https://www.arduino.cc/en/Tutorial/listfiles
  SPI reference:
    https://www.arduino.cc/en/Reference/SPI

  Notes,
    Uses short filenames: 8 character plus 3 for the filename extension, for example a2345678.txt.
    File names are not case sensitive.
    Only one file can be open at a time.
    The card must be formatted FAT16 or FAT32.
    Can open files in a directory. For example, SD.open("/myfiles/example.txt").
    Input voltage: from 3.3V – 5V.

  Other related links,
  https://www.instructables.com/id/Micro-SD-Card-Tutorial/
  https://www.youtube.com/watch?v=8MvRRNYxy9c
  https://lastminuteengineers.com/arduino-micro-sd-card-module-tutorial/

*/
// -----------------------------------------------------------------------------
// Used with the SD Card module.
// I have SD library by Arduiono, SparkFun version 1.2.3.

#include <SPI.h>
#include <SD.h>

String theFilename = "f1.txt";  // Files are created using uppercase: F1.TXT.

// Set to match your SD module to the Arduino pin.
// The CS pin is the only one that is not really fixed as any of the Arduino digital pin.
const int csPin = 10;  // SD Card module is connected to Nano pin 10.
// const int csPin = 53;  // SD Card module is connected to Mega pin 53.

File myFile;
File root;

// -----------------------------------------------------------------------------
int HLDA_PIN = 13;  // For testing, flash the onboard LED light.

void ledFlashSuccess() {
  int delayTime = 200;
  for (int i = 0; i < 3; i++) {
    digitalWrite(HLDA_PIN, HIGH);
    delay(delayTime);
    digitalWrite(HLDA_PIN, LOW);
    delay(delayTime);
  }
}
void ledFlashError() {
  int delayTime = 500;
  for (int i = 0; i < 6; i++) {
    digitalWrite(HLDA_PIN, HIGH);
    delay(delayTime);
    digitalWrite(HLDA_PIN, LOW);
    delay(delayTime);
  }
}

// -----------------------------------------------------------------------------
// --------------------------------------------------------------------------
// Handle the case if the card is not inserted. Once inserted, the module will be re-initialized.
boolean sdcardFailed = false;
void initSdcard() {
  sdcardFailed = false;
  if (!SD.begin(csPin)) {
    sdcardFailed = true;
    Serial.println(F("- Error initializing SD card."));
    return;
  }
  Serial.println(F("+ SD card initialized."));
}

// -----------------------------------------------------------------------------
// Open and write a file.

void openWriteFile() {
  if (sdcardFailed) {
    initSdcard();
  }
  myFile = SD.open(theFilename, FILE_WRITE);
  if (!myFile) {
    Serial.print(F("- Error opening file: "));
    Serial.println(theFilename);
    ledFlashError();
    sdcardFailed = true;
    return;
  }
  if (SD.exists(theFilename)) {
    Serial.print(F("+ File exists, append text to file: "));
  } else {
    Serial.print(F("+ Write text into the file: "));
  }
  Serial.println(theFilename);
  //
  // Starts writing from the end of file, i.e. appends to the file.
  myFile.println(F("Hello there,"));
  myFile.println(F("Line 2: 1, 2, 3."));
  myFile.println(F("Last line."));
  myFile.close();
  Serial.println(F("+ File closed."));
  ledFlashSuccess();
}

// -----------------------------------------------------------------------------
// Open and read a file.

void openReadFile() {
  if (sdcardFailed) {
    initSdcard();
  }
  Serial.println(F("+ Open read from the file."));
  myFile = SD.open(theFilename);
  if (!myFile) {
    Serial.print(F("- Error opening file: "));
    Serial.println(theFilename);
    ledFlashError();
    sdcardFailed = true;
    return;
  }
  while (myFile.available()) {
    // Reads one character at a time.
    // Serial.print("+ :");
    // Serial.print(myFile.read());  // Prints ascii character number, one per-line.
    // Serial.println(":");
    Serial.write(myFile.read());  // Prints as it was written.
  }
  myFile.close();
  Serial.println(F("+ File closed."));
  ledFlashSuccess();
}

// -----------------------------------------------------------------------------
// Delete the file and confirm it was deleted.

void deleteFileAndConfirm() {
  Serial.print(F("+ Delete the file: "));
  Serial.println(theFilename);
  SD.remove(theFilename);
  //
  Serial.println(F("+ Confirm file was deleted."));
  if (SD.exists(theFilename)) {
    Serial.print(F("++ File exists, it was not deleted: "));
    ledFlashError();
  } else {
    Serial.print(F("++ File was deleted: "));
    ledFlashSuccess();
  }
  Serial.println(theFilename);
}

// -----------------------------------------------------------------------------
void printSpacing(String theString) {
  for (int i = theString.length(); i < 14; i++) {
    Serial.print(" ");
  }
}

int numTabs;
void listDirectories(File dir, int numTabs) {
  String tabString = "   ";
  File entry =  dir.openNextFile();
  while (entry) {
    for (int i = 0; i < numTabs; i++) {
      Serial.print(tabString);
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      // Directory
      Serial.println("/");
      listDirectories(entry, numTabs + 1);
    } else {
      // File
      printSpacing(entry.name());
      Serial.println(entry.size(), DEC);
    }
    entry.close();
    entry =  dir.openNextFile();
  }
}

void listDirectory(File dir) {
  String tabString = "   ";
  File entry = dir.openNextFile();
  while (entry) {
    if (entry.isDirectory()) {
      Serial.print(F("++ Directory: "));
      Serial.print(entry.name());
    } else {
      Serial.print(F("++ File:      "));
      Serial.print(entry.name());
      printSpacing(entry.name());
      Serial.print(entry.size(), DEC);
    }
    Serial.println("");
    entry.close();
    entry =  dir.openNextFile();
  }
}

// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  // Give the serial connection time to start before the first print.
  delay(1000);
  Serial.println(""); // Newline after garbage characters.
  Serial.println(F("+++ Setup."));

  // Note, csPin is optional. The default is the hardware SS line (pin 10) of the SPI bus.
  // If using pin, other than 10, add: pinMode(otherPin, OUTPUT);
  // The pin connected to the chip select pin (CS) of the SD card.
  // Consider using: initSdcard();
  if (!SD.begin(csPin)) {
    Serial.println(F("- Error initializing SD card."));
    Serial.print(F("-- Check that SD card is inserted"));
    Serial.println(F(", and that SD card adapter is wired properly."));
  }
  int counter = 0;
  while (!SD.begin(csPin)) {
    Serial.print(".");
    if (counter == 30) {
      Serial.println("");
      counter = 0;
    }
    delay(500);
  }
  Serial.println("+ SD card initialized.");
  ledFlashSuccess();

  // -----------------------------------------------------------------------------
  Serial.println("");
  Serial.println(F("---------------------------------------------"));
  Serial.println(F("+ List SD card directories and files,"));
  root = SD.open("/");
  listDirectories(root, 0);
  Serial.println(F("+ End listing."));
  Serial.println("");

  String aDirName = "/TESTDIR";
  if (SD.exists(aDirName)) {
    Serial.print(F("+ Directory exists: "));
    Serial.println(aDirName);
  } else {
    Serial.print(F("+ Create directory: "));
    Serial.println(aDirName);
    SD.mkdir("/TESTDIR");
  }

  Serial.println(F("---------------------------------------------"));
  Serial.println(F("+ List SD card root directory,"));
  root.rewindDirectory();
  listDirectory(root);
  Serial.println(F("+ End listing."));

  if (SD.exists(aDirName)) {
    SD.rmdir(aDirName);
    Serial.print(F("+ Deleted directory: "));
    Serial.println(aDirName);
  }
  Serial.println("");

  // -----------------------------------------------------------------------------
  // Start by ensuring that the test files do not exist.
  deleteFileAndConfirm();

  Serial.println(F("+++ Go to loop."));
}

// -----------------------------------------------------------------------------
// Device Loop
void loop() {
  Serial.println("");
  Serial.println(F("---------------------------------------------"));
  Serial.println(F("+ Loop: do a number of reads and writes."));
  Serial.println(F("---------------------------------------------"));
  Serial.println("");
  openWriteFile();
  openReadFile();
  delay(3000);
  Serial.println("");
  Serial.println(F("---------------------------------------------"));
  openWriteFile();
  openReadFile();
  delay(3000);
  //
  Serial.println(F("---------------------------------------------"));
  deleteFileAndConfirm();
  delay(10000);

}
// -----------------------------------------------------------------------------
