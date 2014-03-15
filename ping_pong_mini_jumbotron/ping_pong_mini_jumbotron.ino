/* ==================== Import Necessary Libraries ==================== */

#include <Adafruit_GFX.h>   // For Matrix - Core graphics library
#include <RGBmatrixPanel.h> // For Matrix - Hardware-specific library

/* ==================== Define Pins For Use on Arduino Uno Board ==================== */

// Pins For Matrix Display
// Uses 8 Digital Pins (D2-D9) and 4 Analog pins (A0-A3)
#define CLK  8
#define LAT A3
#define OE   9
#define A   A0
#define B   A1
#define C   A2

// Pins For Momentary Buttons
// Uses 4 Digital Pins (D10-D13)
const int pinP1B1 = 11; // Player 1, Green Button
const int pinP1B2 = 13; // Player 1, Red Button
const int pinP2B1 = 10; // Player 2, Green Button
const int pinP2B2 = 12; // Player 2, Red Button

/* ==================== Setup Variables For Keeping Track of Score, Game Status, Etc ==================== */

// Rules for Ping Pong
int winningScore = 11;
int servesAtATime = 2;

//keep track of the player's scores
int scoreP1 = 0;
int scoreP2 = 0;

//used to see if the score changed so we know to update the display
int scoreLastP1 = 0;
int scoreLastP2 = 0;

//keep track of who is serving
int whoWonServe = 0;
int whoIsCurrentlyServing = 0;

//keep track of who served last
int whoServedLast = 0;

// Who is the winner
int winner = 0;

// Is the game over
int isGameOver = 0;

// Have we reached the game point yet?
//int haveReachedGamePoint = false;

// Who gets the courtesy serve (aka, who is losing OR who did NOT get to 10 first)
int courtesyServePlayer = 0;

// Keep track of current button states. These will be set to 1 (or HIGH) if pressed, 0 (or LOW) if unpressed
int stateP1B1 = 0; // Player 1, Green Button
int stateP1B2 = 0; // Player 1, Red Button   
int stateP2B1 = 0; // Player 2, Green Button   
int stateP2B2 = 0; // Player 2, Red Button      

// Keep track of current
int currentStateP1B1 = 0;   
int currentStateP1B2 = 0;   
int currentStateP2B1 = 0;   
int currentStateP2B2 = 0;   

// previous state of the button
int lastStateP1B1 = 0;     
int lastStateP1B2 = 0;     
int lastStateP2B1 = 0;
int lastStateP2B2 = 0;

/* ==================== Setup Display Matrix - 16 x 32 RGB Pixels ==================== */

// Initialize the Matrix variable
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

// Define some colors to reuse
uint16_t red = matrix.Color333(7,0,0);
uint16_t green = matrix.Color333(0,7,0);
uint16_t blue = matrix.Color333(0,0,7);

/* ==================== Arduino's Default setup() Function - Runs Once At Beginning ==================== */

void setup() 
{
  // initialize the pushbutton pins as an input
  pinMode(pinP1B1, INPUT);  
  pinMode(pinP1B2, INPUT);    
  pinMode(pinP2B1, INPUT);  
  pinMode(pinP2B2, INPUT);  

  // initialize the matrix
  matrix.begin();
  matrix.setTextWrap(true); // Allow text to run off right edge
  matrix.setTextSize(1);    // size 1 == 8 pixels high

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);  
  
  // Make sure we're ready to start our first game
  resetGame();
}

/* ==================== Arduino's Default loop() Function - Runs Continuously After setup() Function Finishes ==================== */

void loop() 
{
  //Check for button 1 press
  stateP1B1 = digitalRead(pinP1B1);
  stateP1B2 = digitalRead(pinP1B2);
  stateP2B1 = digitalRead(pinP2B1);
  stateP2B2 = digitalRead(pinP2B2);  
  
//  printStatesSerial();

  checkSimulatenousPressing(stateP1B1, stateP1B2, stateP2B1, stateP2B2);
  
//  printSerialGameVars();
  
  updateButtonState(stateP1B1, lastStateP1B1, scoreP1,  1);
  updateButtonState(stateP1B2, lastStateP1B2, scoreP1, -1);
  updateButtonState(stateP2B1, lastStateP2B1, scoreP2,  1);
  updateButtonState(stateP2B2, lastStateP2B2, scoreP2, -1);  

  updateWhoWonServe();
  
  updateWhoIsCurrentlyServing();

//  currentStateP1B1 = getButtonState(stateP1B1, lastStateP1B1);
//  currentStateP1B2   = getButtonState(stateP1B2,   lastStateP1B2);
//  currentStateP2B1 = getButtonState(stateP2B1, lastStateP2B1);
//  currentStateP2B2   = getButtonState(stateP2B2,   lastStateP2B2);
  
//  printCurrentStatesSerial();

//  printScoresSerial();
  
  if(scoreP1 != scoreLastP1 || scoreP2 != scoreLastP2 || whoWonServe != whoServedLast)
  {
//    Serial.println("Score changed or whoWonServe changed");
    //only change the display is something happened
    int winner = getWinner();
    
    if(winner == 0)
    {
      //game still continuing
      printScores();           
    }
    else if(isGameOver == 0)
    {
      //we have a winner
      printWinner();
      isGameOver = 1;
    }
//    int isGameOver = scoreGame();
    
//    int whichPlayerIsServing = getWhichPlayerIsServing();
    
  }
  
  scoreLastP1 = scoreP1;
  scoreLastP2 = scoreP2;  

  delay(100);        // delay in between reads for stability
}

void resetGame()
{
  //reset states
  stateP1B1 = 0;
  stateP1B2 = 0;
  stateP2B1 = 0;
  stateP2B2 = 0;  
  
  //Reset everything
  scoreP1 = 0;
  scoreP2 = 0;
  scoreLastP1 = 0;
  scoreLastP2 = 0;
  whoWonServe = 0;
  whoIsCurrentlyServing = 0;
  whoServedLast = 0;
  winner = 0;
  isGameOver = 0;  
  courtesyServePlayer = 0;

  // Update the display  
  printScores();
}

int getTotalScore()
{
  return scoreP1 + scoreP2;
}

void checkSimulatenousPressing(int stateP1B1, int stateP1B2, int stateP2B1, int stateP2B2)
{
  //if any 2 buttons are pressed simultaneously, reset game
  if(stateP1B1 + stateP1B2 + stateP2B1 + stateP2B2 == 2)
  {
    //reset game
    resetGame();
    
    //give everyone time to release any buttons
    delay(500);
  } 
}

void updateButtonState(int state, int &lastState, int &score, int amount)
{
  if(state != lastState) 
  {
    if(state == HIGH) 
    {
      score += amount;
      
      if(score < 0)
      {
        score = 0;
      }
//      score = (score < 0) ? 0 : score; //no negative scores
    } 
  }  
  
  lastState = state;
}

int getButtonState(int state, int &lastState)
{
  int result = 0;
  
  if(state != lastState && state == HIGH)
  {
    result = 1;
  }

  // Update the lastState (since we're passing by reference)
  lastState = state;

  return result;
}

void updateWhoWonServe()
{
  if(whoWonServe == 0) // If we haven't determined who is serving, then the green button will act as server selection
  {
    if(scoreP1 > 0 && scoreP2 == 0)
    {
      scoreP1 = 0;
      whoWonServe = 1;
      whoIsCurrentlyServing = 1;
    }
    else if(scoreP2 > 0 && scoreP1 == 0)
    {
      scoreP2 = 0;
      whoWonServe = 2;
      whoIsCurrentlyServing = 2;    
    }
  }
  else if(getTotalScore() == 0 && stateP1B2 == HIGH) // check P1 red button
  {
    whoWonServe = 1;
  }
  else if(getTotalScore() == 0 && stateP2B2 == HIGH) // check P2 red button
  {
    whoWonServe = 2;
  }
}

void updateWhoIsCurrentlyServing()
{
  
  if(courtesyServePlayer == 0) // Check to see if we should assign the courtesy serve FOR THE FIRST TIME
  {
    if(scoreP1 == 10)
    {
      courtesyServePlayer = 2;
    }
    else if(scoreP2 == 10)
    {
      courtesyServePlayer = 1;
    }
  }
  
  if(courtesyServePlayer == 0) // Normal serving mode
  {
    if(getTotalScore() % (servesAtATime * 2) == 0 || getTotalScore() % (servesAtATime * 2) == 1)
    {
      whoIsCurrentlyServing = whoWonServe;
    }
    else
    {
      if(whoWonServe == 2)
        whoIsCurrentlyServing = 1;
      else
        whoIsCurrentlyServing = 2;
    }
  }
  else //courtesy serving mode
  {
    whoIsCurrentlyServing = courtesyServePlayer;
    
    if(courtesyServePlayer == 1 && scoreP1 > scoreP2)
    {
        courtesyServePlayer = 2;
    }
    else if(courtesyServePlayer == 2 && scoreP2 > scoreP1)
    {
        courtesyServePlayer = 1;
    }
    
  }
  
//  printSerialGameVars();
  
}

/**
 * Returns:
 *     0 if the game is still going
 *     1 if player 1 wins
 *     2 if player 2 wins 
 */
int getWinner()
{
  int winner = 0;

  if(scoreP1 >= 11 && scoreP1 - scoreP2 >= 2)
  {
    winner = 1;
  }
  else if(scoreP2 >= 11 && scoreP2 - scoreP1 >= 2)  
  {
    winner = 2;
  }

  return winner;
}


/* ==================== Functions below update the Matrix Display ==================== */

void printScores()
{
  // fill the screen with 'black'
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  
  // ---------- Player 1 Score ---------- //
  
  if(scoreP1 > scoreP2)
    matrix.setTextColor(green);
  else if(scoreP1 == scoreP2)    
    matrix.setTextColor(blue);
  else
    matrix.setTextColor(red);

  matrix.setCursor(1, 1);    

  matrix.print(scoreP1);

  // ---------- Player 2 Score ---------- //

  if(scoreP2 > scoreP1)
    matrix.setTextColor(green);
  else if(scoreP1 == scoreP2)    
    matrix.setTextColor(blue);    
  else
    matrix.setTextColor(red);
  
  matrix.setCursor(20, 1);
  //indent if single digit score
  if(scoreP2 < 10)
    matrix.print(" ");    
  
  matrix.print(scoreP2);

  // ---------- Show server  ---------- //  
  
  if(whoIsCurrentlyServing == 1)
  {
    drawServingBall(1, 11);
  }
  else if(whoIsCurrentlyServing == 2)
  {
    drawServingBall(27, 11);
  }
  else
  {
    drawServingBall(14, 11);
  }
}

/**
  * Draws a ball (Overall dimensions 4w x 4h)
  */
void drawServingBall(int x, int y)
{
  matrix.drawRect(x+1, y, 2, 4, matrix.Color333(1, 1, 1));
  matrix.drawRect(x, y+1, 4, 2, matrix.Color333(1, 1, 1));
}


/**
  * Draws an arrow (Overall dimensions 10w x 8h)
  */
void drawArrow(int x, int y, int direction, uint16_t color)
{
  matrix.drawRect(x, y+3, 10, 2, color);
  
  if(direction == 1)
  {
    matrix.drawLine(x, y+3, x+3, y, color);
    matrix.drawLine(x, y+4, x+3, y+7, color);    
  }
  if(direction == 2)
  {
    matrix.drawLine(x+9, y+3, x+6, y, color);
    matrix.drawLine(x+9, y+4, x+6, y+7, color);
  }
//Draw corner dots
//  matrix.drawPixel(x,y,matrix.Color333(7, 7, 0));
//  matrix.drawPixel(x+9,y+7,matrix.Color333(7, 0, 7));  
}


int printWinner()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0)); // fill the screen with 'black'     
  matrix.setTextColor(green);
  matrix.setTextSize(1);

  if(scoreP1 > scoreP2)
  {
    matrix.setCursor(14, 1);
    matrix.print("You"); 
    matrix.setCursor(14, 9);    
    matrix.print("Win");
    drawArrow(1, 5, 1, matrix.Color333(0,7,7));
  }
  else if(scoreP2 > scoreP1)
  {
    matrix.setCursor(1, 1);
    matrix.print("You"); 
    matrix.setCursor(1, 9);    
    matrix.print("Win");
    drawArrow(20, 5, 2, matrix.Color333(0,7,7));
  }
  else
  {
    //should never be here
    matrix.print("???");
  }
}

/* ==================== Serial Printing Functions below ==================== */

void printSerialGameVars()
{
  //Print everything
  Serial.print(scoreP1);
  Serial.print(" - ");  
  Serial.print(scoreP2);
  Serial.print(" - ");  
  Serial.print(scoreLastP1);
  Serial.print(" - ");  
  Serial.print(scoreLastP2);
  Serial.print(" - ");  
  Serial.print(whoWonServe);
  Serial.print(" - ");  
  Serial.print(whoIsCurrentlyServing);
  Serial.print(" - ");  
  Serial.print(whoServedLast);
  Serial.print(" - ");  
  Serial.print(winner);
  Serial.print(" - ");  
  Serial.print(isGameOver);
  Serial.print(" - ");  
  Serial.print(courtesyServePlayer);
  Serial.println();   
  
}
void printStatesSerial()
{
  Serial.print("[");  
  Serial.print(stateP1B1);
  Serial.print("][");
  Serial.print(stateP1B2);
  Serial.print("][");
  Serial.print(stateP2B1);
  Serial.print("][");
  Serial.print(stateP2B2);
  Serial.print("]");  
  Serial.println();   
}

void printCurrentStatesSerial()
{
  Serial.print(currentStateP1B1);
  Serial.print(" - ");
  Serial.print(currentStateP1B2);
  Serial.print(" - ");
  Serial.print(currentStateP2B1);
  Serial.print(" - ");
  Serial.print(currentStateP2B2);
  Serial.println();   
}

void printScoresSerial()
{
  Serial.print("             ");
  Serial.print(scoreP1);
  Serial.print(" - ");
  Serial.print(scoreP2);
  Serial.print(" Last: ");  
  Serial.print(scoreLastP1);
  Serial.print(" - ");
  Serial.print(scoreLastP2);
  Serial.println();   
}
