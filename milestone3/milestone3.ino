#include <QTRSensors.h>
#include <Servo.h>

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

#define NUM_SENSORS             3  // number of sensors used
#define NUM_SAMPLES_PER_SENSOR  4  // average 4 analog samples per sensor reading
#define EMITTER_PIN             2  // emitter is controlled by digital pin 2

#define BACK                    1  // Array indices for back, right, and left line sensors
#define RIGHT                   0
#define LEFT                    2

#define LINETHRESHOLD           600 // Threshold value for line sensors
#define corrector               30

#define NORTH     0
#define EAST      1
#define SOUTH     2
#define WEST      3

#define MUXPIN0                 3   //Address bits for analog multiplexer
#define MUXPIN1                 4   //Addresses: 00 = 660Hz, 01 = front wall sensor, 10 = left, 11 = right
#define WALLTHRESHOLD           100 //200

// ========================== Linse Sensor Setup ============================
// sensors 0 through 2 are connected to analog inputs 4, 1, and 2, respectively
QTRSensorsAnalog qtra((unsigned char[]) {4, 1, 2}, 
NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, EMITTER_PIN);
unsigned int sensorValues[NUM_SENSORS];

byte robotkill = 0;    //Variable used for noise suppression in robot detection
int startRobot = 0;   //Variable used for noise suppression in tone detection
// ===========================================================================


// ============= Radio Setup ===============
RF24 radio(9,10);
const uint64_t pipes[2] = { 0x0000000042LL, 0x0000000043LL };
// =========================================


// ====== Variables for Maze Drawing ======  <-- This is lab 3 stuff
short orientation = 1000 + EAST; //change this to change starting orientation
static byte xpos = 0;
static byte ypos = 0;
short walls = 0;
short transmission;
static byte maze[9][9]; 
// ==== Maze Array Explanation ====
//                       N E S W
//   [ 0 0 0 |    0    | 0 0 0 0 ]
//             Visited    Walls
// ================================

/*
Maze Coordinates explanation:
xpos = 0,ypos = 0 is the top left hand corner of the maze. The robot assumes it starts at (0, 0). X increases
left to right and Y increases top to bottom.
To calculate orientation, we do modulus 4 with the orientation variable (orientation % 4) and the remainder
is the orientation, where 0 is North, 1 is East, 2 is South, and 3 is West. This allows us to simply increment
when turning right or decrement when turning left to update our orientation. To find position, we check our
orientation each time we move forward one square and either increment or decrement the appropriate coordinate
(i.e. if (orientation % 4 == 1) so we are facing East, we would increment xpos on moving forward).
Right now I am formatting the 16-bit radio transmission like this:

              N E S W
  [ 0 0 0 0 | 0 0 0 0 | 0 0 0 0 | 0 0 0 0 ]
    Unused     Walls     ypos       xpos
*/
// ========================================


// ====== Correction Function Variables ======
volatile short leftspeed = 180;
volatile short rightspeed = 0;
// ===========================================


// ========= State machine variables =========
short linePos = 0;
static short state;
static short nextstate = 0;
static short waitingcommand = 0;
static short turncommand = 0;
// ===========================================


// ==== Servo Declarations ====
Servo wheelRight;
Servo wheelLeft;
// ============================

void setup()
{    
  radio.begin();
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  radio.setChannel(0x50);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.setPayloadSize(2);
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.startListening();
  
  Serial.begin(9600); // set the data rate in bits per second for serial data transmission

// ============= Analog Mux Setup and 660Hz Detection =============
  pinMode(MUXPIN0, OUTPUT);
  pinMode(MUXPIN1, OUTPUT);
  digitalWrite(MUXPIN0, LOW); // default to reading microphone amp
  digitalWrite(MUXPIN1, LOW);  
  delay(10);
  while (startRobot < 20) { //Tone detection
    Serial.println("Listening for 660 Hz tone!");
    listen660();
  }

  digitalWrite(MUXPIN1, LOW);
// ================================================================

  
// ============== Wheel Setup (AFTER 660Hz FFT) =================
  wheelRight.attach(5); // wheels
  wheelLeft.attach(6);

  wheelRight.write(90); //Start wheels stationary
  wheelLeft.write(90);
// ==============================================================


  radio.stopListening();
  short t = 0xFFFF;
  radio.startWrite(&t, sizeof(short));
  
  delay(500);
  getTurn();
  turncommand = 0;
  transmission = (walls << 8) | (1<<8); // FORMATION OF RADIO TRANSMISSION (explanation above)
  radio.startWrite(&transmission, sizeof(short)); 
}


void loop() 
{
  if(waitingcommand == 1){
    getTurn();
    transmission = (walls << 8) | (ypos << 4) | xpos; // FORMATION OF RADIO TRANSMISSION (explanation above)
    maze[ypos][xpos] |=  (1 << 4) | walls; //Update maze array
    radio.startWrite(&transmission, sizeof(short));
    waitingcommand = 0;
  }

  correctionMachine();
  
  //doFFT();
  
  wheelRight.write(rightspeed);
  wheelLeft.write(leftspeed);
  
  delay(5);
  
}

//constrain(analog, ANALOG_MIN, 255);

 /******************
 * Path Correction *
 ******************/
 
void correctionMachine(){
  state = nextstate;
  linePos = getLinePos();
  switch (state){
    case 0:
      digitalWrite(8, LOW);
      correctStraight();
      
      switch(linePos){
        case 0:
          nextstate = 0;
          break;
        case 1000:
          nextstate = 1;
          break;
        case 2000:
          nextstate = 0;
          break;
        case 3000:
          nextstate = 2;
          break;
        case 4000:
          nextstate = 3;
          break;
        default:
          nextstate = 0;
          break;
      }
      break;
      
    case 1:
      correctLeft();
      nextstate = (linePos == 1000)? 1 : ((linePos == 4000) ? 3 : 0);
      break;

    case 2:
      correctRight();
      nextstate = (linePos == 3000)? 2 : ((linePos == 4000) ? 3 : 0);
      break;
      
    case 3:
      correctStraight();
      switch(orientation % 4){
        case 0:
          ypos--;
          break;
        case 1:
          xpos++;
          break;
        case 2:
          ypos++;
          break;
        case 3:
          xpos--;
          break;
      }
      delay(250);
      nextstate = 4;
      //nextstate = (linePos == 4000)? 3 : 4;
      break;
      
    case 4:
      halt();
      waitingcommand = 1;
      if(turncommand == 0){
        nextstate = 4;
      }
      else{
        waitingcommand = 0;
        nextstate = (turncommand == -1) ? 5 : ((turncommand == 1) ? 8 : 0);
        turncommand = 0;
      }
      break;
      
    //================== RIGHT TURN SEQUENCE ================  
    case 5:
      turnRight();
      nextstate = 6; 
      wheelRight.write(rightspeed);
      wheelLeft.write(leftspeed);
      delay(250);
      break;
      
    case 6:
      turnRight();
      turncommand = 0;
      nextstate = (linePos == 3000) ? 7 : 6;
      break;

    case 7:
      turnRight();
      if(linePos == 2000){
        delay(150);
        halt();
        orientation++; //Upon completion of turn, update orientation
        nextstate = 4;
      }
      else{
        nextstate = 7;
      }
      break;
    //=======================================================

    //================= LEFT TURN SEQUENCE ==================
    case 8:
      turnLeft();
      nextstate = 9;
      wheelRight.write(rightspeed);
      wheelLeft.write(leftspeed);
      delay(250);
      break;

    case 9:
      turnLeft();
      turncommand = 0;
      nextstate = (linePos == 1000) ? 10 : 9;
      break;

    case 10:
      turnLeft();
      if(linePos == 2000){
        delay(150);
        halt();
        nextstate = 4;
        orientation--; //Upon completion of turn, update orientation
      }
      else{
        nextstate = 10;
      }
      break;
  }
  //=========================================================  
}


void correctStraight(){
  if(rightspeed > 0){
    rightspeed -= corrector;
  }
  if(leftspeed < 180){
  leftspeed += corrector;
  }
}

void correctLeft(){
  if(rightspeed > 0){
    rightspeed -= corrector;
  }
  else if(rightspeed == 0 && leftspeed > 90){
    leftspeed -= corrector;
  }
}

void correctRight(){
  if(leftspeed < 180){
    leftspeed += corrector;
  }
  else if(leftspeed == 180 && rightspeed < 90){
    rightspeed += corrector;
  }
}

/******************
 * ROBOT1 Methods *
 ******************/

void turnLeft(){
  rightspeed = 0;
  leftspeed = 0;
}

void turnRight(){
  rightspeed = 180;;
  leftspeed = 180;
}

void halt() {
  rightspeed = 90;
  leftspeed = 90;
}

 /*********************
 * Position Estimator *
 *********************/
int getLinePos(){
  qtra.read(sensorValues);
  int LP, L, B, R, sum;
  L = (sensorValues[LEFT] < LINETHRESHOLD)? 100 : 0;
  B = (sensorValues[BACK] < LINETHRESHOLD)? 10 : 0;
  R = (sensorValues[RIGHT] < LINETHRESHOLD)? 1 : 0;
  sum = L + B + R;
  switch(sum){
    case 000:
      LP = 2000;
      break;
    case 001:
      LP = 3000;
      break;
    case 011:
      LP = 3000;
      break;
    case 010:
      LP = 2000;
      break;
    case 100:
      LP = 1000;
      break;
    case 110:
      LP = 1000;
      break;
    case 111:
      LP = 4000;   
      break;
    case 101:
      LP = linePos;
      break;
    default:
      LP = 2000;
      break; 
  }
  return LP;
}

void getTurn() {
  //MUX Addresses: 00 = 660Hz, 01 = front wall sensor, 10 = left, 11 = right
  short wallval = 0;
  short wallvalleft = 0;
  short wallvalright = 0;
  delay(250);
  //Set MUX for front wall sensor
  digitalWrite(MUXPIN0, HIGH);
  digitalWrite(MUXPIN1, LOW);
  delay(1);
  for(int i = 0; i < 3; i++){
    wallval += analogRead(A5);
    delay(5);
  }
  wallval = wallval / 3;

  //Set MUX for left wall sensor
  digitalWrite(MUXPIN0, LOW);
  digitalWrite(MUXPIN1, HIGH);
  delay(1);
  for(int i = 0; i < 3; i++){
    wallvalleft += analogRead(A5);
    delay(5);
  }
  wallvalleft = wallvalleft / 3;

  //Set MUX for right wall sensor
  digitalWrite(MUXPIN0, HIGH);
  digitalWrite(MUXPIN1, HIGH);
  delay(1);
  for(int i = 0; i < 3; i++){
    wallvalright += analogRead(A5);
    delay(5);
  }
  wallvalright = wallvalright / 3;
  
//  Serial.println(wallval);
//  Serial.println(wallvalleft);
//  Serial.println(wallvalright);
  
  
  byte WP, L, B, R;
  L = (wallvalleft > WALLTHRESHOLD) ? B01 : 0;
  B = (wallval > WALLTHRESHOLD) ? B01 : 0;
  R = (wallvalright > WALLTHRESHOLD + 30) ? B01 : 0;
  WP = (L<<2) | (B<<1) | R ;

  //Assign walls for maze update and transmission
  switch(orientation % 4){
        case 0:
          walls = (B << 3) | (R << 2) | L ; // 1 1 0 1
          break;
        case 1:
          walls = (L << 3) | (B << 2) | (R << 1) ; // 1 1 1 0
          break;
        case 2:
          walls = (L << 2) | (B << 1) | R ; // 0 1 1 1
          break;
        case 3:
          walls = (R << 3) | (L << 1) | B ; // 1 0 1 1
          break;
      }
  //Check whether adjacent squares are visited
  switch(orientation % 4){
        case 0:
          L = (xpos > 0) ? maze[xpos - 1][ypos] & (1 << 4) : 0;
          B = (ypos > 0) ? maze[xpos][ypos - 1] & (1 << 4) : 0;
          R = (xpos < 8) ? maze[xpos + 1][ypos] & (1 << 4) : 0;
          break;
        case 1:
          L = (ypos > 0) ? maze[xpos][ypos - 1] & (1 << 4) : 0;
          B = (xpos < 8) ? maze[xpos + 1][ypos] & (1 << 4) : 0;
          R = (ypos < 8) ? maze[xpos][ypos + 1] & (1 << 4) : 0;
          break;
        case 2:
          L = (xpos < 8) ? maze[xpos + 1][ypos] & (1 << 4) : 0;
          B = (ypos < 8) ? maze[xpos][ypos + 1] & (1 << 4) : 0;
          R = (xpos > 0) ? maze[xpos - 1][ypos] & (1 << 4) : 0;
          break;
        case 3:
          L = (ypos < 8) ? maze[xpos][ypos + 1] & (1 << 4) : 0;
          B = (xpos > 0) ? maze[xpos - 1][ypos] & (1 << 4) : 0;
          R = (ypos > 0) ? maze[xpos][ypos - 1] & (1 << 4) : 0;
          break;
      }
  
  //-1 for right turn, 1 for left turn, 2 for straight
  
  switch(WP){
    case B000:
      //check unvisited
      if(B == 0) turncommand = 2;
      else if(R == 0) turncommand = -1;
      else if(L == 0)turncommand = 1;
      else turncommand = 2;
      break;
    case B001:
      //check unvisited
      if(B == 0) turncommand = 2;
      else if (L == 0)turncommand = 1;
      else turncommand = 2;
      break;
    case B010:
      //check unvisited
      if(L == 0) turncommand = 1;
      else if (R == 0) turncommand = -1;
      else turncommand = -1;
      break;
    case B011:
      turncommand = 1;
      break;
    case B0100:
      //check unvisited
      if(B == 0) turncommand = 2;
      else if (R == 0) turncommand = -1;
      else turncommand = 2;
      break;
    case B0101:
      turncommand = 2;
      break;
    case B0110:
      turncommand = -1;
      break;
    case B0111:
      turncommand = 1;
      break;
  }
  digitalWrite(8, HIGH);
}


void listen660() { //On analog 5

  byte temp_TIMSK0 = TIMSK0;
  byte temp_ADMUX = ADMUX;
  byte temp_ADCSRA = ADCSRA;
  byte temp_DIDR0 = DIDR0;



  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x45; // use adc0
  DIDR0 = 0b00100000; // turn off the digital input for adc0

  cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei();
    if(fft_log_out[5] >= 110){
      startRobot++;
    }
    else{
      startRobot = 0;
    }

    TIMSK0 = temp_TIMSK0;
    ADCSRA = temp_ADCSRA;
    ADMUX = temp_ADMUX;
    DIDR0 = temp_DIDR0;
}

void doFFT() {
  
  byte temp_TIMSK0 = TIMSK0; // save register values to avoid turning timer off completely
  byte temp_ADCSRA = ADCSRA;
  byte temp_ADMUX = ADMUX;
  

  /* FROM robotsense_ir */
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0

  cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei();
    //Serial.println(fft_log_out[42]);
    if(fft_log_out[42] >= 55){
      
      robotkill++;
    }
    else{
      
      robotkill = 0;
    }
    if(robotkill > 10){
      wheelRight.write(90);
      wheelLeft.write(90);
      while(1);
    }

    
  TIMSK0 = temp_TIMSK0;
  ADCSRA = temp_ADCSRA;
  ADMUX = temp_ADMUX;
}


