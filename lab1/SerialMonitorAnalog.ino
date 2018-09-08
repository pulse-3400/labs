int pot = A0;
int potValue = 0;

int analogLED = 3;
int outputValue = 0;

//min:102 max: 1023
void setup() {
  // put your setup code here, to run once:
  pinMode(analogLED, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  potValue = analogRead(pot);
  Serial.println(potValue);
  outputValue = map(potValue, 0, 1023, 0, 142);
  analogWrite(analogLED, outputValue);
  delay(2);
}
