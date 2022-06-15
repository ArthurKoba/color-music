#ifndef FASTFFT_UTILS_H
#define FASTFFT_UTILS_H

#include <iostream>
using namespace std;

void showConfig(FFTConfig &config) {
    cout << "Samples length: " << (int) config.samplesLength << endl;
    cout << "Amplitudes length: " << (int) config.amplitudesLength << endl;
    cout << "Accuracy: " << (int) config.accuracy << endl;
    cout << "Log2 length: " << (int) config.log2Length << endl;
}


void showArray(int16_t *array, uint16_t arrayLength) {
    cout << "[" << array[0];
    for (uint16_t i = 1; i < arrayLength;) {
        cout << ", " << array[i++];
    }
    cout << "]" << endl;

}void showArray(int *array, uint16_t arrayLength) {
    cout << "[" << array[0];
    for (uint16_t i = 1; i < arrayLength;) {
        cout << ", " << array[i++];
    }
    cout << "]" << endl;
}

void testCurrentFastSin(int16_t number, int16_t angle) {
    cout << "Входное число: "<< number   << endl;
    float rightAngle = (float)(angle * 360) / 1024;
    cout << "Значение угла: "<< angle  << " (" << rightAngle << " градусов)"  << endl;
    float real = sin((angle * 3.1415926535) / 512)*number;
    cout << "Точное значение: " << real  << endl;

    int16_t newNumber = fastSin(number, angle, 5);
    cout << "Результирующее значение: "<< newNumber << endl;
    cout << "Отклонение: " << abs(100 - (real * 100 / newNumber)) << "%"<< endl << endl;

}

void testFastSin(void) {
    uint16_t value = 500;
    testCurrentFastSin(500, 85);
    testCurrentFastSin(500, 128);
    testCurrentFastSin(500, 171);

    testCurrentFastSin(500, 43);
    testCurrentFastSin(500, 128 + 40);
    testCurrentFastSin(500, 512 + 40);
}

void testCurrentFastSSQ (int16_t a, int16_t b) {
    cout << "A: " << a << ". B: " << b << endl;
    int16_t  realRSS = sqrt(a*a + b*b);
    int16_t  RSS = fastRSS(a, b);
    cout << "Точное значение: " << realRSS << endl;
    cout << "Полученное значение: " << RSS << endl;
    cout << "Отклонение: " << abs(realRSS - RSS) << endl  << endl;

}

void testFastSSQ(void) {
    testCurrentFastSSQ(1, 2);
    testCurrentFastSSQ(10,11);
    testCurrentFastSSQ(50,60);
}

#endif //FASTFFT_UTILS_H
