#define AUDIO_PIN 2

#define RED_PIN 10
#define GREEN_PIN 9
#define BLUE_PIN 11

#define CALIBRATE_MAX_RED_LEVEL 1
#define CALIBRATE_MAX_GREEN_LEVEL 0.58
#define CALIBRATE_MAX_BLUE_LEVEL 0.82

//#define SEND_SAMPLES true
//#define SEND_AMPLITUDES true

//#define SEND_LOW_HISTORY true
//#define SEND_MIDDLE_HISTORY true
//#define SEND_HIGH_HISTORY true

#define SEND_LEDS true


#include "core.h"

ColorMusicConfig cfg;

void setup() {
  setupTimers();
  analogReference(EXTERNAL);
  Serial.begin(500000);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  Serial.println("\n Start Koba Color Music. Version: " + String(COLOR_MUSIC_VERSION));
  setupConfigs(cfg);
  calibrateOffsetOfSamples(cfg);
//  sendSamplingConfigs(cfg.sampling);
}
 
#define LOW_SMOOTH 3
#define MIDDLE_SMOOTH 1
#define HIGH_SMOOTH 2

void calculateColors(HistoriesOfColorAmplitudes &history, Leds &leds) {


  int8_t high = (history.high.data[0] + history.high.data[1]) >> 1;
  high -= history.high.minimum;
  leds.blue = high;
  
//  int8_t low = map(history.low.data[0], history.low.minimum, history.low.maximum, -lowMedian, lowMedian);
//  int8_t low = map(history.low.data[0], history.low.minimum, history.low.maximum, -lowMedian, lowMedian);
  int8_t low = history.low.data[0];
  low -= history.low.minimum;
  leds.red = low;




  int8_t middle = history.middle.data[0];
//  middle -= history.middle.minimum;
  
  leds.green = middle;

}




void colorMusic(ColorMusicConfig &cfg) {
  readSamples(cfg.sampling);
  uint32_t deltaTimeLastSignalMs = millis() - cfg.sampling.timeOfLastSignalMs;
  if (deltaTimeLastSignalMs > DELAY_BEFORE_ACTIVATION_OF_SILENT_MODE_MS || cfg.sampling.timeOfLastSignalMs == 0) {
    if (deltaTimeLastSignalMs < FADE_OUT_TIME_AFTER_SILENT_MODE_MS) {
      uint8_t brightness = map(deltaTimeLastSignalMs, FADE_OUT_TIME_AFTER_SILENT_MODE_MS, DELAY_BEFORE_ACTIVATION_OF_SILENT_MODE_MS, 255, 0);
//      Serial.println("{\"dataType\":\"testData\", \"data\":[" + String(power) + "]}");
//      lowerBrightness(cfg.leds, power);
      cfg.leds = {brightness, brightness, brightness};
    } else {
      transfusionColorMode(cfg.leds, cfg.colorModes.transfusionIndex, 0);
    }
  } else {
    fhtProcess(cfg.fhtData);
    calculateAndWriteColorAmplitudesToHistory(cfg.fhtData, cfg.colorsAmplitudesHistory);
    calculateColors(cfg.colorsAmplitudesHistory, cfg.leds);
  }
  writeValuesToLeds(cfg.leds);
  
}

void loop() {
//  glideColorMode(cfg.leds, cfg.colorModes.glideIndex, cfg.colorModes.glideDirection, 15);
//  colorMusic(cfg);
//  cfg.leds = {255, 255, 255};
  transfusionColorMode(cfg.leds, cfg.colorModes.transfusionIndex);
  writeValuesToLeds(cfg.leds);

}
