// -----------------------------------------------------------------------------
/*
  Arduino communications: Sender and Receiver code in one program.
  For example: Nano to Nano data communications.

  +++ Nana has 2 interrupt pins: 2 and 3.
  + pin 2: Communications clock
  + pin 3: rotary encoder

  Connect a KY-040 rotary encoder to a Nano:
  + "+" to Nano 5v, note, also works with 3.3v, example: NodeMCU.
  + GND to Nano ground.
  + CLK (clock) to Nano pin D2, the interrupt pin.
  + DT (data)   to Nano pin A4.

  Add LEDs:
  + pin 2: LED(+) to resister to ground.
  + pin 5: LED(+) to resister to ground.

  Connect Nanos together for communications. On the sender Nano:
  + 5v: positive
  + GND: ground   Sender  Receiver
  + pin 2: clock  A2      D3
  + pin 5: data   A1      A0

  Reference:
    https://www.youtube.com/watch?v=eq5YpKHXJDM
    https://github.com/beneater/error-detection-videos
  Sender:
    https://github.com/beneater/error-detection-videos/blob/master/transmitter/transmitter.ino
  Receiver:
    https://github.com/beneater/error-detection-videos/blob/master/receiver/receiver.ino

*/
// -----------------------------------------------------------------------------
// Used with the 2 x 7 segment display
#include <SevSeg.h>
SevSeg sevseg;

// -----------------------------------------------------------------------------
// Arduino communications: transmission sender
#define TX_CLOCK A2  // Works on pin 3, 4, 6 and A1.
#define TX_DATA  A1  // Works on pin 4 and A1.

// Rate notes:
//  300 nice to watch the bits show.
//   10 fast for transfer.
//    1 works fine, even with serial print statements.
#define TX_CLOCK_DELAY 1

const char *message = "TX";

void sendBit(bool tx_bit) {
  // Serial.print(tx_bit);
  // Set/write the data bit to transmit: either HIGH (1) or LOW (0) value.
  digitalWrite(TX_DATA, tx_bit);
  // Transit the bit: pulse the clock to let the other Arduino know that the data is ready to read.
  delay(TX_CLOCK_DELAY);
  digitalWrite(TX_CLOCK, HIGH);
  delay(TX_CLOCK_DELAY);
  digitalWrite(TX_CLOCK, LOW);
}
void sendByte(char tx_byte) {
  // Serial.print("< ");
  Serial.print("< Send byte:");
  Serial.println(tx_byte, DEC);   // Note, BIN prints the binary value, example: DEC:12: BIN: 1100.
  // Serial.print(": bits: ");
  for (int bit_idx = 0; bit_idx < 8; bit_idx++) {
    // Transmit each bit of the byte.
    // Get the bit to transmit, and transmit it.
    bool tx_bit = tx_byte & (0x80 >> bit_idx);
    sendBit(tx_bit);
  }
  // Serial.println(".");
  //
  // Set data bit to LOW (0).
  digitalWrite(TX_DATA, LOW);
}
// Nano to Nano (N2N) Communications: sender.
void sendMessage() {
  // Transmit each bit in the byte.
  for (int byte_idx = 0; byte_idx < strlen(message); byte_idx++) {
    // Get each byte of the message, and transmit it.
    char tx_byte = message[byte_idx];
    sendByte(tx_byte);
  }
}

// -----------------------------------------------------------------------------
// Arduino Communications: Receiver.
#define RX_CLOCK 3    // Requires to be on an interrupt pin. For a Nano: 2 or 3.
#define RX_DATA  A0   // Works on pin 4, A0, and A1.

volatile byte rx_bit = 0;
volatile byte rx_byte = 0;
volatile int bit_position = 0;
volatile bool bitReceived = false;
volatile bool byteReceived = false;
volatile byte messageByte = 0;

void onClockPulse() {
  bitReceived = true;
  // Read a bit.
  rx_bit = digitalRead(RX_DATA);
  // Add bit to byte.
  if (rx_bit) {
    rx_byte |= (0x80 >> bit_position);
  }
  /*
  Serial.print("+");
  Serial.print(" bit_position: ");
  Serial.print(bit_position);
  Serial.print(" bit: ");
  Serial.println(rx_bit);
  */
  bit_position++;
  if (bit_position == 8) {
    // 8 bits is a byte.
    byteReceived = true;
    messageByte = rx_byte;
    bit_position = 0;
    rx_byte = 0;
  }
}

// -----------------------------------------------------------------------------
// Rotary Encoder module connections

const int PinCLK = 2;  // Requires to be on an interrupt pin. For a Nano: D2 or D3.
const int PinDT =  A4; // Reading DT signal

static int virtualPosition = 0; // Loop number of turns counter. Up(+1) for right, down(-1) for left.

// Interrupt routine runs if rotary encoder CLK pin changes state.
volatile boolean TurnDetected;  // Type volatile for interrupts.
volatile boolean turnRight;
void rotarydetect ()  {
  // Set direction for TurnDetected: turnRight or left (!turnRight).
  // Note, have as little code as possible here because interrupt timing with the loop.
  TurnDetected = false;
  if (digitalRead(PinDT) == 1) {
    TurnDetected = true;
    turnRight = false;
    if (digitalRead(PinCLK) == 0) {
      turnRight = true;
    }
  }
}

// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  // Give the serial connection time to start before the first print.
  delay(1000);
  Serial.println("+++ Setup.");

  byte hardwareConfig = COMMON_CATHODE; // COMMON_ANODE or COMMON_CATHODE
  byte segmentPins[] = {6, 7, 8, 9, 10, 11, 12};  // Mapping segment pins A..G, to Nano pins.
  byte numDigits = 2;                 // Number of display digits.
  byte digitPins[] = {5, 13};          // Multi-digit display ground/set pins: can use pin 13.
  bool resistorsOnSegments = true;    // Set to true when using a single resister per display digit.
  bool updateWithDelays = false;      // Doesn't work when true.
  bool leadingZeros = true;           // Clock leading 0. When true: "01" rather that " 1".
  bool disableDecPoint = true;        // Use 'true' if your decimal point doesn't exist or isn't connected
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros, disableDecPoint);
  Serial.println("+ Digit display configured.");
  sevseg.setNumber(42, 1);

  pinMode(TX_CLOCK, OUTPUT);
  pinMode(TX_DATA, OUTPUT);
  Serial.println("+ Configured: Nano to Nano (N2N) Communications for sending.");

  attachInterrupt (digitalPinToInterrupt(PinCLK), rotarydetect, CHANGE);
  Serial.println("+ Configured: rotary encoder.");

  pinMode(RX_DATA, INPUT);
  attachInterrupt(digitalPinToInterrupt(RX_CLOCK), onClockPulse, RISING);
  Serial.println("+ Ready to receive communications from the other Nano.");

  Serial.println("+++ Go to loop.");
}

// -----------------------------------------------------------------------------
// Device Loop

void loop() {

  if (byteReceived) {
    Serial.print("> Receive byte: ");
    Serial.println(messageByte);
    sevseg.setNumber(messageByte, 1);
    /* For sending a test.
    if (messageByte == 3) {
      Serial.print("+");
      Serial.print(" Send back messageByte: ");
      Serial.println(messageByte);
      // sendByte(messageByte);
    }
    */
    byteReceived = false;
  }

  if (TurnDetected)  {
    TurnDetected = false;  // Reset, until new rotation detected
    if (turnRight) {
      virtualPosition++;
      if (virtualPosition > 12) {
        virtualPosition = 1;
      }
      // Serial.print ("> right count = ");
      // Serial.println (virtualPosition);
    } else {
      virtualPosition--;
      if (virtualPosition < 1) {
        virtualPosition = 12;
      }
      // Serial.print ("< left  count = ");
      // Serial.println (virtualPosition);
    }
    // sendMessage();
    sendByte(virtualPosition);
  } else {
    delay(10);
  }
  sevseg.refreshDisplay();

}
// -----------------------------------------------------------------------------
