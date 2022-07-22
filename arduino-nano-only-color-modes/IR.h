#ifndef IR_PIN
  #error IR_PIN not defined!
#endif

#include "IRLremote.h"

CNec IRLremote;

enum IRButtons
  // Перечисление кнопок пульта.
{
  BRIGHT_UP_BUTTON, BRIGHT_DOWN_BUTTON, OFF_BUTTON, ON_BUTTON,
  RED_BUTTON, GREEN_BUTTON, BLUE_BUTTON, WHITE_BUTTON,
  ORANGERED_BUTTON, LIME_BUTTON, VIOLET_BUTTON, FLASH_BUTTON,
  ORANGE_BUTTON, CYAN_BUTTON, PURPLE_BUTTON, STROBE_BUTTON,
  GOLD_BUTTON, SKY_BUTTON, ORCHID_BUTTON, FADE_BUTTON,
  YELLOW_BUTTON, NAVY_BUTTON, PINK_BUTTON, SMOOTH_BUTTON
};


void setColor(Leds &leds, uint8_t ledID)
  // Установка цветов согласно схеме пульта.
{
  switch (ledID) {
    case RED_BUTTON: leds = {255, 0, 0, leds.bright}; break;
    case GREEN_BUTTON: leds = {0, 255, 0, leds.bright}; break;
    case BLUE_BUTTON: leds = {0, 0, 255, leds.bright}; break;
    case WHITE_BUTTON: leds = {255, 255, 255, leds.bright}; break;
    
    case ORANGERED_BUTTON: leds = {255, 63, 0, leds.bright}; break;
    case LIME_BUTTON: leds = {63, 255, 63, leds.bright}; break;
    case VIOLET_BUTTON: leds = {63, 63, 255, leds.bright}; break;
    
    case ORANGE_BUTTON: leds = {255, 162, 0, leds.bright}; break;
    case CYAN_BUTTON: leds = {0, 255, 255, leds.bright}; break;
    case PURPLE_BUTTON: leds = {255, 0, 255, leds.bright}; break;
    
    case GOLD_BUTTON: leds = {255, 192, 0, leds.bright}; break;
    case SKY_BUTTON: leds = {63, 63, 255, leds.bright}; break;
    case ORCHID_BUTTON: leds = {255, 63, 255, leds.bright}; break;
    
    case YELLOW_BUTTON: leds = {255, 255, 0, leds.bright}; break;
    case NAVY_BUTTON: leds = {20, 10, 120, leds.bright}; break;
    case PINK_BUTTON: leds = {255, 63, 150, leds.bright}; break;
  }
}


void execIRCommand (Leds &leds, ColorModes &colorModes)
  // Обработка сигналов полученных с пульта.
{
  auto data = IRLremote.read();
  Serial.println("CMD: " + String(data.command));
  switch (data.command) {
    case BRIGHT_UP_BUTTON:
      leds.bright += 0.1;
      if (leds.bright > 1) leds.bright = 1.0;
      break;
    case BRIGHT_DOWN_BUTTON:
      leds.bright -= 0.1;
      if (leds.bright < 0.3) leds.bright = 0.3;
      break;
    case OFF_BUTTON:
      colorModes.mode = OFF_MODE;
      leds = {0, 0, 0, leds.bright};
      break;
    case ON_BUTTON: colorModes.mode = SWAP_MODE; break;
    case FLASH_BUTTON: colorModes.mode = FLASH_MODE; break;
    case STROBE_BUTTON: colorModes.mode = STROBE_MODE; break;
    case FADE_BUTTON: colorModes.mode = FADE_MODE; break;
    case SMOOTH_BUTTON: colorModes.mode = SMOOTH_MODE; break;
    default:
      colorModes.mode = COLOR_MODE;
      setColor(leds, data.command);
  }
  writeLeds(leds);
}
