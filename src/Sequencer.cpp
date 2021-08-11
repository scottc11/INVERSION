#include "TouchChannel.h"

void TouchChannel::initSequencer()
{
    sequence.init();
    quantization = QUANT_8th;
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
                // Handle Sequence Overdubing
                if (sequence.overdub && position != sequence.newEventPos) // when a node is being created (touched degree has not yet been released), this flag gets set to true so that the sequence handler clears existing nodes
                {
                    // if new event overlaps succeeding events, clear those events
                    clearEvent(position);
                    
                    // NOTE may not need to do this
                    // if (events[sequence.getNextPosition(position)].active)
                    // {
                    //     clearEvent(sequence.getNextPosition(position));
                    // }
                }
                // Handle Sequence Events
                else
                {
                    if (events[position].gate == HIGH)
                    {
                        sequence.prevEventPos = position;                             // store position into variable
                        triggerNote(events[position].noteIndex, currOctave, ON); // trigger note ON
                    }
                    else
                    {
                        // CLEAN UP: if this 'active' LOW node does not match the last active HIGH node, delete it - it is a remnant of a previously deleted node
                        if (events[sequence.prevEventPos].noteIndex != events[position].noteIndex)
                        {
                            clearEvent(position);
                        }
                        else // set event.gate LOW
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
                if (sequence.overdub) {
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
        case QUANT_NONE:
            return pos;
        case QUANT_Quarter:
            return arr_find_closest_int(Q_QUARTER_NOTE, 2, pos);
        case QUANT_8th:
            return arr_find_closest_int(Q_EIGTH_NOTE, 3, pos);
        case QUANT_16th:
            return arr_find_closest_int(Q_SIXTEENTH_NOTE, 4, pos);
        case QUANT_32nd:
            return arr_find_closest_int(Q_THIRTY_SECOND_NOTE, 9, pos);
        case QUANT_64th:
            return arr_find_closest_int(Q_SIXTY_FOURTH_NOTE, 12, pos);
        case QUANT_128th:
            return arr_find_closest_int(Q_ONE_28TH_NOTE, 33, pos);
    }
    return pos;
};

void TouchChannel::createEvent(int position, int noteIndex, bool gate, Quantization quant)
{
    if (sequenceContainsEvents == false) { sequenceContainsEvents = true; }

    // handle quantization first, for overdubbing purposes
    position = (sequence.currStep * PPQN) + quantizePosition(position, quant);

    // if the previous event was a GATE HIGH event, re-position its succeeding GATE LOW event to the new events position - 1
    // NOTE: you will also have to trigger the GATE LOW, so that the new event will generate a trigger event
    // TODO: intsead of "position - 1", do "position - (quant / 2)"
    if (events[sequence.prevEventPos].gate == HIGH)
    {
        int newPosition = position == 0 ? sequence.lengthPPQN - 1 : position - 1;
        events[newPosition].noteIndex = events[sequence.prevEventPos].noteIndex;
        events[newPosition].gate = LOW;
        events[newPosition].active = true;
        sequence.prevEventPos = newPosition; // pretend like this event got executed
    }

    // if this new event is a GATE LOW event, and after quantization there exists an event at the same position in the sequence,
    // move this new event to the next available position @ the curr quantize level divided by 2 (to not interfere with next event), which will also have to be checked for any existing events.
    // If there is an existing GATE HIGH event at the next position, this new event will have to be placed right before it executes, regardless of quantization
    if (gate == LOW && events[position].active && events[position].gate == HIGH) {
        position = position + (quant / 2);
    }

    sequence.newEventPos = position; // store all new events position

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