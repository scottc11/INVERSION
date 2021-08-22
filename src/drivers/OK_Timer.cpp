#include "OK_Timer.h"

OK_Timer *OK_Timer::_instance = NULL;

void OK_Timer::init()
{
    // Enable the HAL TIM peripheral
    if (_TIM == TIM2)
    {
        __HAL_RCC_TIM2_CLK_ENABLE();
        HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM2_IRQn);
    }

    _htim.Instance = const_cast<TIM_TypeDef *>(_TIM); /* Pointer to timer descriptor  */
    _htim.Init.Prescaler = _prescaler;
    _htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    _htim.Init.Period = _period;
    _htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    _htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&_htim) != HAL_OK)
    {
        InitError("HAL_TIM_Base_Init");
    }

    // The external Clock can be configured, if needed (the default clock is the internal clock from the APBx),
    // using the following function: HAL_TIM_ConfigClockSource, the clock configuration should be done before any start function.
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;

    if (HAL_TIM_ConfigClockSource(&_htim, &sClockSourceConfig) != HAL_OK)
    {
        InitError("HAL_TIM_ConfigClockSource");
    }

    TIM_MasterConfigTypeDef sMasterConfig = {0};
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&_htim, &sMasterConfig) != HAL_OK)
    {
        InitError("HAL_TIMEx_MasterConfigSynchronization");
    }

    // set static var to this instance
    _instance = this;

}

void OK_Timer::initInputCaptureMode(GPIO_TypeDef *gpio, uint32_t pin, IC_Channel chan)
{
    if (HAL_TIM_IC_Init(&_htim) != HAL_OK)
    {
        InitError("HAL_TIM_IC_Init");
    }

    this->_IC_Chan = chan;

    TIM_IC_InitTypeDef sConfigIC = {0};
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 4;
    if (HAL_TIM_IC_ConfigChannel(&_htim, &sConfigIC, chan) != HAL_OK)
    {
        InitError("HAL_TIM_IC_ConfigChannel");
    }

    // configure a gpio pin based for input signal
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2; // specifies which peripheral to associate to the pin.
    HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

TIM_HandleTypeDef * OK_Timer::getInstance()
{
    return &_htim;
}

void OK_Timer::start()
{
    HAL_TIM_Base_Start_IT(&_htim);
}

void OK_Timer::startInputCapture() {
    HAL_TIM_IC_Start_IT(&_htim, _IC_Chan);
}

/**
 * @brief read the timers count register
*/
uint32_t OK_Timer::read()
{
    return __HAL_TIM_GET_COUNTER(&_htim);
}

/**
 * @brief read the current Input Capture register
*/
uint32_t OK_Timer::readIC()
{
    return __HAL_TIM_GET_COMPARE(&_htim, _IC_Chan);
}

/**
 * @brief For routing HAL interupt functions into class member
*/
void OK_Timer::RoutePeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == _instance->getInstance()->Instance)
    {
        if (_instance->_periodElapsedCallback) {
            _instance->_periodElapsedCallback();
        }
    }
}

/**
 * @brief sets the callback function that gets called every time the timer counter overflows
*/
void OK_Timer::attachPeriodElapsedCallback(Callback<void()> func)
{
    _periodElapsedCallback = func;
}

/**
 * @brief For routing HAL interupt functions into class member
*/
void OK_Timer::RouteInputCaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == _instance->_TIM) {
        
        __HAL_TIM_SET_COUNTER(htim, 0); // reset counter back to zero after every compare
        
        if (_instance->_inputCaptureCallback) {
            _instance->_inputCaptureCallback();
        }
    }
}

/**
 * @brief sets the callback function that gets called at the end of every input capture event
*/ 
void OK_Timer::attachInputCaptureCallback(Callback<void()> func)
{
    _inputCaptureCallback = func;
}

/**
  * @brief This function handles TIM2 global interrupt.
*/
extern "C" void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(OK_Timer::_instance->getInstance());
}

/**
 * @brief This callback is automatically called by the HAL on the UEV event
*/
extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    OK_Timer::RoutePeriodElapsedCallback(htim);
}


extern "C" void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    OK_Timer::RouteInputCaptureCallback(htim);
}