/********
   A basic implementation of Pong on the Arduino using a TV for output.

   Arduino Pong By Pete Lamonica,
   modified by duboisvb,
   updated by James Bruce: http://www.makeuseof.com/tag/author/jbruce
   
   To compile, from the library manager (Sketch/Include library/Manage libraries), install: tvout.
   Move the directory, TVoutfonts, to the directory: Documents/Arduino/libraries.
*/
#include <TVout.h>
#include <fontALL.h>

// Game states
#define IN_MENU 0
#define PLAYING_GAME 1
#define GAME_OVER 2
#define PLAYING_GAME_PAUSE 3

#define BUTTON_ONE_PIN 2 // digital to control the game state
#define PLAYER_ONE_PIN 0 // analog paddle control
#define PLAYER_TWO_PIN 1 // analog paddle control

#define PADDLE_HEIGHT 14
#define PADDLE_WIDTH 1
#define RIGHT_PADDLE_X (TV.hres()-4)
#define LEFT_PADDLE_X 2

#define MAX_Y_VELOCITY 6
#define PLAY_TO 6

// Display message locations
#define LEFT_SCORE_X (TV.hres()/2-15)
#define RIGHT_SCORE_X (TV.hres()/2+10)
#define SCORE_Y 4
//
#define DISPLAY_MESSAGE_X 20
#define GAME_OVER_Y 12

TVout TV;

int state = IN_MENU;
boolean button1Status = false;
boolean buttonPressed = false;
boolean doDrawMenu = true;
boolean doDrawWin = true;
char volX = 3;
char volY = 3;
unsigned char x, y;
int counter = 0;
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
// Start menu

void drawMenu() {
  Serial.println("+ drawMenu");
  TV.clear_screen();
  TV.select_font(font8x8);
  TV.print(6, 5, "Arduino Pong V3");
  drawStartGame();
}
void drawStartGame() {
  TV.select_font(font8x8);
  TV.print(DISPLAY_MESSAGE_X, 35, "Press Button");
  TV.print(DISPLAY_MESSAGE_X, 45, "For new Game.");
}
void drawMenuBall() {
    // TV.delay_frame(3);
    if (x + volX < 1 || x + volX > TV.hres() - 1) volX = -volX;
    if (y + volY < 1 || y + volY > TV.vres() - 1) volY = -volY;
    if (TV.get_pixel(x + volX, y + volY)) {
      TV.set_pixel(x + volX, y + volY, 0);
      if (TV.get_pixel(x + volX, y - volY) == 0) {
        volY = -volY;
      }
      else if (TV.get_pixel(x - volX, y + volY) == 0) {
        volX = -volX;
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

// -----------------------------------------------------------------------
// Game screen

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
void drawScore() {
  TV.select_font(font4x6);
  TV.print_char(LEFT_SCORE_X, SCORE_Y, '0' + leftPlayerScore);
  TV.print_char(RIGHT_SCORE_X, SCORE_Y, '0' + rightPlayerScore);
}

// -----------------------------------------------------------------------
// Game action

void gameBall() {
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
  }
  if (ballX >= TV.hres() - 1) {
    playerScored(LEFT);
  }
  //
  // draw the ball
  TV.set_pixel(ballX, ballY, 2);
}

void playerScored(byte player) {
  if (player == RIGHT) rightPlayerScore++;
  if (player == LEFT) leftPlayerScore++;
  if (leftPlayerScore == PLAY_TO || rightPlayerScore == PLAY_TO) {
    state = GAME_OVER;
  }
  ballVolX = -ballVolX;
}

// -----------------------------------------------------------------------
void setup()  {
  Serial.begin(9600);
  Serial.println("+++ Setup start.");
  TV.begin(_NTSC);       //for devices with only 1k sram(m168) use TV.begin(_NTSC,128,56)
  state = IN_MENU;
  x = 0;
  y = 0;
  ballX = TV.hres() / 2;
  ballY = TV.vres() / 2;
  Serial.println("+ Setup complete.");
}

// -----------------------------------------------------------------------
void getInputs() {
  paddleOnePosition = analogRead(PLAYER_ONE_PIN);
  paddleTwoPosition = analogRead(PLAYER_TWO_PIN);
  button1Status = (digitalRead(BUTTON_ONE_PIN));
}

void loop() {
  getInputs();
  delay(50);
  if (counter == 60) counter = 0; // increment or reset frame counter
  counter++;
  /*  For testing. However, when used, the TV output doesn't work.
  Serial.print("+ loop counter = ");
  Serial.print(counter);
  Serial.print(" State: ");
  Serial.print(state);
  Serial.print(" One Two Button: ");
  Serial.print(paddleOnePosition);
  Serial.print(" ");
  Serial.print(paddleTwoPosition);
  Serial.print(" ");
  Serial.println(button1Status);
  */
  
  if (state == IN_MENU) {
    if (doDrawMenu) {
      drawMenu();
      doDrawMenu = false;
    }
    drawMenuBall();
  }
  if (button1Status) {
    buttonPressed = true;
      state = PLAYING_GAME;
      doDrawWin = true;
    /*
    if (state == IN_MENU) {
      state = PLAYING_GAME;
      doDrawWin = true;
    }
    if (state == PLAYING_GAME) {
      // Pause
      state == PLAYING_GAME_PAUSE;
    }
    if (state == PLAYING_GAME_PAUSE) {
      state == PLAYING_GAME;
    }
    */
  }
  /*
  */
  if (state == PLAYING_GAME) {
    if (counter % 2 == 0) {
      // Note, if display every loop, the ball looks duplicated.
      drawGameBoard();
      drawScore();
      drawPaddles();
      gameBall();
    }
    TV.delay_frame(1);  // Required, else the screen flashes.
  }
  if (state == GAME_OVER) {
    if (doDrawWin) {
      drawGameBoard();
      drawScore();
      //
      TV.select_font(font8x8);
      TV.print(DISPLAY_MESSAGE_X, GAME_OVER_Y, "Game over.");
      drawStartGame();
      rightPlayerScore = 0;
      leftPlayerScore = 0;
      //
      doDrawWin = false;
    }
  }
}

// -----------------------------------------------------------------------
// eof
