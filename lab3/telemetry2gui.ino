
void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("reset\n");
  unsigned int telemetry = 0xF900;
  Serial.print(translate(telemetry) + "\n");
  delay(1000);
  telemetry = 0xFC10;
  Serial.print(translate(telemetry) + "\n");
  delay(1000);
  telemetry = 0xF611;
  Serial.print(translate(telemetry) + "\n");
  delay(1000);
  telemetry = 0xF301;
  Serial.print(translate(telemetry) + "\n");
  delay(1000);
  
}

String translate(int telemetry) {
  String v = "";

 // x position
  v += ((telemetry & 0x000F));
  v += ",";

  // y position
  v += ((telemetry & 0x00F0) >> 4);


  
  // walls
  boolean n = ((telemetry & 0x0F00) >> 8) & B1000;
  boolean e = ((telemetry & 0x0F00) >> 8) & B0100;
  boolean w = ((telemetry & 0x0F00) >> 8) & B0001;
  boolean s = ((telemetry & 0x0F00) >> 8) & B0010;

  if (n) {
    v += ",north=true";
  }
  if (e) {
    v += ",east=true";
  }
  if (s) {
    v += ",south=true";
  }
  if (w) {
    v += ",west=true";
  }

   
  return v;
}
