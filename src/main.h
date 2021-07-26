#ifndef __MAIN_H
#define __MAIN_H

#include <mbed.h>
// #include <arm_math.h> // ARM DSP functions

#define PPQN                 96

#define MIDI_BAUD            31250
#define MIDI_TX              PA_2 // not up to date
#define MIDI_RX              PA_3 // not up to date

#define I2C3_SDA             PC_9
#define I2C3_SCL             PA_8
#define I2C1_SDA             PB_9
#define I2C1_SCL             PB_8

#define SPI2_MOSI            PB_15
#define SPI2_MISO            PB_14
#define SPI2_SCK             PB_13

#define TEMPO_LED            PA_1
#define TEMPO_POT            PA_2
#define EXT_CLOCK_INPUT      PA_3
#define INT_CLOCK_OUTPUT     PB_2

#define ADC_A                PA_6
#define ADC_B                PA_7
#define ADC_C                PC_5
#define ADC_D                PC_4

#define PB_ADC_A             PA_4
#define PB_ADC_B             PA_5
#define PB_ADC_C             PB_0
#define PB_ADC_D             PB_1

#define GLOBAL_GATE_OUT      PB_10

#define GATE_OUT_A           PC_2
#define GATE_OUT_B           PC_3
#define GATE_OUT_C           PC_7
#define GATE_OUT_D           PC_6

#define TOUCH_INT_A          PC_1
#define TOUCH_INT_B          PC_0
#define TOUCH_INT_C          PC_15
#define TOUCH_INT_D          PC_14

#define DEGREES_INT          PB_4

#define CTRL_INT             PB_5

#define REC_LED              PC_13
#define FREEZE_LED           PB_7

#define DAC1_CS              PB_12
#define DAC2_CS              PC_8

#define TCA9548A_ADDR            0x70 // 1110000
#define CAP1208_ADDR             0x50 // 0010100  via mux
#define CAP1208_CTRL_ADDR        0x50 // 0010100

#define MCP23008_IO_ADDR         0x22
#define MCP23017_DEGREES_ADDR    0x20 // 0100000
#define MCP23017_CTRL_ADDR       0x24 // 0100100

#define IO_INT_PIN_A             PA_12
#define IO_INT_PIN_B             PA_11
#define IO_INT_PIN_C             PA_10
#define IO_INT_PIN_D             PA_9

#define SX1509_CHAN_A_ADDR       0x3E
#define SX1509_CHAN_B_ADDR       0x70
#define SX1509_CHAN_C_ADDR       0x3F
#define SX1509_CHAN_D_ADDR       0x71


// DEFAULT INITIALIZATION VALUES
#define DEGREE_COUNT                   8
#define OCTAVE_COUNT                   4
#define EVENT_END_BUFFER               4
#define CV_QUANT_BUFFER                1000
#define SLEW_CV_BUFFER                 1000

#define DEFAULT_SEQ_LENGTH     8
#define MAX_SEQ_LENGTH         32

#define DEFAULT_VOLTAGE_ADJMNT      200
#define MAX_CALIB_ATTEMPTS          20
#define MAX_FREQ_SAMPLES            25    // how many frequency calculations we want to use to obtain our average frequency prediction of the input. The higher the number, the more accurate the result
#define VCO_SAMPLE_RATE_US          125     // 8000hz is equal to 125us (microseconds)
#define VCO_ZERO_CROSSING           60000   // The zero crossing is erelivant as the pre-opamp ADC is not bi-polar. Any value close to the ADC ceiling seems to work
#define VCO_ZERO_CROSS_THRESHOLD    500     // for handling hysterisis at zero crossing point

// 83.333, 166.666, 249.999, 333.332, 416.66499999999996, 499.99799999999993, 583.3309999999999, 666.6639999999999, 749.9969999999998, 833.3299999999998, 916.6629999999998, 999.9959999999998
// 12-bit values => 83, 167, 250, 333, 417, 500, 583, 667, 750, 833, 917, 1000

// 16-bit values => 0,   1092.25, 2184.5, 3276.75, 4369, 5461.25, 6553.5, 7645.75, 8738, 9830.25, 10922.5, 12014.75, 13107
//                       1092     2184    3277     4369  5461     6553    7646     8738  9830     10922    12015     13107   1499
//                  B     C        C#      D        D#    E        F       F#       G     G#       A        A#        B       C

// 65535 / 12 = 5461.25 * 2 = 10922.5

// const int DAC_NOTE_MAP[8][3] = {
//   { 0, 83, 167 },
//   { 167, 250, 333 },
//   { 333, 417, 500 },
//   { 417, 500, 583 },
//   { 583, 667, 750 },
//   { 750, 833, 917 },
//   { 917, 1000, 1083 },
//   { 1000, 1083, 1083 }
// };

// const int DAC_NOTE_MAP[8][3] = {
//   { 0, 1092, 2184 },
//   { 2184, 3277, 4369 },
//   { 4369, 5461, 6553 },
//   { 5461, 6553, 7646 },
//   { 7646, 8738, 9830 },
//   { 9830, 10922, 12015 },
//   { 12015, 13107, 14199 },
//   { 13107, 14199, 15291 }
// };

const int DAC_NOTE_MAP[8][3] = {
  { 0, 1097, 2193 },
  { 2193, 3290, 4387 },
  { 4387, 5461, 6553 },
  { 5461, 6553, 7646 },
  { 7646, 8738, 9830 },
  { 9830, 10922, 12015 },
  { 12015, 13160, 14256 },
  { 13160, 14256, 15291 }
};

const int MIDI_NOTE_MAP[8][3] = {
  { 11, 12, 13 },
  { 13, 14, 15 },
  { 15, 16, 17 },
  { 16, 17, 18 },
  { 18, 19, 20 },
  { 20, 21, 22 },
  { 22, 23, 24 },
  { 23, 24, 25 },
};

const int MIDI_OCTAVE_MAP[4] = { 36, 48, 60, 72 };

#define DAC_1VO_ARR_SIZE   64
#define BENDER_CALIBRATION_SIZE 2
#define BENDER_MIN_CAL_INDEX DAC_1VO_ARR_SIZE
#define BENDER_MAX_CAL_INDEX (DAC_1VO_ARR_SIZE + 1)
#define CALIBRATION_ARR_SIZE (DAC_1VO_ARR_SIZE + BENDER_CALIBRATION_SIZE)
#define NUM_FLASH_CHANNEL_BYTES (((CALIBRATION_ARR_SIZE * 4) * 16) / 8)  // number of bytes = CALIBRATION_ARR_SIZE * number of channels * 16 bits / 8 bits

const int CALIBRATION_LED_MAP[DAC_1VO_ARR_SIZE] = {
  0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 6, 7,
  0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 6, 7,
  0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 6, 7,
  0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 6, 7,
  0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 6, 7,
  0, 1, 2, 4
};

const int DAC_VOLTAGE_VALUES[DAC_1VO_ARR_SIZE] = {
// A     A#     B      C      C#     D      D#     E      F      F#     G      G#
  5630,  6568,  7506,   8445,  9383,  10321, 11260, 12198, 13137, 14075, 15013, 15952,
  16890, 17828, 18767,  19705, 20643, 21582, 22520, 23458, 24397, 25335, 26274, 27212,
  28150, 29089, 30027,  30965, 31904, 32842, 33780, 34719, 35657, 36596, 37534, 38472,
  39411, 40349, 41287,  42226, 43164, 44102, 45041, 45979, 46917, 47856, 48794, 49733,
  50671, 51609, 52548,  53486, 54424, 55363, 56301, 57239, 58178, 59116, 60054, 60993,
  61931, 62870, 63808, 64746
};

#endif
