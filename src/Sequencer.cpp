#include "TouchChannel.h"

void TouchChannel::initSequencer()
{
    sequence.init();
    timeQuantizationMode = QUANT_NONE;
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
                    if (events[sequence.prevEventPos].gate == HIGH) // if previous event overlaps new event
                    {
                        int newPosition = position == 0 ? sequence.lengthPPQN - 1 : position - 1;
                        createEvent(newPosition, events[sequence.prevEventPos].noteIndex, LOW, quantization); // create a copy of event with gate == LOW @ currPos - 1
                        sequence.prevEventPos = newPosition;
                    }
                    // if new event overlaps succeeding events, overwrite those events
                    // NOTE may not need to do this
                    if (events[sequence.getNextPosition(position)].active)
                    {
                        clearEvent(sequence.getNextPosition(position));
                    }
                }
                else
                {
                    if (events[position].gate == HIGH)
                    {
                        sequence.prevEventPos = position;                             // store position into variable
                        triggerNote(events[position].noteIndex, currOctave, ON); // trigger note ON
                    }
                    else
                    {
                        if (events[sequence.prevEventPos].noteIndex != events[position].noteIndex)
                        {
                            clearEvent(position); // cleanup: if this 'active' LOW node does not match the last active HIGH node, delete it - it is a remnant of a previously deleted node
                        }
                        else // set node.gate LOW
                        {
                            sequence.prevEventPos = position;                              // store position into variable
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

int Q_QUARTER_NOTE[2] = { 0, 96 };
int Q_EIGTH_NOTE[3] = { 0, 48, 96 };
int Q_SIXTEENTH_NOTE[4] = { 0, 24, 48, 96 };
int Q_THIRTY_SECOND_NOTE[9] = { 0, 12, 24, 36, 48, 60, 72, 84, 96 };
int Q_SIXTY_FOURTH_NOTE[12] = { 0, 6, 12, 18, 24, 30, 36, 72, 78, 84, 90, 96 };
int Q_ONE_28TH_NOTE[33] = { 0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48, 51, 54, 57, 60, 63, 66, 69, 72, 75, 78, 81, 84, 87, 90, 93, 96 };

int TouchChannel::quantizePosition(int pos, TouchChannel::Quantization target) {
    switch (target) {
        case QUARTER:
            return arr_find_closest_int(Q_QUARTER_NOTE, 2, pos);
        case EIGTH:
            return arr_find_closest_int(Q_EIGTH_NOTE, 3, pos);
        case SIXTEENTH:
            return arr_find_closest_int(Q_SIXTEENTH_NOTE, 4, pos);
        case THIRTY_SECOND:
            return arr_find_closest_int(Q_THIRTY_SECOND_NOTE, 9, pos);
        case SIXTY_FOURTH:
            return arr_find_closest_int(Q_SIXTY_FOURTH_NOTE, 12, pos);
        case ONE_28TH:
            return arr_find_closest_int(Q_ONE_28TH_NOTE, 33, pos);
    }
};

void TouchChannel::createEvent(int position, int noteIndex, bool gate, Quantization quant)
{
    if (sequenceContainsEvents == false) { sequenceContainsEvents = true; }

    // volatile int superQuant = quantizePosition(position, quant);
    // volatile int minorQuant = sequence.currStep * PPQN;
    // position = minorQuant + superQuant;

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