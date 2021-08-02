#include "DegreeDisplay.h"

void DegreeDisplay::init()
{
    ledMatrix.init();
    for (int i = 0; i < 64; i++)
    {
        ledMatrix.setPWM(i, 127);
        if (i != 0)
        {
            ledMatrix.setPWM(i - 1, 0);
        }
        wait_ms(10);
    }
    ledMatrix.setPWM(63, 0);
}

void DegreeDisplay::clear()
{
    for (int i = 0; i < 64; i++)
    {
        ledMatrix.setPWM(i, 0);
    }
}

void DegreeDisplay::setSequenceLEDs(int chan, int length, bool on)
{
    // illuminate each channels sequence length
    for (int i = 0; i < length; i++)
    {
        ledMatrix.setPWM(CHAN_DISPLAY_LED_MAP[chan][i], on ? OK_PWM_MID : 0);
    }
}

void DegreeDisplay::stepSequenceLED(int chan, int currStep, int prevStep) {
    // set currStep PWM High
    ledMatrix.setPWM(CHAN_DISPLAY_LED_MAP[chan][currStep], OK_PWM_HIGH);
    // set prevStep PWM back to Mid
    ledMatrix.setPWM(CHAN_DISPLAY_LED_MAP[chan][prevStep], OK_PWM_MID);
}