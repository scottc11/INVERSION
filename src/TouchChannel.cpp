#include "TouchChannel.h"

void TouchChannel::init() {

  output1V.init();

  this->initIOExpander();

  touchPads->init();
  touchPads->attachCallbackTouched(callback(this, &TouchChannel::onTouch));
  touchPads->attachCallbackReleased(callback(this, &TouchChannel::onRelease));
  touchPads->enable();

  bender.init();
  bender.attachActiveCallback(callback(this, &TouchChannel::benderActiveCallback));
  bender.attachIdleCallback(callback(this, &TouchChannel::benderIdleCallback));
  bender.attachTriStateCallback(callback(this, &TouchChannel::benderTriStateCallback));
  // set bender mode
  setBenderMode(PITCH_BEND);
  setRecordLED(LOW);

  this->initSequencer(); // must be done after pb calibration
  this->initQuantizer();

  // initialize default variables
  currNoteIndex = 0;
  currOctave = 0;
  touched = 0;
  prevTouched = 0;
  mode = MONO;
  uiMode = DEFAULT_UI;
  
  this->setOctave(currOctave);
  setGate(LOW);
  triggerNote(currNoteIndex, currOctave, NoteState::SUSTAIN);
}


void TouchChannel::initIOExpander() {
  io->init();
  io->setBlinkFrequency(SX1509::ULTRA_FAST);

  io->ledConfig(CHANNEL_PB_LED);
  io->ledConfig(CHANNEL_REC_LED);
  io->ledConfig(CHANNEL_RATCHET_LED);

  // initialize IO Led Driver pins
  for (int i = 0; i < 8; i++)
  {
    io->pinMode(CHAN_LED_PINS[i], SX1509::ANALOG_OUTPUT);
    io->setPWM(CHAN_LED_PINS[i], 127);
    io->digitalWrite(CHAN_LED_PINS[i], 1);
  }

  for (int i = 0; i < 4; i++)
  {
    io->pinMode(OCTAVE_LED_PINS[i], SX1509::ANALOG_OUTPUT);
    io->setPWM(OCTAVE_LED_PINS[i], 255);
    io->digitalWrite(OCTAVE_LED_PINS[i], 1);
  }
}


/** ------------------------------------------------------------------------
 *         POLL    POLL    POLL    POLL    POLL    POLL    POLL    POLL    
---------------------------------------------------------------------------- */
// HANDLE ALL INTERUPT FLAGS
void TouchChannel::poll() {
  
  if (!freezeChannel) { // don't do anything if freeze enabled
    
    // Timer polling --> flag if timer is active, and if it is start counting to 3 seconds
    // after 3 seconds, call a function which takes the currTouched variable and applies it to the activeDegreeLimit.
    // then disable timer poll flag.

    touchPads->poll();

    if (tickerFlag) {    // every PPQN, read ADCs and update

      bender.poll();

      if ((mode == QUANTIZE || mode == QUANTIZE_LOOP))    // HANDLE CV QUANTIZATION
      {
        this->handleCVInput();
      }

      
      if ((mode == MONO_LOOP || mode == QUANTIZE_LOOP) && enableLoop)        // HANDLE SEQUENCE
      {
        if (sequence.currStep != sequence.prevStep)
        {
          display->stepSequenceLED(this->channel, sequence.currStep, sequence.prevStep);
        }
        handleSequence(sequence.currPosition);
        seqStepFlag = false;
      }

      tickerFlag = false;
    }
  }
}
// ------------------------------------------------------------------------

void TouchChannel::enableSequencer() {
  if (mode == MONO) {
    display->setSequenceLEDs(this->channel, sequence.length, true);
    setMode(MONO_LOOP);
  } else if (mode == QUANTIZE) {
    setMode(QUANTIZE_LOOP);
  }
}

void TouchChannel::disableSequencer() {

  // a nice feature here would be to only have the LEDs be red when REC is held down, and flash the green LEDs 
  // when a channel contains loop events, but REC is NOT held down. You would only be able to add new events to 
  // the loop when REC is held down (ie. when channel leds are RED)

  // ADDITIONALLY, this would be a good place to count the amount of steps which have passed while the REC button has
  // been held down, and if this value is greater than the current loop length, update the loop length to accomodate.
  // the new loop length would just increase the multiplier by one

  if (sequenceContainsEvents) {   // if a touch event was recorded, remain in loop mode
    return;
  } else {             // if no touch event recorded, revert to previous mode
    display->setSequenceLEDs(this->channel, sequence.length, false);
    if (mode == MONO_LOOP) {
      setMode(MONO);
    } else if (mode == QUANTIZE_LOOP) {
      setMode(QUANTIZE);
    }
  }
}

/** -------------------------------------------------------------------------------------------
 *         TOUCH EVENT METHODS           TOUCH EVENT METHODS            TOUCH EVENT METHODS
----------------------------------------------------------------------------------------------- */

// NOTE: you need a way to trigger events after a series of touches have happened, and the channel is now not being touched

void TouchChannel::onTouch(uint8_t pad) {
  
  if (pad < 8) {
    pad = CHAN_TOUCH_PADS[pad];
    if (uiMode == DEFAULT_UI)
    {
      switch (mode)
      {
      case MONO:
        triggerNote(pad, currOctave, ON);
        break;
      case QUANTIZE:
        /**
       * TODO: start timer for setting max active degrees
       * RESET will now reset activeDegreeLimit to its max value of 8
      */
        setActiveDegrees(bitWrite(activeDegrees, pad, !bitRead(activeDegrees, pad)));
        updateActiveDegreeLeds(activeDegrees);
        break;
      case QUANTIZE_LOOP:
        // every touch detected, take a snapshot of all active degree values and apply them to a EventNode
        setActiveDegrees(bitWrite(activeDegrees, pad, !bitRead(activeDegrees, pad)));
        createChordEvent(sequence.currPosition, activeDegrees);
        break;
      case MONO_LOOP:
        clearExistingNodes = true;
        createEvent(sequence.currPosition, pad, HIGH);
        triggerNote(pad, currOctave, ON);
        break;
      }
    }
    else
    { // LOOP_LENGTH_UI mode
      switch (uiMode)
      {
      case PB_RANGE_UI:
        output1V.setPitchBendRange(pad);
        updatePitchBendRangeUI();
        break;
      }
    }
  } else {
    pad = CHAN_TOUCH_PADS[pad];
    setOctave(pad);
  }
}

void TouchChannel::onRelease(uint8_t pad) {
  
  if (uiMode == DEFAULT_UI)
  {
    if (pad < 8) {
      pad = CHAN_TOUCH_PADS[pad];
      switch (mode)
      {
      case MONO:
        triggerNote(pad, currOctave, OFF);
        break;
      case QUANTIZE:
        // set end time
        // if (endTime - startTime > gestureThreshold) do something fancy
        break;
      case QUANTIZE_LOOP:
        break;
      case MONO_LOOP:
        createEvent(sequence.currPosition, pad, LOW);
        triggerNote(pad, currOctave, OFF);
        clearExistingNodes = false;
        // create note OFF event
        // enableLoop = true;
        break;
      }
    } else {
      setGate(LOW);
    }
  }
}

// -------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------

/**
 * TOGGLE MODE
 * 
 * still needs to be written to handle 3-stage toggle switch.
**/
void TouchChannel::toggleQuantizerMode() {
  if (this->mode == MONO || this->mode == MONO_LOOP) {
    setMode(QUANTIZE);
  }
  else {
    setMode(MONO);
  }
}

void TouchChannel::setMode(ChannelMode targetMode)
{
  prevMode = mode;
  switch (targetMode) {
    case MONO:
      enableLoop = false;
      mode = MONO;
      setAllLeds(LOW);               // I think this is just a "start from a clean slate" kinda thing
      setAllLeds(DIM_HIGH);
      updateOctaveLeds(currOctave);
      setRecordLED(LOW);
      triggerNote(currNoteIndex, currOctave, SUSTAIN);
      break;
    case MONO_LOOP:
      enableLoop = true;
      mode = MONO_LOOP;
      setAllLeds(LOW);
      setAllLeds(DIM_HIGH);
      updateOctaveLeds(currOctave);
      setRecordLED(HIGH);
      triggerNote(currNoteIndex, currOctave, SUSTAIN);
      break;
    case QUANTIZE:
      enableLoop = false;
      mode = QUANTIZE;
      setAllLeds(LOW);
      setAllLeds(DIM_HIGH);
      updateActiveDegreeLeds(activeDegrees);
      updateOctaveLeds(activeOctaves);
      setRecordLED(LOW);
      triggerNote(currNoteIndex, currOctave, OFF);
      break;
    case QUANTIZE_LOOP:
      enableLoop = true;
      mode = QUANTIZE_LOOP;
      setAllLeds(LOW);
      setAllLeds(DIM_HIGH);
      updateActiveDegreeLeds(activeDegrees);
      setRecordLED(HIGH);
      triggerNote(currNoteIndex, currOctave, OFF);
      break;
  }
}

/**
 * take a number between 0 - 3 and apply to currOctave
**/
void TouchChannel::setOctave(int value) {
  currOctave = value;

  switch (mode) {
    case MONO:
      updateOctaveLeds(currOctave);
      triggerNote(currNoteIndex, currOctave, ON);
      break;
    case MONO_LOOP:
      updateOctaveLeds(currOctave);
      triggerNote(currNoteIndex, currOctave, SUSTAIN);
      break;
    case QUANTIZE:
      setActiveOctaves(value);
      updateOctaveLeds(activeOctaves);
      break;
    case QUANTIZE_LOOP:
      setActiveOctaves(value);
      updateOctaveLeds(activeOctaves);
      break;
  }

  prevOctave = currOctave;
}

void TouchChannel::updateDegrees() {
  switch (mode) {
    case MONO:
      triggerNote(currNoteIndex, currOctave, SUSTAIN);
      break;
    case QUANTIZE:
      break;
    case MONO_LOOP:
      break;
  }
}



/** ------------------------------------------------------------------------
 *         LED METHODS
---------------------------------------------------------------------------- */

void TouchChannel::setAllLeds(int state) {
  switch (state) {
    case HIGH:
      for (int i = 0; i < 8; i++) setLed(i, HIGH);
      break;
    case LOW:
      for (int i = 0; i < 8; i++) setLed(i, LOW);
      break;
    case DIM_LOW:
      for (int i = 0; i < 8; i++) setLed(i, DIM_LOW);
      break;
    case DIM_MEDIUM:
      for (int i = 0; i < 8; i++) setLed(i, DIM_MEDIUM);
      break;
    case DIM_HIGH:
      for (int i = 0; i < 8; i++) setLed(i, DIM_HIGH);
      break;
  }
}

void TouchChannel::setLed(int index, LedState state, bool settingUILed /*false*/) {
  if (uiMode == DEFAULT_UI || settingUILed) {

    switch (state) {
      case LOW:
        ledStates &= ~(1 << index);
        io->setOnTime(CHAN_LED_PINS[index], 0);
        io->digitalWrite(CHAN_LED_PINS[index], 1);
        break;
      case HIGH:
        ledStates |= 1 << index;
        io->setOnTime(CHAN_LED_PINS[index], 0);
        io->digitalWrite(CHAN_LED_PINS[index], 0);
        break;
      case BLINK_ON:
        ledStates |= 1 << index;
        io->blinkLED(CHAN_LED_PINS[index], 1, 2, 127, 0);
        break;
      case BLINK_OFF:
        io->setOnTime(CHAN_LED_PINS[index], 0);
        break;
      case DIM_LOW:
        io->setPWM(CHAN_LED_PINS[index], 10);
        break;
      case DIM_MEDIUM:
        io->setPWM(CHAN_LED_PINS[index], 30);
        break;
      case DIM_HIGH:
        io->setPWM(CHAN_LED_PINS[index], 70);
        break;
    }
  }
}


void TouchChannel::setOctaveLed(int octave, LedState state, bool settingUILed /*false*/) {
  if (uiMode == DEFAULT_UI || settingUILed) {  // this is how you keep sequences going whilst "blocking" the sequence from changing any LEDs when a UI mode is active
    switch (state) {
      case LOW:
        io->setOnTime(OCTAVE_LED_PINS[octave], 0);     // reset any blinking state
        io->digitalWrite(OCTAVE_LED_PINS[octave], 1);
        break;
      case HIGH:
        io->setOnTime(OCTAVE_LED_PINS[octave], 0);     // reset any blinking state
        io->digitalWrite(OCTAVE_LED_PINS[octave], 0);
        break;
      case BLINK_ON:
        io->blinkLED(OCTAVE_LED_PINS[octave], 1, 1, 255, 0);
        break;
      case DIM_LOW:
        io->analogWrite(OCTAVE_LED_PINS[octave], 70);
        break;
    }
  }
}

void TouchChannel::updateLeds(uint8_t touched) {
  
}

void TouchChannel::updateOctaveLeds(int octave) {
  if (mode == MONO || mode == MONO_LOOP) {
    for (int i = 0; i < OCTAVE_COUNT; i++) {
      if (i == octave) {
        setOctaveLed(i, HIGH);
      } else {
        setOctaveLed(i, LOW);
      }
    }
  } else {
    for (int i = 0; i < OCTAVE_COUNT; i++) {
      if (bitRead(activeOctaves, i)) {
        setOctaveLed(i, HIGH);
      } else {
        setOctaveLed(i, LOW);
      }
    }
    
  }
}


/** ------------------------------------------------------------------------
 *        TRIGGER NOTE
---------------------------------------------------------------------------- */
 
void TouchChannel::triggerNote(int index, int octave, NoteState state, bool blinkLED /* false */) {
  
  int mappedIndex = DEGREE_INDEX_MAP[index] + DAC_OCTAVE_MAP[octave] + degrees->switchStates[index];

  switch (state) {
    case ON:
      if (mode == MONO || mode == MONO_LOOP) {
        setLed(currNoteIndex, LOW);  // set the 'previous' active note led LOW
        setLed(index, HIGH);         // new active note HIGH
      }
      if (blinkLED) setLed(index, BLINK_ON);
      
      prevOctave = currOctave;       // the following two lines of code used to be outside the switch block
      prevNoteIndex = currNoteIndex;
      currNoteIndex = index;
      currOctave = octave;
      setGate(HIGH);
      output1V.updateDAC(mappedIndex, 0);
      midi->sendNoteOn(channel, calculateMIDINoteValue(index, octave), 100);
      break;
    case SUSTAIN:
      prevOctave = currOctave;       // you might need to remove all this setter.
      prevNoteIndex = currNoteIndex; // you might need to remove all this setter.
      currNoteIndex = index;
      currOctave = octave;
      setLed(index, HIGH);
      output1V.updateDAC(mappedIndex, 0);
      midi->sendNoteOn(channel, calculateMIDINoteValue(index, octave), 100);
      break;
    case OFF:
      setGate(LOW);
      midi->sendNoteOff(channel, calculateMIDINoteValue(index, octave), 100);
      // wait_us(1);
      break;
    case PREV:
      setLed(index, HIGH);
      output1V.updateDAC(mappedIndex, 0);
      midi->sendNoteOn(channel, calculateMIDINoteValue(index, octave), 100);
      break;
  }
}

int TouchChannel::calculateMIDINoteValue(int index, int octave) {
  return MIDI_NOTE_MAP[index][degrees->switchStates[index]] + MIDI_OCTAVE_MAP[octave];
}




/**
 * FREEZE
 * takes boolean to either freeze or unfreeze
 * NOTE: a good way to freeze everything would be to just change the current mode to FREEZE, and then everything in the POLL fn would not execute.
*/ 
void TouchChannel::freeze(bool freeze) {
  this->freezeChannel = freeze;
}

/**
 * @brief reset the sequence
 * @todo you should probably get the currently queued event, see if it has been triggered yet, and disable it if it has been triggered
 * @todo you can't reset 
*/
void TouchChannel::resetSequence()
{
  sequence.reset();
  if (sequenceContainsEvents) {
    display->stepSequenceLED(this->channel, sequence.currStep, sequence.prevStep);
    handleSequence(sequence.currPosition);
  }
}

/**
 * The Global Gate Output that gets triggered by all channels.
*/  
void TouchChannel::setGlobalGate(bool state) {
  globalGateOut->write(state);
}

/**
 * sets the +5v gate output via GPIO into a Schmitt-Trigger which is inverted polarity
*/
void TouchChannel::setGate(bool state)
{
  gateState = !state;
  gateOut.write(gateState);
  this->setGlobalGate(gateState);
}

void TouchChannel::setRecordLED(LedState state) {
  if (state == HIGH)
  {
    io->analogWrite(CHANNEL_REC_LED, 20);
  }
  else
  {
    io->analogWrite(CHANNEL_REC_LED, 0);
  }
}

void TouchChannel::setRatchetLED(LedState state) {
  if (state == HIGH)
  {
    io->analogWrite(CHANNEL_RATCHET_LED, 255);
  }
  else
  {
    io->analogWrite(CHANNEL_RATCHET_LED, 0);
  }
}

void TouchChannel::setPitchBendLED(LedState state)
{
  if (state == HIGH)
  {
    io->analogWrite(CHANNEL_PB_LED, 255);
  }
  else
  {
    io->analogWrite(CHANNEL_PB_LED, 0);
  }
}

/**
 * apply the pitch bend by mapping the ADC value to a value between PB Range value and the current note being outputted
*/
void TouchChannel::benderActiveCallback(uint16_t value)
{
  switch (this->benderMode)
  {
  case BEND_OFF:
    // do nothing
    break;
  case PITCH_BEND:
    uint16_t bend;
    // Pitch Bend UP
    if (bender.currState == Bender::BEND_UP)
    {
      bend = output1V.calculatePitchBend(value, bender.zeroBend, bender.maxBend);
      output1V.setPitchBend(bend); // non-inverted
    }
    // Pitch Bend DOWN
    else if (bender.currState == Bender::BEND_DOWN)
    {
      bend = output1V.calculatePitchBend(value, bender.zeroBend, bender.minBend); // NOTE: inverted mapping
      output1V.setPitchBend(bend * -1);                                           // inverted
    }
    break;
  case RATCHET:
    break;
  case RATCHET_PITCH_BEND:
    break;
  case BEND_MENU:
    break;
  }
}

void TouchChannel::benderTriStateCallback(Bender::BendState state)
{
  switch (this->benderMode) {
    case BEND_OFF:
      break;
    case PITCH_BEND:
      break;
    case RATCHET:
      break;
    case RATCHET_PITCH_BEND:
      break;
    case BEND_MENU:
      if (state == Bender::BendState::BEND_UP) {
        sequence.setLength(sequence.length + 1);
        display->setSequenceLEDs(this->channel, sequence.length, true);
      }
      else if (state == Bender::BendState::BEND_DOWN) {
        display->setSequenceLEDs(this->channel, sequence.length, false);
        sequence.setLength(sequence.length - 1);
        display->setSequenceLEDs(this->channel, sequence.length, true);
      }
      break;
  }
}

void TouchChannel::benderIdleCallback() {
  switch (this->benderMode) {
    case BEND_OFF:
      break;
    case PITCH_BEND:
      break;
    case RATCHET:
      break;
    case RATCHET_PITCH_BEND:
      break;
    case BEND_MENU:
      // set var to no longer active?
      break;
  }
}

int TouchChannel::setBenderMode(BenderMode targetMode /*INCREMENT_BENDER_MODE*/)
{
  if (targetMode != INCREMENT_BENDER_MODE) {
    benderMode = targetMode;
  }
  else if (benderMode < 3)
  {
    benderMode += 1;
  }
  else
  {
    benderMode = 0;
  }
  switch (benderMode) {
    case BEND_OFF:
      setRatchetLED(LOW);
      setPitchBendLED(LOW);
      break;
    case PITCH_BEND:
      setRatchetLED(LOW);
      setPitchBendLED(HIGH);
      break;
    case RATCHET:
      setRatchetLED(HIGH);
      setPitchBendLED(LOW);
      break;
    case RATCHET_PITCH_BEND:
      setRatchetLED(HIGH);
      setPitchBendLED(HIGH);
      break;
    case BEND_MENU:
      display->setSequenceLEDs(channel, sequence.length, true);
      break;
  }
  return benderMode;
}