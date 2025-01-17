// -----------------------------------------------------------------------------
#include <Arduino.h>

#define READ_FILE 1
#define WRITE_FILE 2

// -----------------------------------------------------------------------------
// Altair101a.ino program functions used in cpucore_i8080.cpp
// Program states
#define PROGRAM_WAIT 1
#define SERIAL_DOWNLOAD 5
#define SDCARD_RUN 7

extern int programState;
// extern void ledFlashError();
// extern void ledFlashSuccess();
extern byte hwStatus;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// From cpuIntel8080.h

#define MEMSIZE 1024

byte Mem[MEMSIZE];

#define MWRITE(a,v) { Mem[a]=v; }
#define MREAD(a)    (Mem[a])

// byte MEM_READ_STEP(uint16_t a);
void MEM_WRITE_STEP(uint16_t a, byte v);

// -----------------------------------------
// Read

byte MEM_READ(uint16_t memoryAddress) {
  byte returnByte;
  returnByte = MREAD(memoryAddress);
  Serial.print(F("+ MEM_READ, memoryAddress="));
  Serial.print(memoryAddress);
  Serial.print(F(", returnByte="));
  Serial.println(returnByte);
  /*
    host_set_status_leds_READMEM();
    host_set_addr_leds( memoryAddress );
    host_set_data_leds( returnByte );
    if (status_wait) {
    singleStepWait();
    } else {
    printFrontPanel();  // Status, data/address light values are already set.
    }
  */
  return returnByte;
}



// -----------------------------------------------------------------------------
// eof
