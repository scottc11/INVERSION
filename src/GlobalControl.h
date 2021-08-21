#ifndef __GLOBAL_CONTROL_H
#define __GLOBAL_CONTROL_H

#include "main.h"
#include "BitwiseMethods.h"
#include "TouchChannel.h"
#include "Degrees.h"
#include "VCOCalibrator.h"
#include "DualDigitDisplay.h"
#include "Metronome.h"
#include "MCP23017.h"
#include "DegreeDisplay.h"

class GlobalControl {
public:
  enum Mode
  {
    DEFAULT,
    CALIBRATING_1VO,
    CALIBRATING_BENDER,
    SET_SEQUENCE_LENGTH
  };

  MCP23017 io;
  uint16_t currIOState;
  uint16_t prevIOState;
  uint8_t ledStates = 0x00;          // || chan D || chan C || chan B || chan A ||
  Metronome *metronome;
  DegreeDisplay *display;
  Degrees *degrees;
  VCOCalibrator calibrator;
  TouchChannel *channels[4];
  Timer timer;
  InterruptIn ctrlInterupt;
  DigitalOut freezeLED;
  DigitalOut recLED;
  uint32_t flashAddr = 0x08060000;   // should be 'sector 7', program memory address starts @ 0x08000000

  Mode mode;
  bool gestureFlag;                  // for making sure no artifacts execute after a gesture is performed
  bool recordEnabled;                // used for toggling REC led among other things...
  int selectedChannel;
  bool buttonPressed;
  uint16_t buttonsState;
  uint8_t incrememntor = 0;

  GlobalControl(
      Metronome *metronome_ptr,
      DegreeDisplay *display_ptr,
      Degrees *degrees_ptr,
      I2C *i2c_ptr,
      TouchChannel *chanA_ptr,
      TouchChannel *chanB_ptr,
      TouchChannel *chanC_ptr,
      TouchChannel *chanD_ptr
      ) : io(i2c_ptr, MCP23017_CTRL_ADDR), ctrlInterupt(CTRL_INT), freezeLED(FREEZE_LED), recLED(REC_LED)
  {
    mode = Mode::DEFAULT;
    metronome = metronome_ptr;
    display = display_ptr;
    degrees = degrees_ptr;
    channels[0] = chanA_ptr;
    channels[1] = chanB_ptr;
    channels[2] = chanC_ptr;
    channels[3] = chanD_ptr;
    ctrlInterupt.fall(callback(this, &GlobalControl::handleControlInterupt));
  }

  void init();
  void poll();
  void selectChannel(int channel);
  void calibrateChannel(int chan);
  void saveCalibrationToFlash(bool reset=false);
  void loadCalibrationDataFromFlash();
  void calibrateBenders();

  void handleDegreeChange();
  void handleFreeze(bool enable);
  void handleReset();
  void enableLoopLengthUI();

  void handleStateChange(int currState, int prevState);
  void handleButtonPress(int pad);
  void handleButtonRelease(int pad);
  bool handleGesture();
  void pollButtons();
  void handleOctaveTouched();
  void setChannelOctave(int pad);
  void setChannelBenderMode(int chan);
  void tickChannels();

  void clearChannelSequence(int chan);

  void handleControlInterupt() {
    buttonPressed = true;
  }

private:
  enum PadNames : uint16_t
  { // integers correlate to 8-bit index position
    FREEZE = 0x4000,
    RECORD = 0x2000,
    RESET = 0x1000,
    QUANTIZE_SEQ = 0x0800,
    CLEAR_BEND = 0x0400,
    CLEAR_SEQ = 0x0200,
    BEND_MODE = 0x0100,
    QUANTIZE_AMOUNT = 0x0080,
    SEQ_LENGTH = 0x0020,
    PB_RANGE = 0x0040,
    SHIFT = 0x0010,
    CTRL_A = 0x0008,
    CTRL_B = 0x0004,
    CTRL_C = 0x0002,
    CTRL_D = 0x0001
  };

  enum Gestures : uint16_t
  {
    CALIBRATE_A = SHIFT | CTRL_A, // SHIFT + CTRL_A
    CALIBRATE_B = SHIFT | CTRL_B,
    CALIBRATE_C = SHIFT | CTRL_C,
    CALIBRATE_D = SHIFT | CTRL_D,
    CALIBRATE_BENDER = SHIFT | BEND_MODE,             // SHIFT + BEND_MODE
    RESET_CALIBRATION_TO_DEFAULT = SHIFT | RECORD | BEND_MODE, // SHIFT + REC + BEND_MODE
    BEND_MODE_A = CTRL_A | BEND_MODE,
    BEND_MODE_B = CTRL_B | BEND_MODE,
    BEND_MODE_C = CTRL_C | BEND_MODE,
    BEND_MODE_D = CTRL_D | BEND_MODE,
    CLEAR_SEQ_A = CLEAR_SEQ | CTRL_A,
    CLEAR_SEQ_B = CLEAR_SEQ | CTRL_B,
    CLEAR_SEQ_C = CLEAR_SEQ | CTRL_C,
    CLEAR_SEQ_D = CLEAR_SEQ | CTRL_D,
    CLEAR_BEND_SEQ_A = 0x2008,
    CLEAR_BEND_SEQ_B = 0x2004,
    CLEAR_BEND_SEQ_C = 0x2002,
    CLEAR_BEND_SEQ_D = 0x2001
  };
};


#endif