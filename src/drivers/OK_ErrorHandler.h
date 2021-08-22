#ifndef __OK_ERROR_HANDLER
#define __OK_ERROR_HANDLER

#include "mbed.h"

class OK_ErrorHandler {
public:
    OK_ErrorHandler(){};

    void InitError(const char *err)
    {
    #if DEBUG
        printf("*** init failed: %s", err);
    #endif
        mbed_die();
    }
};

#endif