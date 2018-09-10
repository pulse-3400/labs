#include <QTRSensors.h>
#include <Servo.h>


#define NUM_SENSORS             3  // number of sensors used
#define NUM_SAMPLES_PER_SENSOR  4  // average 4 analog samples per sensor reading
#define EMITTER_PIN             2  // emitter is controlled by digital pin 2

// sensors 0 through 5 are connected to analog inputs 0 through 5, respectively
QTRSensorsAnalog qtra((unsigned char[]) {0, 1, 2},
  NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, EMITTER_PIN);
unsigned int sensorValues[NUM_SENSORS];

int threshold = 400;
int whiteline; // assumes start is on white line


/********************
 * ROBOT1 Variables *
 ********************/

Servo wheelRight;
Servo wheelLeft;

 

void setup()
{  

  // CALIBRATION
//  delay(500);
//  pinMode(13, OUTPUT);
//  digitalWrite(13, HIGH);    // turn on Arduino's LED to indicate we are in calibration mode
//  for (int i = 0; i < 400; i++)  // make the calibration take about 10 seconds
//  {
//    qtra.calibrate();       // reads all sensors 10 times at 2.5 ms per six sensors (i.e. ~25 ms per call)
//  }
//  digitalWrite(13, LOW);     // turn off Arduino's LED to indicate we are through with calibration


  
  Serial.begin(9600); // set the data rate in bits per second for serial data transmission
  wheelRight.attach(5); // wheels
  wheelLeft.attach(6);
  whiteline = getBackSensor();
  delay(1000);
}


void loop() 
{
  // CONDITION: back sensor must start on white line

  moveForwards();
  turnLeft();
  delay(250);
}

void printSensorValues() {
  qtra.read(sensorValues);
  Serial.print(sensorValues[0]); // BACK
  Serial.print("\t");
  Serial.print(sensorValues[1]); // LEFT
  Serial.print("\t");
  Serial.println(sensorValues[2]); //RIGHT
  delay(250);
}

int getBackSensor() {
  qtra.read(sensorValues); // read raw sensor values
  return sensorValues[0];
  
}

boolean reachedCross() {
  qtra.read(sensorValues);
  Serial.print(sensorValues[1]); // print raw values
  Serial.print("\t");
  Serial.println(sensorValues[2]);
  
  if (sensorValues[1] < threshold && sensorValues[2] < threshold) {
    return true;
  }
  else{
    return false;
  }
}

boolean onLine() {
  if (getBackSensor() < threshold)
    return true;
  return false;
}

/******************
 * ROBOT1 Methods *
 ******************/
 
void moveForwards() {
  while (!reachedCross()) {
    wheelRight.write(0);
    wheelLeft.write(180);
  }
  //halt();
}

void halt() {
  while(true) {
    wheelRight.write(90);
    wheelLeft.write(90);
  }
}

// method in progress
void turnLeft() {
  // move forward
  wheelRight.write(0);
  wheelLeft.write(180);
  delay(310); // move forwards to apply rotation corrections

 
  
  wheelRight.write(0);
  wheelLeft.write(0);
  delay(400);
  
  while (!(onLine() && sensorValues[1] > threshold && sensorValues[2] > threshold)){
    wheelRight.write(0);
    wheelLeft.write(0);
  }

  Serial.print(sensorValues[0]);
  Serial.print("\t");
  Serial.print(sensorValues[1]);
  Serial.print("\t");
  Serial.println(sensorValues[2]);
  
  halt();
}

