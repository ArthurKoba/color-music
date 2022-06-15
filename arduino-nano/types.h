#ifndef LENGTH_COLOR_AMPLITUDES
  #define LENGTH_COLOR_AMPLITUDES 16
#endif

struct Leds {
    uint8_t red; 
    uint8_t green; 
    uint8_t blue;  
};

struct History {
    uint8_t minimum;
    uint8_t maximum;
    uint8_t delta;
    uint8_t average;
    uint8_t data[LENGTH_COLOR_AMPLITUDES];
};

struct HistoriesOfColorAmplitudes {
    History low;
    History middle;
    History high;
};

struct ColorModes {
   uint8_t transfusionIndex;
   uint8_t glideIndex;
   bool glideDirection;
};

struct SamplingConfigs {
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

struct FHTData {
    uint8_t *amplitudes;
    uint8_t amplitudesLength;
    uint8_t smoothingAmplitudes[LENGTH_OF_SMOOTHED_AMPLITUDES];
};

struct ColorMusicConfig {
    SamplingConfigs sampling;
    FHTData fhtData;
    ColorModes colorModes;
    HistoriesOfColorAmplitudes colorsAmplitudesHistory;
    Leds leds;
};
