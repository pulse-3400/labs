#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library
int threshold = 400;
short robotkill = 0;
int startRobot = 0;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  int startRobot = 0;
  pinMode(2, OUTPUT);

  pinMode(9, OUTPUT);

}

void loop() {x
  // put your main code here, to run repeatedly:
  while (startRobot < 7) {
    listen660();
  }

  digitalWrite(9, HIGH);

  doFFT();

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
      startRobot = startRobot + 1;
    }
    else{
      digitalWrite(9, LOW);
    }


    TIMSK0 = temp_TIMSK0;
    ADCSRA = temp_ADCSRA;
    ADMUX = temp_ADMUX;
    DIDR0 = temp_DIDR0;

}


void doFFT() {
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
    digitalWrite(2, LOW);
    if(robotkill > 10){
      //wheelRight.write(90);
      //wheelLeft.write(90);
      digitalWrite(2, HIGH);
      while(1);
    }

    
  TIMSK0 = temp_TIMSK0;
  ADCSRA = temp_ADCSRA;
  ADMUX = temp_ADMUX;

}
