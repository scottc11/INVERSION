#ifndef ADC_TASK_H__
#define ADC_TASK_H__

#include "mbed.h"
#include "ADC_DMA.h"

class ADCTask
{
public:

    ADCTask(ADC_DMA *adc_dma, DigitalOut *led_dir);

    void Run();
    
private:

    ADC_DMA *_adc_dma;
    DigitalOut *_led_dir;
};

#endif // ADC_TASK_H__