/*
* === 6-channel ADC with DMA for STM32F030 ===
*/

uint16_t adc_buf[6];							// here DMA stores the ADC values

void GCL_ADC_Init(void){
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	ADC1->CFGR2 |= ADC_CFGR2_CKMODE_1;				// (F0) PCLK divided by 4
	ADC1->CFGR1 &= ~ADC_CFGR1_DISCEN;				// (F0) 0: Discontinuous mode disabled
	ADC1->CFGR1 &= ~ADC_CFGR1_RES;				// (F0) resolution 12 bit
	ADC1->CFGR1 |= ADC_CFGR1_CONT;				// (F0) 1: Continuous con
	
	// sampling time for channels
	ADC1->SMPR |= ADC_SMPR_SMP_2|ADC_SMPR_SMP_1|ADC_SMPR_SMP_0;				// (F0) sampling 239.5 ADC clock cycles

	// Set GPIOA pins as Analog inputs
	GPIOA->MODER |= (3<<0);					// PA0 enable as Analog input
	GPIOA->MODER |= (3<<2);					// PA1 enable as Analog input
	GPIOA->MODER |= (3<<4);					// PA2 enable as Analog input
	GPIOA->MODER |= (3<<6);					// PA3 enable as Analog input
	GPIOA->MODER |= (3<<8);					// PA4 enable as Analog input
	GPIOA->MODER |= (3<<10);					// PA5 enable as Analog input

	// Enable DMA for ADC
	ADC1->CFGR1 |= ADC_CFGR1_DMAEN;				// (F0) DMA mode enabled
	ADC1->CFGR1 |= ADC_CFGR1_DMACFG;				// (F0) DMA circular mode
	ADC1->CHSELR |= 31;						// (F0) channels 0 to 5 selected
}

void GCL_ADC_Enable(void){
	ADC1->ISR |= ADC_ISR_ADRDY;					// 1. Clear the ADRDY bit in ADC_ISR register by programming this bit to 1.
	ADC1->CR |= ADC_CR_ADEN;					// 2. Set ADEN=1 in the ADC_CR register.
	uint32_t del = 10000;
	while (del--);							// wait about 10us ADC to stabilize
}

void GCL_ADC_Start(void){
	ADC1->ISR = 0;							// clear Interrupt and Status Register
	ADC1->CR |= ADC_CR_ADSTART;					// start conversion by software
}

void GCL_ADC_DMA_Init(void){
	RCC->AHBENR |= RCC_AHBENR_DMAEN;				// (F0) Enable DMA2 clock

	DMA1_Channel1->CCR &= ~(DMA_CCR_DIR);		// 0: data transfer direction: read from peripheral
	DMA1_Channel1->CCR |= DMA_CCR_CIRC;				// 3. circular mode
	DMA1_Channel1->CCR |= DMA_CCR_MINC;				// 4. Memory address increment enable (MINC)
	DMA1_Channel1->CCR |=  DMA_CCR_MSIZE_0;			// Memory size = 16bit
	DMA1_Channel1->CCR |=  DMA_CCR_PSIZE_0;			// Peripheral size = 16bit
}

void GCL_ADC_DMA_Config(uint32_t from, uint32_t to, uint16_t size){
	DMA1_Channel1->CNDTR = size;					// size of the transfer
	DMA1_Channel1->CPAR = from;					// source address (peripheral)
	DMA1_Channel1->CMAR = to;					// destination address (memory)
	DMA1_Channel1->CCR |= DMA_CCR_EN;				// enable DMA channel
}


void main(void){
  GCL_ADC_Init();
  GCL_ADC_Enable();
  GCL_ADC_DMA_Init();
  GCL_ADC_DMA_Config((uint32_t)&ADC1->DR, (uint32_t)adc_buf, 6);
  GCL_ADC_Start();
}
