/* 			=== GCLibs ===
	STM32F1xx PWM Input Capture using CMSIS
	This code initializes PA8 pin (5V tolerant) as PWM Input based on TIM1
	TIM1 Capture Compare Interrupt enabled and calculates PWM % when CCR2 (duty_cycle) register loaded
*/

uint16_t duty_cycle = 0;
uint16_t freq = 0;
uint16_t pwm = 0;

void GCL_PWM_Input_Init(void){
	// TIM1 init
	TIM1->CR1 &= ~TIM_CR1_CEN;
	TIM1->SR &= ~TIM_SR_UIF;


	RCC->APB2RSTR |= RCC_APB2RSTR_TIM1RST;
	RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM1RST;
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

	// PB13 GPIO init
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	while (0);
	GPIOA->CRH &= ~GPIO_CRH_MODE8;			// MODEy[1:0]   00: Input mode (reset state)
	GPIOA->CRH |= GPIO_CRH_CNF8_0;
	GPIOA->CRH &= ~GPIO_CRH_CNF8_1;			// 01 Floating input (reset state)

	// TIM1 init
	TIM1->CR1 |= TIM_CR1_ARPE;
	TIM1->PSC = 32;					// Timer prescaler
	TIM1->ARR = 0xFFFF;					// count up to this value
	TIM1->CR1 &= ~TIM_CR1_UDIS;
	TIM1->EGR |= TIM_EGR_UG | TIM_EGR_CC1G;
	TIM1->SMCR &= ~(TIM_SMCR_SMS | TIM_SMCR_TS | TIM_SMCR_ETF | TIM_SMCR_ETPS | TIM_SMCR_ECE | TIM_SMCR_ETP);
	TIM1->CR1 |= TIM_CR1_ARPE;
	while (!TIM1->SR & TIM_SR_UIF);

	TIM1->DIER &= ~TIM_DIER_TIE;					// 0: Trigger interrupt enabled
//	TIM1->DIER |= TIM_DIER_UIE;					// 0: Trigger interrupt enabled
	TIM1->DIER |= TIM_DIER_CC1IE;				// for Capture Compare 2
	TIM1->DIER &= ~TIM_DIER_TDE;				// 0: Trigger DMA request disabled.


	// NOTE: CHANNEL 3 and 4 could not be configured as PWM input, only CH1 and CH2

	// Select the active input for TIMx_CCR1: write the CC1S bits to 01 in the TIMx_CCMR1 register (TI1 selected).
	TIM1->CCER &= ~TIM_CCER_CC1E;				// disable IC1 capture
	TIM1->CCMR1 &= ~TIM_CCMR1_CC1S_1;			// Bits 1:0 CC1S: Capture/Compare 1 selection /
	TIM1->CCMR1 |= TIM_CCMR1_CC1S_0;			// 01: CC1 channel is configured as input, IC1 is mapped on TI1

	// Select the active polarity for TI1FP1 (used both for capture in TIMx_CCR1 and counter clear): write the CC1P to ‘0’ (active on rising edge).
	TIM1->CCER &= ~TIM_CCER_CC1P;				// RESET to capture on RISING edge
	TIM1->CCMR1 &= ~TIM_CCMR1_IC1PSC;			// IC1PSC: Input capture 1 prescaler / 00: no prescaler, capture is done each time an edge is detected on the capture input
	TIM1->CCMR1 &= ~TIM_CCMR1_IC1F;			// Bits 7:4 IC1F: Input capture 1 filter / 0000: No filter, sampling is done at f DTS

	// Select the active input for TIMx_CCR2: write the CC2S bits to 10 in the TIMx_CCMR1 register (TI1 selected).
	TIM1->CCER &= ~TIM_CCER_CC2E;				// disable IC2 capture
	TIM1->CCMR1 &= ~TIM_CCMR1_CC2S_0;			// CC2S: Capture/compare 2 selection
	TIM1->CCMR1 |= TIM_CCMR1_CC2S_1;			// 10: CC2 channel is configured as input, IC2 is mapped on TI1

	// Select the active polarity for TI1FP2 (used for capture in TIMx_CCR2): write the CC2P bit to ‘1’ (active on falling edge).
	TIM1->CCER |= TIM_CCER_CC2P;				// SET to capture on FALLING edge
	TIM1->CCMR1 &= ~TIM_CCMR1_IC2PSC;			// IC2PSC[1:0]: Input capture 2 prescaler
	TIM1->CCMR1 &= ~TIM_CCMR1_IC2F;			// Bits 15:12 IC2F: Input capture 2 filter

	// Select the valid trigger input: write the TS bits to 101 in the TIMx_SMCR register (TI1FP1 selected).
	TIM1->SMCR = 0x00;
	TIM1->SMCR |= (TIM_SMCR_TS_2 | TIM_SMCR_TS_0);	// 101: Filtered Timer Input 1 (TI1FP1)

	// Configure the slave mode controller in reset mode: write the SMS bits to 100 in the TIMx_SMCR register.
	TIM1->SMCR |= TIM_SMCR_SMS_2;				// 100: Reset mode - Rising edge of the selected trigger input (TRGI) reinitializes the counter and generates an update of the registers.

	__NVIC_EnableIRQ(TIM1_CC_IRQn);

	// Enable the captures: write the CC1E and CC2E bits to ‘1 in the TIMx_CCER register
	TIM1->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);

	TIM1->CR1 |= TIM_CR1_CEN;				// Start TIM1
}

void TIM1_CC_IRQHandler(void){
	if (((TIM1->SR) & TIM_SR_CC2IF)>0) {			// Calculate PWM when duty cycle has been captured
		freq = TIM1->CCR1;
		duty_cycle = (TIM1->CCR2) + 1;
		if (freq>0){					// div by 0 check
			pwm = duty_cycle/(freq/100);
		}
	}
	TIM1->SR &= ~(1<<0);					// clear Update Interrupt flag
	TIM1->SR &= ~(1<<1);					// clear CC1 interrupt flag
}


void main(void){
	GCL_PWM_Input_Init();
	HAL_Delay(1);						// magic delay
	while (1) {
		HAL_Delay(10);
	}
}
