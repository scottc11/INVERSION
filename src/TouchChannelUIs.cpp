#include "TouchChannel.h"


void TouchChannel::enableUIMode(UIMode target) {
    switch (target) {
        case PB_RANGE_UI:
            uiMode = PB_RANGE_UI;
            for (int i = 0; i < 4; i++) setOctaveLed(i, LOW, true); // turn all octave leds OFF. Not used in this UI
            updatePitchBendRangeUI();
            break;
    }
}

void TouchChannel::disableUIMode()
{
    prevMode = mode;      // important
    switch (uiMode) {
        case PB_RANGE_UI:
            uiMode = DEFAULT_UI;
            setMode(mode);
            break;
    }
}


/** ------------------------------------------------------------------------
 *         PITCH BEND RANGE UI METHODS
---------------------------------------------------------------------------- */

/**
 * value: the last touched index (0..7)
*/ 
void TouchChannel::updatePitchBendRangeUI()
{
    setAllLeds(LOW); // reset
    for (int i = 0; i < output1V.pbRangeIndex + 1; i++)
    {
        setLed(i, HIGH, true);
    }
}