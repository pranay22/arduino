// -----------------------------------------------------------------------------
/*
  Serial Communications using a Mega or Due which have multiple hardware ports

  Wire the serial component to the Mega Serial2 pins.
  Serial component RX to Mega TX pin 16.
  Serial component TX to Mega RX pin 17.

  View serial ports on a Mac for the Mega or Due default serial port(14120) and the component's port(14110):
  $ ls /dev/tty.*
  crw-rw-rw-  1 root  wheel   20,   0 Nov 13 15:20 /dev/tty.Bluetooth-Incoming-Port
  crw-rw-rw-  1 root  wheel   20, 0x00000104 Dec  5 20:13 /dev/tty.wchusbserial14110
  crw-rw-rw-  1 root  wheel   20, 0x00000106 Dec  5 20:13 /dev/tty.wchusbserial14120

  Arduino serial port reference:
    https://www.arduino.cc/reference/en/language/functions/communication/serial/

  Second serial port module that I bought, requireed a new driver.
  + CP2102 USB 2.0 to TTL UART Module 6 Pin Serial Converter STC FT232
  ++ Download driver:
      https://www.silabs.com/community/interface/knowledge-base.entry.html/2017/01/10/legacy_os_softwarea-bgvU
      http://www.silabs.com/Support%20Documents/Software/Mac_OSX_VCP_Driver_10_6.zip
  ++ I unzipped it and renamed it: CP2102-MacSerialDriver.dmg.
  + Next time consider buying a serial port module that matches the Arduino current CH340 USB driver:
  ++ CH340 USB To RS232 TTL Auto Converter Module Serial Port FOR Arduino STC TA-02L
*/
// -----------------------------------------------------------------------------
void setup() {
  // ------------------------
  // Serial speed needs to match the sending program speed such as the Arduino IDE monitor program.
  Serial.begin(9600);             // Sample baud rates: 9600 115200
  delay(1000);
  Serial.println("");               // Newline after garbage characters.
  Serial.println("+++ Setup");
  //
  Serial.println("+ Ready to use port 0 serial port with the Arduino Serial Monitor.");

  // ------------------------
  Serial2.begin(9600);
  Serial.println("+ SerialHw on the second port is listening.");
  Serial.println("+ Ready to use the second serial port.");

  // ------------------------
  Serial.println("+ Go to loop.");
  Serial.println("++ Send characters using the Arduino Serial Monitor.");
  Serial.print("+ Port 0: ");
  // ------------------------
  Serial2.print("+ Port Serial2: ");
}

// -----------------------------------------------------------------------------
// Device Loop
void loop() {
  byte readByte = 0;
  //--------------------------------------
  // Serial port 0 processing
  if (Serial.available() > 0) {
    // Read and process an incoming byte.
    readByte = Serial.read();
    // Process the byte.
    Serial.write(readByte);
    if (readByte == 10) {
      // Arduino IDE monitor line feed (LF).
      Serial.print("+ Port Serial: ");
    } else if (readByte == 13) {
      // Terminal uses carriage return (CR).
      Serial.println();
      Serial.print("+ Port Serial: ");
    }
  }
  //--------------------------------------
  // Hardware serial port processing
  if (Serial2.available() > 0) {
    // Read and process an incoming byte.
    readByte = Serial2.read();
    // Process the byte.
    Serial.write(readByte);                 // For testing with a serial upload program.
    Serial2.write(readByte);
    if (readByte == 10) {
      // IDE monitor (LF).
      Serial2.print("+ Port Serial2: ");
      Serial.print("+ Port Serial2: ");    // For testing with a serial upload program.
    } else if (readByte == 13) {
      // Terminal uses carriage return (CR).
      Serial2.println();
      Serial2.print("+ Port Serial2: ");
      Serial.println();                     // For testing with a serial upload program.
      Serial.print("+ Port Serial2: ");
    }
  }
}

// -----------------------------------------------------------------------------
// eof
