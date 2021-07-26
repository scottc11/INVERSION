#include "TouchChannel.h"

void TouchChannel::initSequencer()
{
    numLoopSteps = DEFAULT_SEQ_LENGTH;
    loopMultiplier = 1;
    timeQuantizationMode = QUANT_NONE;
    currStep = 0;
    currTick = 0;
    currPosition = 0;
    setLoopTotalSteps();
    setLoopTotalPPQN();
    clearEventSequence(); // initialize values in sequence array
}

/**
 * Sequence Handler which gets called during polling / every clocking event
*/ 
void TouchChannel::handleSequence(int position)
{
    switch (mode) {
        case MONO_LOOP:
            if (events[position].active)
            {
                if (clearExistingNodes) // when a node is being created (touched degree has not yet been released), this flag gets set to true so that the sequence handler clears existing nodes
                {
                    if (events[prevNodePosition].gate == HIGH) // if previous event overlaps new event
                    {
                        int newPosition = position == 0 ? totalPPQN : position - 1;
                        createEvent(newPosition - 1, events[prevNodePosition].noteIndex, LOW); // create a copy of event with gate == LOW @ currPos - 1
                    }
                    if (events[position].active) // if new event overlaps succeeding events, overwrite those events
                    {
                        clearEvent(position);
                    }
                }
                else
                {
                    if (events[position].gate == HIGH)
                    {
                        prevNodePosition = position;                             // store position into variable
                        triggerNote(events[position].noteIndex, currOctave, ON); // trigger note ON
                    }
                    else
                    {
                        if (events[prevNodePosition].noteIndex != events[position].noteIndex)
                        {
                            clearEvent(position); // cleanup: if this 'active' LOW node does not match the last active HIGH node, delete it - it is a remnant of a previously deleted node
                        }
                        else // set node.gate LOW
                        {
                            prevNodePosition = position;                              // store position into variable
                            triggerNote(events[position].noteIndex, currOctave, OFF); // trigger note OFF
                        }
                    }
                }
            }
            break;
        case QUANTIZE_LOOP:
            if (events[position].active) {
                if (clearExistingNodes) {
                    clearEvent(position);
                } else {
                    setActiveDegrees(events[position].activeNotes);
                }
                
            }
            break;
    }

    triggerNote(currNoteIndex, currOctave, BEND_PITCH); // always handle pitch bend value
}

void TouchChannel::clearEventSequence()
{
    // deactivate all events in list
    for (int i = 0; i < PPQN * MAX_SEQ_LENGTH; i++)
    {
        clearEvent(i);
    }
    sequenceContainsEvents = false; // after deactivating all events in list, set this flag to false
};

void TouchChannel::clearPitchBendSequence()
{
    // deactivate all events in list
    for (int i = 0; i < PPQN * MAX_SEQ_LENGTH; i++)
    {
        // events[i].pitchBend = pbZero;
    }
};

void TouchChannel::createEvent(int position, int noteIndex, bool gate)
{
    if (sequenceContainsEvents == false) { sequenceContainsEvents = true; }

    events[position].noteIndex = noteIndex;
    events[position].gate = gate;
    events[position].active = true;
};

void TouchChannel::createPitchBendEvent(int position, uint16_t pitchBend) {
    if (sequenceContainsEvents == false) { sequenceContainsEvents = true; }

    events[position].pitchBend = pitchBend;
}

void TouchChannel::clearEvent(int position)
{
    events[position].noteIndex = NULL_NOTE_INDEX;
    events[position].active = false;
    events[position].gate = LOW;
    // events[position].pitchBend = pbZero;
}

void TouchChannel::createChordEvent(int position, uint8_t notes)
{

    if (sequenceContainsEvents == false)
    {
        sequenceContainsEvents = true;
    }

    events[position].activeNotes = notes;
    events[position].active = true;
};