#include <Arduino.h>

#define FHT_N 64         // ширина спектра х2
#define LOG_OUT 1
#include <FHT.h>         // преобразование Хартли

#define SOUND_R_FREQ A3  // analog audio in (through capasitor)

// relay triggers setup
#define TRIG_COUNT 4 // ON VAR CHANGE: update func setTrigerStatus,initTriggers and define TRIG_N
bool trigStatus[TRIG_COUNT];  // current triggers status
// digital pins to relay trigger
#define TRIG_1 2  
#define TRIG_2 3
#define TRIG_3 4
#define TRIG_4 5
// trigger fht_log_out min,max range
//int trig_range[TRIG_COUNT*2];
// trigger frequency range
//uint16_t trig_freq[TRIG_COUNT*2];
// tmp cycle trigger status
bool tmpTrigStatus[TRIG_COUNT];
int trigFoundCount[TRIG_COUNT];
//DELETE ME
//uint16_t max_freq_val = 0;
//int max_freq_id = 0;

void setTrigerStatus(int trig_id, bool value) {
  int trig_pin = 0;
  switch (trig_id) {
    case 0:
      trig_pin = TRIG_1;
      break;
    case 1:
      trig_pin = TRIG_2;
      break;
    case 2:
      trig_pin = TRIG_3;
      break;
    case 3:
      trig_pin = TRIG_4;
      break;
  }
  if (trig_pin > 0) {
    // we use triggers, who works with zero volt trigger, so "ON" is low voltage, and "OFF" is high voltage
    if (false == value) {
      digitalWrite(trig_pin, LOW); //HIGH
    } else {
      digitalWrite(trig_pin, HIGH);
    }
    Serial.print((String)"D" + String(trig_pin) + ">" + String(value) + "|");
  }
}

// set trigger vars
void initTriggers() {
  pinMode(TRIG_1, OUTPUT);
  pinMode(TRIG_2, OUTPUT);
  pinMode(TRIG_3, OUTPUT);
  pinMode(TRIG_4, OUTPUT);

  for (int i = 0; i < TRIG_COUNT; i ++) {
    trigStatus[i] = false;
    tmpTrigStatus[i] = false;
    setTrigerStatus(i, false);
    trigFoundCount[i] = 0;
  }
}

void relay_blink(int relay_id, int delay_time) {
    setTrigerStatus(relay_id, true);
    delay(delay_time);
    setTrigerStatus(relay_id, false);
}

void trigger(int relay_id, int delay_time, bool trigger_found = true) {
  if (trigger_found == false) {
    if (trigFoundCount[relay_id] != 0) {
      relay_blink(relay_id, delay_time);
    }
    trigFoundCount[relay_id] = 0;
  } else {
    trigFoundCount[relay_id] += 1;
  }
  /*
  if (trigger_found == true) {
    trigFoundCount[relay_id] += 1;
    if (trigFoundCount[relay_id] > 1) {
      trigFoundCount[relay_id] = 0;
      relay_blink(relay_id, delay_time);
    }
  } else {
    trigFoundCount[relay_id] = 0;
  }
  */
  
  
}

// init vals
void setup() {
  Serial.begin(9600);
  Serial.println(F("Init"));
  initTriggers();

  for (int i = 0; i < TRIG_COUNT; i ++) {
    relay_blink(i, 500);
    delay(100);
  }
  delay(1000);

  // debug init vals
  /*
  trig_range[0] = 2;
  trig_range[1] = 31;
  trig_freq[0] = 40;
  trig_freq[1] = 20000;
  */
  Serial.println(F("Start"));
}

// get range of input sound values
void analyzeAudio() {
  for (int i = 0 ; i < FHT_N ; i++) {
    int sample = analogRead(SOUND_R_FREQ);
    fht_input[i] = sample; // put real data into bins
  }
  fht_window();  // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run();     // process the data in the fht
  fht_mag_log(); // take the output of the fht
}

void triggersLoop() {
  // get sound data
  analyzeAudio();
  //Serial.print("fht_log_out:");
  //max_freq_val = 0;
  //max_freq_id = 0;

  if (fht_log_out[2] >= 88){
    //relay_blink(0, 50);
    trigger(0, 50);
  } else {
    trigger(0, 50, false);
  }

  //if (fht_log_out[7] >= 40){
  //  relay_blink(1, 10);
  //}
  
  /*
  if (fht_log_out[2] >= 63 && fht_log_out[2] < 90){
    relay_blink(2, 10);
  }
  */
  
  for(int i = 2; i < 32; i++) {

    if (fht_log_out[i] >= 20) {
      Serial.print(String(fht_log_out[i]));
    } else {
      Serial.print("00");
    }
    Serial.print(" ");
    //if (fht_log_out[i] > max_freq_val) {
    //  max_freq_val = fht_log_out[i];
    //  max_freq_id = i;
    //}
  }

  
  //Serial.print("max_freq_id:" + String(max_freq_id) + " freq_val=" + String(max_freq_val));
  /*
  // fht_log_out 0 and 1 are noisy, ignore it
  for (int i = 2; i < 32; i++) {
    Serial.print(String(fht_log_out[i]) + ",");
    // for every trigger
    for (int j = 0; j < TRIG_COUNT; j++) {
        // check range min/max
        if (i >= trig_range[j*2] && i <= trig_range[j*2+1]) {
            // check freq range
            if (fht_log_out[i] >= trig_freq[j*2] && fht_log_out[i] <= trig_freq[j*2+1]) {
                tmpTrigStatus[j] = true;
            }
            else {
                tmpTrigStatus[j] = false;
            }
        }
    }
  }
  Serial.print(F(">>"));
  //update relay status
  for (int i = 0; i < TRIG_COUNT; i++) {
    setTrigerStatus(i, tmpTrigStatus[i]);
  }
  */

  Serial.println(F("."));
  //delay(10);
}

// main loop
void loop() {
  triggersLoop();
}