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

// Byte bit switch values
#define SW_SLOW       3
#define SW_RESET      8
#define SW_STEP       2

// Byte bit status values
#define ST_WAIT    0x0800
#define ST_HLDA    0x0400
//
#define ST_INTE    0x0200
#define ST_PROT    0x0100
#define ST_MEMR    0x0080
#define ST_INP     0x0040
#define ST_M1      0x0020
#define ST_OUT     0x0010
#define ST_HLTA    0x0008
#define ST_STACK   0x0004
#define ST_WO      0x0002
#define ST_INT     0x0001

// Not implemented at this time.
#define INT_SW_STOP     0x80000000
#define INT_SW_RESET    0x40000000
#define INT_SW_CLR      0x20000000
#define INT_SW_AUX2UP   0x10000000
#define INT_SW_AUX2DOWN 0x08000000
#define INT_SWITCH      0xff000000

// -----------------------------------------------------------------------------
// From Processor.ino

byte readByte = 0;

// Program states
#define LIGHTS_OFF 0
#define PROGRAM_WAIT 1
#define PROGRAM_RUN 2
#define CLOCK_RUN 3
#define PLAYER_RUN 4
#define SERIAL_DOWNLOAD 5
int programState = LIGHTS_OFF;  // Intial, default.

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
byte opcode = 0xff;

static uint16_t p_regPC = 0xFFFF;
uint16_t cswitch = 0;
uint16_t dswitch = 0;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void print_panel_serial() {
  byte dbus;
  static uint16_t p_dswitch = 0, p_cswitch = 0, p_abus = 0xffff, p_dbus = 0xffff, p_status = 0xffff;
  uint16_t status, abus;

  status = host_read_status_leds();
  abus   = host_read_addr_leds();
  dbus   = host_read_data_leds();

  // Even if n change, print anyway.
  // if ( force || p_cswitch != cswitch || p_dswitch != dswitch || p_abus != abus || p_dbus != dbus || p_status != status ) {

  Serial.print(F("INTE PROT MEMR INP M1 OUT HLTA STACK WO INT  D7  D6  D5  D4  D3  D2  D1  D0\r\n"));

  if ( status & ST_INTE  ) Serial.print(F(" *  "));    else Serial.print(F(" .  "));
  if ( status & ST_PROT  ) Serial.print(F("  *  "));   else Serial.print(F("  .  "));
  if ( status & ST_MEMR  ) Serial.print(F("  *  "));   else Serial.print(F("  .  "));
  if ( status & ST_INP   ) Serial.print(F("  * "));    else Serial.print(F("  . "));
  if ( status & ST_M1    ) Serial.print(F(" * "));     else Serial.print(F(" . "));
  if ( status & ST_OUT   ) Serial.print(F("  * "));    else Serial.print(F("  . "));
  if ( status & ST_HLTA  ) Serial.print(F("  *  "));   else Serial.print(F("  .  "));
  if ( status & ST_STACK ) Serial.print(F("   *  "));  else Serial.print(F("   .  "));
  if ( status & ST_WO    ) Serial.print(F(" * "));     else Serial.print(F(" . "));
  if ( status & ST_INT   ) Serial.print(F("  *"));    else Serial.print(F("  ."));

  if ( dbus & 0x80 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( dbus & 0x40 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( dbus & 0x20 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( dbus & 0x10 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( dbus & 0x08 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( dbus & 0x04 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( dbus & 0x02 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  if ( dbus & 0x01 )   Serial.print(F("   *")); else Serial.print(F("   ."));
  Serial.print(("\r\nWAIT HLDA   A15 A14 A13 A12 A11 A10  A9  A8  A7  A6  A5  A4  A3  A2  A1  A0\r\n"));
  // if ( status & ST_WAIT ) Serial.print(F(" *  "));   else Serial.print(F(" .  "));
  if ( host_read_status_led_WAIT() ) Serial.print(F(" *  "));   else Serial.print(F(" .  "));
  if ( status & ST_HLDA ) Serial.print(F("  *   ")); else Serial.print(F("  .   "));
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
  Serial.print(F("\r\n            S15 S14 S13 S12 S11 S10  S9  S8  S7  S6  S5  S4  S3  S2  S1  S0\r\n"));
  Serial.print(F("          "));
  if ( dswitch & 0x8000 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( dswitch & 0x4000 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( dswitch & 0x2000 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( dswitch & 0x1000 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( dswitch & 0x0800 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( dswitch & 0x0400 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( dswitch & 0x0200 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( dswitch & 0x0100 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( dswitch & 0x0080 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( dswitch & 0x0040 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( dswitch & 0x0020 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( dswitch & 0x0010 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( dswitch & 0x0008 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( dswitch & 0x0004 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( dswitch & 0x0002 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  if ( dswitch & 0x0001 ) Serial.print(F("   ^")); else Serial.print(F("   v"));
  Serial.print(F("\r\n ------ \r\n"));
  p_cswitch = cswitch;
  p_dswitch = dswitch;
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
  res |= statusByteD & 0x80 ? ST_INTE : 0;
  res |= statusByteG & 0x04 ? ST_PROT : 0;
  res |= statusByteG & 0x02 ? ST_WAIT : 0;
  res |= statusByteG & 0x01 ? ST_HLDA : 0;
  return res;
}

// -----------------------------------------------------------------------------
void altair_set_outputs(uint16_t a, byte v) {
  // Stacy, When not using serial, display on front panel lights.
  host_set_addr_leds(a);
  host_set_data_leds(v);
  print_panel_serial();
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
  cswitch &= BIT(SW_RESET); // clear everything but RESET status
  /* Stacy, here is the loop for waiting when single stepping during WAIT mode.
    while ( host_read_status_led_WAIT() && (cswitch & (BIT(SW_STEP) | BIT(SW_SLOW) | BIT(SW_RESET))) == 0 ) {
    read_inputs();
    delay(10);
    }
  */
  if ( cswitch & BIT(SW_SLOW) ) delay(500);
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
  cswitch &= BIT(SW_RESET); // clear everything but RESET status
  /* Stacy, here is the loop for waiting when single stepping during WAIT mode.
    while ( host_read_status_led_WAIT() && (cswitch & (BIT(SW_STEP) | BIT(SW_SLOW) | BIT(SW_RESET))) == 0 ) {
    read_inputs();
    delay(10);
    }
  */
  if ( cswitch & BIT(SW_SLOW) ) delay(500);

}

// -----------------------------------------------------------------------------
void read_inputs() {
  byte readByte;
  readByte = "";
  // cswitch = 0;
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
  // cswitch = host_read_function_switches_edge();
  // ...except for the SLOW switch which is active as long as it is held down
  // if ( host_read_function_switch_debounced(SW_SLOW) ) cswitch |= BIT(SW_SLOW);
  // #if STANDALONE==0
  //   dswitch = host_read_addr_switches();
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
  host_set_addr_leds(port | port * 256);
  if ( host_read_status_led_WAIT() ) {
    cswitch &= BIT(SW_RESET); // clear everything but RESET status
    // keep reading input data while we are waiting
    while ( host_read_status_led_WAIT() && (cswitch & (BIT(SW_STEP) | BIT(SW_SLOW) | BIT(SW_RESET))) == 0 ) {
      host_set_status_led_INP();
      // stacy data = io_inp(port);
      altair_set_outputs(port | port * 256, data);
      host_clr_status_led_INP();
      // stacy read_inputs();
      // advance simulation time (so timers can expire)
      // stacy TIMER_ADD_CYCLES(50);
    }
    // if ( cswitch & BIT(SW_SLOW) ) delay(500);
  }
  else {
    host_set_status_led_INP();
    // stacy data = io_inp(port);
    host_set_data_leds(data);
    host_clr_status_led_INP();
  }
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
  int cnt;
  //
  // Process an address/data toggle.
  //
  int data = readByte;
  if ( data >= '0' && data <= '9' ) {
    dswitch = dswitch ^ (1 << (data - '0'));
    return;
  }
  if ( data >= 'a' && data <= 'f' ) {
    // Stacy, change to uppercase A...F
    dswitch = dswitch ^ (1 << (data - 'a' + 10));
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
      regPC = dswitch;
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
      MWRITE(regPC, dswitch & 0xff);
      altair_set_outputs(regPC, MREAD(regPC));
      break;
    case 'P':
      regPC++;
      Serial.print("+ P, DEPOSIT NEXT to: ");
      Serial.println(regPC);
      MWRITE(regPC, dswitch & 0xff);
      altair_set_outputs(regPC, MREAD(regPC));
      break;
    case 'R':
      Serial.println("+ R, RESET.");
      // altair_wait_reset();
      host_clr_status_led_HLTA();
      //
      // For now, do EXAMINE 0 to reset to the first memory address.
      regPC = 0;
      p_regPC = ~regPC;
      altair_set_outputs(regPC, MREAD(regPC));
      //
      p_regPC = regPC;
      // set bus/data LEDs on, status LEDs off
      altair_set_outputs(0xffff, 0xff);
      host_clr_status_led_MEMR();
      host_clr_status_led_INP();
      host_clr_status_led_M1();
      host_clr_status_led_OUT();
      host_clr_status_led_HLTA();
      host_clr_status_led_STACK();
      host_set_status_led_WO();
      host_clr_status_led_INT();
      // stacy altair_interrupt_disable();
      break;
    // -------------------------------------
    case 'h':
      Serial.println("----------------------------------------------------");
      Serial.println("+ h, Help.");
      Serial.println("-------------");
      Serial.println("+ r, RUN.");
      Serial.println("+ S, SINGLE STEP when in WAIT mode.");
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
      cnt = -1;
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
      /* */
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
      // numsys_read_word(&dswitch);
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
      // New line character.
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
  Serial.println(F("+ runProcessorWait()"));
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
  dataByte = memoryData[curProgramCounter];
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
