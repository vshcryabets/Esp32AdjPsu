#ifndef NEURAL_H
#define NEURAL_H

class NeuralNetwork {
public:
    // layer 1
    float w1[2] = {0.1f, -0.2f};
    float b1[2] = {0.0f, 0.0f};
    // layer 2
    float w2[2] = {0.3f, -0.4f};
    float b2 {0.f};
    
public:
    // Sigmoid function
    float sigmoid(float x);
    float sigmoid_derivative(float y);
    void train_step(float input, float target, float learning_rate);
    float predict(float input);
    void debugOutput(float input, float target);
};

#endif