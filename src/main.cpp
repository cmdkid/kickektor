#include <Arduino.h>
#include <TM1637.h>
#include "fix_fft.h"  // fix_fft.ccp & fix_fft.h in same directory as sketch

#define PIN_AUX_IN 3  //digital audio in
#define PIN_DEBUG 6 
#define PIN_SETUP 7 
#define PIN_RELAY1 2 
#define PIN_GATE_VAL_POT 1 

// relay1 config
const int rel0_gate_ch = 1; //20
int rel0_gate_val = 12;
const int rel0_delay = 100;
const int rel0_delay_after = 50;
int rel0_gate_valx3 = 1; //rel0_gate_val * 3;
int tmp_int;

// relay board config
const int rel_count = 1;

// fft processing vars
bool debug = false, setup_screen = false;
char im[64], ttf_data[64];
int float_window_width = 3;
int last_fl_idx = -1, last_idx = 0; //float window idx'es - 1
float last_val[10][3]; // float window vals
bool is_trig_rel[rel_count];

TM1637 tm(10, 9); //init display (CLK, DIO) digital pins

void tm_update_config() {
  tmp_int = analogRead(PIN_GATE_VAL_POT) / 16;
  if(tmp_int != rel0_gate_val) {
    rel0_gate_val = tmp_int;
    //rel0_gate_valx3 = rel0_gate_val * 3;
    //Serial.println("rel0_gate_val=" + (String)rel0_gate_val);
    tm.display(rel0_gate_val);
  }
}

void setup() {
  pinMode(PIN_RELAY1, OUTPUT);

  // init relays
  for (int i = 0; i < rel_count; i++) {
    is_trig_rel[i] = false;
  }
  
  if (digitalRead(PIN_DEBUG) == 0) {
    debug = true;
    Serial.begin(9600);
    Serial.println("Debug ON");
  }

  if (digitalRead(PIN_SETUP) == 0) {
    setup_screen = true;
    tm.begin();
    if (true == debug) {
      Serial.println("Setup ON");
    }
  }
}

int get_last_idx() {
  if (last_fl_idx >= float_window_width) {
    last_fl_idx = -1;
  }
  return ++last_fl_idx;
}

void append_to_float_window(int channel_id, char val) {
  for (int i = float_window_width-1; i > 0; i--) {
    last_val[channel_id][i] = last_val[channel_id][i-1];
  }
  last_val[channel_id][0] = val;
}

bool is_triggered(int channel_id, int gate_val) {
  long sum = 0L;
  for (int i = 0; i < float_window_width; i++) {
    sum += last_val[channel_id][i];
  }

  if (true == debug) {
    if (rel0_gate_valx3 < sum) {
      Serial.print(sum/float_window_width);
    } else {
      Serial.print(0);
    }
    Serial.print(" ");
  }

  if ((((float) sum) / float_window_width) >= gate_val) {
    return true;
  } else {
    return false;
  }
}

void trig_relay(int relay_id, int delay_time, int rel0_delay_after, int channel_id) {
  if (is_trig_rel[relay_id] == false) {
    digitalWrite(PIN_RELAY1, true);
    is_trig_rel[relay_id] = true;
    for (int i = 0; i < float_window_width; i++) {
      last_val[channel_id][i] = 0;
    }
    delay(delay_time);
    digitalWrite(PIN_RELAY1, false);
    delay(rel0_delay_after);
  }
}

void loop() {
  if (true == setup_screen) {
    tm_update_config();
  }
  // get eq snapshot
  for (int i = 0; i < 64; i++) {
    ttf_data[i] = ((analogRead(PIN_AUX_IN) / 4 ) - 128); // normalize analog in
    im[i]  = 0;   // imaginary component  
  }
  fix_fft(ttf_data, im, 6, 0);   // Send normalized analog values through fft

  // get last index in floating window
  last_idx = get_last_idx();

  // calculate the absolute values of bins in the array - only want positive values
  for (int i = 0; i < 10; i++) {
    append_to_float_window(i, (float)(sqrt(ttf_data[i]  *  ttf_data[i] +  im[i] *  im[i])));
  }

  // check value limits
  if (is_triggered(rel0_gate_ch, rel0_gate_val)) {
    trig_relay(rel0_gate_ch, rel0_delay, rel0_delay_after, rel0_gate_ch);
  } else {
    is_trig_rel[0] = false;
  }
}