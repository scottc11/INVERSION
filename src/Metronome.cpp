#include "Metronome.h"

// clock initialization
void Metronome::init() {
  tempoOutput.write(0);
  this->updateTempo(tickInterval);
  this->pollTempoPot();
}

void Metronome::poll() {
  this->pollTempoPot();
}

void Metronome::pollTempoPot() {
  newTempoPotValue = tempoPot.read_u16();
  int debounce = 800;
  if (newTempoPotValue != oldTempoPotValue) {
    if (newTempoPotValue > oldTempoPotValue + debounce || newTempoPotValue < oldTempoPotValue - debounce)
    {
      int index = newTempoPotValue / (65535 / (BPM_MAX - BPM_MIN));
      this->updateTempo(usTempoMap[index]);
      oldTempoPotValue = newTempoPotValue;
    }
  }
}

void Metronome::updateTempo(int us) {
  tickInterval = us / 2; // tempo value is currently based on quarter notes, so divide by two here to get 8th notes for better clock output speeds
  ticker.attach_us(callback(this, &Metronome::tick), tickInterval);
}

void Metronome::tick() {
  if (currTick == 1) {
    tempoLed.write(1);
    tempoOutput.write(1);
  } else {
    tempoLed.write(0);
    tempoOutput.write(0);
  }
  
  currTick += 1;
  position += 1;

  // if a callback function exists, call it now
  if (callbackFn) {
    callbackFn();
  }

  if (currTick > ticksPerStep) {
    this->step();
  }
}

void Metronome::step() {
  currTick = 1;
  currStep += 1;
  
  // reset step count / reset loop
  if (currStep > numSteps) {
    currStep = 1;
    position = 1;
  }
}

void Metronome::start() {
  ticker.attach_us(callback(this, &Metronome::tick), tickInterval);
}

void Metronome::stop() {
  ticker.detach();
}

void Metronome::attachTickCallback(Callback<void()> func) {
  callbackFn = func;
}

void Metronome::setNumberOfSteps(int num) {
  numSteps = num;
}