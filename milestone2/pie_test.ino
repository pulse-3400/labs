#include <QTRSensors.h>
#include <Servo.h>



#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library



#define NUM_SENSORS             3  // number of sensors used
#define NUM_SAMPLES_PER_SENSOR  4  // average 4 analog samples per sensor reading
#define EMITTER_PIN             2  // emitter is controlled by digital pin 2

#define BACK                    0
#define RIGHT                   1
#define LEFT                    2
//Distance sensor in A3

// sensors 0 through 2 are connected to analog inputs 0 through 2, respectively
QTRSensorsAnalog qtra((unsigned char[]) {0, 1, 2},
  NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, EMITTER_PIN);
unsigned int sensorValues[NUM_SENSORS];

int threshold = 400;

//controller shit
int linePos = 0;
int desiredPos = 2000;
int e_p = 0;
int prev_e_p = 0;
int e_d = 0;

//Correction Function Variables
volatile int leftspeed = 180;
volatile int rightspeed = 0;
int corrector = 30;

//State machine variables
int state;
int nextstate = 0;
int waitingcommand = 0;
int turncommand = 0;
int wallval = 0;

//Variables for figure 8 sequence
int turnsequence[] = {-1, -1, -1, -1, 1, 1, 1, 1};
int i = 0;

/********************
 * ROBOT1 Variables *
 ********************/
Servo wheelRight;
Servo wheelLeft;


void setup()
{    

  Serial.begin(115200); // use the serial port
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);


  
  //Serial.begin(9600); // set the data rate in bits per second for serial data transmission
  
  wheelRight.attach(5); // wheels
  wheelLeft.attach(6);

  wheelRight.write(90);
  wheelLeft.write(90);

  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  
  delay(1000);
}


void loop() 
{


  while(1) { // reduces jitter
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
    if(fft_log_out[42] >= 45){
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else{
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
  

  
  wallval = analogRead(A3);
  if(waitingcommand == 1){
//    turncommand = turnsequence[i];
//    i = (i == 7)? 0 : i + 1;
    if(wallval >= 200){
      turncommand = 1;
    }
    else {
      turncommand = 2;
    }
    waitingcommand = 0;
    digitalWrite(8, HIGH);
  }
 
  correctionMachine();
  
  
  Serial.println(wallval);
  /*
  e_p = desiredPos - linePos; //find proportional error term
  e_d = e_p - prev_e_p; //find first derivative error term
  prev_e_p = e_p;
  if((e_p + e_d) > 0){
    correctLeft();
  }
  else if((e_p + e_d) < 0){
    correctRight();
  }
  */
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
          nextstate = 9;
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
        nextstate = (turncommand == -1) ? 13 : ((turncommand == 1) ? 14 : 0);
        turncommand = 0;
      }
      break;
      
    case 13:
      turnRight();
      nextstate = 5;
      wheelRight.write(rightspeed);
      wheelLeft.write(leftspeed);
      delay(250);
      break;
      
    case 14:
      turnLeft();
      nextstate = 7;
      wheelRight.write(rightspeed);
      wheelLeft.write(leftspeed);
      delay(250);
      break;
      
    case 5:
      turnRight();
      turncommand = 0;
      nextstate = (linePos == 3000) ? 6 : 5;
      break;

    case 6:
      turnRight();
      if(linePos == 2000){
        delay(150);
        halt();
        nextstate = 0;
      }
      else{
        nextstate = 6;
      }
      break;

    case 7:
      turnLeft();
      turncommand = 0;
      nextstate = (linePos == 1000) ? 8 : 7;
      break;

    case 8:
      turnLeft();
      if(linePos == 2000){
        delay(150);
        halt();
        nextstate = 0;
      }
      else{
        nextstate = 8;
      }
      break;
  }

  
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
  L = (sensorValues[LEFT] < threshold)? 100 : 0;
  B = (sensorValues[BACK] < threshold)? 10 : 0;
  R = (sensorValues[RIGHT] < threshold)? 1 : 0;
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
      LP = 2000;   
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
