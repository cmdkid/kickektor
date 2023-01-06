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
int trig_range[TRIG_COUNT*2];
// trigger frequency range
uint16_t trig_freq[TRIG_COUNT*2];
// tmp cycle trigger status
bool tmpTrigStatus[TRIG_COUNT];

void setTrigerStatus(int trig_id, bool value) {
  int trig_pin = 0;
  switch (trig_id) {
    case 0:
      trig_pin = TRIG_1;
      break;
    case 1:
      trig_pin = TRIG_2;
      break;
    case 3:
      trig_pin = TRIG_3;
      break;
    case 4:
      trig_pin = TRIG_4;
      break;
  }
  if (trig_pin > 0) {
    // we use triggers, who works with zero volt trigger, so "ON" is low voltage, and "OFF" is high voltage
    if (false == value) {
      digitalWrite(trig_pin, HIGH);
    } else {
      digitalWrite(trig_pin, LOW);
    }
    Serial.println((String)"Trigger " + String(trig_pin) + " changed to " + String(value));
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
  }
}

// init vals
void setup() {
  Serial.begin(9600);
  Serial.println(F("Start"));
  initTriggers();

  for (int i = 0; i < TRIG_COUNT; i ++) {
    delay(1000);
    setTrigerStatus(i, true);
    delay(1000);
    setTrigerStatus(i, false);
  }
  delay(2000);

  trig_range[0] = 2;
  trig_range[1] = 31;
  trig_freq[0] = 40;
  trig_freq[1] = 20000;
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
  
  // fht_log_out 0 and 1 are noisy, ignore it
  for (int i = 2; i < 32; i++) {
    Serial.println((String)"fht_log_out[" + String(i) + "]=" + String(fht_log_out[i]));
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

  //update relay status
  for (int i = 0; i < TRIG_COUNT; i++) {
    setTrigerStatus(i, tmpTrigStatus[i]);
  }
  delay(10);
}

// main loop
void loop() {
  triggersLoop();
}