// -----------------------------------------------------------------------------
/*
  Micro SD Card Module

  + Read/Write to SD card
  https://www.youtube.com/watch?v=8MvRRNYxy9c

  SPI Reader Micro SD Memory Card TF Memory Card Shield Module for Arduino $1.00
  https://lastminuteengineers.com/arduino-micro-sd-card-module-tutorial/
  https://www.instructables.com/id/Micro-SD-Card-Tutorial/

  SD Card Module uses the standard SPI interface for communication.
  SCK, MOSI, MISO are the SPI hardware pins. These pins are declared in the SPI library.
  Connect to Nana:
    CS (chip select)             to Nano pin 10 (can be any digital pin) used by Arduino(Master) to enable and disable specific devices on SPI bus.
    SCK (serial clock)           to Nano pin 13 accepts clock pulses which synchronize data transmission generated by Arduino
    MOSI (master out slave in)   to Nano pin 11 SPI input to the Micro SD Card Module.
    MISO (master in slave Out)   to Nano pin 12 SPI output from the Micro SD Card Module.
    VCC (3.3V or 5V)             to Nano pin 5V
    GND (ground)                 to Nano pin GRN

    Sample programs: Arduino IDE, File/Examples/SD/ReadWrite.

  Notes,
    Only one file can be open at a time.
    The card must be formatted FAT16 or FAT32. Optional formatting utility:
      https://www.sdcard.org/downloads/formatter_4/index.html
    Can open files in a directory. For example, SD.open("/myfiles/example.txt").
    The SD card library does not support ‘long filenames’.
    File names do not have ‘case’ sensitivity.

  Other functions:
    File exists: SD.exists("filename.txt")
    Delete a file: SD.remove("unwanted.txt")
    Create a subdirectory: mkdir("/mynewdir")
    Determine if a file is a directory: isDirectory().
    
    To get a file directory, open the first file,
      then use the following open each next files and get the name:
      openNextFile() and Serial.print( name() ).
      See example: listfiles.
*/
// -----------------------------------------------------------------------------
// Used with the SD Card module.
#include <SPI.h>
#include <SD.h>

String theFilename = "f1.txt";  // Files are created using uppercase: F1.TXT.

// Set to match your SD module to the Nano pin.
// The CS pin is the only one that is not really fixed as any of the Arduino digital pin.
// const int chipSelect = 10;  // SD Card module is connected to Nano pin 10.
File myFile;

// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  // Give the serial connection time to start before the first print.
  delay(1000);
  Serial.println(""); // Newline after gargage characters.
  Serial.println("+++ Setup.");

  // CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)
  // if (!SD.begin(4)) {
  if (!SD.begin()) {
    Serial.println("- Error initializing SD card.");
    return; // When used in setup(), causes jump to loop().
  }
  Serial.println("+ SD card initialized.");

  Serial.println("+ Open and write to the file.");
  myFile = SD.open(theFilename, FILE_WRITE);
  if (!myFile) {
    Serial.print("- Error opening file: ");
    Serial.println(theFilename);
    return; // When used in setup(), causes jump to loop().
  }
  Serial.print("++ File open: ");
  Serial.print(theFilename);
  Serial.println(", write text to file.");
  //
  // Starts writing from the end of file, i.e. appends to the file.
  myFile.println("Hello there,");
  myFile.println("Line 2: 1, 2, 3.");
  myFile.println("Last line.");
  myFile.close();
  Serial.println("+ File closed.");

  Serial.println("+ Open read from the file.");
  myFile = SD.open(theFilename);
  if (!myFile) {
    Serial.print("- Error opening file: ");
    Serial.println(theFilename);
    return; // When used in setup(), causes jump to loop().
  }
  while (myFile.available()) {
    // Reads one character at a time.
    // Serial.print("+ :");
    // Serial.print(myFile.read());  // Prints ascii character number, one per-line.
    // Serial.println(":");
    Serial.write(myFile.read());  // Prints as it was written.
  }
  myFile.close();
  Serial.println("+ File closed.");

  Serial.print("+ Delete the file: ");
  Serial.println(theFilename);
  SD.remove(theFilename);
  //
  Serial.println("+ Check if file exists or not.");
  if (SD.exists(theFilename)) {
    Serial.print("++ Exists: ");
  } else {
    Serial.print("++ Doesn't exist: ");
  }
  Serial.println(theFilename);

  Serial.println("+++ Go to loop.");
}

// -----------------------------------------------------------------------------
// Device Loop
void loop() {
  delay(3000);
  Serial.println("+ Looping.");
}
