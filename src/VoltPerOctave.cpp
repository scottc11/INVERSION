#include "VoltPerOctave.h"

void VoltPerOctave::init() {
    
    dac->init();
    setPitchBendRange(1); // default to a whole tone
}

/**
 * Set the pitch bend range to be applied to 1v/o output
 * NOTE: There are 12 notes, but only 8 possible PB range options, meaning there are preset values for each PB range option via PB_RANGE_MAP global
 * @param value number beyween 0..7
*/
void VoltPerOctave::setPitchBendRange(int value)
{
    if (value < 8)
    {
        pbRangeIndex = value;
        maxPitchBend = dacSemitone * PB_RANGE_MAP[pbRangeIndex]; // multiply semitone DAC value by the max desired number of semitones to be bent
    }
}

void VoltPerOctave::setPitchBend(uint16_t value)
{
    currPitchBend = value;
    this->updateDAC(currNoteIndex, currPitchBend);
}

/**
 * Scale an input value to a number between 0 and maxPitchBend
 * @param input ADC input
 * @param min range floor of input
 * @param max range ceiling of input
*/
uint16_t VoltPerOctave::calculatePitchBend(int input, int min, int max)
{
    return scaleIntToRange(input, min, max, minPitchBend, maxPitchBend);
}

/**
 * @param index index to be mapped to voltage map. ranging 0..DAC_1VO_ARR_SIZE
 * @param pitchBend DAC value corrosponing to the amount of pitch bend to apply to output. positive or negative
*/
void VoltPerOctave::updateDAC(int index, uint16_t pitchBend)
{
    if (index < DAC_1VO_ARR_SIZE) {
        currNoteIndex = index;
        currOutput = dacVoltageMap[currNoteIndex] + pitchBend;
        dac->write(dacChannel, currOutput);
    }
}

/**
 * @brief copy default pre-calibrated dac voltage values into class object member
*/
void VoltPerOctave::resetVoltageMap() {
    for (int i = 0; i < DAC_1VO_ARR_SIZE; i++)
    {
        dacVoltageMap[i] = DAC_VOLTAGE_VALUES[i];
    }
}