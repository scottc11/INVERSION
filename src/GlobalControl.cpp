#include "GlobalControl.h"

void GlobalControl::init() {

  metronome->init();

  // this function is causing the calibration routine to crash the program
  // try adding back mbedignore libraries and debug
  metronome->attachTickCallback(callback(this, &GlobalControl::tickChannels));

  degrees->attachCallback(callback(this, &GlobalControl::handleDegreeChange));

  display->init();

  io.init();
  io.setDirection(MCP23017_PORTA, 0xff);
  io.setDirection(MCP23017_PORTB, 0xff);
  io.setInterupt(MCP23017_PORTA, 0xff);
  io.setInterupt(MCP23017_PORTB, 0xff);
  io.setPullUp(MCP23017_PORTA, 0xff);
  io.setPullUp(MCP23017_PORTB, 0xff);
  io.setInputPolarity(MCP23017_PORTA, 0xff);
  io.setInputPolarity(MCP23017_PORTB, 0b01111111);
  io.digitalReadAB(); // clear any stray interupts
  

  freezeLED.write(0);
  recLED.write(0);

  selectChannel(0);  // select a default channel
}



void GlobalControl::tickChannels() {
  for (int i = 0; i < 4; i++)
  {
    channels[i]->sequence.advance();
    channels[i]->tickerFlag = true;
  }
  
}


void GlobalControl::poll() {
  switch (mode) {
    case CALIBRATING_BENDER:
      this->pollButtons();
      this->calibrateBenders();
      break;
    case DEFAULT:
      metronome->poll();
      degrees->poll();
      this->pollButtons();
      channels[0]->poll();
      channels[1]->poll();
      channels[2]->poll();
      channels[3]->poll();
      break;
  }
}


void GlobalControl::handleDegreeChange() {
  channels[0]->updateDegrees();
  channels[1]->updateDegrees();
  channels[2]->updateDegrees();
  channels[3]->updateDegrees();
}

/**
 * CHANNEL SELECT
*/
void GlobalControl::selectChannel(int channel) {
  for (int i = 0; i < 4; i++) {
    if (i != channel) {
      channels[i]->isSelected = false;
    }
  }
  
  selectedChannel = channel;
  channels[selectedChannel]->isSelected = true;
}

void GlobalControl::setChannelBenderMode(int chan)
{
  channels[chan]->setBenderMode();
}

/**
 * Poll IO and see if any buttons have been either pressed or released
*/
void GlobalControl::pollButtons()
{
  if (buttonPressed)
  {
    wait_us(2000);
    currIOState = io.digitalReadAB();
    if (currIOState != prevIOState)
    {
      for (int i = 0; i < 16; i++)
      {
        // if state went HIGH and was LOW before
        if (bitRead(currIOState, i) && !bitRead(prevIOState, i))
        {
          this->handleButtonPress(currIOState);
        }
        // if state went LOW and was HIGH before
        if (!bitRead(currIOState, i) && bitRead(prevIOState, i))
        {
          this->handleButtonRelease(prevIOState);
        }
      }
    }

    // reset polling
    prevIOState = currIOState;
    buttonPressed = false;
  }
}

/**
 * HANDLE TOUCH TOUCHED
 * 
*/
void GlobalControl::handleButtonPress(int pad) {
    
  switch (pad) {
    case CTRL_A:
      channels[0]->toggleQuantizerMode();
      break;
    case CTRL_B:
      channels[1]->toggleQuantizerMode();
      break;
    case CTRL_C:
      channels[2]->toggleQuantizerMode();
      break;
    case CTRL_D:
      channels[3]->toggleQuantizerMode();
      break;
    case FREEZE:
      handleFreeze(true);
      break;

    case RESET:
      handleReset();
      break;
    
    case Gestures::CALIBRATE_A:
      calibrateChannel(0);
      break;
    case Gestures::CALIBRATE_B:
      calibrateChannel(1);
      break;
    case Gestures::CALIBRATE_C:
      calibrateChannel(2);
      break;
    case Gestures::CALIBRATE_D:
      calibrateChannel(3);
      break;
    
    case Gestures::CALIBRATE_BENDER:
      if (this->mode == CALIBRATING_BENDER) {
        this->saveCalibrationToFlash();
        display->clear();
        this->mode = DEFAULT;
      } else {
        this->mode = CALIBRATING_BENDER;
        display->benderCalibration();
      }
      break;
    
    case Gestures::RESET_CALIBRATION_TO_DEFAULT:
      display->benderCalibration();
      saveCalibrationToFlash(true);
      display->clear();
      break;
    
    case BEND_MODE:
      break;
    
    case BEND_MODE_A:
      setChannelBenderMode(0);
      break;
    case BEND_MODE_B:
      setChannelBenderMode(1);
      break;
    case BEND_MODE_C:
      setChannelBenderMode(2);
      break;
    case BEND_MODE_D:
      setChannelBenderMode(3);
      break;
    
    case CLEAR_SEQ_A:
      this->clearChannelSequence(0);
      break;
    case CLEAR_SEQ_B:
      this->clearChannelSequence(1);
      break;
    case CLEAR_SEQ_C:
      this->clearChannelSequence(2);
      break;
    case CLEAR_SEQ_D:
      this->clearChannelSequence(3);
      break;

    case PB_RANGE:
      channels[0]->enableUIMode(TouchChannel::PB_RANGE_UI);
      channels[1]->enableUIMode(TouchChannel::PB_RANGE_UI);
      channels[2]->enableUIMode(TouchChannel::PB_RANGE_UI);
      channels[3]->enableUIMode(TouchChannel::PB_RANGE_UI);
      break;
    case SEQ_LENGTH:
      this->display->clear();
      for (int chan = 0; chan < 4; chan++)
      {
        channels[chan]->setBenderMode(TouchChannel::BenderMode::BEND_MENU);
      }
      break;
    case RECORD:
      if (!recordEnabled) {
        recLED.write(1);
        channels[0]->enableSequenceRecording();
        channels[1]->enableSequenceRecording();
        channels[2]->enableSequenceRecording();
        channels[3]->enableSequenceRecording();
        recordEnabled = true;
      } else {
        recLED.write(0);
        channels[0]->disableSequenceRecording();
        channels[1]->disableSequenceRecording();
        channels[2]->disableSequenceRecording();
        channels[3]->disableSequenceRecording();
        recordEnabled = false;
      }
      break;
  }
}

/**
 * HANDLE TOUCH RELEASE
*/
void GlobalControl::handleButtonRelease(int pad)
{
  switch (pad) {
    case FREEZE:
      handleFreeze(false);
      break;
    case RESET:
      break;
    case PB_RANGE:
      channels[0]->disableUIMode();
      channels[1]->disableUIMode();
      channels[2]->disableUIMode();
      channels[3]->disableUIMode();
      break;
    case CLEAR_SEQ:
      if (!gestureFlag) {
        for (int i = 0; i < 4; i++)
        {
          channels[i]->clearEventSequence();
          channels[i]->disableSequenceRecording();
        }
      }
      gestureFlag = false;
      break;
    case SEQ_LENGTH:
      this->display->clear();
      for (int chan = 0; chan < 4; chan++)
      {
        if (channels[chan]->sequenceContainsEvents) {
          display->setSequenceLEDs(chan, channels[chan]->sequence.length, true);
        }
        channels[chan]->setBenderMode(TouchChannel::BenderMode::BEND_OFF);
      }
      break;
    case RECORD:
      break;
  }
}

bool GlobalControl::handleGesture() {
  // switch (currTouched) {
  //   case RESET_CALIBRATION:
  //     // TODO: there should be some kind of UI signaling successful clear
  //     saveCalibrationToFlash(true);   // reset calibration to default values
  //     loadCalibrationDataFromFlash(); // then load the 'new' values into all the channel instances
  //     return true;
  return false;
}

/**
 * HANDLE FREEZE
*/
void GlobalControl::handleFreeze(bool enable) {
  // freeze all channels
  freezeLED.write(enable);
  channels[0]->freeze(enable);
  channels[1]->freeze(enable);
  channels[2]->freeze(enable);
  channels[3]->freeze(enable);
}


/**
 * HANDLE RESET
*/
void GlobalControl::handleReset() {
  // reset all channels
  for (int i = 0; i < 4; i++)
  {
    channels[i]->resetSequence();
  }
  
}


void GlobalControl::calibrateChannel(int chan) {
  metronome->stop();
  calibrator.setChannel(channels[chan]);
  calibrator.startCalibration();
  calibrator.bruteForceCalibration();
  this->saveCalibrationToFlash();
  metronome->start();
}

/**
 * every time we calibrate a channel, we need to save all 4 channels data due to the fact 
 * that we must delete/clear an entire sector of data first.
*/ 
void GlobalControl::saveCalibrationToFlash(bool reset /* false */)
{

  char16_t buffer[CALIBRATION_ARR_SIZE * 4]; // create array of 16 bit chars to hold ALL 4 channels data

  // iterate through each channel
  for (int chan = 0; chan < 4; chan++) {
    
    // load 1VO calibration data into buffer
    for (int i = 0; i < DAC_1VO_ARR_SIZE; i++) // leave the last two indexes for bender values
    {
      // determine buffer index position based on channel
      int index = i + CALIBRATION_ARR_SIZE * chan;
      
      // reset to default values before copying to buffer
      if (reset) {
        channels[chan]->output1V.resetVoltageMap();
      }

      // copy values to buffer
      buffer[index] = channels[chan]->output1V.dacVoltageMap[i];
    }
    // load max and min Bender calibration data into buffer (two 16bit chars)
    buffer[BENDER_MIN_CAL_INDEX + CALIBRATION_ARR_SIZE * chan] = channels[chan]->bender.minBend;
    buffer[BENDER_MAX_CAL_INDEX + CALIBRATION_ARR_SIZE * chan] = channels[chan]->bender.maxBend;
  }
  FlashIAP flash;
  flash.init();
  flash.erase(flashAddr, flash.get_sector_size(flashAddr));     // must erase all data before a write
  flash.program(buffer, flashAddr, NUM_FLASH_CHANNEL_BYTES);
  flash.deinit();
  
  this->pollButtons(); // BUG: need to call this or else interupt gets stuck
}

void GlobalControl::loadCalibrationDataFromFlash() {
  volatile uint16_t buffer[CALIBRATION_ARR_SIZE * 4];
  FlashIAP flash;
  flash.init();
  flash.read((void *)buffer, flashAddr, NUM_FLASH_CHANNEL_BYTES);
  flash.deinit();
  for (int chan = 0; chan < 4; chan++) {
    for (int i = 0; i < DAC_1VO_ARR_SIZE; i++)
    {
      int index = i + CALIBRATION_ARR_SIZE * chan; // determine falshData index position based on channel
      channels[chan]->output1V.dacVoltageMap[i] = buffer[index];
    }
    channels[chan]->bender.minBend = buffer[BENDER_MIN_CAL_INDEX + CALIBRATION_ARR_SIZE * chan];
    channels[chan]->bender.maxBend = buffer[BENDER_MAX_CAL_INDEX + CALIBRATION_ARR_SIZE * chan];
  }
}

void GlobalControl::calibrateBenders() {
  for (int i = 0; i < 4; i++)
  {
    channels[i]->bender.calibrateMinMax();
  }
}

void GlobalControl::clearChannelSequence(int chan) {
  gestureFlag = true;
  channels[chan]->clearEventSequence();
  channels[chan]->disableSequenceRecording();
}