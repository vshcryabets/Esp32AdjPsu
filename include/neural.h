#ifndef NEURAL_H
#define NEURAL_H

#include <stdint.h>

struct NeuralNetwork {
    float min, max;
    // layer 1
    float w1[2];
    float b1[2];
    // layer 2
    float w2[2];
    float b2;
    uint8_t calibrateStep;
};

#ifdef __cplusplus
    class NeuralNetworkMethods: public NeuralNetwork {
    public:    
        // Sigmoid function
        float sigmoid(float x);
        float sigmoid_derivative(float y);
        void train_step(float input, float target, float learning_rate);
        float predict(float input);
        void debugOutput(float input, float target);
        void doCalibrateStep();
    };
    extern "C" {
        void neuralSaveModel(struct NeuralNetwork *nn);
        void neuralLoadModel(struct NeuralNetwork *nn);
    }
#endif

#endif