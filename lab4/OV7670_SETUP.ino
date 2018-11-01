#include <Wire.h>

#define OV7670_I2C_ADDRESS 0x21


///////// Main Program //////////////
void setup() {
  Wire.begin();
  Serial.begin(9600);
  
  write_key_registers();
  
  delay(100);
  
  read_key_registers();
 
}

void loop(){
 }


///////// Function Definition //////////////
void read_key_registers(){
  Serial.println(read_register_value(0x12), BIN); //COM7

  Serial.println(read_register_value(0x40), BIN); //COM15

  Serial.println(read_register_value(0x11), BIN); //CLKRC

  Serial.println(read_register_value(0x0C), BIN); //COM3

  Serial.println(read_register_value(0x3E), BIN); //COM14
}

void write_key_registers(){
  byte regval;
  
  //Reset using COM7
  regval = 0x80;
  Serial.println(OV7670_write_register(0x12, regval));
  
  //Set QCIF and RGB Using COM7
  regval = B00001100;
  Serial.println(OV7670_write_register(0x12, regval));
  
  //Set RGB 565 Using COM15
  regval = B11010000;
  Serial.println(OV7670_write_register(0x40, regval));

  //Set internal clock to use external clock using CLKRC
  regval = B11000000;
  Serial.println(OV7670_write_register(0x11, regval));

  //Enable Scaler Using COM3
  regval = B00001000;
  Serial.println(OV7670_write_register(0x0C, regval));

  //Enable Scaling for Predefined Formats using COM14
  regval = B00001000;
  Serial.println(OV7670_write_register(0x3E, regval));

  //Color Bar Using COM7
  regval = B00001110;
  Serial.println(OV7670_write_register(0x12, regval));
  
  //70 and 71 for scaling

  
}



byte read_register_value(int register_address){
  byte data = 0;
  Wire.beginTransmission(OV7670_I2C_ADDRESS);
  Wire.write(register_address);
  Wire.endTransmission();
  Wire.requestFrom(OV7670_I2C_ADDRESS,1);
  while(Wire.available()<1);
  data = Wire.read();
  return data;
}

String OV7670_write(int start, const byte *pData, int size){
    int n,error;
    Wire.beginTransmission(OV7670_I2C_ADDRESS);
    n = Wire.write(start);
    if(n != 1){
      return "I2C ERROR WRITING START ADDRESS";   
    }
    n = Wire.write(pData, size);
    if(n != size){
      return "I2C ERROR WRITING DATA";
    }
    error = Wire.endTransmission(true);
    if(error != 0){
      return String(error);
    }
    return "no errors :)";
 }

String OV7670_write_register(int reg_address, byte data){
  return OV7670_write(reg_address, &data, 1);
 }

void set_color_matrix(){
    OV7670_write_register(0x4f, 0x80);
    OV7670_write_register(0x50, 0x80);
    OV7670_write_register(0x51, 0x00);
    OV7670_write_register(0x52, 0x22);
    OV7670_write_register(0x53, 0x5e);
    OV7670_write_register(0x54, 0x80);
    OV7670_write_register(0x56, 0x40);
    OV7670_write_register(0x58, 0x9e);
    OV7670_write_register(0x59, 0x88);
    OV7670_write_register(0x5a, 0x88);
    OV7670_write_register(0x5b, 0x44);
    OV7670_write_register(0x5c, 0x67);
    OV7670_write_register(0x5d, 0x49);
    OV7670_write_register(0x5e, 0x0e);
    OV7670_write_register(0x69, 0x00);
    OV7670_write_register(0x6a, 0x40);
    OV7670_write_register(0x6b, 0x0a);
    OV7670_write_register(0x6c, 0x0a);
    OV7670_write_register(0x6d, 0x55);
    OV7670_write_register(0x6e, 0x11);
    OV7670_write_register(0x6f, 0x9f);
    OV7670_write_register(0xb0, 0x84);
}
