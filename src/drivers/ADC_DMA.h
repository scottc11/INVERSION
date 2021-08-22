#ifndef ADC_DMA_H__
#define ADC_DMA_H__

#include "mbed.h"

#define NUM_ADC_DMA_INSTANCES 1

// This could be made better by templating it into ADC_DMA
// but not really needed to hardcode for just one instance
#define ADC_DMA_BUFSIZE 2048 


class ADC_DMA 
{
public:

    ADC_DMA(
        ADC_TypeDef *adc_instance,
        uint32_t adc_channel,
        GPIO_TypeDef *adc_pin_port,
        uint16_t adc_pin,
        DMA_Stream_TypeDef *dma_instance,
        IRQn_Type dma_stream_irqn,
        uint32_t dma_channel,
        TIM_TypeDef *tim_instance
    );

    void Init();

    void Start();

    bool GetDirection() const { return _direction; }

    static void RouteIRQ_DMA_Stream_IRQ(DMA_Stream_TypeDef *dma);
    static void RouteCB_ConversionComplete(ADC_HandleTypeDef *hadc);
    static void RouteCB_ConversionHalfComplete(ADC_HandleTypeDef *hadc);

private:

    void ADC_Init();
    void DMA_Init();
    void TIM_Init();
    void GPIO_Init();

    void InitError(const char *err);

    void IRQ_DMA_Stream_IRQ();
    void CB_DMAConversionComplete();
    void CB_DMAConversionHalfComplete();

    ADC_HandleTypeDef _hadc;
    DMA_HandleTypeDef _hdma;
    TIM_HandleTypeDef _htim;

    volatile bool _direction;

    volatile uint16_t _dma_buffer[ADC_DMA_BUFSIZE];

    static ADC_DMA *_instances[NUM_ADC_DMA_INSTANCES];

    const ADC_TypeDef *_ADC;
    const uint32_t _ADC_CHANNEL;
    const GPIO_TypeDef *_ADC_PIN_PORT;
    const uint16_t _ADC_PIN;

    const DMA_Stream_TypeDef *_DMA;
    const IRQn_Type _DMA_STREAM_IRQn;
    const uint32_t _DMA_CHANNEL;
    
    const TIM_TypeDef *_TIM;
};

#endif