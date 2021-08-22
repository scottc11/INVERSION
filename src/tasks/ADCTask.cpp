#include "ADCTask.h"

ADCTask::ADCTask(ADC_DMA *adc_dma, DigitalOut *led_dir)
{
    _adc_dma = adc_dma;
    _led_dir = led_dir;
}

// Thread entrypoint
void ADCTask::Run()
{
    _adc_dma->Start();

    while (1)
    {
        _led_dir->write(_adc_dma->GetDirection());
        // _led_dir->write(!_led_dir->read());
        // ThisThread::sleep_for(100ms);
    }
}
