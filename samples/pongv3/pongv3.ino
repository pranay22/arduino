/********
   A basic implementation of Pong on the Arduino using a TV for output.

   Arduino Pong By Pete Lamonica,
   modified by duboisvb,
   updated by James Bruce: http://www.makeuseof.com/tag/author/jbruce
   updated by Stacy David: https://github.com/tigerfarm/arduino/tree/master/samples/pongv3

   Library setup:
   + From the library manager (Sketch/Include library/Manage libraries), install: tvout.
   + Move the directory, TVoutfonts, to the directory: Documents/Arduino/libraries.

  --- Wiring ---

  Connect infrared receiver, pins from top left to right:
    Left most (next to the X) - Nano pin 9
    Center - 5V
    Right  - ground
  
  + 2x Variable resisters: 10k ohm Potentiometers
  + Variable resisters: left pin to v+
  + Variable resisters: right pin to v-
  + Nano A0 : variable resister, center pin: right paddle controller
  + Nano A1 : variable resister, center pin: left paddle controller
  + Nano D2 : button, pull down resister to v-, other button side to v+.
  
  Following pins are set by the TV library.
  + Nano D7 : to 1K resister, to the RCA video cable signal wire (center wire).
  + Nano D8 : to 470 ohm resister, to signal wire.
  + RCA video cable shielding (ground) wire, to v-.
  2nd RCA cable for sound.
  + Wire the signal pin to D11 for TV tones (TV.tone).

   TVout Documentation: https://code.google.com/archive/p/arduino-tvout/wikis
   + clear_screen() Clear the screen.
   + hres() returns the horizontal resolution
   + vres() returns the vertical resolution
   + select_font(font): font4x6, font6x8, font8x8, font8x8ext
   + print(x,y,string) or println(x,y,string)
   + draw_line(x0,y0,x1,y1,color)
   + set_pixel(x,y,color) Set a pixel.
   + get_pixel(x,y) Get a pixel.
   + delay_frame(1): To prevent screen flashes.

   To make the game more interesting to play:
   + Long wires (3 wire cable) for each controller.
   + User set game ball speed.
   + Fix ball bounce from boarders and paddles.
*/
// -----------------------------------------------------------------------
#include <TVout.h>
#include <fontALL.h>

TVout TV;

// -----------------------------------------------------------------------
#define PLAYER_ONE_PIN 0 // analog paddle control: right paddle
#define PLAYER_TWO_PIN 1 // analog paddle control: left paddle

#define PADDLE_HEIGHT 14
#define PADDLE_WIDTH 1
#define RIGHT_PADDLE_X (TV.hres()-4)
#define LEFT_PADDLE_X 2

#define PLAY_TO 6
#define MAX_Y_VELOCITY 12

// -----------------------
// Game states
#define IN_MENU 0
#define PLAYING_GAME 1
#define PLAYING_GAME_PAUSE 2
#define GAME_OVER 3
#define BUTTON_ONE_PIN 2 // Control the game state

int state = IN_MENU;

// -----------------------
// Display message locations
#define LEFT_SCORE_X (TV.hres()/2-15)
#define RIGHT_SCORE_X (TV.hres()/2+10)
#define SCORE_Y 4
//
#define DISPLAY_MESSAGE_X 20
#define GAME_OVER_Y 12

// -----------------------
char volX = 3;
char volY = 3;
unsigned char x, y;
//
int leftPlayerScore = 0;
int rightPlayerScore = 0;
int paddleOnePosition = 0;
int paddleTwoPosition = 0;
int rightPaddleY = 0;
int leftPaddleY = 0;
//
unsigned char ballX = 0;
unsigned char ballY = 0;
char ballVolX = 2;
char ballVolY = 2;

// -----------------------------------------------------------------------
// Draw game components

void drawMenu() {
  Serial.println("+ drawMenu");
  TV.clear_screen();
  TV.select_font(font6x8);
  TV.print(6, 5, "   Pong V2.1");
  drawGameStart();
}
void drawGameStart() {
  TV.select_font(font4x6);
  TV.print(DISPLAY_MESSAGE_X, 45, "For new Game.");
  TV.select_font(font8x8);
  TV.print(DISPLAY_MESSAGE_X, 35, "Press Button");
}
void drawGameOver() {
  drawGameBoard();
  drawScore();
  TV.select_font(font8x8);
  TV.print(DISPLAY_MESSAGE_X, GAME_OVER_Y, "Game over.");
  drawGameStart();
}
void drawGameBoard() {
  TV.clear_screen();
  for (int i = 1; i < TV.vres() - 4; i += 6) {
    TV.draw_line(TV.hres() / 2, i, TV.hres() / 2, i + 3, 1);
  }
  // had to make box a bit smaller to fit tv
  TV.draw_line(0, 0, 126, 0, 1 );     // top
  TV.draw_line(126, 0, 126, 95, 1 );  // right
  TV.draw_line(0, 95, 126, 95, 1 );   // bottom
  TV.draw_line(0, 0, 0, 95, 1 );      // left
}
void drawScore() {
  TV.select_font(font4x6);
  TV.print_char(LEFT_SCORE_X, SCORE_Y, '0' + leftPlayerScore);
  TV.print_char(RIGHT_SCORE_X, SCORE_Y, '0' + rightPlayerScore);
}

// -----------------------------------------------------------------------
// Game moving components

void drawMenuBall() {
    TV.delay_frame(2);
    delay(25);
    //
    if (x + volX < 1 || x + volX > TV.hres() - 1) {
      volX = -volX;
      delay(100); TV.tone( 2000,30  );  // need to test tones.
    }
    if (y + volY < 1 || y + volY > TV.vres() - 1) {
      volY = -volY;
      delay(100); TV.tone( 2000,30  );
    }
    if (TV.get_pixel(x + volX, y + volY)) {
      TV.set_pixel(x + volX, y + volY, 0);
      if (TV.get_pixel(x + volX, y - volY) == 0) {
        volY = -volY;
        delay(100); TV.tone( 2000,30  );
      }
      else if (TV.get_pixel(x - volX, y + volY) == 0) {
        volX = -volX;
        delay(100); TV.tone( 2000,30  );
      }
      else {
        volX = -volX;
        volY = -volY;
      }
    }
    TV.set_pixel(x, y, 0);
    x += volX;
    y += volY;
    TV.set_pixel(x, y, 1);
}

void drawPaddles() {
  rightPaddleY = ((paddleOnePosition / 8) * (TV.vres() - PADDLE_HEIGHT)) / 128;
  x = RIGHT_PADDLE_X;
  for (int i = 0; i < PADDLE_WIDTH; i++) {
    TV.draw_line(x + i, rightPaddleY, x + i, rightPaddleY + PADDLE_HEIGHT, 1);
  }
  leftPaddleY = ((paddleTwoPosition / 8) * (TV.vres() - PADDLE_HEIGHT)) / 128;
  x = LEFT_PADDLE_X;
  for (int i = 0; i < PADDLE_WIDTH; i++) {
    TV.draw_line(x + i, leftPaddleY, x + i, leftPaddleY + PADDLE_HEIGHT, 1);
  }
}

void drawGameBall() {
  ballX += ballVolX;
  ballY += ballVolY;
  //
  // change if hit top or bottom
  if ( ballY <= 1 || ballY >= TV.vres() - 1 ) {
    ballVolY = -ballVolY;
  }
  // test left side for wall hit
  if (ballVolX < 0 && ballX == LEFT_PADDLE_X + PADDLE_WIDTH - 1 && ballY >= leftPaddleY && ballY <= leftPaddleY + PADDLE_HEIGHT) {
    ballVolX = -ballVolX;
    ballVolY += 2 * ((ballY - leftPaddleY) - (PADDLE_HEIGHT / 2)) / (PADDLE_HEIGHT / 2);
  }
  // test right side for wall hit
  if (ballVolX > 0 && ballX == RIGHT_PADDLE_X && ballY >= rightPaddleY && ballY <= rightPaddleY + PADDLE_HEIGHT) {
    ballVolX = -ballVolX;
    ballVolY += 2 * ((ballY - rightPaddleY) - (PADDLE_HEIGHT / 2)) / (PADDLE_HEIGHT / 2);
  }
  //
  // limit vertical speed
  if (ballVolY > MAX_Y_VELOCITY) ballVolY = MAX_Y_VELOCITY;
  if (ballVolY < -MAX_Y_VELOCITY) ballVolY = -MAX_Y_VELOCITY;
  //
  // Scoring
  if (ballX <= 1) {
    playerScored(RIGHT);
    delay(100); TV.tone( 500,300 );
  }
  if (ballX >= TV.hres() - 1) {
    playerScored(LEFT);
    delay(100); TV.tone( 500,300 );
  }
  //
  // draw the ball
  TV.set_pixel(ballX, ballY, 2);
}

void playerScored(byte player) {
  if (player == RIGHT) rightPlayerScore++;
  if (player == LEFT) leftPlayerScore++;
  if (leftPlayerScore == PLAY_TO || rightPlayerScore == PLAY_TO) {
    drawGameOver();
    state = GAME_OVER;
    rightPlayerScore = 0;
    leftPlayerScore = 0;
  }
  ballVolX = -ballVolX;
}

// -----------------------------------------------------------------------
void buttonSetState() {
  // Manage control for multiple button presses:
  //  + If pressed while in a game, pause the game.
  //  + If pressed while in game pause state, continue the game.
  //  + If in the menu, start a new game in 1 second.
  //  ++ (to do) If pressed again, within 1 second, set faster ball speed.
  if (state == PLAYING_GAME) {
    state = PLAYING_GAME_PAUSE;
    TV.select_font(font8x8);
    TV.print(DISPLAY_MESSAGE_X, GAME_OVER_Y, "Game Paused.");
  } else if (state == PLAYING_GAME_PAUSE) {
    state = PLAYING_GAME;
  } else if (state == IN_MENU) {
    state = PLAYING_GAME;
  } else if (state == GAME_OVER) {
    state = PLAYING_GAME;
  }
}

// -----------------------------------------------------------------------
void setup()  {
  Serial.begin(115200);
  // Give the serial connection time to start before the first print.
  delay(1000);
  Serial.println("+++ Setup.");

  TV.begin(_NTSC);       //for devices with only 1k sram(m168) use TV.begin(_NTSC,128,56)
  x = 0;
  y = 0;
  ballX = TV.hres() / 2;
  ballY = TV.vres() / 2;
  state = IN_MENU;
  drawMenu();

  Serial.println("+ Start loop()");
}

// -----------------------------------------------------------------------
boolean buttonPressed = false;
int counter = 0;
void loop() {
  delay(25);
  // --------------------------------
  if (digitalRead(BUTTON_ONE_PIN)) {
    // Button logic to handle a quick click, or a click and hold.
    if (!buttonPressed) {
      buttonSetState();
      buttonPressed = true;
    }
  } else {
    buttonPressed = false;
  }

  // --------------------------------
  // Game state loop activities
  //
  if (counter == 60) counter = 10; // increment or reset frame counter
  counter++;
  //
  if (state == PLAYING_GAME) {
    paddleOnePosition = analogRead(PLAYER_ONE_PIN);
    paddleTwoPosition = analogRead(PLAYER_TWO_PIN);
    if (counter % 2 == 0) {
      // Note, if display every loop, then the ball looks duplicated.
      //
      // Clear and redraw.
      drawGameBoard();
      drawScore();
      drawPaddles();
      drawGameBall();
    }
    TV.delay_frame(1);  // Required, else the screen flashes.
  } else if (state == IN_MENU || state == GAME_OVER) {
    drawMenuBall();
  }
  
}
// -----------------------------------------------------------------------
// eof
