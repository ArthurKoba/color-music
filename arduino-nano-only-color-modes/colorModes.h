#ifndef FLASH_MODE_DELAY_MS
  #define FLASH_MODE_DELAY_MS 200
#endif
#ifndef FLASH_MODE_LOW_LEVEL
  #define FLASH_MODE_LOW_LEVEL 0
#endif
#ifndef FLASH_MODE_MAX_LEVEL
  #define FLASH_MODE_MAX_LEVEL 255
#endif

#ifndef FADE_MODE_DELAY_MS
  #define FADE_MODE_DELAY_MS 10
#endif
#ifndef FADE_MODE_LOW_LEVEL
  #define FADE_MODE_LOW_LEVEL 20
#endif
#ifndef FADE_MODE_MAX_LEVEL
  #define FADE_MODE_MAX_LEVEL 255
#endif

#ifndef STROBE_MODE_DELAY_MS
  #define STROBE_MODE_DELAY_MS 50
#endif
#ifndef STROBE_MODE_LOW_LEVEL
  #define STROBE_MODE_LOW_LEVEL 0
#endif
#ifndef STROBE_MODE_MAX_LEVEL
  #define STROBE_MODE_MAX_LEVEL 255
#endif

#ifndef SMOOTH_MODE_DELAY_MS
  #define SMOOTH_MODE_DELAY_MS 20
#endif
#ifndef SMOOTH_MODE_LOW_LEVEL
  #define SMOOTH_MODE_LOW_LEVEL 60
#endif
#ifndef SMOOTH_MODE_MAX_LEVEL
  #define SMOOTH_MODE_MAX_LEVEL 255
#endif

#ifndef SWAP_HOLD_TIME_MS
  #define SWAP_HOLD_TIME_MS 60000
#endif
#ifndef SWAP_WITHOUT_STROBE
  #define SWAP_WITHOUT_STROBE true
#endif

#ifndef DEFAULT_MODE
  #define DEFAULT_MODE SWAP_MODE
#endif

enum ColorMode
  // Перечисление возможных режимов работы.
{
  OFF_MODE, SWAP_MODE, COLOR_MODE, 
  FLASH_MODE, STROBE_MODE, 
  FADE_MODE, SMOOTH_MODE
};

struct SwapModeData
  // Структура необходимая для работы режима переключения режимов.
{
  uint8_t currentMode;
  uint32_t lastSwap;
};

struct FlashModeData
  // Структура необходимая для работы режима вспышек цветов.
{
  uint8_t index;
  uint8_t delayMs;
  bool dir;
};

struct StrobeModeData
  // Структура необходимая для работы режима стробоскопа.
{
  bool activeState;
  uint8_t delayMs;
};

struct FadeModeData
  // Структура необходимая для работы режима нарастания и понижения яркости.
{
  uint8_t index;
  bool dir;
  uint8_t delayMs;
};

struct SmoothModeData
  // Структура необходимая для работы режима плавного переливания цветов.
{
  uint8_t index;
  uint8_t delayMs;
};

struct ColorModes
  // Структура содержащая активный режим и структуры режимов. 
{
  ColorMode mode;
  SwapModeData swap;
  FlashModeData flash;
  StrobeModeData strobe;
  FadeModeData fade;
  SmoothModeData smooth;
} colorModes;


void initColorModes(ColorModes &colorModes)
  // Функуция инициализации стрктуры световых режимов.
{
  colorModes.mode = DEFAULT_MODE; // Стандартный режим при запуске микроконтроллера.
  colorModes.swap.lastSwap = 0;
  colorModes.swap.currentMode = FLASH_MODE;
  colorModes.flash.delayMs = FLASH_MODE_DELAY_MS;
  colorModes.flash.index = 3;
  colorModes.strobe.delayMs = STROBE_MODE_DELAY_MS;
  colorModes.fade.delayMs = FADE_MODE_DELAY_MS;
  colorModes.fade.index = (FADE_MODE_MAX_LEVEL - FADE_MODE_LOW_LEVEL) >> 1;
  colorModes.smooth.delayMs = SMOOTH_MODE_DELAY_MS;
}


void flashColorMode(Leds &leds, FlashModeData &data)
  // Режим двойных вспышек отдельных цветов.
  // Порядок цветов красный -> зеленый -> синий -> циановый -> желтый -> фиолетовый и в обратном направлении.
  // Между вспышками также появляются затемнения.
{
  switch (data.index) {
    case 1: case 3: case 5: case 7: case 9: case 11:
    case 13: case 15: case 17: case 19: case 21:
      leds = {FLASH_MODE_LOW_LEVEL, FLASH_MODE_LOW_LEVEL, FLASH_MODE_LOW_LEVEL, leds.bright}; 
      break;
    case 0: case 2: 
      leds = {FLASH_MODE_MAX_LEVEL, FLASH_MODE_LOW_LEVEL, FLASH_MODE_LOW_LEVEL, leds.bright}; break;
    case 4: case 6: 
      leds = {FLASH_MODE_LOW_LEVEL, FLASH_MODE_MAX_LEVEL, FLASH_MODE_LOW_LEVEL, leds.bright}; break;
    case 8: case 10: 
      leds = {FLASH_MODE_LOW_LEVEL, FLASH_MODE_LOW_LEVEL, FLASH_MODE_MAX_LEVEL, leds.bright}; break;
    case 12: case 14: 
      leds = {FLASH_MODE_LOW_LEVEL, FLASH_MODE_MAX_LEVEL, FLASH_MODE_MAX_LEVEL, leds.bright}; break;
    case 16: case 18: 
      leds = {FLASH_MODE_MAX_LEVEL, FLASH_MODE_MAX_LEVEL, FLASH_MODE_LOW_LEVEL, leds.bright}; break;
    case 20: 
      leds = {FLASH_MODE_MAX_LEVEL, FLASH_MODE_LOW_LEVEL, FLASH_MODE_MAX_LEVEL, leds.bright}; break;
  }
  
  if (data.dir) {
    data.index += 1;
  } else {
    data.index -= 1;
  }
  
  if (data.index > 20 || data.index < 2) {
    data.dir = !data.dir; // Разворот направления
  }
  delay(data.delayMs);
}


void strobeColorMode(Leds &leds, StrobeModeData &data)
  // Режим стробоскопа.
{
  if (data.activeState) {
    leds = {STROBE_MODE_MAX_LEVEL, STROBE_MODE_MAX_LEVEL, STROBE_MODE_MAX_LEVEL, leds.bright};  
  } else {
    leds = {STROBE_MODE_LOW_LEVEL, STROBE_MODE_LOW_LEVEL, STROBE_MODE_LOW_LEVEL, leds.bright};
  }
  data.activeState = !data.activeState;
  delay(data.delayMs);
}


void fadeColorMode(Leds &leds, FadeModeData &data) 
  // Режим наростания и понижения яркости.
  // Переменная dir отвечает за направление наростания или понижения яркости.
{
  if (data.dir) {
    data.index += 1;
  } else {
    data.index -= 1;
  }

  leds = {data.index, data.index, data.index, leds.bright};
  
  if (data.index >= FADE_MODE_MAX_LEVEL || data.index <= FADE_MODE_LOW_LEVEL) {
    data.dir = !data.dir; // Разворот направления.
  }
  delay(data.delayMs);
}


void smoothColorMode(Leds &leds, SmoothModeData &data) 
  // Режим плавного (бесшовного) переливания цветов за счет индекса, который храниться в структуре режима.
  // Так как индекс принимает значения от 0 до 255, а цвета три, диапазон индекса поделен на три и равен 85.
{
  if (data.index <= 85) { // Наростание яркости красного канала и понижение яркости синего канала.
    leds.red = map(data.index, 0, 85, SMOOTH_MODE_LOW_LEVEL, SMOOTH_MODE_MAX_LEVEL);
    leds.green = SMOOTH_MODE_LOW_LEVEL;
    leds.blue = map(data.index, 0, 85, SMOOTH_MODE_MAX_LEVEL, SMOOTH_MODE_LOW_LEVEL);
  } else if (data.index <= 170) { // Наростание яркости зеленого канала и понижение яркости красного канала.
    leds.red = map(data.index, 86, 170, SMOOTH_MODE_MAX_LEVEL, SMOOTH_MODE_LOW_LEVEL);
    leds.green = map(data.index, 86, 170, SMOOTH_MODE_LOW_LEVEL, SMOOTH_MODE_MAX_LEVEL);
    leds.blue = SMOOTH_MODE_LOW_LEVEL;
  } else { // Наростание яркости синего канала и понижение яркости зеленого канала.
    leds.red = SMOOTH_MODE_LOW_LEVEL;
    leds.green = map(data.index, 171, 255, SMOOTH_MODE_MAX_LEVEL, SMOOTH_MODE_LOW_LEVEL);
    leds.blue = map(data.index, 171, 255, SMOOTH_MODE_LOW_LEVEL, SMOOTH_MODE_MAX_LEVEL);
  }
  data.index++;
  delay(data.delayMs);
}


void swapColorMode(Leds &leds, ColorModes &colorModes)
  // Режим переключения режимов.
{
  uint32_t deltaTime = millis() - colorModes.swap.lastSwap;
  if (deltaTime > SWAP_HOLD_TIME_MS) {
    colorModes.swap.lastSwap = millis();
    colorModes.swap.currentMode += 1;
  }
  if (SWAP_WITHOUT_STROBE && colorModes.swap.currentMode == STROBE_MODE) {
    colorModes.swap.currentMode += 1; // Пропуск режима стробоскопа.
  }
  switch (colorModes.swap.currentMode) {
    case FLASH_MODE: flashColorMode(leds, colorModes.flash); break;
    case STROBE_MODE: strobeColorMode(leds, colorModes.strobe); break; 
    case FADE_MODE: fadeColorMode(leds, colorModes.fade); break;
    case SMOOTH_MODE: smoothColorMode(leds, colorModes.smooth); break;
    case SMOOTH_MODE + 1: colorModes.swap.currentMode = FLASH_MODE;
  }
}
