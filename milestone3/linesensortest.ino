#include <Servo.h>

Servo right;
Servo left;

#define MUXPIN0                 3   //Address bits for analog multiplexer
#define MUXPIN1                 4   //Addresses: 00 = 660Hz, 01 = front wall sensor, 10 = left, 11 = right

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  left.attach(5);
  right.attach(6);
  left.write(90);
  right.write(90);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("\nLine Sensors: ");
  Serial.print(analogRead(A2));
  Serial.print("   ");
  Serial.print(analogRead(A1));
  Serial.print("   ");
  Serial.print(analogRead(A4));
  Serial.print("\nWall Sensors: ");
  
  digitalWrite(MUXPIN0, LOW);
  digitalWrite(MUXPIN1, HIGH);
  delay(1);
  Serial.print("\nLeft: ");
  Serial.println(analogRead(A5));
  
  digitalWrite(MUXPIN0, HIGH);
  digitalWrite(MUXPIN1, LOW);
  delay(1);
  Serial.print("Center: ");
  Serial.println(analogRead(A5));
  
  digitalWrite(MUXPIN0, HIGH);
  digitalWrite(MUXPIN1, HIGH);
  delay(1);
  Serial.print("Right: ");
  Serial.println(analogRead(A5));
  
  delay(1000);
}
