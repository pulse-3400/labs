
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);

const uint64_t pipes[2] = { 0x0000000042LL, 0x0000000043LL };


void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

    radio.begin();

    // optionally, increase the delay between retries & # of retries
    radio.setRetries(15,15);
    radio.setAutoAck(true);
    // set the channel
    radio.setChannel(0x50);
    // set the power
    // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
    radio.setPALevel(RF24_PA_HIGH);
    //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
    radio.setDataRate(RF24_250KBPS);
  
    // optionally, reduce the payload size.  seems to
    // improve reliability
    radio.setPayloadSize(2);

    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);

    radio.startListening();



}

void loop() {

      unsigned int telemetry;
      
      if (radio.available()) {
        bool done = false;
        while (!done)
        {
         
          // Fetch the payload, and see if this was the last one.
          done = radio.read( &telemetry, sizeof(unsigned short) );       
        }

        
        Serial.print(translate(telemetry) + "\n");
      }
  
}

String translate(int telemetry) {

  if (telemetry == 0xFFFF)
    return "reset\n";
    
  String v = "";


  // y position
  v += ((telemetry & 0x00F0) >> 4);
  v += ",";

  // x position
  v += ((telemetry & 0x000F));

  
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
