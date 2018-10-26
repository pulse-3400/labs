#include <QTRSensors.h>
#include <Servo.h>

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define NUM_SENSORS             3  // number of sensors used
#define NUM_SAMPLES_PER_SENSOR  4  // average 4 analog samples per sensor reading
#define EMITTER_PIN             2  // emitter is controlled by digital pin 2

#define BACK                    0  // Array indices for back, right, and left line sensors
#define RIGHT                   1
#define LEFT                    2

#define LINETHRESHOLD           400 // Threshold value for line sensors
#define corrector               30

#define NORTH     0
#define EAST      1
#define SOUTH     2
#define WEST      3

#define MUXPIN                  3   // When high, read from signal amps. When low, read from side wall sensors.

// ========================== Linse Sensor Setup ============================
// sensors 0 through 2 are connected to analog inputs 4, 1, and 2, respectively
QTRSensorsAnalog qtra((unsigned char[]) {4, 1, 2}, 
NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, EMITTER_PIN);
unsigned int sensorValues[NUM_SENSORS];

byte robotkill = 0;    //Variable used for noise suppression in robot detection
byte startRobot = 0;   //Variable used for noise suppression in tone detection
//============================================================================


// ====== Variables for Maze Drawing ======  <-- This is lab 3 stuff
short orientation = 1000 + NORTH;
byte xpos = 0;
byte ypos = 0;
byte walls = 0;
short transmission;
//maybe have a maze array later? There's no algorithm yet so it doesn't really matter right now.
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
short state;
short nextstate = 0;
short waitingcommand = 0;
short turncommand = 0;
// ===========================================


// ==== Servo Declarations ====
Servo wheelRight;
Servo wheelLeft;
// ============================

void setup()
{    
  //Serial.begin(9600); // set the data rate in bits per second for serial data transmission
  pinMode(MUXPIN, OUTPUT);
  digitalWrite(MUXPIN, HIGH); // default to reading signal amps
  
  wheelRight.attach(5); // wheels
  wheelLeft.attach(6);

  pinMode(8, OUTPUT);  //Debug LED for front wall sensor
  digitalWrite(8, LOW);

  pinMode(2, OUTPUT); //Debug LED for IR detection
  pinMode(9, OUTPUT); //Debug LED for tone detection
  digitalWrite(2, LOW);

  wheelRight.write(90); //Start wheels stationary
  wheelLeft.write(90);
  
  while (startRobot < 20) { //Tone detection
    listen660();
    delay(10);
  }
  digitalWrite(9, HIGH);
  
}


void loop() 
{
  if(waitingcommand == 1){
    getTurn();
    transmission = (walls << 8) | (ypos << 4) | xpos; // FORMATION OF RADIO TRANSMISSION (explanation above)
    waitingcommand = 0;
    digitalWrite(8, HIGH);
  }
 
  correctionMachine();
  
  doFFT();
  
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
          ypos--;
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
  digitalWrite(MUXPIN, LOW); //read from wall sensors
  short wallval = 0;
  short wallvalleft = 0;
  short wallvalright = 0;
  wallval = analogRead(A3);
  wallvalleft = analogRead(A0);
  wallvalright = analogRead(A5);
  
  byte WP, L, B, R;
  L = (wallvalleft > 200) ? B01 : 0;
  B = (wallval > 200) ? B01 : 0;
  R = (wallvalright > 200) ? B01 : 0;
  WP = (L<<2) | (B<<1) | R ;

  //Assign walls
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
  
  //-1 for right turn, 1 for left turn, 2 for straight
  
  switch(WP){
    case B000:
      turncommand = 2;
      break;
    case B001:
      turncommand = 2;
      break;
    case B010:
      turncommand = 1;
      break;
    case B011:
      turncommand = 1;
      break;
    case B0100:
      turncommand = 2;
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


void listen660() {

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
      digitalWrite(9, LOW);
    }

    TIMSK0 = temp_TIMSK0;
    ADCSRA = temp_ADCSRA;
    ADMUX = temp_ADMUX;
    DIDR0 = temp_DIDR0;
}

void doFFT() {
  digitalWrite(MUXPIN, HIGH); //read from signal amps
  
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
      digitalWrite(2, HIGH);
      while(1);
    }

    
  TIMSK0 = temp_TIMSK0;
  ADCSRA = temp_ADCSRA;
  ADMUX = temp_ADMUX;
}


