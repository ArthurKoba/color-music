#ifndef FASTFFT_H
#define FASTFFT_H

#include <inttypes.h>

struct FFTConfig {
    int16_t samplesLength;
    int16_t amplitudesLength;
    uint8_t accuracy;
    uint8_t log2Length;
    uint8_t scale;
};



int16_t fastSin(int16_t value, int16_t angle, uint8_t accuracy = 5);

int16_t fastCos(int16_t value, int16_t angle, uint8_t accuracy = 5);

int16_t fastRSS(int16_t a, int16_t b);

uint8_t fastLog2(uint16_t &number);

void initConfig(FFTConfig &fftConfig, uint16_t samplesLength, uint8_t accuracy = 5);

void FastFFT(FFTConfig cfg, int16_t *samples, int16_t *real);

#endif //FASTFFT_H