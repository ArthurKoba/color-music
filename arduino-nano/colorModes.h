#define GLIDE_MODE_DELAY_MS 5

#define TRANSFUSION_MODE_DELAY_MS 13
#define TRANSFUSION_MODE_LOW_LEVEL 80
#define TRANSFUSION_MODE_MAX_LEVEL 255



void transfusionColorMode(Leds &leds, uint8_t &index, uint16_t delayTimeMs = TRANSFUSION_MODE_DELAY_MS) {

  // Режим плавного переливания цветов за счет индекса, который получается и храниться во вне.
  // За счет внешнего индекса можно сохранять состояние и делать паузы.
  // Так как индекс принимает значения от 0 до 255, а цвета три, диапазон индекса поделен на три и равен 85.
  
  if (index <= 85) {
    leds.red = map(index, 0, 85, TRANSFUSION_MODE_LOW_LEVEL, TRANSFUSION_MODE_MAX_LEVEL);
    leds.green = TRANSFUSION_MODE_LOW_LEVEL;
    leds.blue = map(index, 0, 85, TRANSFUSION_MODE_MAX_LEVEL, TRANSFUSION_MODE_LOW_LEVEL);
  } else if (index <= 170) {
    leds.red = map(index, 86, 170, TRANSFUSION_MODE_MAX_LEVEL, TRANSFUSION_MODE_LOW_LEVEL);
    leds.green = map(index, 86, 170, TRANSFUSION_MODE_LOW_LEVEL, TRANSFUSION_MODE_MAX_LEVEL);
    leds.blue = TRANSFUSION_MODE_LOW_LEVEL;
  } else {
    leds.red = TRANSFUSION_MODE_LOW_LEVEL;
    leds.green = map(index, 171, 255, TRANSFUSION_MODE_MAX_LEVEL, TRANSFUSION_MODE_LOW_LEVEL);
    leds.blue = map(index, 171, 255, TRANSFUSION_MODE_LOW_LEVEL, TRANSFUSION_MODE_MAX_LEVEL);
  }
  
  index++;
  delay(delayTimeMs);
  
}

void glideColorMode(Leds &leds, uint8_t &index, bool &dir, uint16_t delayTimeMs = GLIDE_MODE_DELAY_MS) {
  
  if (dir) index += 5;
  else index -= 5;

  leds = {index, index, index};
  Serial.println("{\"dataType\":\"testData\", \"data\":[" + String(index) + "]}");
  if (index > 254 || index < 4) {
    dir = !dir;
  }
  
  delay(delayTimeMs);
}
