/**
 * ============================================ 
 * ------------ QUANTIZATION ------------------
 * 
 * determin which degreese are active / inactive
 * apply the number of active degrees to numActiveDegrees variable
 * read the voltage on analog input pin
 * divide the maximum possible value from analog input pin by the number numActiveDegrees
 * map this value to an array[8]
 * map voltage input values (num between 0 and 1023)
*/

#include "TouchChannel.h"

#define CV_MAX     65535

// Init quantizer defaults
void TouchChannel::initQuantizer() {
  activeDegrees = 0xFF;
  activeOctaves = 0xF;
  numActiveDegrees = DEGREE_COUNT;
  numActiveOctaves = OCTAVE_COUNT;

  this->setActiveDegrees(activeDegrees);
  this->setActiveOctaves(OCTAVE_COUNT);
}

void TouchChannel::setActiveDegreeLimit(int value) {
  activeDegreeLimit = value;
}



void TouchChannel::handleCVInput()
{
  currCVInputValue = CV_MAX - cvInput.read_u16(); // NOTE: CV voltage input is inverted, so everything needs to be flipped to make more sense

  if (gateState == HIGH) setGate(LOW); // We only want trigger events in quantizer mode, so if the gate gets set HIGH, make sure to set it back to low the very next tick

  if (currCVInputValue >= prevCVInputValue + CV_QUANT_BUFFER || currCVInputValue <= prevCVInputValue - CV_QUANT_BUFFER)
  {
    int refinedValue = 0;  // we want a number between 0 and CV_OCTAVE for mapping to degrees. The octave is added afterwords via CV_OCTAVES
    int octave = 0;

    // determin which octave the CV value will get mapped to
    for (int i = 0; i < numActiveOctaves; i++)
    {
      if (currCVInputValue < activeOctaveValues[i].threshold)
      {
        octave = activeOctaveValues[i].octave;
        refinedValue = i == 0 ? currCVInputValue : currCVInputValue - activeOctaveValues[i - 1].threshold; // remap adc value to a number between 0 and octaveThreshold
        break;
      }
    }

    // latch incoming ADC value to DAC value
    for (int i = 0; i < numActiveDegrees; i++)
    {
      if (refinedValue < activeDegreeValues[i].threshold)
      { // break from loop as soon as we can
        if (prevNoteIndex != activeDegreeValues[i].noteIndex || prevOctave != octave)
        { // catch duplicate triggering of that same note.
          this->triggerNote(prevNoteIndex, prevOctave, OFF);
          if (bitRead(activeDegrees, prevNoteIndex))
          { // if prevNote still active, its led needs to be set from dimmed back fully ON
            setLed(prevNoteIndex, HIGH);
          }
          this->triggerNote(activeDegreeValues[i].noteIndex, octave, ON, true);

          if (bitRead(activeOctaves, prevOctave))
          {
            this->setOctaveLed(prevOctave, LedState::HIGH);
          }
          this->setOctaveLed(octave, LedState::BLINK_ON);
        }
        break;
      }
    }
  }
  prevCVInputValue = currCVInputValue;
}

/**
 * when a channels degree is touched, toggle the active/inactive status of the 
 * touched degree by flipping the bit of the given index that was touched
*/
void TouchChannel::setActiveDegrees(int degrees) {
  // activeDegrees = bitFlip(activeDegrees, index); // this did not work for some reason....
  activeDegrees = degrees;
  
  // determine the number of active degrees
  numActiveDegrees = 0;
  for (int i = 0; i < 8; i++) {
    if (bitRead(activeDegrees, i)) {
      activeDegreeValues[numActiveDegrees].noteIndex = i;
      numActiveDegrees += 1;
    }
  }
  
  int octaveThreshold = CV_MAX / numActiveOctaves;        // divide max ADC value by num octaves
  int min_threshold = octaveThreshold / numActiveDegrees; // then numActiveDegrees
  
  // for each active octave, generate an ADC threshold for mapping ADC values to DAC octave values
  for (int i = 0; i < numActiveOctaves; i++) {
    activeOctaveValues[i].threshold = octaveThreshold * (i + 1); // can't multiply by zero
  }
  

  // for each active degree, generate a ADC 'threshold' for latching CV input values
  for (int i = 0; i < numActiveDegrees; i++) {
    activeDegreeValues[i].threshold = min_threshold * (i + 1); // can't multiply by zero
  }
}

/**
 * take the newly touched octave, and either add it or remove it from the activeOctaves list
*/
void TouchChannel::setActiveOctaves(int octave)
{  
  if (bitFlip(activeOctaves, octave) != 0) // one octave must always remain active.
  {
    activeOctaves = bitFlip(activeOctaves, octave);
    
    // determine the number of active octaves (required for calculating thresholds)
    numActiveOctaves = 0;
    for (int i = 0; i < OCTAVE_COUNT; i++)
    {
      if (bitRead(activeOctaves, i)) {
        activeOctaveValues[numActiveOctaves].octave = i;
        numActiveOctaves += 1;
      }
    }
    
    // update active degrees thresholds
    this->setActiveDegrees(activeDegrees);
  }
  
}

void TouchChannel::updateActiveDegreeLeds(uint8_t degrees) {
  for (int i = 0; i < 8; i++) {
    if (bitRead(degrees, i)) {
      setLed(i, HIGH);
    } else {
      setLed(i, LOW);
    }
  }
}