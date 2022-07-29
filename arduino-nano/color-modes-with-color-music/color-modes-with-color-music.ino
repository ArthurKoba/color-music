#define AUDIO_PIN_ANALOG 2
#define IR_PIN_DIGITAL 2

#define RED_PIN 10
#define GREEN_PIN 9
#define BLUE_PIN 11

#define CALIBRATE_MAX_RED_LEVEL 1
#define CALIBRATE_MAX_GREEN_LEVEL 0.48
#define CALIBRATE_MAX_BLUE_LEVEL 0.62

#define SILENCE_AMPLITUDE_LEVEL 10
#define DELAY_BEFORE_ACTIVATION_OF_SILENT_MODE_MS 3000
#define SWAP_HOLD_TIME_MS 20000

#include "leds.h"
#include "colorModes.h"
#include "colorMusic.h"
#include "serialPortInteraction.h"
#include "IR.h"


void setup() {
  Serial.begin(500000);
  Serial.println("\nStart Home Color Music.");
  initLeds(leds);
  initColorModes(colorModes);
  initColorMusic(colorMusicConfig);
  IRLremote.begin(IR_PIN_DIGITAL);
}


void calculateColors(HistoriesOfColorAmplitudes &history, Leds &leds) {
  int8_t low = constrain(history.low.data[0], 0, 200);
  if (history.low.average < 60 && history.low.delta < 50) low = 0;
  else low = map(low, 0, 200, 5, 255);
  if (history.low.average > 180) low -= history.low.minimum * 0.65;
  leds.red = low;
  
  int8_t middle = history.middle.data[0];
  if (history.middle.average < 10 && history.middle.delta < 50) middle = 0;
  else if (history.middle.average > 160) middle -= history.middle.minimum * 0.65;
  leds.green = middle;
  
  int8_t high = history.high.data[0];
  if (history.high.average < 20 && history.high.delta < 50) high = 0; 
  else if (history.high.average > 200) high -= history.high.minimum * 0.65;
  leds.blue = high;
}


void colorMusic (Leds &leds, ColorModes &colorModes, ColorMusicConfig &cfg)
{
  if (cfg.needCalibrate == true) {
    calibrateOffsetOfSamples(cfg, leds);
  }
  readSamples(cfg.sampling);
//  sendSamples(cfg.sampling);  
  uint32_t deltaTimeLastSignalMs = millis() - colorMusicConfig.sampling.timeOfLastSignalMs;
  if (deltaTimeLastSignalMs > DELAY_BEFORE_ACTIVATION_OF_SILENT_MODE_MS || cfg.sampling.timeOfLastSignalMs == 0) {
    smoothColorMode(leds, colorModes.smooth); colorModes.smooth.index += 1;
  } else {
//    Serial.println("{\"dataType\":\"testData\", \"data\":[" + String(cfg.sampling.samplesAmplitude) + "]}");
    fhtProcess(cfg.fhtData);
//    sendAmplitudes(cfg.fhtData);
    calculateAndWriteColorAmplitudesToHistory(cfg.fhtData, cfg.colorsAmplitudesHistory);
    sendLowHistory(cfg.colorsAmplitudesHistory.low);
    sendMiddleHistory(cfg.colorsAmplitudesHistory.middle);
    sendHighHistory(cfg.colorsAmplitudesHistory.high);
    calculateColors(cfg.colorsAmplitudesHistory, leds);
  }
}


void loop()
  // Основной цикл микроконтроллера.
{
  if (IRLremote.available()) { // Проверка наличия команд с пульта.
    if (execIRCommand(leds, colorModes) == ON_BUTTON) {
      colorMusicConfig.needCalibrate = true; //При запуске режима светомузыки необходимость повторной калибровки
    }
  }
  switch (colorModes.mode) { // Асинхронная обработка режимов.
    case OFF_MODE: case COLOR_MODE: return;
    case COLOR_MUSIC_MODE: colorMusic(leds, colorModes, colorMusicConfig); break;
    case FLASH_MODE: flashColorMode(leds, colorModes.flash); break;
    case STROBE_MODE: strobeColorMode(leds, colorModes.strobe); break; 
    case FADE_MODE: fadeColorMode(leds, colorModes.fade); break;
    case SMOOTH_MODE: smoothColorMode(leds, colorModes.smooth); break;
  }
  writeLeds(leds);
}
