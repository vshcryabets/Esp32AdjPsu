#include "nerual.h"
#include "math.h"
#include <Arduino.h>

// Sigmoid function
float NeuralNetwork::sigmoid(float x) {
    return 1.0f / (1.0f + exp(-x));
}

float NeuralNetwork::sigmoid_derivative(float y) {
    return y * (1.0f - y);
}

void NeuralNetwork::train_step(float input, float target, float learning_rate) {
    float h[2];
    for (int i = 0; i < 2; i++) {
        h[i] = sigmoid(input * w1[i] + b1[i]);
    }

    float output = sigmoid(h[0] * w2[0] + h[1] * w2[1] + b2);
    float error = target - output;
    float d_output = error * sigmoid_derivative(output);

    float d_h[2];
    for (int i = 0; i < 2; i++) {
        d_h[i] = d_output * w2[i] * sigmoid_derivative(h[i]);
    }

    for (int i = 0; i < 2; i++) {
        w2[i] += learning_rate * d_output * h[i];
        w1[i] += learning_rate * d_h[i] * input;
        b1[i] += learning_rate * d_h[i];
    }
    b2 += learning_rate * d_output;
}

float NeuralNetwork::predict(float input) {
    float h[2];
    for (int i = 0; i < 2; i++) {
        h[i] = sigmoid(input * w1[i] + b1[i]);
    }
    return sigmoid(h[0] * w2[0] + h[1] * w2[1] + b2);
}

void NeuralNetwork::debugOutput(float input, float target) {
    // Debug output
    float h[2] = {
        sigmoid(input * w1[0] + b1[0]),
        sigmoid(input * w1[1] + b1[1])
    };
    float output = sigmoid(h[0] * w2[0] + h[1] * w2[1] + b2);

    Serial.printf("In: %.2f | Target: %.2f | Out: %.2f\n", input, target, output);     
}