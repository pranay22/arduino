// -----------------------------------------------------------------------------
/*
  Processor
  + Using David Hansel's Altair 8800 Simulator code to process machine code instructions.
  + Interactivity is over the Arduino IDE monitor USB serial port.

  Next:
  + WAIT mode, WAIT light,
*/
// -----------------------------------------------------------------------------
#include "Altair101a.h"
#include "cpucore_i8080.h"

#define LOG_MESSAGES 1    // For debugging.

// -----------------------------------------------------------------------------
// For Byte bit comparisons.
#define BIT(n) (1<<(n))

// -----------------------------------------------------------------------------
byte opcode = 0xff;
static uint16_t p_regPC = 0xFFFF;
uint16_t controlSwitch = 0;
uint16_t addressSwitch = 0;

#define ST_MEMR    0x0080
#define ST_INP     0x0040
#define ST_M1      0x0020
#define ST_OUT     0x0010
#define ST_HLTA    0x0008
#define ST_STACK   0x0004
#define ST_WO      0x0002
//
// Byte bit status values
#define ST_WAIT    0x0800
#define ST_HLDA    0x0400

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// From Processor.ino

// Program states
#define LIGHTS_OFF 0
#define PROGRAM_WAIT 1
#define PROGRAM_RUN 2
#define CLOCK_RUN 3
#define PLAYER_RUN 4
#define SERIAL_DOWNLOAD 5
int programState = LIGHTS_OFF;  // Intial, default.

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Front Panel Status LEDs

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Output LED lights shift register(SN74HC595N) pins

//           Mega/Nano pins        74HC595 Pins
const int dataPinLed  = 5;    // pin 5 (was pin A14) Data pin.
const int latchPinLed = 6;    // pin 6 (was pin A12) Latch pin.
const int clockPinLed = 7;    // pin 7 (was pin A11) Clock pin.

void ledFlashSuccess() {}

// ------------------------------
// Status Indicator LED lights

// Program wait status.
const int WAIT_PIN = A9;      // Processor program wait state: off/LOW or wait state on/HIGH.

// HLDA : 8080 processor goes into a hold state because of other hardware running.
const int HLDA_PIN = A10;     // Emulator processing (off/LOW) or clock/player processing (on/HIGH).

// Use OR to turn ON. Example:
//  statusByte = statusByte | MEMR_ON;
const byte MEMR_ON =    B10000000;  // MEMR   The memory bus will be used for memory read data.
const byte INP_ON =     B01000000;  // INP    The address bus containing the address of an input device. The input data should be placed on the data bus when the data bus is in the input mode
const byte M1_ON =      B00100000;  // M1     Machine cycle 1, fetch opcode.
const byte OUT_ON =     B00010000;  // OUT    The address contains the address of an output device and the data bus will contain the out- put data when the CPU is ready.
const byte HLTA_ON =    B00001000;  // HLTA   Machine opcode hlt, has halted the machine.
const byte STACK_ON =   B00000100;  // STACK  Stack process
const byte WO_ON =      B00000010;  // WO     Write out (inverse logic)
const byte INT_ON =     B00000001;  // INT    Interrupt

// Use AND to turn OFF. Example:
//  statusByte = statusByte & M1_OFF;
const byte MEMR_OFF =   B01111111;
const byte INP_OFF =    B10111111;
const byte M1_OFF =     B11011111;
const byte OUT_OFF =    B11101111;
const byte HLTA_OFF =   B11110111;
const byte STACK_OFF =  B11111011;
const byte WO_OFF =     B11111101;
const byte INT_OFF =    B11111110;
// const byte WAIT_OFF =   B11111110;   // WAIT   Changed to a digital pin control.

byte readByte = 0;
// byte dataByte = 0;
byte statusByte = B00000000;        // By default, all are OFF.

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
char charBuffer[17];

void printByte(byte b) {
  for (int i = 7; i >= 0; i--)
    Serial.print(bitRead(b, i));
}
void printOctal(byte b) {
  String sValue = String(b, OCT);
  for (int i = 1; i <= 3 - sValue.length(); i++) {
    Serial.print("0");
  }
  Serial.print(sValue);
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
void print_panel_serial() {
  byte dbus;
  static uint16_t p_addressSwitch = 0, p_controlSwitch = 0, p_abus = 0xffff, p_dbus = 0xffff, p_status = 0xffff;
  uint16_t status, abus;

  status = host_read_status_leds();
  abus   = host_read_addr_leds();
  dbus   = host_read_data_leds();

  // Even if n change, print anyway.
  // if ( force || p_controlSwitch != controlSwitch || p_addressSwitch != addressSwitch || p_abus != abus || p_dbus != dbus || p_status != status ) {
  //
  // Status
  Serial.print(F("INTE PROT MEMR INP M1 OUT HLTA STACK WO INT  D7  D6  D5  D4  D3  D2  D1  D0\r\n"));
  if ( false  ) Serial.print(F(" *  "));    else Serial.print(F(" .  "));
  if ( false  ) Serial.print(F("  *  "));   else Serial.print(F("  .  "));
  if ( status & ST_MEMR  ) Serial.print(F("  *  "));   else Serial.print(F("  .  "));
  if ( status & ST_INP   ) Serial.print(F("  * "));    else Serial.print(F("  . "));
  if ( status & ST_M1    ) Serial.print(F(" * "));     else Serial.print(F(" . "));
  if ( status & ST_OUT   ) Serial.print(F("  * "));    else Serial.print(F("  . "));
  if ( status & ST_HLTA  ) Serial.print(F("  *  "));   else Serial.print(F("  .  "));
  if ( status & ST_STACK ) Serial.print(F("   *  "));  else Serial.print(F("   .  "));
  if ( status & ST_WO    ) Serial.print(F(" * "));     else Serial.print(F(" . "));
  if ( false   ) Serial.print(F("  *"));    else Serial.print(F("  ."));
  //
  // Data
  if ( dbus & 0x80 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( dbus & 0x40 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( dbus & 0x20 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( dbus & 0x10 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( dbus & 0x08 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( dbus & 0x04 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( dbus & 0x02 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( dbus & 0x01 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  //
  // Address
  Serial.print(("\r\nWAIT HLDA   A15 A14 A13 A12 A11 A10  A9  A8  A7  A6  A5  A4  A3  A2  A1  A0\r\n"));
  if ( host_read_status_led_WAIT() ) Serial.print(F(" *  "));   else Serial.print(F(" .  "));
  if ( false ) Serial.print(F("  *   ")); else Serial.print(F("  .   "));
  if ( abus & 0x8000 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( abus & 0x4000 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( abus & 0x2000 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( abus & 0x1000 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( abus & 0x0800 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( abus & 0x0400 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( abus & 0x0200 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( abus & 0x0100 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( abus & 0x0080 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( abus & 0x0040 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( abus & 0x0020 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( abus & 0x0010 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( abus & 0x0008 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( abus & 0x0004 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( abus & 0x0002 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( abus & 0x0001 ) Serial.print(F("   *")); else Serial.print(F("   ."));
  //
  // Address/Data switches
  Serial.print(F("\r\n            S15 S14 S13 S12 S11 S10  S9  S8  S7  S6  S5  S4  S3  S2  S1  S0\r\n"));
  Serial.print(F("          "));
  if ( addressSwitch & 0x8000 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( addressSwitch & 0x4000 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( addressSwitch & 0x2000 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( addressSwitch & 0x1000 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( addressSwitch & 0x0800 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( addressSwitch & 0x0400 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( addressSwitch & 0x0200 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( addressSwitch & 0x0100 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( addressSwitch & 0x0080 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( addressSwitch & 0x0040 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( addressSwitch & 0x0020 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( addressSwitch & 0x0010 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( addressSwitch & 0x0008 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( addressSwitch & 0x0004 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( addressSwitch & 0x0002 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( addressSwitch & 0x0001 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  Serial.print(F("\r\n ------ \r\n"));
  p_controlSwitch = controlSwitch;
  p_addressSwitch = addressSwitch;
  p_abus = abus;
  p_dbus = dbus;
  p_status = status;
  // }
  Serial.println("+ Ready to receive command.");
}

// -----------------------------------------------------------------------------
uint16_t host_read_status_leds() {
  uint16_t res;
  res = statusByteB;
  // res |= statusByteD & 0x80 ? ST_INTE : 0;
  // res |= statusByteG & 0x04 ? ST_PROT : 0;
  res |= statusByteG & 0x02 ? ST_WAIT : 0;
  res |= statusByteG & 0x01 ? ST_HLDA : 0;
  return res;
}

// -----------------------------------------------------------------------------
void altair_set_outputs(uint16_t a, byte v) {
  // Stacy, When not using serial, display on front panel lights.
  host_set_addr_leds(a);
  host_set_data_leds(v);
  // print_panel_serial();
}

void altair_out(byte dataByte, byte regAdata) {
  // Opcode: out <port>
  // Called from: cpu_OUT()
#ifdef LOG_MESSAGES
  Serial.print(F("< OUT, port# "));
  Serial.print(dataByte);
  Serial.print(". regA=");
  Serial.print(regAdata);
  Serial.print(".");
#endif
  host_set_addr_leds(dataByte | dataByte * 256);
  host_set_data_leds(regAdata);
  host_set_status_led_OUT();
  host_set_status_led_WO();
  //
  // stacy io_out(dataByte, regAdata);
  //
  // Actual output of bytes. Example output a byte to the serial port (IDE monitor).
  //
  if ( host_read_status_led_WAIT() ) {
    // If single stepping, need to wait.
    altair_set_outputs(dataByte | dataByte * 256, 0xff);
    singleStepWait();
  }
  host_clr_status_led_OUT();
  host_clr_status_led_WO();
}

void altair_wait_step() {
  //
  // Stacy, If WAIT mode, return to WAIT loop?
  // Also used in: MEM_READ_STEP(...) and MEM_WRITE_STEP(...).
  //
  // controlSwitch &= BIT(SW_RESET); // clear everything but RESET status
  /* Stacy, here is the loop for waiting when single stepping during WAIT mode.
    while ( host_read_status_led_WAIT() && (controlSwitch & (BIT(SW_STEP) | BIT(SW_SLOW) | BIT(SW_RESET))) == 0 ) {
    read_inputs();
    delay(10);
    }
  */
  // if ( controlSwitch & BIT(SW_SLOW) ) delay(500);
}

void singleStepWait() {
  // dave
  Serial.println(F("+ singleStepWait()"));
  print_panel_serial();           // Status, data/address lights already set.
  bool singleStepWaitLoop = true;
  while (singleStepWaitLoop) {
    if (Serial.available() > 0) {
      readByte = Serial.read();    // Read and process an incoming byte.
      if (readByte == 's') {
        singleStepWaitLoop = false;
        // processRunSwitch(readByte);
      }
    }
  }
  // From previous:
  // Stacy, If WAIT mode, return to WAIT loop?
  // Also used in: MEM_READ_STEP(...) and MEM_WRITE_STEP(...).
  //
  // controlSwitch &= BIT(SW_RESET); // clear everything but RESET status
  /* Stacy, here is the loop for waiting when single stepping during WAIT mode.
    while ( host_read_status_led_WAIT() && (controlSwitch & (BIT(SW_STEP) | BIT(SW_SLOW) | BIT(SW_RESET))) == 0 ) {
    read_inputs();
    delay(10);
    }
  */
  // if ( controlSwitch & BIT(SW_SLOW) ) delay(500);

}

// -----------------------------------------------------------------------------
void read_inputs() {
  byte readByte;
  readByte = "";
  // controlSwitch = 0;
  // ---------------------------
  // Device read options.
  // read_inputs_panel();
  //
  // dave, implement:
  if ( config_serial_input_enabled() ) {
    read_inputs_serial();
  }
  // ---------------------------
  if (readByte != "") {
    // processWaitSwitch(readByte);
  }
}
void read_inputs_panel() {
  // we react on positive edges on the function switches...
  // controlSwitch = host_read_function_switches_edge();
  // ...except for the SLOW switch which is active as long as it is held down
  // if ( host_read_function_switch_debounced(SW_SLOW) ) controlSwitch |= BIT(SW_SLOW);
  // #if STANDALONE==0
  //   addressSwitch = host_read_addr_switches();
  // #endif
}
void read_inputs_serial() {
  return;
  if (Serial.available() > 0) {
    readByte = Serial.read();    // Read and process an incoming byte.
    processRunSwitch(readByte);
  }
}

byte altair_in(byte port) {
  // Opcode: out <port>
  // cpu_OUT()
  byte data = 0;
  return data;
}

// -----------------------------------------------------------------------------
void altair_hlt() {
  host_set_status_led_HLTA();
  // regPC--;
  // altair_interrupt(INT_SW_STOP);
  programState = PROGRAM_WAIT;
  Serial.print(F("++ HALT, host_read_status_led_WAIT() = "));
  Serial.println(host_read_status_led_WAIT());
  if (!host_read_status_led_WAIT()) {
    host_set_status_led_WAIT();
    print_panel_serial();
  }
}

void processDataOpcode() {
#ifdef LOG_MESSAGES
  Serial.print(F("++ regPC:"));
  Serial.print(regPC);
  Serial.print(F(": data:"));
  printData(MREAD(regPC));
  Serial.println("");
#endif
  //
  host_set_status_leds_READMEM_M1();
  host_set_addr_leds(regPC);
  opcode = MREAD(regPC);
  host_set_data_leds(opcode);
  regPC++;
  //
  host_clr_status_led_M1();
  CPU_EXEC(opcode);
  host_set_status_led_MEMR();
  host_set_status_led_M1();
  host_clr_status_led_WO();
}

void processRunSwitch(byte readByte) {
  switch (readByte) {
    case 's':
      Serial.println(F("+ STOP"));
      programState = PROGRAM_WAIT;
      host_set_status_led_WAIT();
      break;
    case 'R':
      Serial.println(F("+ RESET"));
      // Stacy, For now, do EXAMINE 0 to reset to the first memory address.
      regPC = 0;
      p_regPC = ~regPC;
      altair_set_outputs(regPC, MREAD(regPC));
    // Then continue running.
    // -------------------------------------
    case 10:
      // New line character.
      // For testing, require enter key from serial to run the next opcode. This is like a step operation.
      // processRunSwitch(readByte);
      break;
    default:
      // Serial.println(F("+ Default"));
      break;
  }
}

void runProcessor() {
  Serial.println(F("+ runProcessor()"));
  // put PC on address bus LEDs
  host_set_addr_leds(regPC);
  // Serial.println(F("+ Send serial character, example hit enter key, to process first opcode. Send 's' to STOP running."));
  programState = PROGRAM_RUN;
  while (programState == PROGRAM_RUN) {
    processDataOpcode(); // For now, require an serial character to process each opcode.
    if (Serial.available() > 0) {
      readByte = Serial.read();    // Read and process an incoming byte.
      processRunSwitch(readByte);
    }
  }
}

// -----------------------------------------------------------------------------
void processWaitSwitch(byte readByte) {
  uint16_t cnt;
  //
  // Process an address/data toggle.
  //
  int data = readByte;
  if ( data >= '0' && data <= '9' ) {
    addressSwitch = addressSwitch ^ (1 << (data - '0'));
    return;
  }
  if ( data >= 'a' && data <= 'f' ) {
    addressSwitch = addressSwitch ^ (1 << (data - 'a' + 10));
    return;
  }
  //
  // Process command switches. Tested: RUN, EXAMINE, EXAMINE NEXT, DEPOSIT, DEPOSIT NEXT
  //
  switch (readByte) {
    case 'r':
      Serial.println("+ r, RUN.");
      host_clr_status_led_WAIT();
      host_clr_status_led_HLTA();
      // runProcessor();
      // dave
      programState = PROGRAM_RUN;
      break;
    case 's':
      Serial.println("+ s, SINGLE STEP: ");
      host_clr_status_led_HLTA();
      processDataOpcode();
      break;
    case 'x':
      regPC = addressSwitch;
      Serial.print("+ x, EXAMINE: ");
      Serial.println(regPC);
      p_regPC = ~regPC;
      altair_set_outputs(regPC, MREAD(regPC));
      break;
    case 'X':
      regPC = regPC + 1;
      Serial.print("+ y, EXAMINE NEXT: ");
      Serial.println(regPC);
      altair_set_outputs(regPC, MREAD(regPC));
      break;
    case 'p':
      Serial.print("+ p, DEPOSIT to: ");
      Serial.println(regPC);
      MWRITE(regPC, addressSwitch & 0xff);
      altair_set_outputs(regPC, MREAD(regPC));
      break;
    case 'P':
      regPC++;
      Serial.print("+ P, DEPOSIT NEXT to: ");
      Serial.println(regPC);
      MWRITE(regPC, addressSwitch & 0xff);
      altair_set_outputs(regPC, MREAD(regPC));
      break;
    case 'R':
      Serial.println("+ R, RESET.");
      // For now, do EXAMINE 0 to reset to the first memory address.
      regPC = 0;
      p_regPC = ~regPC;
      altair_set_outputs(regPC, MREAD(regPC));
      //
      host_clr_status_led_HLTA();
      /* The above is the same as the following:
      host_set_status_led_MEMR();
      host_clr_status_led_INP();
      host_set_status_led_M1();
      host_clr_status_led_OUT();
      host_clr_status_led_HLTA();
      host_clr_status_led_STACK();
      host_clr_status_led_WO();
      host_clr_status_led_INT();
      */
      //
      // p_regPC = regPC;
      // Actual RESET action ?- set bus/data LEDs on, status LEDs off: altair_set_outputs(0xffff, 0xff);
      //
      break;
    // -------------------------------------
    case 'h':
      Serial.println("----------------------------------------------------");
      Serial.println("+ h, Help.");
      Serial.println("-------------");
      Serial.println("+ r, RUN.");
      Serial.println("+ s, SINGLE STEP when in WAIT mode.");
      Serial.println("+ s, STOP        when in RUN mode.");
      Serial.println("+ x, EXAMINE switch address.");
      Serial.println("+ X, EXAMINE NEXT address, current address + 1.");
      Serial.println("+ p, DEPOSIT at current address");
      Serial.println("+ P, DEPOSIT NEXT address, current address + 1");
      Serial.println("+ R, RESET, set address to zero.");
      Serial.println("-------------");
      Serial.println("+ i, Information: print registers.");
      Serial.println("+ l, loaded a simple program.");
      Serial.println("----------------------------------------------------");
      break;
    // -------------------------------------
    case 'l':
      Serial.println("+ l, loaded a simple program.");
      cnt = 0;
      /*
        MWRITE(   cnt++, B00111110 & 0xff);  // ++ opcode:mvi:00111110:a:6
        MWRITE(   cnt++, B00000110 & 0xff);  // ++ immediate:6:6
        MWRITE(   cnt++, B00000110 & 0xff);  // ++ opcode:mvi:00000110:b:0
        MWRITE(   cnt++, B00000000 & 0xff);  // ++ immediate:0:0
        MWRITE(   cnt++, B00001110 & 0xff);  // ++ opcode:mvi:00001110:c:1
        MWRITE(   cnt++, B00000001 & 0xff);  // ++ immediate:1:1
        MWRITE(   cnt++, B00010110 & 0xff);  // ++ opcode:mvi:00010110:d:2
        MWRITE(   cnt++, B00000010 & 0xff);  // ++ immediate:2:2
        MWRITE(   cnt++, B00011110 & 0xff);  // ++ opcode:mvi:00011110:e:3
        MWRITE(   cnt++, B00000011 & 0xff);  // ++ immediate:3:3
        MWRITE(   cnt++, B00100110 & 0xff);  // ++ opcode:mvi:00100110:h:4
        MWRITE(   cnt++, B00000100 & 0xff);  // ++ immediate:4:4
        MWRITE(   cnt++, B00101110 & 0xff);  // ++ opcode:mvi:00101110:l:5
        MWRITE(   cnt++, B00000101 & 0xff);  // ++ immediate:5:5
      */
      MWRITE(    cnt++, B00111110 & 0xff);  // ++ opcode:mvi:00111110:a:6
      MWRITE(    cnt++, B00000110 & 0xff);  // ++ immediate:6:6
      MWRITE(    cnt++, B00110010 & 0xff);  // ++ opcode:sta:00110010:96
      MWRITE(    cnt++, B01100000 & 0xff);  // ++ lb:96:96
      MWRITE(    cnt++, B00000000 & 0xff);  // ++ hb:0
      MWRITE(    cnt++, B01110110 & 0xff);  // ++ opcode:hlt:01110110
      MWRITE(    cnt++, B00111100 & 0xff);  // ++ opcode:inr:00111100:a
      MWRITE(    cnt++, B11000011 & 0xff);  // ++ opcode:jmp:11000011:Store
      MWRITE(    cnt++, B00000010 & 0xff);  // ++ lb:Store:2
      MWRITE(    cnt++, B00000000 & 0xff);  // ++ hb:0
      // Common closing to restart the code.
      MWRITE(   cnt++, B01110110 & 0xff);  // ++ opcode:hlt:01110110
      MWRITE(   cnt++, B11000011 & 0xff);  // ++ opcode:jmp:11000011:Start
      MWRITE(   cnt++, B00000000 & 0xff);  // ++ lb:0
      MWRITE(   cnt++, B00000000 & 0xff);  // ++ hb:0
      // Do EXAMINE 0, or RESET, after the load;
      regPC = 0;
      p_regPC = ~regPC;
      altair_set_outputs(regPC, MREAD(regPC));
      break;
    // -------------------------------------
    case '/':
      Serial.print(F("\r\nSet Addr switches to: "));
      // numsys_read_word(&addressSwitch);
      Serial.println('\n');
      break;
    case 'i':
      Serial.println("+ i: Information.");
      Serial.print(F("++ CPU: "));
      Serial.println(THIS_CPU);
      Serial.print(F("++ host_read_status_led_WAIT()="));
      Serial.println(host_read_status_led_WAIT());
      cpucore_i8080_print_registers();
      break;
    // -------------------------------------
    case 10:
      Serial.println(F("+ New line character."));
      print_panel_serial();
      break;
    // -------------------------------------
    default:
      Serial.print("- Ignored <");
      Serial.write(readByte);
      Serial.println(">");
  }
}

void runProcessorWait() {
#ifdef LOG_MESSAGES
  Serial.println(F("+ runProcessorWait()"));
#endif
  while (programState == PROGRAM_WAIT) {
    // Program control: RUN, SINGLE STEP, EXAMINE, EXAMINE NEXT, Examine previous, RESET.
    if (Serial.available() > 0) {
      readByte = Serial.read();    // Read and process an incoming byte.
      processWaitSwitch(readByte);
    }
    delay(60);
  }
}

// -----------------------------------------------------------------------------
void setup() {
  // Speed for serial read, which matches the sending program.
  Serial.begin(9600);   // 115200 19200
  delay(2000);
  Serial.println(""); // Newline after garbage characters.
  Serial.println("+++ Setup.");
  //
  Serial.println("+++ Altair 101a initialized.");
  // ----------------------------------------------------
  // ----------------------------------------------------
  // Front panel LED lights.

  // System application status LED lights
  pinMode(WAIT_PIN, OUTPUT);    // Indicator: program wait state: LED on or LED off.
  pinMode(HLDA_PIN, OUTPUT);    // Indicator: clock process (LED on) or emulator (LED off).
  digitalWrite(WAIT_PIN, HIGH); // Default to wait state.
  digitalWrite(HLDA_PIN, HIGH); // Default to emulator.

  // ------------------------------
  // Set status lights.
  statusByte = MEMR_ON | M1_ON | WO_ON; // WO: on, Inverse logic: off when writing out. On when not.
  // programCounter and curProgramCounter are 0 by default.
  // dataByte = memoryData[curProgramCounter];
  Serial.println(F("+ Initialized: statusByte, programCounter & curProgramCounter, dataByte."));
  //
  pinMode(latchPinLed, OUTPUT);
  pinMode(clockPinLed, OUTPUT);
  pinMode(dataPinLed, OUTPUT);
  delay(300);
  ledFlashSuccess();
  Serial.println(F("+ Front panel LED lights are initialized."));
  //
  // ----------------------------------------------------
  // ----------------------------------------------------
  // host_set_status_leds_READMEM_M1();
  host_set_status_led_MEMR();
  host_set_status_led_M1();
  host_clr_status_led_WO();
  regPC = 0;
  opcode = MREAD(regPC);
  host_set_addr_leds(regPC);
  host_set_data_leds(opcode);
  print_panel_serial();
  programState = PROGRAM_WAIT;
  host_set_status_led_WAIT();
  // ----------------------------------------------------
  // programLights();    // Uses: statusByte, curProgramCounter, dataByte
  Serial.println(F("+ Starting the processor loop."));
}

// -----------------------------------------------------------------------------
// Device Loop
void loop() {
  switch (programState) {
    // ----------------------------
    case PROGRAM_RUN:
      runProcessor();
      break;
    // ----------------------------
    case PROGRAM_WAIT:
      runProcessorWait();
      break;
  }
  // delay(30); // Arduino sample code, doesn't use a delay.
}
// -----------------------------------------------------------------------------
