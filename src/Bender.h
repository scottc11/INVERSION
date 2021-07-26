#ifndef __BENDER_H
#define __BENDER_H

#include "main.h"
#include "DAC8554.h"
#include "ExpoFilter.h"

#define PB_CALIBRATION_RANGE 64
#define DEFAULT_MAX_BEND 40000
#define DEFAULT_MIN_BEND 25000

class Bender {
public:
    
    DAC8554 *dac;                    // pointer to Pitch Bends DAC
    DAC8554::Channels dacChan;       // which dac to address
    AnalogIn adc;                    // CV input via Instrumentation Amplifier
    ExpoFilter inputFilter;               //
    ExpoFilter outputFilter;
    Callback<void()> idleCallback;   // MBED Callback which gets called when the Bender is idle / not-active
    Callback<void(uint16_t bend)> activeCallback; // MBED Callback which gets called when the Bender is active / being bent

    int mode;
    int currBend;                                 // 16 bit value (0..65,536)
    float dacOutputRange = 32767;                 // range in which the DAC can output (in either direction)
    int dacOutput;                                // the amount of Control Voltage to apply Pitch Bend DAC
    int calibrationSamples[PB_CALIBRATION_RANGE]; // an array which gets populated during initialization phase to determine a debounce value + zeroing

    uint16_t zeroBend;                          // the average ADC value when pitch bend is idle
    uint16_t idleDebounce;                       // for debouncing the ADC when Pitch Bend is idle
    uint16_t maxBend = DEFAULT_MAX_BEND;         // the minimum value the ADC can achieve when Pitch Bend fully pulled
    uint16_t minBend = DEFAULT_MIN_BEND;         // the maximum value the ADC can achieve when Pitch Bend fully pressed

    Bender(DAC8554 *dac_ptr, DAC8554::Channels _dacChan, PinName adcPin) : adc(adcPin)
    {
        dac = dac_ptr;
        dacChan = _dacChan;
    };
    void init();
    void poll();
    uint16_t read();
    void calibrateIdle();
    void calibrateMinMax();
    void updateDAC(uint16_t value);
    bool isIdle();
    int setMode(int targetMode = 0);
    int calculateOutput(uint16_t value);
    void attachIdleCallback(Callback<void()> func);
    void attachActiveCallback(Callback<void(uint16_t bend)> func);
};

#endif