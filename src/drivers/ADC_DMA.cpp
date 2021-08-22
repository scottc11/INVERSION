#include "ADC_DMA.h"

ADC_DMA *ADC_DMA::_instances[NUM_ADC_DMA_INSTANCES] = {0};

ADC_DMA::ADC_DMA(
    ADC_TypeDef *adc_instance,
    uint32_t adc_channel,
    GPIO_TypeDef *adc_pin_port,
    uint16_t adc_pin,
    DMA_Stream_TypeDef *dma_instance,
    IRQn_Type dma_stream_irqn,
    uint32_t dma_channel,
    TIM_TypeDef *tim_instance
    ) : 
    _ADC(adc_instance),
    _ADC_CHANNEL(adc_channel),
    _ADC_PIN_PORT(adc_pin_port),
    _ADC_PIN(adc_pin),
    _DMA(dma_instance),
    _DMA_STREAM_IRQn(dma_stream_irqn),
    _DMA_CHANNEL(dma_channel),
    _TIM(tim_instance)
{
    _direction = false;

    // Add constructed instance to the static list of instances
    // This is only required for IRQ routing
    for (auto ins : _instances)
    {
        if (ins == NULL)
        {
            ins = this;
            break;
        }
    }
}

void ADC_DMA::RouteIRQ_DMA_Stream_IRQ(DMA_Stream_TypeDef *dma)
{
    for (auto ins: _instances)
    {
        if (ins && ins->_hdma.Instance == dma)
        {
            HAL_DMA_IRQHandler(&(ins->_hdma));
        }
    }    
}

void ADC_DMA::RouteCB_ConversionComplete(ADC_HandleTypeDef *hadc)
{
    for (auto ins: _instances)
    {
        if (ins && ins->_hadc.Instance == hadc->Instance)
        {
            ins->CB_DMAConversionComplete();
        }
    }
}

void ADC_DMA::RouteCB_ConversionHalfComplete(ADC_HandleTypeDef *hadc)
{
    for (auto ins: _instances)
    {
        if (ins && ins->_hadc.Instance == hadc->Instance)
        {
            ins->CB_DMAConversionHalfComplete();
        }
    }
}

void ADC_DMA::Init()
{
    DMA_Init();
    ADC_Init();
    TIM_Init();
    GPIO_Init();
}

void ADC_DMA::Start()
{
    HAL_TIM_Base_Start(&_htim);
    HAL_ADC_Start_DMA(&_hadc, (uint32_t *)_dma_buffer, ADC_DMA_BUFSIZE);
}

void ADC_DMA::ADC_Init()
{
    __HAL_RCC_ADC1_CLK_ENABLE();

    _hadc.Instance = const_cast<ADC_TypeDef *>(_ADC);
    _hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
    _hadc.Init.Resolution = ADC_RESOLUTION_12B;
    _hadc.Init.ScanConvMode = ENABLE;
    _hadc.Init.ContinuousConvMode = DISABLE;
    _hadc.Init.DiscontinuousConvMode = DISABLE;
    _hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    _hadc.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
    _hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    _hadc.Init.NbrOfConversion = 1;
    _hadc.Init.DMAContinuousRequests = ENABLE;
    _hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    if (HAL_ADC_Init(&_hadc) != HAL_OK)
    {
        InitError("HAL_ADC_Init");
    }   

    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = _ADC_CHANNEL;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    if (HAL_ADC_ConfigChannel(&_hadc, &sConfig) != HAL_OK)
    {
        InitError("HAL_ADC_ConfigChannel");
    }
}

void ADC_DMA::DMA_Init()
{
    __HAL_RCC_DMA2_CLK_ENABLE();

    HAL_NVIC_SetPriority(_DMA_STREAM_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(_DMA_STREAM_IRQn);

    _hdma.Instance = const_cast<DMA_Stream_TypeDef *>(_DMA);
    _hdma.Init.Channel = _DMA_CHANNEL;
    _hdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
    _hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    _hdma.Init.MemInc = DMA_MINC_ENABLE;
    _hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    _hdma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    _hdma.Init.Mode = DMA_CIRCULAR;
    _hdma.Init.Priority = DMA_PRIORITY_LOW;
    _hdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&_hdma) != HAL_OK)
    {
        InitError("HAL_DMA_Init");;
    }

    __HAL_LINKDMA((&_hadc),DMA_Handle,_hdma);
}

void ADC_DMA::TIM_Init()
{
    __HAL_RCC_TIM2_CLK_ENABLE();

    _htim.Instance = const_cast<TIM_TypeDef *>(_TIM);
    _htim.Init.Prescaler = 1;
    _htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    _htim.Init.Period = 2000;
    _htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    _htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&_htim) != HAL_OK)
    {
        InitError("HAL_TIM_Base_Init");
    }

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&_htim, &sClockSourceConfig) != HAL_OK)
    {
        InitError("HAL_TIM_ConfigClockSource");
    }

    TIM_MasterConfigTypeDef sMasterConfig = {0};
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC1;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&_htim, &sMasterConfig) != HAL_OK)
    {
        InitError("HAL_TIMEx_MasterConfigSynchronization");
    }
}

void ADC_DMA::GPIO_Init()
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = _ADC_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(
        const_cast<GPIO_TypeDef *>(_ADC_PIN_PORT),
        &GPIO_InitStruct
    );
}

void ADC_DMA::InitError(const char *err)
{
#if DEBUG
    printf("*** ADC_DMA init failed: %s", err);
#endif
    mbed_die();
}

void ADC_DMA::IRQ_DMA_Stream_IRQ()
{
    HAL_DMA_IRQHandler(&_hdma);
}

// These are the irqs and callbacks for the class instances
// The way these get called are through the static router functions
// which handle decicding which instance to call from the interrupts
//
// HAL_ADC_ConvCpltCallback (called from CubeMX HAL IRQ)
//  |
//  +--> RouteCB_ConversionComplete (static so can be called from C function)
//        |
//        +--> CB_DMAConversionComplete() (for instance corresponding to the ADC that triggered the interrupt)
void ADC_DMA::CB_DMAConversionComplete()
{
    // buffer values from index (ADC_DMA_BUFSIZE / 2)..ADC_DMA_BUFSIZE are ready
    uint32_t sum = 0;
    uint16_t avg = 0;
    for (uint32_t i = 0; i < ADC_DMA_BUFSIZE; i++)
    {
        sum += _dma_buffer[i];
    }
    avg = sum / ADC_DMA_BUFSIZE;

    _direction = (avg > 4000);
}

void ADC_DMA::CB_DMAConversionHalfComplete()
{
    // TODO
}

extern "C" void DMA2_Stream0_IRQHandler()
{
    ADC_DMA::RouteIRQ_DMA_Stream_IRQ(DMA2_Stream0);
}

extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    ADC_DMA::RouteCB_ConversionComplete(hadc);
}

extern "C" void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    ADC_DMA::RouteCB_ConversionHalfComplete(hadc);
}