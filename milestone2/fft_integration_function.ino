/* INITIAL SETUP */
#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
}


/* CALL THIS FUNCTION IN MAIN LOOP WHEN SENSING OTHER ROBOTS */
void doFFT() {
  byte temp_TIMSK0 = TIMSK0; // save register values to avoid turning timer off completely
  byte temp_ADCSRA = ADCSRA;
  byte temp_ADMUX = ADMUX;

  /* FROM robotsense_ir */
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  ADMUX = 0x01; // turn off the digital input for adc0

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

    
  TIMSK0 = temp_TIMSK0;
  ADCSRA = temp_ADCSRA;
  ADMUX = temp_ADMUX;

}
