// -----------------------------------------------------------------------------
#include <Arduino.h>

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Comment out the following, if compiling for Altair101a,
//    which does not have an MP3 player, real time clock, or an SD card module.
// #define Altair101b 1

#ifdef Altair101b
#define SOFTWARE_NAME "Altair101b"
#define SOFTWARE_VERSION "1.63.b"
#else
#define SOFTWARE_NAME "Altair101a"
#define SOFTWARE_VERSION "1.63.a"
#endif

// -----------------------------------------------------------------------------
// From cpucore_i8080.h
extern word status_wait;
extern word status_hlda;
extern word status_inte;
//
extern byte fpStatusByte;         // Status byte bits: MEMR INP M1 OUT HLTA STACK WO INT
extern uint16_t fpAddressWord;    // Status byte bits: MEMR INP M1 OUT HLTA STACK WO INT
extern byte fpDataByte;           // Front panel data byte.

// -----------------------------------------------------------------------------
// Altair101a/b.ino program functions used in other programs.

// Program states
#define LIGHTS_OFF 0
#define PROGRAM_WAIT 1
#define PROGRAM_RUN 2
#define PROGRAM_LOAD 6
#define CLOCK_RUN 3
#define PLAYER_RUN 4
#define SERIAL_DOWNLOAD 5
#define SDCARD_RUN 7
extern int programState;

void singleStepWait();                      // Wait for "s" when single stepping.
void printFrontPanel();                     // Print the front panel display.
void printData(byte theByte);               // To echo data bytes.
void printByte(byte theByte);               // To echo bytes.
void Serial_print(String regAdata);         // Print a string to the Serial or Serial2.
byte altair_in(byte addr);                  // Called from cpu_in();
void altair_out(byte addr, byte val);       // Called from cpu_out();
void altair_hlt();                          // Called from cpu_hlt();

extern void playerLights(uint8_t statusByte, uint8_t playerVolume, uint8_t songNumberByte);

// -----------------------------------------------------------------------------
// From AltairSample.cpp program.

void loadProgram();
void loadProgramList();

void loadBasic4kArray();
void loadKillTheBitArray();
void loadKillTheBit();
void loadMviRegisters();

// -----------------------------------------------------------------------------
// From Altair101b.ino program, for AltairSample.cpp.
void printByte(byte b);
void printOctal(byte b);
void printHex(byte b);

// From Altair101b.ino program, for other programs.
extern boolean VIRTUAL_FRONT_PANEL;
extern void initVirtualFrontPanel();
extern void printVirtualFrontPanel();
extern uint16_t fpAddressToggleWord;
//
extern void setAddressData(uint16_t addressWord, byte dataByte);
extern String loadProgramName;
extern byte readByte;

// From here, for the Mp3Player.cpp program.
extern void ledFlashError();
extern void ledFlashSuccess();
extern byte hwStatus;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Infrared definitions that can be used in multiple programs.
// Motherboard Specific setup for Infrared and DFPlayer communications

// -----------------------------------------------
#if defined(__AVR_ATmega2560__)
// ------------------
#include <IRremote.h>
// ------------------
// Mega uses a hardware serial port (RX/TX) for communications with the DFPlayer.
// For Arduino Mega, I use pin 24 because it's closer to where I'm doing my wiring.
//  Feel free to use another digital or analog pin.
#define IR_PIN 24
//
// -----------------------------------------------
#elif defined(__SAM3X8E__)
// ------------------
#include <IRremote2.h>    // Special infrared library for the Due.
// For Arduino Due, I use pin 24 because it's closer to where I'm doing my wiring.
//  Feel free to use another digital or analog pin.
#define IR_PIN 24
// ------------------
// Due uses a hardware serial port (RX/TX) for communications with the DFPlayer.
//
// -----------------------------------------------
#else
// ------------------
#include <IRremote.h>
// Digital and analog pins work. Also tested with other analog pins.
#define IR_PIN A1
#endif

// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
// Add other header files so that Altair101a will compile.


// -----------------------------------------------------------------------------
// eof
