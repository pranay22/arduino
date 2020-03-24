// -----------------------------------------------------------------------------
/*
  Micro SD Card Module

  + Read/Write to SD card
  https://www.youtube.com/watch?v=8MvRRNYxy9c

  SPI Reader Micro SD Memory Card TF Memory Card Shield Module for Arduino $1.00
  https://lastminuteengineers.com/arduino-micro-sd-card-module-tutorial/
  https://www.instructables.com/id/Micro-SD-Card-Tutorial/

  //     Mega Nano - SPI module pins
  // Pin 53   10   - CS   : chip/slave select (SS pin). Can be any master(Nano) digital pin to enable/disable this device on the SPI bus.
  // Pin 52   13   - SCK  : serial clock, SPI: accepts clock pulses which synchronize data transmission generated by Arduino.
  // Pin 51   11   - MOSI : master out slave in, SPI: input to the Micro SD Card Module.
  // Pin 50   12   - MISO : master in slave Out, SPI: output from the Micro SD Card Module.
  // Pin 5V+  - VCC  : can use 3.3V or 5V
  // Pin GND  - GND  : ground
  // Notes,
        + pins are declared in the SPI library for SCK, MOSI, and MISO.
        + This program was tested successfully with a Mega and a Nano.

  About CS:
    Since this library supports only master mode, the Nano SS pin (pin 10) must be set as OUTPUT or the SD library functions will not work.
    You can use a different Nano pin to connect to the SD card chip select pin (CS), other than the hardware SS pin (pin 10).
    This allows you to have multiple SPI devices sharing the same MISO, MOSI, and CLK lines.
        When a device's Slave Select (CS) pin is low, it communicates with the master.
        When it's high, it ignores the master.

  Sample programs: Arduino IDE, File/Examples/SD/ReadWrite.

  Notes,
    Only one file can be open at a time.
    The card must be formatted FAT16 or FAT32. Optional formatting utility:
      https://www.sdcard.org/downloads/formatter_4/index.html
    Can open files in a directory. For example, SD.open("/myfiles/example.txt").
    The SD card library uses short (8.3, example a2345678.txt) filenames.
    File names do not have ‘case’ sensitivity.
    Works fine with Nano because it has it's own voltage convertor: from 3.3V – 6V to 3.3V.

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
// SPI reference: https://www.arduino.cc/en/Reference/SPI
// SD card library reference: https://www.arduino.cc/en/reference/SD

#include <SPI.h>
#include <SD.h>

String theFilename = "f1.txt";  // Files are created using uppercase: F1.TXT.

// Set to match your SD module to the Nano pin.
// The CS pin is the only one that is not really fixed as any of the Arduino digital pin.
// const int csPin = 10;  // SD Card module is connected to Nano pin 10.
const int csPin = 53;  // SD Card module is connected to Mega pin 53.
File myFile;

// -----------------------------------------------------------------------------
// Open and write a file.

void openWriteFile() {
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
}

// -----------------------------------------------------------------------------
// Open and read a file.

void openReadFile() {
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
}

// -----------------------------------------------------------------------------
// Delete the file and confirm it was deleted.

void deleteAndCheckFile() {
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
}

// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  // Give the serial connection time to start before the first print.
  delay(1000);
  Serial.println(""); // Newline after garbage characters.
  Serial.println("+++ Setup.");

  // Note, csPin is optional. The default is the hardware SS line (pin 10) of the SPI bus.
  // If using pin, other than 10, add: pinMode(otherPin, OUTPUT);
  // The pin connected to the chip select pin (CS) of the SD card.
  if (!SD.begin(csPin)) {
    Serial.println("- Error initializing SD card.");
    return; // When used in setup(), causes jump to loop().
  }
  Serial.println("+ SD card initialized.");
  
  // Start by ensuring that the files does not exist.
  deleteAndCheckFile();

  Serial.println("+++ Go to loop.");
}

// -----------------------------------------------------------------------------
// Device Loop
void loop() {
  Serial.println("");
  Serial.println("---------------------------------------------");
  Serial.println("+ Loop: do a number of reads and writes.");
  Serial.println("---------------------------------------------");
  Serial.println("");
  openWriteFile();
  delay(3000);
  openReadFile();
  delay(3000);
  Serial.println("");
  Serial.println("---------------------------------------------");
  openWriteFile();
  delay(3000);
  openReadFile();
  delay(3000);
  //
  Serial.println("---------------------------------------------");
  deleteAndCheckFile();
  delay(10000);
}
// -----------------------------------------------------------------------------
