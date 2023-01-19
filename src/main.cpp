/* FFT_TEST4
 Ray Burnette 20130810 function clean-up & 1284 port (328 verified)
 Modified by varind in 2013: this code is public domain, enjoy!
 http://www.variableindustries.com/audio-spectrum-analyzer/
 328P = Binary sketch size: 5,708 bytes (of a 32,256 byte maximum)
 */
#include <Arduino.h>
#include "fix_fft.h"  // fix_fft.ccp & fix_fft.h in same directory as sketch

#define LCHAN 3

const int Yres = 8;
const int gain = 3;
float peaks[64];
char im[64], data[64];
char data_avgs[64];
int debugLoop;


int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void Two16_LCD(){
  for (int x = 1; x < 16; x++) {  // init 0 to show lowest band overloaded with hum
    if (data_avgs[x] > peaks[x]) peaks[x] = data_avgs[x];

    if (peaks[x] == 0) {
      Serial.print(" .   ");  // less LCD artifacts than " "
    }
    else {
      Serial.print(peaks[x]);
      Serial.print(" ");
    }
  }
  Serial.println("");
}

void decay(int decayrate){
  int DecayTest = 1;
  // reduce the values of the last peaks by 1 
  if (DecayTest == decayrate){
    for (int x = 0; x < 32; x++) {
      peaks[x] = peaks[x] - 1;  // subtract 1 from each column peaks
      DecayTest = 0;
    }
  }

  DecayTest++;
}

void setup() {
  Serial.begin(9600); // hardware serial
  Serial.print("Debug ON");
  Serial.println("");
}

void loop() {
  for (int i = 0; i < 64; i++) {
    data[i]  = ((analogRead(LCHAN) / 4 ) - 128);  // chose how to interpret the data from analog in  
    im[i]  = 0;   // imaginary component  
  }

  fix_fft(data, im, 6, 0);   // Send Left channel normalized analog values through fft

  // At this stage, we have two arrays of [0-31] frequency bins deep [32-63] duplicate

  // calculate the absolute values of bins in the array - only want positive values
  for (int i = 0; i < 16; i++) {
    data[i] = sqrt(data[i]  *  data[i] +  im[i] *  im[i]);

    // COPY the Right low-band (0-15) into the Left high-band (16-31) for display ease
    data_avgs[i] = data[i];

    // Remap values to physical display constraints... that is, 8 display custom character indexes + "_"
    //data_avgs[i] = constrain(data_avgs[i], 0, 9 - gain);     //data samples * range (0-9) = 9
    //data_avgs[i] = map(data_avgs[i], 0, 9 - gain, 0, Yres);  // remap averaged values
  }

  Two16_LCD();
  decay(1);
}