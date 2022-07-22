#ifndef RED_PIN
  #error RED_PIN not defined!
#endif
#ifndef GREEN_PIN
  #error GREEN_PIN not defined!
#endif
#ifndef BLUE_PIN
  #error BLUE_PIN not defined!
#endif

#ifndef CALIBRATE_MAX_RED_LEVEL
  #define CALIBRATE_MAX_RED_LEVEL 1.0
#endif
#ifndef CALIBRATE_MAX_GREEN_LEVEL
  #define CALIBRATE_MAX_GREEN_LEVEL 1.0
#endif
#ifndef CALIBRATE_MAX_BLUE_LEVEL
  #define CALIBRATE_MAX_BLUE_LEVEL 1.0
#endif

#ifndef DEFAULT_BRIGHT
  #define DEFAULT_BRIGHT 1.0
#endif

struct Leds
// Структура для хранения значений яркостей светодиодов и общей яркости.
{
    uint8_t red; 
    uint8_t green; 
    uint8_t blue;  
    float bright;
} leds;

void initLeds (Leds &leds)
  // Инициализвация выводов каналов RGB ленты и стандартной яркости.
{ 
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  leds.bright = DEFAULT_BRIGHT;
}


void lowerBrightness(Leds &leds, float power) 
  //  Умножение значений светодиодов на дробный коэфициент.
  //  Коэффициент может принимать занчения от 0 до 1.
{
  leds.red *= power; 
  leds.green *= power; 
  leds.blue *= power; 
}


uint8_t getBrightCRT(uint8_t power, float calibrate)
  // Калибровка яркости по CRT гамме.
{
  return power > 0 ? 1 + (uint16_t)(calibrate * power * power + 255) >> 8 : 0;
}


void writeLeds(Leds leds) 
  // Вывод значений на пинах микроконтролллера с представрительной коррекцией яркости.
{
  lowerBrightness(leds, leds.bright); 
  analogWrite(RED_PIN, getBrightCRT(leds.red, CALIBRATE_MAX_RED_LEVEL));
  analogWrite(GREEN_PIN, getBrightCRT(leds.green, CALIBRATE_MAX_GREEN_LEVEL));
  analogWrite(BLUE_PIN, getBrightCRT(leds.blue, CALIBRATE_MAX_BLUE_LEVEL));
}
