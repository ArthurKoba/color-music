#include "FastFFT.h"

#include <iostream>
using namespace std;

uint8_t ARC_SIN_MAP[128] = {0, 1, 3, 4, 5, 6, 8, 9, 10, 11, 13, 14, 15, 17, 18, 19, 20, 22, 23, 24, 26, 27, 28, 29, 31, 32, 33, 35, 36, 37, 39, 40, 41, 42, 44, 45, 46, 48, 49, 50, 52, 53, 54, 56, 57, 59, 60, 61, 63, 64, 65, 67, 68, 70, 71, 72, 74, 75, 77, 78, 80, 81, 82, 84, 85, 87, 88, 90, 91, 93, 94, 96, 97, 99, 100, 102, 104, 105, 107, 108, 110, 112, 113, 115, 117, 118, 120, 122, 124, 125, 127, 129, 131, 133, 134, 136, 138, 140, 142, 144, 146, 148, 150, 152, 155, 157, 159, 161, 164, 166, 169, 171, 174, 176, 179, 182, 185, 188, 191, 195, 198, 202, 206, 210, 215, 221, 227, 236};
uint16_t POW2[14] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
uint8_t FAST_RSS_MAP[20] = {7, 6, 6, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2};

int16_t fastSin(int16_t number, int16_t angle, uint8_t accuracy) {

    // Установите значение точности от 1 до 7. При точности 7 измерения наилучшие, но поиск медленный.
    // Точность 5 рекомендуется для типичного применения.
    // Угол равный 1024 соответствует 2*pi или 360 градусам.

    while (angle > 1024) {
        angle -= 1024;
    }

    while (angle < 0) {
        angle += 1024;
    }

    uint8_t quad = angle >> 8;

    switch (quad) {
        case 1:
            angle = 512 - angle;
            break;
        case 2:
            angle -= 512;
            break;
        case 3:
            angle = 1024 - angle;
    }

    int16_t middlePoint = number >> 1;
    number = middlePoint;

    uint8_t start = 0, end = 128;

    uint8_t mapIndex;
    for (uint8_t i = 0; i < accuracy; i++) {
        mapIndex = (start + end) >> 1;
        middlePoint >>= 1;
        if (angle > ARC_SIN_MAP[mapIndex]) {
            start = mapIndex;
            number += middlePoint;
        } else if (angle < ARC_SIN_MAP[mapIndex]) {
            end = mapIndex;
            number -= middlePoint;
        }
    }

    if (quad == 2 || quad == 3) {
        number = -number;
    }
    return number;
}

int16_t fastCos(int16_t number, int16_t angle, uint8_t accuracy) {

    // Функция декоратор для fastSin
    // Расчет косинуса идентичен расчету синуса, отличается лишь период.

    return fastSin(number, 256 - angle, accuracy);
}

int16_t fastRSS(int16_t a, int16_t b) {

    // Расчет примерной остаточной суммы квадратов.

    if (a == 0 && b == 0) {
        return 0;
    }
    uint8_t mapIndex = 0;
    uint16_t min, max, tempMin, tempMax;
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    if (a > b) {
        max = a;
        min = b;
    } else {
        max = b;
        min = a;
    }

    if (max > 3 * min) {
        return max;
    } else {
        tempMin = min >> 3;
        if (tempMin == 0) {
            tempMin = 1;
        }
        tempMax = min;
        while (tempMax < max) {
            tempMax += tempMin;
            mapIndex++;
        }
        tempMax = FAST_RSS_MAP[mapIndex];
        tempMin >>= 1;
        for (int i = 0; i < tempMax; i++) {
            max += tempMin;
        }
        return max;
    }
}

uint8_t fastLog2(uint16_t &number) {

    // Поиск логарифма по основания два числа number
    // Для поиска используется карта предрасчитанных значений.

    uint8_t index = 0;
    while (index < 12) {
        if (POW2[index] <= number) {
            index++;
        } else {
            break;
        }
    }
    return index - 1;
}

void initConfig(FFTConfig &cfg, uint16_t samplesLength, uint8_t accuracy) {
    cfg.samplesLength = samplesLength;
    cfg.amplitudesLength = samplesLength >> 1;
    cfg.accuracy = accuracy;
    cfg.log2Length = fastLog2(samplesLength);
    cfg.scale = 0;
}

void scaleSamples(FFTConfig cfg, int16_t *samples) {

    // Масштабирование семлов к диапазону от -512 до 512
    // При низком уровне громкости сигнала, расчет будет происходить более точнее при масштабировании.
    // Вызов масштабирвоания не обязателен если семплы уже в данном диапазоне.

    int16_t maxSample = 0, minSample = 0;
    uint32_t avgSamples = 0;

    // Поиск наибольшего, наименьшего, суммарного значения для масштабирования.
    for (int16_t i = 0; i < cfg.samplesLength; i++) {
        avgSamples += samples[i];
        if (samples[i] > maxSample) {
            maxSample = samples[i];
        }
        if (samples[i] < minSample) {
            minSample = samples[i];
        }
    }
    // Вычисление среднеарифмитического значения сигнала с помощью побитового деления на количество семплов.
    avgSamples >>= cfg.log2Length;
    int16_t magSamples = maxSample - minSample; //Вычисление магнитуды (медианы) массива сигналов.
    int16_t tempMagSamples = magSamples;

    //Масштабирование данных от +512 до -512
    if (magSamples > 1024) {
        while (tempMagSamples > 1024) {
            tempMagSamples >>= 1;
            cfg.scale++;
        }
    } else if (magSamples != 0) {
        while (tempMagSamples < 1024) {
            tempMagSamples <<= 1;
            cfg.scale++;
        }
    }

    if (magSamples > 1024) {
        for (int16_t i = 0; i < cfg.samplesLength; i++){
            samples[i] -= avgSamples;
            samples[i] >>= cfg.scale;
        }
    } else {
        cfg.scale --;
        for (int16_t i = 0; i < cfg.samplesLength; i++) {
            samples[i] -= avgSamples;
            samples[i] <<= cfg.scale;
        }
    }

}

void FastFFT(FFTConfig cfg, int16_t *samples, int16_t *real) {
    int16_t imag[cfg.samplesLength]; //Массив мнимой части преобразования.
//    int16_t real[cfg.samplesLength]; //Массив действительной части преобразования.

    // Предварительное масштабирование всех семплов в пределы от -512 до 512.
    // Необязательно если данные уже в необходимом формате.
    scaleSamples(cfg, samples);

    // Заполнение мнимой части преобразования нулями.
    for (int16_t i = 0; i < cfg.samplesLength; i++) {
        imag[i] = 0;
    }

    // Изменение порядка реверсирования битов для правильного расположения пар перемножений.
    // Массив мнимой части преобразования используется временно как буфер для экомомии оперативной памяти.
    int16_t x = 1;
    real[0] = samples[0];
    for (int16_t b = 0; b < cfg.log2Length; b++) {
        for (int16_t j = 0; j < POW2[b]; j++) {
            imag[x] = imag[j] + POW2[cfg.log2Length - b - 1];
            real[x] = samples[imag[x]];
            x++;
        }
    }

    // Повторное заполнение мнимой части преобразования нулями.
    for (int16_t i = 0; i < cfg.samplesLength; i++) {
        imag[i] = 0;
    }

    // Объявление переменных для быстрого преобразования Фурье.
    int16_t tr, ti, angle;
    uint16_t rotateCount, firstIndexPair, secondIndexPair;
    bool check = false;

    //Расчет быстрого преобразования Фурье
    for (int8_t i = 0; i < cfg.log2Length; i++) {
        rotateCount = POW2[cfg.log2Length - i - 1]; // Количество поворотов синуса/косинуса
        for (int16_t j = 0; j < POW2[i]; j++) {
            angle = - POW2[cfg.log2Length - i + 1] * j; // Угол поворота
            while (angle < 0) {
                angle += 1024;
            }
            while (angle > 1024){
                angle -= 1024;
            }
            firstIndexPair = j; // Индекс первого элемента поворотной пары
            for (int16_t k = 0; k < rotateCount ; k++) {
                secondIndexPair = POW2[i] + firstIndexPair; // Индекс второго элемента поворотной пары
                switch (angle) {
                    case 0:
                    case 1024:
                        tr = real[secondIndexPair];
                        ti = imag[secondIndexPair];
                        break;
                    case 256:
                        tr = -imag[secondIndexPair];
                        ti = real[secondIndexPair];
                        break;
                    case 512:
                        tr = -real[secondIndexPair];
                        ti = -imag[secondIndexPair];
                        break;
                    case 768:
                        tr = imag[secondIndexPair];
                        ti = -real[secondIndexPair];
                        break;
                    default:
                        //Функция быстрого синуса и косинуса дает приблизительный вывод для A*sinx
                        tr = fastCos(real[secondIndexPair], angle, cfg.accuracy) - fastSin(imag[secondIndexPair], angle, cfg.accuracy);
                        ti = fastSin(real[secondIndexPair], angle, cfg.accuracy) + fastCos(imag[secondIndexPair], angle, cfg.accuracy);
                }
                real[secondIndexPair] = real[firstIndexPair] - tr;
                real[firstIndexPair] += tr;
                imag[secondIndexPair] = imag[firstIndexPair] - ti;
                imag[firstIndexPair] += ti;

                //Проверка значения int, оно может быть в диапазоне только от +31000 до -31000,
                if (abs(real[secondIndexPair]) > 15000 || abs(imag[secondIndexPair]) > 15000) {
                    check = true;
                }
                firstIndexPair += POW2[i + 1];
            }
        }

        // Если значение числа превысило 15000 производится процедура обратного масштабирования.
        if (check) {
            for (int16_t i = 0; i < cfg.samplesLength; i++) {
                real[i] >>= 1;
                imag[i] >>= 1;
            }
            check = false;
            cfg.scale--;
        }
    }

    // Процедура обратного масштабирования.
    if (cfg.scale > 128) {
        cfg.scale -= 128;
        for (uint16_t i = 0; i < cfg.samplesLength; i++) {
            real[i] >>= cfg.scale;
            imag[i] >>= cfg.scale;
        }
    }

    // Получение амплитуд из действительной и мнимой части.
    // Приблизительная функция RSS, позволяет найти остаточную сумму квадратов.
    for (int16_t i = 1; i < cfg.amplitudesLength; i++) {
        real[i] = fastRSS(real[i], imag[i]);
    }
}