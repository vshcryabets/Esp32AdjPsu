#include "esp32_neural.h"
#include <Arduino.h>
#include <SPIFFS.h>

void save_weights(NeuralNetwork *nn)
{
    File file = SPIFFS.open("/weights.txt", FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    for (int i = 0; i < 2; i++) {
        file.printf("1,0,%d=%.6f\n", i, nn->w1[i]);
        file.printf("1,1,%d=%.6f\n", i, nn->b1[i]);
        file.printf("2,0,%d=%.6f\n", i, nn->w2[i]);
    }
    file.printf("2,1,0=%.6f\n", nn->b2);
    file.close();
}

void load_weights(NeuralNetwork *nn)
{
    File file = SPIFFS.open("/weights.txt", FILE_READ);
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