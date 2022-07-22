#define IR_PIN 2

#define RED_PIN 10
#define GREEN_PIN 9
#define BLUE_PIN 11

#define CALIBRATE_MAX_RED_LEVEL 1
#define CALIBRATE_MAX_GREEN_LEVEL 0.48
#define CALIBRATE_MAX_BLUE_LEVEL 0.62

#define SWAP_HOLD_TIME_MS 20000

#include "leds.h"
#include "colorModes.h"
#include "IR.h"

void setup() {
//  Serial.begin(500000);
//  Serial.println("\nStart Home Color Music.");
  initLeds(leds);
  initColorModes(colorModes);
  IRLremote.begin(IR_PIN);
}

void loop()
  // Основной цикл микроконтроллера.
{
  if (IRLremote.available()) { // Проверка наличия команд с пульта.
    execIRCommand(leds, colorModes);
  }
  switch (colorModes.mode) { // Асинхронная обработка режимов.
    case OFF_MODE: case COLOR_MODE: return;
    case SWAP_MODE: swapColorMode(leds, colorModes); break;
    case FLASH_MODE: flashColorMode(leds, colorModes.flash); break;
    case STROBE_MODE: strobeColorMode(leds, colorModes.strobe); break; 
    case FADE_MODE: fadeColorMode(leds, colorModes.fade); break;
    case SMOOTH_MODE: smoothColorMode(leds, colorModes.smooth); break;
  }
  writeLeds(leds);
}
