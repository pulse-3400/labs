#include <Servo.h>

Servo wheelA; // right wheel when moving away from you
Servo wheelB; // left wheel when moving away from you

int clockwiseA = 0;
int counterclockwiseA = 180;
int clockwiseB = 180;
int counterclockwiseB = 0;
int halt = 90;

int heartbeat = 3; // led indicator

void setup() {
  Serial.begin(9600);
  wheelA.attach(5);
  wheelB.attach(6);
  pinMode(heartbeat, OUTPUT);
}

/**
 * Main function
 */
void loop() {
  digitalWrite(heartbeat, HIGH);
  figureEight(2000,3600);
  quit();

}

void freeze() {
  wheelA.write(halt);
  wheelB.write(halt);
}

void forwards(int side) {
  wheelA.write(clockwiseA);
  wheelB.write(clockwiseB);
  delay(side);
}

void backwards(int side) {
  wheelA.write(counterclockwiseA);
  wheelB.write(counterclockwiseB);
  delay(side);
}

void turn(int speedA, int speedB, int delay1) {
  wheelA.write(speedA);
  wheelB.write(speedB);
  delay(delay1); // amount of time to turn 90 deg
}

void squareRight(int side) {
  forwards(side);
  turn(counterclockwiseA, clockwiseB, 800);
  forwards(side);
  turn(counterclockwiseA, clockwiseB, 800);
  forwards(side);
  turn(counterclockwiseA, clockwiseB, 800);
  forwards(side);
  turn(counterclockwiseA, clockwiseB, 800);
}

void figureEight(int side, int turnSide) {
  turn(clockwiseA,90,turnSide);
  forwards(side);
  turn(90,clockwiseB,turnSide);
  forwards(side);
}

void quit() {
  while (true) {
    freeze();
  }
}
