#include "TouchChannel.h"


void TouchChannel::enableUIMode(UIMode target) {
    switch (target) {
        case LOOP_LENGTH_UI:
            uiMode = LOOP_LENGTH_UI;
            updateLoopLengthUI();
            break;
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
        case LOOP_LENGTH_UI:
            uiMode = DEFAULT_UI;
            setMode(mode);
            break;
        case PB_RANGE_UI:
            uiMode = DEFAULT_UI;
            setMode(mode);
            break;
    }
}

/** ------------------------------------------------------------------------
 *         LOOP UI METHODS
---------------------------------------------------------------------------- */

void TouchChannel::updateLoopLengthUI()
{
    setAllLeds(LOW); // reset
    updateLoopMultiplierLeds();
    for (int i = 0; i < numLoopSteps; i++)
    {
        if (i == currStep)
        {
            setLed(i, BLINK_ON, true);
        }
        else
        {
            setLed(i, HIGH, true);
        }
    }
}

void TouchChannel::handleLoopLengthUI()
{
    // take current clock step and flash the corrosponding channel LED and Octave LED
    if (currTick == 0)
    {
        int modulo = currStep % numLoopSteps;

        if (modulo != 0)
        { // setting the previous LED back to normal
            setLed(modulo - 1, HIGH, true);
        }
        else
        { // when modulo rolls past 7 and back to 0
            setLed(numLoopSteps - 1, HIGH, true);
        }

        setLed(modulo, BLINK_ON, true);

        for (int i = 0; i < loopMultiplier; i++)
        {
            if (currStep < (numLoopSteps * (i + 1)) && currStep >= (numLoopSteps * i))
            {
                setOctaveLed(i, BLINK_ON, true);
            }
            else
            {
                setOctaveLed(i, HIGH, true);
            }
        }
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