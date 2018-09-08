#include <Servo.h>

Servo servo; 

int pos = 90;

void setup() {
  // put your setup code here, to run once:
 servo.attach(6);
}

void loop() {
  // put your main code here, to run repeatedly:
  servo.write(pos);
}

//0 - clockwise full speed
//180 - counterclockwise full speed
//90 - stop
//if 90 doesn't stop, use screwdriver to adjust the screw on the side so that it is fully stopped
