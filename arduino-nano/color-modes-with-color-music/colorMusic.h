#ifndef FHT_N
  #define FHT_N 256
#endif

#ifndef LOG_OUT
  #define LOG_OUT 1
#endif

#include <FHT.h>

#ifndef AUDIO_PIN_ANALOG
  #error AUDIO_PIN_ANALOG not defined!
#endif


#ifndef LENGTH_OF_SMOOTHED_AMPLITUDES
  #define LENGTH_OF_SMOOTHED_AMPLITUDES 32
#endif

#ifndef LENGTH_COLOR_AMPLITUDES
  #define LENGTH_COLOR_AMPLITUDES 16
#endif


#ifndef SILENCE_AMPLITUDE_LEVEL
  #define SILENCE_AMPLITUDE_LEVEL 15
#endif

#ifndef DELAY_BEFORE_ACTIVATION_OF_SILENT_MODE_MS
  #define DELAY_BEFORE_ACTIVATION_OF_SILENT_MODE_MS 1000
#endif


#ifndef LOW_START_AMPLITUDE_INDEX
  #define LOW_START_AMPLITUDE_INDEX 1
#endif
#ifndef LOW_END_AMPLITUDE_INDEX
  #define LOW_END_AMPLITUDE_INDEX 2
#endif
#ifndef LOW_MULTIPLICATION_AMPLITUDE
  #define LOW_MULTIPLICATION_AMPLITUDE 2.5
#endif

#ifndef MIDDLE_START_AMPLITUDE_INDEX
  #define MIDDLE_START_AMPLITUDE_INDEX 8
#endif
#ifndef MIDDLE_END_AMPLITUDE_INDEX
  #define MIDDLE_END_AMPLITUDE_INDEX 32
#endif
#ifndef MIDDLE_MULTIPLICATION_AMPLITUDE
  #define MIDDLE_MULTIPLICATION_AMPLITUDE 1.2
#endif

#ifndef HIGH_START_AMPLITUDE_INDEX
  #define HIGH_START_AMPLITUDE_INDEX 61
#endif
#ifndef HIGH_END_AMPLITUDE_INDEX
  #define HIGH_END_AMPLITUDE_INDEX 93
#endif
#ifndef HIGH_MULTIPLICATION_AMPLITUDE
  #define HIGH_MULTIPLICATION_AMPLITUDE 1.5
#endif


struct History
{
    uint8_t minimum;
    uint8_t maximum;
    uint8_t delta;
    uint8_t average;
    uint8_t data[LENGTH_COLOR_AMPLITUDES];
};

struct HistoriesOfColorAmplitudes
{
    History low;
    History middle;
    History high;
};

struct SamplingConfigs
{
    uint8_t audioPin;
    int16_t delayReadingSamplesUs;
    int16_t offsetReadingSamples;
    int16_t *samples;
    uint16_t samplesLength;
    uint32_t timeOfLastSignalMs;
    uint16_t samplingFrequrency;
    int16_t medianOfSamplesAmplitude;
    int16_t samplesAmplitude;
};

struct FHTData
{
    uint8_t *amplitudes;
    uint8_t amplitudesLength;
    uint8_t smoothingAmplitudes[LENGTH_OF_SMOOTHED_AMPLITUDES];
};

struct ColorMusicConfig
{
    bool needCalibrate;
    SamplingConfigs sampling;
    FHTData fhtData;
    HistoriesOfColorAmplitudes colorsAmplitudesHistory;
} colorMusicConfig;


void putAmplitudeToHistory(uint8_t value, History &history)
{    
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


void initColorAmplitudesHistory(HistoriesOfColorAmplitudes &colorsHistory)
{
    for (uint8_t i = 0; i < LENGTH_COLOR_AMPLITUDES - 1; i++) {
        colorsHistory.low.data[i] = 0;
        colorsHistory.middle.data[i] = 0;
        colorsHistory.high.data[i] = 0;
    }
    putAmplitudeToHistory(0, colorsHistory.low);
    putAmplitudeToHistory(0, colorsHistory.middle);
    putAmplitudeToHistory(0, colorsHistory.high);
}

void setupTimers(void)
  //  Повышение частоты дискретизации АЦП.
  //  Повышение частоты ШИМ цифровых выводов управляющих светодиодами.
  //  Если этого не сделать, микроконтроллер при генерации ШИМ сигнала будет передавать наводки на аудио тракт.
{
  //_SFR_BYTE(ADCSRA) &= ~_BV(ADPS2); // АЦП 66 kHz
  _SFR_BYTE(ADCSRA) &= ~_BV(ADPS1); // АЦП 33 kHz
  //_SFR_BYTE(ADCSRA) &= ~_BV(ADPS0); // АЦП 17 kHz
  TCCR1A = 0b00000001;  // 8bit //Пины D9 и D10 - 62.5 кГц
  TCCR1B = 0b00001001;  // x1 fast pwm
  TCCR2B = 0b00000001;  // x1 // Пины D3 и D11 - 62.5 кГц
  TCCR2A = 0b00000011;  // fast pwm
}


void initColorMusic(ColorMusicConfig &cfg)
{
  cfg.needCalibrate = true;
  cfg.sampling.audioPin = AUDIO_PIN_ANALOG;
  cfg.sampling.offsetReadingSamples = 516;
  cfg.sampling.timeOfLastSignalMs = 0;
  cfg.sampling.delayReadingSamplesUs = 30;
  cfg.sampling.samplesLength = FHT_N;
  cfg.sampling.samples = fht_input;
  cfg.fhtData.amplitudesLength = FHT_N >> 1;
  cfg.fhtData.amplitudes = fht_log_out;
  initColorAmplitudesHistory(cfg.colorsAmplitudesHistory);
  setupTimers();
}

void limiterSum(uint8_t &value, int16_t addValue)
{
    addValue += value;
    value = constrain(addValue, 0, 255);
}


void calculateAndWriteColorAmplitudesToHistory(FHTData &fhtData, HistoriesOfColorAmplitudes &history)
  //  Расчет амплитуд цветов по амплитудно-частотным рядам.
  //  Низким частотам соответсвует красный цвет, средним - зеленый и высоким - синий.
{
  uint16_t amplitudeValue;
  
  amplitudeValue = 0;  
  for (uint8_t i = LOW_START_AMPLITUDE_INDEX; i <= LOW_END_AMPLITUDE_INDEX; i++) {
    if (fhtData.amplitudes[i] > amplitudeValue) {
      if (fhtData.amplitudes[i] < 25) continue;
      amplitudeValue = fhtData.amplitudes[i] - 25;
    }
    /// ПОСМОТРЕТЬ
  }
  amplitudeValue *= LOW_MULTIPLICATION_AMPLITUDE;
  amplitudeValue = (amplitudeValue + history.low.data[0]) >> 1;
  amplitudeValue = constrain(amplitudeValue, 0, 255);
  putAmplitudeToHistory(amplitudeValue, history.low);  

  amplitudeValue = 0;
  uint16_t maxValue = 0;
  for (uint8_t i = MIDDLE_START_AMPLITUDE_INDEX; i <= MIDDLE_END_AMPLITUDE_INDEX; i++) {
    if (fhtData.amplitudes[i] < 25) continue;
    if (fhtData.amplitudes[i] > maxValue) maxValue = fhtData.amplitudes[i];
    amplitudeValue += fhtData.amplitudes[i] - 25;
  }
  amplitudeValue *= MIDDLE_MULTIPLICATION_AMPLITUDE;
  amplitudeValue = constrain(amplitudeValue, 0, 255);
  putAmplitudeToHistory(amplitudeValue, history.middle);  


  amplitudeValue = 0;
  for (uint8_t i = HIGH_START_AMPLITUDE_INDEX; i <= HIGH_END_AMPLITUDE_INDEX; i++) {
      if (fhtData.amplitudes[i] < 20) continue;
      amplitudeValue += fhtData.amplitudes[i];
  }
  
  amplitudeValue *= HIGH_MULTIPLICATION_AMPLITUDE;
  amplitudeValue = constrain(amplitudeValue, 0, 255);
  amplitudeValue = (amplitudeValue + history.high.data[0]) >> 1;
  putAmplitudeToHistory(amplitudeValue, history.high); 
}


void readSamples(SamplingConfigs &cfg)
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
{
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
}


void calibrateOffsetOfSamples(ColorMusicConfig &cfg, Leds &leds)
  //  Калибровка смещения сигнала для дальнейшего преобразовании Хартли.
  //  Если медиана сигнала установлена неверно и не равна нулю, то в первой полученной амплитуде после преобразования будет 
  //  находиться значение смещения сигнала, которое также будет вносить в несколько последующих искажения.
{
  SmoothModeData smoothData = {0, 1};
  while (true) {
    readSamples(cfg.sampling);
    
    //  Если амплитуда сигнала болльше амплитуды уровня тишины, то включается ускоренный режим цветового переливания.
    //  Данный режим сигнализирует наличие звукогового сигнала на пине микроконтроллера, 
    //  который мешает калибровке. При отключении сигнала происходит калибровка средней точки и выход из функции.
        
    if (cfg.sampling.samplesAmplitude > SILENCE_AMPLITUDE_LEVEL) {
      smoothColorMode(leds, smoothData);
      smoothData.index += 15;
      writeLeds(leds);
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
  leds = {0,0,0, leds.bright}; writeLeds(leds);
  cfg.sampling.timeOfLastSignalMs = 0; // Сброс времени последнего сигнала, если он присутствовал.
  cfg.needCalibrate = false;
}


void fhtProcess(FHTData &fhtData)
{
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
}
