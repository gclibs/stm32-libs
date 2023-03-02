/*
* === 6-channel ADC with DMA for STM32F407 ===
*/


uint16_t adc_buf[6];							// here DMA stores the ADC values

void GCL_ADC_Init(void){
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

	ADC->CCR |= ADC_CCR_ADCPRE_0;					// PCLK2 div by 4

	ADC1->CR1 = ADC_CR1_SCAN;					// Scan mode
	ADC1->CR1 &= ~ADC_CR1_RES_0;					// 12 bit resolution

	ADC1->CR2 = ADC_CR2_CONT;					// Continuous mode
	ADC1->CR2 |= ADC_CR2_EOCS;					// 1: The EOC bit is set at the end of each regular conversion. Overrun detection is enabled.
	ADC1->CR2 &= ~ADC_CR2_ALIGN;					// Data alignment

	// sampling time for channels
	ADC1->SMPR2 |= ((7<<3) | (7<<12) | (7<<0) | (7<<6) | (7<<9) | (7<<15));	// channel 1 and channel 4 sampling rate 3 cycles

	// set regular channel sequence length
	ADC1->SQR1 |= ADC_SQR1_L_0 | ADC_SQR1_L_2;			// 0101: 5 here means 6 conversions

	// Set GPIOA pins as Analog inputs
	GPIOA->MODER |= (3<<0);					// PA0 enable as Analog input
	GPIOA->MODER |= (3<<2);					// PA1 enable as Analog input
	GPIOA->MODER |= (3<<4);					// PA2 enable as Analog input
	GPIOA->MODER |= (3<<6);					// PA3 enable as Analog input
	GPIOA->MODER |= (3<<8);					// PA4 enable as Analog input
	GPIOA->MODER |= (3<<10);					// PA5 enable as Analog input

	// Enable DMA for ADC
	ADC1->CR2 |= ADC_CR2_DMA;					// DMA mode enabled
	ADC1->CR2 |= ADC_CR2_DDS; 					// DMA requests are issued as long as data are converted and DMA=1s

	ADC1->SQR3 = 0;
	ADC1->SQR3 |= (0<<0)|(1<<5)|(2<<10)|(3<<15)|(4<<20)|(5<<25);
}

void GCL_ADC_Enable(void){
	ADC1->CR2 |= ADC_CR2_ADON;					// wake up (enable) ADC
	uint32_t del = 10000;
	while (del--);							// wait about 10us ADC to stabilize
}

void GCL_ADC_Start(void){
	ADC1->SR = 0;							// clear status register
	ADC1->CR2 |= ADC_CR2_SWSTART;					// start conversion by software
}

void GCL_ADC_DMA_Init(void){
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;				// Enable DMA2 clock

	// Configure DM2 stream (Channel 0, based on table 43 of RM0090)
	// see 10.3.17 Stream configuration procedure
	DMA2_Stream0->CR &= ~(DMA_SxCR_DIR_0 | DMA_SxCR_DIR_1);	// set peripheral flow controller, data transfer direction
	DMA2_Stream0->CR |= DMA_SxCR_CIRC;				// 3. circular mode
	DMA2_Stream0->CR |= DMA_SxCR_MINC;				// 4. Memory address increment enable (MINC)
	DMA2_Stream0->CR |= DMA_SxCR_MSIZE_0;				// Memory size = 16bit
	DMA2_Stream0->CR |= DMA_SxCR_PSIZE_0;				// Peripheral size = 16bit
	DMA2_Stream0->CR &= ~(7<<25);					// Select channel 0 for DMA ADC. Bits 27:25 CHSEL[2:0]
}

void GCL_ADC_DMA_Config(uint32_t from, uint32_t to, uint16_t size){
	DMA2_Stream0->NDTR = size;					// size of the transfer
	DMA2_Stream0->PAR = from;					// source address (peripheral)
	DMA2_Stream0->M0AR = to;					// destination address (memory)
	DMA2_Stream0->CR |= DMA_SxCR_EN;				// enable DMA channel
}


void main(void){
  GCL_ADC_Init();
  GCL_ADC_Enable();
  GCL_ADC_DMA_Init();
  GCL_ADC_DMA_Config((uint32_t)&ADC1->DR, (uint32_t)adc_buf, 6);
  GCL_ADC_Start();
}
