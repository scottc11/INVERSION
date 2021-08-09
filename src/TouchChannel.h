#ifndef __TOUCH_CHANNEL_H
#define __TOUCH_CHANNEL_H

#include "main.h"
#include "Bender.h"
#include "Metronome.h"
#include "Degrees.h"
#include "DAC8554.h"
#include "MPR121.h"
#include "TCA9544A.h"
#include "SX1509.h"
#include "MIDI.h"
#include "QuantizeMethods.h"
#include "BitwiseMethods.h"
#include "ArrayMethods.h"
#include "VoltPerOctave.h"
#include "SuperSeq.h"
#include "DegreeDisplay.h"

#define CHANNEL_REC_LED 11
#define CHANNEL_RATCHET_LED 10
#define CHANNEL_PB_LED 9
#define NULL_NOTE_INDEX 99  // used to identify a 'null' or 'deleted' sequence event

static const int OCTAVE_LED_PINS[4] = { 3, 2, 1, 0 };               // io pin map for octave LEDs
static const int CHAN_LED_PINS[8] = { 15, 14, 13, 12, 7, 6, 5, 4 }; // io pin map for channel LEDs
static const int CHAN_TOUCH_PADS[12] = { 7, 6, 5, 4, 3, 2, 1, 0, 3, 2, 1, 0 };

static const int DAC_OCTAVE_MAP[4] = {0, 12, 24, 36};
static const int DEGREE_INDEX_MAP[8] = {0, 2, 4, 6, 8, 10, 12, 14};

typedef struct QuantDegree {
  int threshold;
  int noteIndex;
} QuantDegree;

typedef struct QuantOctave {
  int threshold;
  int octave;
} QuantOctave;

typedef struct SequenceNode {
  uint8_t activeNotes; // byte for holding active/inactive notes for a chord
  uint8_t noteIndex;   // note index between 0 and 7 NOTE: you could tag on some extra data in the bottom most bits, like gate on / off for example
  uint16_t pitchBend;  // raw ADC value from pitch bend
  bool gate;           // set gate HIGH or LOW
  bool active;         // this will tell the loop whether to trigger an event or not
} SequenceNode;

class TouchChannel {
  private:
    enum SWITCH_STATES {
      OCTAVE_UP = 0b00001000,
      OCTAVE_DOWN = 0b00000100,
    };

    enum NoteState {
      ON,
      OFF,
      SUSTAIN,
      PREV,
      BEND_PITCH
    };

    enum LedColor {
      RED,
      GREEN
    };

  public:
    enum LedState : int
    {
      LOW = 0,
      HIGH = 1,
      BLINK_ON = 2,
      BLINK_OFF = 3,
      DIM_LOW = 4,
      DIM_MEDIUM = 5,
      DIM_HIGH = 6
    };

    enum ChannelMode {
      MONO = 0,
      MONO_LOOP = 1,
      QUANTIZE = 2,
      QUANTIZE_LOOP = 3,
    };

    enum BenderMode {
      BEND_OFF = 0,
      PITCH_BEND = 1,
      RATCHET = 2,
      RATCHET_PITCH_BEND = 3,
      INCREMENT_BENDER_MODE = 4
    };

    enum UIMode { // not yet implemented
      DEFAULT_UI,
      LOOP_LENGTH_UI,
      PB_RANGE_UI
    };

    int channel;                    // 0 based index to represent channel
    bool isSelected;
    bool gateState;                 // the current state of the gate output pin
    ChannelMode mode;               // which mode channel is currently in
    ChannelMode prevMode;           // used for reverting to previous mode when toggling between UI modes
    UIMode uiMode;                  // for settings and alt LED uis
    int benderMode;
    DigitalOut gateOut;             // gate output pin
    DigitalOut *globalGateOut;      // 
    Timer *timer;                   // timer for handling duration based touch events
    Ticker *ticker;                 // for handling time based callbacks
    MIDI *midi;                     // pointer to mbed midi instance
    MPR121 *touchPads;
    SX1509 *io;                     // IO Expander
    DegreeDisplay *display;
    Degrees *degrees;
    AnalogIn cvInput;               // CV input pin for quantizer mode

    volatile bool seqStepFlag;
    volatile bool tickerFlag;        // each time the clock gets ticked, this flag gets set to true - then false in polling loop
    volatile bool switchHasChanged;  // toggle switches interupt flag
    volatile bool touchDetected;

    VoltPerOctave output1V;
    Bender bender;
    SuperSeq sequence;

    // SEQUENCER variables
    SequenceNode events[PPQN * MAX_SEQ_LENGTH];
    QuantizeMode timeQuantizationMode;
    int prevEventIndex; // index for disabling the last "triggered" event in the loop
    bool sequenceContainsEvents;
    bool clearExistingNodes;   
    bool enableLoop = false;   // "Event Triggering Loop" -> This will prevent looped events from triggering if a new event is currently being created

    // quantizer variables
    int activeDegrees;                    // 8 bits to determine which scale degrees are presently active/inactive (active = 1, inactive= 0)
    int activeOctaves;                    // 4-bits to represent which octaves external CV will get mapped to (active = 1, inactive= 0)
    int numActiveDegrees;                 // number of degrees which are active (to quantize voltage input)
    int numActiveOctaves;                 // number of active octaves for mapping CV to
    int activeDegreeLimit;                // the max number of degrees allowed to be enabled at one time.
    QuantDegree activeDegreeValues[8];    // array which holds noteIndex values and their associated DAC/1vo values
    QuantOctave activeOctaveValues[OCTAVE_COUNT];
    
    uint16_t ledStates;                   // 16 bits to represent each bi-color led  | 0-Red | 0-Green | 1-Red | 1-Green | 2-Red | 2-Green | etc...
    
    unsigned int currCVInputValue;        // 16 bit value (0..65,536)
    unsigned int prevCVInputValue;        // 16 bit value (0..65,536)

    int touched;                          // variable for holding the currently touched degrees
    int prevTouched;                      // variable for holding the previously touched degrees
    int currOctave;                       // current octave value between 0..3
    int prevOctave;                       // previous octave value

    int currNoteIndex;
    int prevNoteIndex;
    
    bool freezeChannel;          //

    TouchChannel(
        int _channel,
        Timer *timer_ptr,
        Ticker *ticker_ptr,
        DigitalOut *globalGateOut_ptr,
        PinName gateOutPin,
        PinName cvInputPin,
        PinName pbInputPin,
        bool pbInverted,
        MPR121 *touch_ptr,
        SX1509 *io_ptr,
        DegreeDisplay *display_ptr,
        Degrees * degrees_ptr,
        MIDI *midi_p,
        DAC8554 *dac_ptr,
        DAC8554::Channels _dacChannel,
        DAC8554 *pb_dac_ptr,
        DAC8554::Channels pb_dac_channel) : gateOut(gateOutPin), cvInput(cvInputPin), bender(pb_dac_ptr, pb_dac_channel, pbInputPin, pbInverted)
    {
      globalGateOut = globalGateOut_ptr;
      timer = timer_ptr;
      ticker = ticker_ptr;
      touchPads = touch_ptr;
      io = io_ptr;
      display = display_ptr;
      degrees = degrees_ptr;
      output1V.dac = dac_ptr;
      output1V.dacChannel = _dacChannel;
      midi = midi_p;
      channel = _channel;
    };

    void init();
    void poll();
    void onTouch(uint8_t pad);
    void onRelease(uint8_t pad);

    void initIOExpander();
    void setLed(int index, LedState state, bool settingUILed=false);
    void setOctaveLed(int octave, LedState state, bool settingUILed=false);
    void setRecordLED(LedState state);
    void setRatchetLED(LedState state);
    void setPitchBendLED(LedState state);
    void setAllLeds(int state);
    void updateOctaveLeds(int octave);
    void updateLeds(uint8_t touched);  // could be obsolete

    // BENDER
    void benderActiveCallback(uint16_t value);
    void benderIdleCallback();
    int setBenderMode(BenderMode targetMode = INCREMENT_BENDER_MODE);

    void updateDegrees();
    void handleIOInterupt();
    void setMode(ChannelMode targetMode);
    
    int quantizePosition(int position);
    int calculateMIDINoteValue(int index, int octave);

    void setOctave(int value);
    void triggerNote(int index, int octave, NoteState state, bool blinkLED=false);
    void setGate(bool state);
    void setGlobalGate(bool state);
    void freeze(bool enable);

    // UI METHODS
    void enableUIMode(UIMode target);
    void disableUIMode();
    void updatePitchBendRangeUI();

    // SEQUENCER METHODS
    void initSequencer();
    void resetSequence();
    void clearEvent(int position);
    void clearEventSequence();
    void clearPitchBendSequence();
    void createEvent(int position, int noteIndex, bool gate);
    void createChordEvent(int position, uint8_t notes);
    void createPitchBendEvent(int position, uint16_t pitchBend);
    void enableSequencer();
    void disableSequencer();
    void handleSequence(int position);

    // QUANTIZER METHODS
    void initQuantizer();
    void toggleQuantizerMode();
    void handleCVInput();
    void setActiveDegrees(int degrees);
    void updateActiveDegreeLeds(uint8_t degrees);
    void setActiveDegreeLimit(int value);
    void setActiveOctaves(int octave);
    
};

#endif