#ifndef __SUPER_SEQ_H
#define __SUPER_SEQ_H

#include "main.h"

class SuperSeq {
public:

    SuperSeq() {};

    int length;           // how many steps the sequence contains
    int lengthPPQN;       // how many PPQN the sequence contains
    int currStep;         // current sequence step
    int prevStep;         // the previous step executed in the sequence
    int currPosition;     // current
    int prevPosition;
    int prevEventPos;     // represents the position of the last event which got triggered (either HIGH or LOW)

    void init() {
        this->setLength(DEFAULT_SEQ_LENGTH);
    };

    void reset() {
        currPosition = 0;
        currStep = 0;
    };
    
    void setLength(int steps) {
        if (steps <= MAX_SEQ_LENGTH) {
            length = steps;
            lengthPPQN = length * PPQN;
        }
    };

    int getLength();

    /**
     * @brief advance the sequencer position by 1
     * @note you cant make any I2C calls in these functions, you must defer them to a seperate thread to be executed later
    */
    void advance() {
        prevPosition = currPosition;
        currPosition += 1;

        if (currPosition % PPQN == 0) {
            prevStep = currStep;
            currStep += 1;
            // isrSafeCallback() ?
        }

        if (currPosition > lengthPPQN - 1) {
            currPosition = 0;
            currStep = 0;
        }
    }
    
    /**
     * @brief advance the sequencer to a specific step
    */ 
    void advanceToStep(int step);
};

#endif