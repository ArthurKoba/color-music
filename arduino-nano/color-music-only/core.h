#include <Arduino.h>

#define COLOR_MUSIC_VERSION "1.0"

#ifndef FHT_N
  #define FHT_N 256
#endif

#ifndef LOG_OUT
  #define LOG_OUT 1
#endif

#include <FHT.h>

#ifndef RED_PIN
  #error RED_PIN not defined!
#endif

#ifndef GREEN_PIN
  #error GREEN_PIN not defined!
#endif

#ifndef BLUE_PIN
  #error BLUE_PIN not defined!
#endif

#ifndef AUDIO_PIN
  #error AUDIO_PIN not defined!
#endif

#ifndef LENGTH_OF_SMOOTHED_AMPLITUDES
  #define LENGTH_OF_SMOOTHED_AMPLITUDES 32
#endif

#ifndef SILENCE_AMPLITUDE_LEVEL
  #define SILENCE_AMPLITUDE_LEVEL 15
#endif

#ifndef DELAY_BEFORE_ACTIVATION_OF_SILENT_MODE_MS
  #define DELAY_BEFORE_ACTIVATION_OF_SILENT_MODE_MS 1000
#endif

#ifndef FADE_OUT_TIME_AFTER_SILENT_MODE_MS
  #define FADE_OUT_TIME_AFTER_SILENT_MODE_MS 6000
#endif


#include "types.h"
#include "colorModes.h"
#include "serialPortInteraction.h"


void lowerBrightness(Leds &leds, float power) {

  //  Умножение значений светодиодов на дробный коэфициент.
  //  Коэффициент может принимать занчения от 0 до 1.
  
  leds.red *= power; 
  leds.green *= power; 
  leds.blue *= power; 
}

uint8_t getBrightCRT(uint8_t power, float calibrate = 1.0) {
  return power > 0 ? 1 + (uint16_t)(calibrate * power * power + 255) >> 8 : 0;
}

//uint8_t getBrightCRT(uint8_t val) {
////  return pgm_read_byte(&(CRTgamma[val]));
//  return CRTgamma[val];
//}

void writeValuesToLeds(Leds &leds) {
  
  #ifdef SEND_LEDS
    //  Отправка значений светодиодов в последовательный порт (Формат JSON).
    sendLeds(leds);
  #endif

  
  #ifdef CALIBRATE_MAX_RED_LEVEL
//    leds.red = constrain(leds.red, 0, CALIBRATE_MAX_RED_LEVEL);
    leds.red *= CALIBRATE_MAX_RED_LEVEL;
//    leds.red = map(leds.red, 0, 255, 0, CALIBRATE_MAX_RED_LEVEL);
  #endif
  analogWrite(RED_PIN, getBrightCRT(leds.red));
  
  #ifdef CALIBRATE_MAX_GREEN_LEVEL
//    leds.green = constrain(leds.green, 0, CALIBRATE_MAX_GREEN_LEVEL);
    leds.green *= CALIBRATE_MAX_GREEN_LEVEL;
//    leds.green = map(leds.green, 0, 255, 0, CALIBRATE_MAX_GREEN_LEVEL);
  #endif
  analogWrite(GREEN_PIN, getBrightCRT(leds.green));
  
  #ifdef CALIBRATE_MAX_BLUE_LEVEL
//    leds.blue = constrain(leds.blue, 0, CALIBRATE_MAX_BLUE_LEVEL);
    leds.blue *= CALIBRATE_MAX_BLUE_LEVEL;
//    leds.blue = map(leds.blue, 0, 255, 0, CALIBRATE_MAX_BLUE_LEVEL);
  #endif
  analogWrite(BLUE_PIN, getBrightCRT(leds.blue));

}


void putAmplitudeToHistory(uint8_t value, History &history) {
    
    history.minimum = value;
    history.maximum = value;
    uint16_t average = value;
    for (uint8_t i = LENGTH_COLOR_AMPLITUDES - 1; i > 0; i--) {
        history.data[i] = history.data[i-1];
        average += history.data[i];
        if (history.minimum > history.data[i]) history.minimum = history.data[i];
        if (history.maximum < history.data[i]) history.maximum = history.data[i];
    }
    history.average = average / LENGTH_COLOR_AMPLITUDES;
    history.delta = history.maximum - history.minimum;
    history.data[0] = value;
}


void initColorAmplitudesHistory(HistoriesOfColorAmplitudes &colorsHistory) {
  
    for (uint8_t i = 0; i < LENGTH_COLOR_AMPLITUDES - 1; i++) {
        colorsHistory.low.data[i] = 0;
        colorsHistory.middle.data[i] = 0;
        colorsHistory.high.data[i] = 0;
    }
    putAmplitudeToHistory(0, colorsHistory.low);
    putAmplitudeToHistory(0, colorsHistory.middle);
    putAmplitudeToHistory(0, colorsHistory.high);
}

void setupConfigs(ColorMusicConfig &cfg) {
  
  cfg.sampling.audioPin = AUDIO_PIN;
  cfg.sampling.offsetReadingSamples = 516;
  cfg.sampling.timeOfLastSignalMs = 0;
  cfg.sampling.delayReadingSamplesUs = 30;
  cfg.sampling.samplesLength = FHT_N;
  cfg.sampling.samples = fht_input;
  cfg.fhtData.amplitudesLength = FHT_N >> 1;
  cfg.fhtData.amplitudes = fht_log_out;
  initColorAmplitudesHistory(cfg.colorsAmplitudesHistory);
  
}

void limiterSum(uint8_t &value, int16_t addValue) {
  
    addValue += value;
    value = constrain(addValue, 0, 255);
    
}

#define LOW_START_AMPLITUDE_INDEX 1 
#define LOW_END_AMPLITUDE_INDEX 1
#define LOW_MULTIPLICATION_AMPLITUDE 2

#define MIDDLE_START_AMPLITUDE_INDEX 8
#define MIDDLE_END_AMPLITUDE_INDEX 32
#define MIDDLE_MULTIPLICATION_AMPLITUDE 0.1

#define HIGH_START_AMPLITUDE_INDEX 119 
#define HIGH_END_AMPLITUDE_INDEX 127
#define HIGH_MULTIPLICATION_AMPLITUDE 0.7


void calculateAndWriteColorAmplitudesToHistory(FHTData &fhtData, HistoriesOfColorAmplitudes &history) {
  
  //  Расчет амплитуд цветов по амплитудно-частотным рядам.
  //  Низким частотам соответсвует красный цвет, средним - зеленый и высоким - синий.

  uint16_t amplitudeValue;
  
  amplitudeValue = 0;  
  for (uint8_t i = LOW_START_AMPLITUDE_INDEX; i <= LOW_END_AMPLITUDE_INDEX; i++) {
    if (fhtData.amplitudes[i] > amplitudeValue) {
      amplitudeValue = fhtData.amplitudes[i];
    }
  }
  putAmplitudeToHistory(amplitudeValue * LOW_MULTIPLICATION_AMPLITUDE, history.low);  

  #ifdef SEND_LOW_HISTORY
    sendLowHistory(history.low);
  #endif

//  amplitudeValue = 0;
//  for (uint8_t i = MIDDLE_START_AMPLITUDE_INDEX; i <= MIDDLE_END_AMPLITUDE_INDEX; i++) {
//    if (fhtData.amplitudes[i] > amplitudeValue) {
//      amplitudeValue = fhtData.amplitudes[i];
//    } 
//  }
  amplitudeValue = 0;
  uint16_t maxValue = 0;
  for (uint8_t i = MIDDLE_START_AMPLITUDE_INDEX; i <= MIDDLE_END_AMPLITUDE_INDEX; i++) {
    amplitudeValue += fhtData.amplitudes[i];
    if (fhtData.amplitudes[i] > maxValue) {
      maxValue = fhtData.amplitudes[i];
    }
  }
  amplitudeValue = amplitudeValue + maxValue;
//  amplitudeValue >>= 3;
  
//  amplitudeValue = amplitudeValue * MIDDLE_MULTIPLICATION_AMPLITUDE;
  putAmplitudeToHistory(constrain(amplitudeValue * MIDDLE_MULTIPLICATION_AMPLITUDE, 0, 255), history.middle);  

  #ifdef SEND_MIDDLE_HISTORY
    sendMiddleHistory(history.middle);
  #endif
  

  amplitudeValue = 0;
  for (uint8_t i = HIGH_START_AMPLITUDE_INDEX; i <= HIGH_END_AMPLITUDE_INDEX; i++) {
      amplitudeValue += fhtData.amplitudes[i];
  }
  
  amplitudeValue *= HIGH_MULTIPLICATION_AMPLITUDE;
  amplitudeValue = (amplitudeValue + history.high.data[0]) >> 1;
  putAmplitudeToHistory(constrain(amplitudeValue, 0, 255), history.high); 

  #ifdef SEND_HIGH_HISTORY
    sendHighHistory(history.high);
  #endif
}


void readSamples(SamplingConfigs &cfg) {

  //  Функция для считывания аналоговых значений значений.
  //  Важнымы моментами при семплировании являются: задержка между измерениями (мкс) и смещение значения семпла.
  //  - Задержка необходима для более точной настройки частоты дискретизации. 
  //    При изменении таймера микроконтроллера можно получать значения дискретизации АЦП кратные 2^14.
  //    В этом случае можно повысить частоту дискретизации выше необходимой и с помощью регилировки коэффициента задержки подогнать до необходимой.
  //  - Смещение необходимо для того, чтобы положительная полуволна находилась выше нуля, отрицательная соответственно - ниже.
  //    Если сигнал не сместить, то при преобразовании к амплитудно-частотным рядам в первой амплитуде будет присутствовать амплитуда смещения.
  //    Смещение автоматически вычисляется с помощью функции calibrateOffsetOfSamples при запуске микроконтроллера.
  //  В процеесе выполнения семплировия дополнительно находятся максимальное и минимальное значения.
  //  С помощью найденых значений осуществляется расчет амплитуды и мидианы сигнала.
  //  Также, функция при полном считывании массива семплов вычисляет частоту дискретизации.
  
  cfg.samplingFrequrency = micros(); // Засекается начальное время считывания.
  
  cfg.samples[0] = analogRead(cfg.audioPin) - cfg.offsetReadingSamples; // Получения значения семпла с вычитом смещения.
  int16_t minValue = cfg.samples[0];
  int16_t maxValue = cfg.samples[0];

  for (uint16_t i = 1; i < cfg.samplesLength; i++) {
    if (cfg.delayReadingSamplesUs > 0) delayMicroseconds(cfg.delayReadingSamplesUs);
    cfg.samples[i] = analogRead(cfg.audioPin) - cfg.offsetReadingSamples;
    if (cfg.samples[i] > maxValue) maxValue = cfg.samples[i];
    if (cfg.samples[i] < minValue) minValue = cfg.samples[i];
  }

  cfg.samplingFrequrency = micros() - cfg.samplingFrequrency; // Время полного семплирования (мкс).
  cfg.samplingFrequrency = cfg.samplingFrequrency / cfg.samplesLength; // Время затраченное на считывания одного семпла (мкс). 
  cfg.samplingFrequrency = 1000000 / cfg.samplingFrequrency; // Вычисление частоты дискретизации (1000000 мкс (1 секунда) / время единичного считывания (мкс).
  
  cfg.samplesAmplitude = (maxValue - minValue) >> 1; // Вычисление амплитуды сигнала.
  cfg.medianOfSamplesAmplitude = maxValue - cfg.samplesAmplitude; // Вычисление медианы сигнала.
  
  if (cfg.samplesAmplitude > SILENCE_AMPLITUDE_LEVEL) {
      cfg.timeOfLastSignalMs = millis();
  }

  #ifdef SEND_SAMPLES
    sendSamples(cfg);
  #endif
}


void calibrateOffsetOfSamples(ColorMusicConfig &cfg) {
  
  //  Калибровка смещения сигнала для дальнейшего преобразовании Хартли.
  //  Если медиана сигнала установлена неверно и не равна нулю, то в первой полученной амплитуде после преобразования будет 
  //  находиться значение смещения сигнала, которое также будет вносить в несколько последующих искажения.
   
  while (true) {
    readSamples(cfg.sampling);
    
    //  Если амплитуда сигнала болльше амплитуды уровня тишины, то включается ускоренный режим цветового переливания.
    //  Данный режим сигнализирует наличие звукогового сигнала на пине микроконтроллера, 
    //  который мешает калибровке. При отключении сигнала происходит калибровка средней точки и выход из функции.
        
    if (cfg.sampling.samplesAmplitude > SILENCE_AMPLITUDE_LEVEL) {
      transfusionColorMode(cfg.leds, cfg.colorModes.transfusionIndex, 0);
      cfg.colorModes.transfusionIndex += 20;
      writeValuesToLeds(cfg.leds);
    } else if (abs(cfg.sampling.medianOfSamplesAmplitude) > 20) {
      cfg.sampling.offsetReadingSamples += cfg.sampling.medianOfSamplesAmplitude >> 1;
    } else if (cfg.sampling.medianOfSamplesAmplitude > 0) {
      cfg.sampling.offsetReadingSamples += 1;
    } else if (cfg.sampling.medianOfSamplesAmplitude < 0) {
      cfg.sampling.offsetReadingSamples -= 1;
    } else {
      break; // Если средняя точка окажется равна нулю - происходит остановка цикла.
    }
  }
  
  cfg.leds = {0, 0, 0}; // Сброс значений светодиодов после режима режима переливания цветов.
  cfg.sampling.timeOfLastSignalMs = 0; // Сброс времени последнего сигнала, если он присутствовал.
}


void fhtProcess(FHTData &fhtData) {
  
  fht_window();  // Вызов оконной функции перед преобразованием
  fht_reorder(); // Переупорядочивание данных перед преобразованием Хартли
  fht_run();     // Процедура преобразования данных
  fht_mag_log(); // Приведеление данных магнитуд в логарифмическом формате

  //  Усреднение амплитуд текущего преобразования с предыдущими. 
  //  Усреднение происходит с нулевого элемента по элемент объявленный в "LENGTH_OF_SMOOTHED_AMPLITUDES".
  for (uint8_t i = 0; i < LENGTH_OF_SMOOTHED_AMPLITUDES; i++ ) {
    fhtData.amplitudes[i] = (fhtData.amplitudes[i] + fhtData.smoothingAmplitudes[i]) >> 1; 
    fhtData.smoothingAmplitudes[i] = fhtData.amplitudes[i];
  }
  
  #ifdef SEND_AMPLITUDES
    sendAmplitudes(fhtData);
  #endif
}


void setupTimers(void) {
  //  Повышение частоты дискретизации АЦП.
  //_SFR_BYTE(ADCSRA) &= ~_BV(ADPS2); // 66 kHz
  _SFR_BYTE(ADCSRA) &= ~_BV(ADPS1); // 33 kHz
  //_SFR_BYTE(ADCSRA) &= ~_BV(ADPS0); // 17 kHz

  //  Повышение частоты ШИМ цифровых выводов управляющих светодиодами.
  //  Если этого не сделать, микроконтроллер при генерации ШИМ сигнала будет передавать наводки на аудио тракт.
  //Пины D9 и D10 - 62.5 кГц
  TCCR1A = 0b00000001;  // 8bit
  TCCR1B = 0b00001001;  // x1 fast pwm
  //  // Пины D3 и D11 - 62.5 кГц
  TCCR2B = 0b00000001;  // x1
  TCCR2A = 0b00000011;  // fast pwm
}
