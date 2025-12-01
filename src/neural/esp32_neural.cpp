#include "neural.h"
#include <Arduino.h>
#include <LittleFS.h>

#ifdef ESP32

extern "C"
void neuralSaveModel(NeuralNetwork *nn)
{
    File file = LittleFS.open("/weights.txt", FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    file.printf("%.6f\n", nn->min);
    file.printf("%.6f\n", nn->max);
    for (int i = 0; i < 2; i++) {
        file.printf("1,0,%d=%.6f\n", i, nn->w1[i]);
        file.printf("1,1,%d=%.6f\n", i, nn->b1[i]);
        file.printf("2,0,%d=%.6f\n", i, nn->w2[i]);
    }
    file.printf("2,1,0=%.6f\n", nn->b2);
    file.close();
}

extern "C"
void neuralLoadModel(NeuralNetwork *nn)
{
    // layer 1
    nn->w1[0] = 0.1f;
    nn->w1[1] = -0.2f;
    nn->b1[0] = 0.0f;
    nn->b1[1] = 0.0f;
    // layer 2
    nn->w2[0] = 0.3f;
    nn->w2[1] = -0.4f;
    nn->b2 = 0.f;
    // calibration
    nn->min = 0.f;
    nn->max = 1.f;
    // calibration step
    nn->calibrateStep = 0;

    File file = LittleFS.open("/weights.txt", FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return;
    }
    while (file.available()) {
        char line[64];
        file.readBytesUntil('\n', line, sizeof(line));
        float value;
        uint8_t layer, type, index;
        if (sscanf(line, "%d,%d,%d=%f", &layer, &type, &index, &value) == 4) {
            if (layer == 1 && type == 0)
                nn->w1[index] = value;
            else if (layer == 1 && type == 1)
                nn->b1[index] = value;
            else if (layer == 2 && type == 0)
                nn->w2[index] = value;
            else if (layer == 2 && type == 1)
                nn->b2 = value;
            else
                Serial.printf("Unknown layer/type: %d/%d\n", layer, type);
        }
    }
    file.close();
}
#endif