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
#include "MCP23008.h"

class GlobalControl {
public:
  enum Mode
  {
    DEFAULT,
    CALIBRATING_1VO,
    CALIBRATING_BENDER
  };

  MCP23017 io;
  uint16_t currIOState;
  uint16_t prevIOState;
  MCP23008 leds;
  uint8_t ledStates = 0x00;          // || chan D || chan C || chan B || chan A ||
  Metronome *metronome;
  Degrees *degrees;
  VCOCalibrator calibrator;
  TouchChannel *channels[4];
  Timer timer;
  InterruptIn ctrlInterupt;
  DigitalOut freezeLED;
  DigitalOut recLED;
  uint32_t flashAddr = 0x08060000;   // should be 'sector 7', program memory address starts @ 0x08000000

  Mode mode;
  bool recordEnabled;                // used for toggling REC led among other things...
  int selectedChannel;
  bool buttonPressed;
  uint16_t buttonsState;

  GlobalControl(
      Metronome *metronome_ptr,
      Degrees *degrees_ptr,
      I2C *i2c_ptr,
      TouchChannel *chanA_ptr,
      TouchChannel *chanB_ptr,
      TouchChannel *chanC_ptr,
      TouchChannel *chanD_ptr
      ) : io(i2c_ptr, MCP23017_CTRL_ADDR), leds(i2c_ptr, MCP23008_IO_ADDR), ctrlInterupt(CTRL_INT), freezeLED(FREEZE_LED), recLED(REC_LED)
  {
    mode = Mode::DEFAULT;
    metronome = metronome_ptr;
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
  void clearAllChannelEvents();
  void calibrateChannel(int chan);
  void saveCalibrationToFlash(bool reset=false);
  void loadCalibrationDataFromFlash();
  void calibrateBenders();

  void handleDegreeChange();
  void handleFreeze(bool enable);
  void handleClockReset();
  void enableLoopLengthUI();

  void handleStateChange(int currState, int prevState);
  void handleButtonPress(int pad);
  void handleButtonRelease(int pad);
  bool handleGesture();
  void pollButtons();
  void handleOctaveTouched();
  void setChannelOctave(int pad);
  void setChannelLoopMultiplier(int pad);
  void setChannelBenderMode();
  void setChannelBenderMode(int chan);
  void tickChannels();

  void handleControlInterupt() {
    buttonPressed = true;
  }

private:
  enum PadNames
  {                  // integers correlate to 8-bit index position
    SEQ_LENGTH = 0x0040,
    PB_RANGE =   0x0020,
    RECORD =     0x1000,
    CLEAR_SEQ =  0x4000,
    CLEAR_BEND = 0x2000,
    BEND_MODE =  0x8000,
    RESET =      0x0100,
    FREEZE =     0x0400,
    QUANTIZE_SEQ = 0x0200,
    QUANTIZE_AMOUNT = 0x0010,
    SHIFT =      0x0080,
    CTRL_A =     0x0008,
    CTRL_B =     0x0004,
    CTRL_C =     0x0002,
    CTRL_D =     0x0001
  };

  enum Gestures
  {
    CALIBRATE_A = 0x0088, // SHIFT + CTRL_A
    CALIBRATE_B = 0x0084,
    CALIBRATE_C = 0x0082,
    CALIBRATE_D = 0x0081,
    CALIBRATE_BENDER = 0x00A0,
    BEND_MODE_A = 0x8008,
    BEND_MODE_B = 0x8004,
    BEND_MODE_C = 0x8002,
    BEND_MODE_D = 0x8001,
    CLEAR_SEQ_A = 0x4008,
    CLEAR_SEQ_B = 0x4004,
    CLEAR_SEQ_C = 0x4002,
    CLEAR_SEQ_D = 0x4001,
    CLEAR_BEND_SEQ_A = 0x2008,
    CLEAR_BEND_SEQ_B = 0x2004,
    CLEAR_BEND_SEQ_C = 0x2002,
    CLEAR_BEND_SEQ_D = 0x2001
  };
};


#endif