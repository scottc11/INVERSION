#ifndef __OK_TIMER
#define __OK_TIMER

/*

TIM_CLK / ((TIM_PSC+1) / (TIM_ARR + 1))

A single timer is referenced by using an instance of the C struct TIM_HandleTypeDef, which is defined in the following way:

typedef struct { 
    TIM_TypeDef *Instance;           // Pointer to timer descriptor
    TIM_Base_InitTypeDef Init;       // TIM Time Base required parameters
    HAL_TIM_ActiveChannel Channel;   // Active channel
    DMA_HandleTypeDef *hdma[7];      // DMA Handlers array
    HAL_LockTypeDef  Lock;           // Locking object
    __IO HAL_TIM_StateTypeDef State; // TIM operation state 
} TIM_HandleTypeDef;
*/

#include "mbed.h"
#include "OK_ErrorHandler.h"

class OK_Timer : OK_ErrorHandler {
public:
    enum IC_Channel {
        CHAN1 = TIM_CHANNEL_1,
        CHAN2 = TIM_CHANNEL_2,
        CHAN3 = TIM_CHANNEL_3,
        CHAN4 = TIM_CHANNEL_4
    };
    /**
     * @param tim_instance ex/ TIM1, TIM2..TIM8 etc.
     * @param prescaler devides against the timers clock source to determine how fast the timer will count
     * @param period    the MAX number timer will count before it resets back to 0
    */
    OK_Timer(TIM_TypeDef *tim_instance, uint16_t prescaler = 1, uint32_t period = 0xFFFFFFFF) : _TIM(tim_instance)
    {
        _prescaler = prescaler;
        _period = period;
    };

    static OK_Timer *_instance; // for HAL callback handlers

    void init();
    void initInputCaptureMode(GPIO_TypeDef *gpio, uint32_t pin, IC_Channel chan);
    void start();
    void startInputCapture();
    
    uint32_t read();
    uint32_t readIC();

    TIM_HandleTypeDef * getInstance();
    void attachPeriodElapsedCallback(Callback<void()> func);
    void attachInputCaptureCallback(Callback<void()> func);

    static void RoutePeriodElapsedCallback(TIM_HandleTypeDef *htim);
    static void RouteInputCaptureCallback(TIM_HandleTypeDef *htim);

    IC_Channel _IC_Chan;

private:

    TIM_HandleTypeDef _htim;
    TIM_TypeDef *_TIM;
    
    uint16_t _prescaler;
    uint32_t _period;

    Callback<void()> _periodElapsedCallback;
    Callback<void()> _inputCaptureCallback;
};


#endif