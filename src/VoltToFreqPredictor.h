#ifndef __VOLT_TO_FREQ_H
#define __VOLT_TO_FREQ_H

#include "main.h"
#include "math.h"

typedef struct CalibrationSample
{
    uint16_t voltage; // represented as a 16bit num via DAC
    float frequency;  // frequency in hZ
} CalibrationSample;

class VoltToFreqPredictor
{
public:
    CalibrationSample s1;
    CalibrationSample s2;
    CalibrationSample s3;

    VoltToFreqPredictor(){};

    uint16_t predictVoltage(float frequency)
    {
        float A = calculateA();
        float B = calculateB();
        float C = calculateC();
        float plus = (-B + sqrt(pow(B, 2) - 4 * A * (C - frequency))) / (2 * A);
        float minus = (-B - sqrt(pow(B, 2) - 4 * A * (C - frequency))) / (2 * A);
        return std::max(plus, minus);
    };

    float predictFrequency(uint16_t voltage)
    {
        float A = calculateA();
        float B = calculateB();
        float C = calculateC();
        return A * pow(voltage, 2) + B * voltage + C;
    };

private:
    float calculateA()
    {
        uint16_t v1 = s1.voltage;
        float f1 = s1.frequency;
        uint16_t v2 = s2.voltage;
        float f2 = s2.frequency;
        uint16_t v3 = s3.voltage;
        float f3 = s3.frequency;

        float D1 = calculateDenominator(v1, v2, v3);
        float D2 = calculateDenominator(v2, v1, v3);
        float D3 = calculateDenominator(v3, v1, v2);
        return f1 / D1 + f2 / D2 + f3 / D3;
    };

    float calculateB()
    {
        uint16_t v1 = s1.voltage;
        float f1 = s1.frequency;
        uint16_t v2 = s2.voltage;
        float f2 = s2.frequency;
        uint16_t v3 = s3.voltage;
        float f3 = s3.frequency;

        float D1 = calculateDenominator(v1, v2, v3);
        float D2 = calculateDenominator(v2, v1, v3);
        float D3 = calculateDenominator(v3, v1, v2);
        return -(f1 * (v2 + v3) / D1) - (f2 * (v1 + v3) / D2) - (f3 * (v1 + v2) / D3);
    };

    double calculateC()
    {
        uint16_t v1 = s1.voltage;
        float f1 = s1.frequency;
        uint16_t v2 = s2.voltage;
        float f2 = s2.frequency;
        uint16_t v3 = s3.voltage;
        float f3 = s3.frequency;

        float D1 = calculateDenominator(v1, v2, v3);
        float D2 = calculateDenominator(v2, v1, v3);
        float D3 = calculateDenominator(v3, v1, v2);
        return ((f1 * v2 * v3) / D1) + ((f2 * v1 * v3) / D2) + ((f3 * v1 * v2) / D3);
    };

    float calculateDenominator(uint16_t x1, uint16_t x2, uint16_t x3)
    {
        return pow(x1, 2) - x1 * x3 - x1 * x2 + x2 * x3;
    };
};

#endif