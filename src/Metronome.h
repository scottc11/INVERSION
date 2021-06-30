#ifndef BEAT_CLOCK_H
#define BEAT_CLOCK_H

/**
 * Beats-per-minute: 120 BPM
 * Beats-per-second: 2 Hz
 * Length of 1 beat: 0.5 second = 500 msec
 * Length of 1 bar (4 beats): 2 second
*/

#include "main.h"


#define BPM_MAX 300
#define BPM_MIN 60

const int usTempoMap[BPM_MAX - BPM_MIN] = { 10416, 10245, 10080, 9920, 9765, 9615, 9469, 9328, 9191, 9057, 8928, 8802, 8680, 8561, 8445, 8333, 8223, 8116, 8012, 7911, 7812, 7716, 7621, 7530, 7440, 7352, 7267, 7183, 7102, 7022, 6944, 6868, 6793, 6720, 6648, 6578, 6510, 6443, 6377, 6313, 6250, 6188, 6127, 6067, 6009, 5952, 5896, 5841, 5787, 5733, 5681, 5630, 5580, 5530, 5482, 5434, 5387, 5341, 5296, 5252, 5208, 5165, 5122, 5081, 5040, 5000, 4960, 4921, 4882, 4844, 4807, 4770, 4734, 4699, 4664, 4629, 4595, 4562, 4528, 4496, 4464, 4432, 4401, 4370, 4340, 4310, 4280, 4251, 4222, 4194, 4166, 4139, 4111, 4084, 4058, 4032, 4006, 3980, 3955, 3930, 3906, 3881, 3858, 3834, 3810, 3787, 3765, 3742, 3720, 3698, 3676, 3654, 3633, 3612, 3591, 3571, 3551, 3531, 3511, 3491, 3472, 3453, 3434, 3415, 3396, 3378, 3360, 3342, 3324, 3306, 3289, 3272, 3255, 3238, 3221, 3205, 3188, 3172, 3156, 3140, 3125, 3109, 3094, 3078, 3063, 3048, 3033, 3019, 3004, 2990, 2976, 2962, 2948, 2934, 2920, 2906, 2893, 2880, 2866, 2853, 2840, 2828, 2815, 2802, 2790, 2777, 2765, 2753, 2741, 2729, 2717, 2705, 2693, 2682, 2670, 2659, 2648, 2637, 2626, 2615, 2604, 2593, 2582, 2572, 2561, 2551, 2540, 2530, 2520, 2510, 2500, 2490, 2480, 2470, 2460, 2450, 2441, 2431, 2422, 2413, 2403, 2394, 2385, 2376, 2367, 2358, 2349, 2340, 2332, 2323, 2314, 2306, 2297, 2289, 2281, 2272, 2264, 2256, 2248, 2240, 2232, 2224, 2216, 2208, 2200, 2192, 2185, 2177, 2170, 2162, 2155, 2147, 2140, 2133, 2125, 2118, 2111, 2104, 2097, 2090 };

class Metronome {
public:
  DigitalOut tempoLed;
  AnalogIn tempoPot;
  DigitalOut tempoOutput;
  Ticker ticker; //

  Callback<void()> callbackFn;  // copying how ticker class does it

  int tickInterval;          // how long, in microseconds, a tick lasts. 120bpm == 0.5s == 500000us (microseconds)
  uint16_t newTempoPotValue; // Analog value rep. the position of the potentiometer
  uint16_t oldTempoPotValue; // Analog value rep. the position of the potentiometer
  uint8_t bpm;
  uint8_t currStep;       // used to calculate an events position
  uint8_t numSteps;       // used to calculate total clock loop length (in ticks)
  uint8_t ticksPerStep;   // equal to PPQN
  uint8_t currTick;       // relative to PPQN
  uint16_t position;      // the clocks current position with the loop. Will be a multiplication of currTick and currStep
  uint32_t loopStart;     // time when the first step occurs on the system clock
  uint32_t pulseDuration; // how long, in microseconds, the clock led will be lit
  uint32_t lastClock;     // time of the last clocked event

  Metronome(
    PinName ledPin,
    PinName potPin,
    PinName clockOutPin,
    int ppqn,
    int defaultNumSteps
    ) : tempoLed(ledPin), tempoPot(potPin), tempoOutput(clockOutPin)
  {
    ticksPerStep = ppqn;
    numSteps = defaultNumSteps;
    tickInterval = 5208; // init @ 120bpm ::: (0.5s * 1e+6) / 96ppqn = 5208us
    bpm = 120;
    currStep = 1;
    currTick = 1;
    pulseDuration = 5;
  };

  void init();
  void poll();
  void tick();
  void step();
  void start();
  void stop();
  void setNumberOfSteps(int num);
  void handleEncoder();
  void pollTempoPot();
  void updateTempo(int us);
  void attachTickCallback(Callback<void()> func);
};

#endif