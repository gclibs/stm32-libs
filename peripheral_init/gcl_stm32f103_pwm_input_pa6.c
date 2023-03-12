/* 			=== GCLibs ===
	STM32F1xx PWM Input Capture using CMSIS
	This code initializes PA6 pin as PWM Input based on TIM3
*/	


void GCL_PWM_Input_Init(void){
	// TIM3 init
	TIM3->CR1 &= ~TIM_CR1_CEN;
	TIM3->SR &= ~TIM_SR_UIF;

	RCC->APB1RSTR |= RCC_APB1RSTR_TIM3RST;
	RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM3RST;
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

	// PA6 GPIO init
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	GPIOA->CRL &= ~GPIO_CRL_MODE6;			// MODEy[1:0]   00: Input mode (reset state)
	GPIOA->CRL |= GPIO_CRL_CNF6_0;
	GPIOA->CRL &= ~GPIO_CRL_CNF6_1;			// 01 Floating input (reset state)

	// TIM3 init
	TIM3->CR1 |= TIM_CR1_ARPE;
	TIM3->PSC = 32;					// Timer prescaler
	TIM3->ARR = 0xFFFF;					// count up to this value
	TIM3->CR1 &= ~TIM_CR1_UDIS;
	TIM3->EGR |= TIM_EGR_UG;
	TIM3->SMCR &= ~(TIM_SMCR_SMS | TIM_SMCR_TS | TIM_SMCR_ETF | TIM_SMCR_ETPS | TIM_SMCR_ECE | TIM_SMCR_ETP);
	TIM3->CR1 |= TIM_CR1_ARPE;
	while (!TIM3->SR & TIM_SR_UIF);

	TIM3->DIER &= ~TIM_DIER_TIE;				// 0: Trigger interrupt disabled.
	TIM3->DIER &= ~TIM_DIER_TDE;				// 0: Trigger DMA request disabled.


	// NOTE: CHANNEL 3 and 4 could not be configured as PWM input, only CH1 and CH2

	// Select the active input for TIMx_CCR1: write the CC1S bits to 01 in the TIMx_CCMR1 register (TI1 selected).
	TIM3->CCER &= ~TIM_CCER_CC1E;				// disable IC1 capture
	TIM3->CCMR1 &= ~TIM_CCMR1_CC1S_1;			// Bits 1:0 CC1S: Capture/Compare 1 selection /
	TIM3->CCMR1 |= TIM_CCMR1_CC1S_0;			// 01: CC1 channel is configured as input, IC1 is mapped on TI1

	// Select the active polarity for TI1FP1 (used both for capture in TIMx_CCR1 and counter clear): write the CC1P to ‘0’ (active on rising edge).
	TIM3->CCER &= ~TIM_CCER_CC1P;				// RESET to capture on RISING edge
	TIM3->CCMR1 &= ~TIM_CCMR1_IC1PSC;			// IC1PSC: Input capture 1 prescaler / 00: no prescaler, capture is done each time an edge is detected on the capture input
	TIM3->CCMR1 &= ~TIM_CCMR1_IC1F;			// Bits 7:4 IC1F: Input capture 1 filter / 0000: No filter, sampling is done at f DTS

	// Select the active input for TIMx_CCR2: write the CC2S bits to 10 in the TIMx_CCMR1 register (TI1 selected).
	TIM3->CCER &= ~TIM_CCER_CC2E;				// disable IC2 capture
	TIM3->CCMR1 &= ~TIM_CCMR1_CC2S_0;			// CC2S: Capture/compare 2 selection
	TIM3->CCMR1 |= TIM_CCMR1_CC2S_1;			// 10: CC2 channel is configured as input, IC2 is mapped on TI1

	// Select the active polarity for TI1FP2 (used for capture in TIMx_CCR2): write the CC2P bit to ‘1’ (active on falling edge).
	TIM3->CCER |= TIM_CCER_CC2P;				// SET to capture on FALLING edge
	TIM3->CCMR1 &= ~TIM_CCMR1_IC2PSC;			// IC2PSC[1:0]: Input capture 2 prescaler
	TIM3->CCMR1 &= ~TIM_CCMR1_IC2F;			// Bits 15:12 IC2F: Input capture 2 filter

	// Select the valid trigger input: write the TS bits to 101 in the TIMx_SMCR register (TI1FP1 selected).
	TIM3->SMCR = 0x00;
	TIM3->SMCR |= (TIM_SMCR_TS_2 | TIM_SMCR_TS_0);	// 101: Filtered Timer Input 1 (TI1FP1)

	// Configure the slave mode controller in reset mode: write the SMS bits to 100 in the TIMx_SMCR register.
	TIM3->SMCR |= TIM_SMCR_SMS_2;				// 100: Reset mode - Rising edge of the selected trigger input (TRGI) reinitializes the counter and generates an update of the registers.

	// Enable the captures: write the CC1E and CC2E bits to ‘1 in the TIMx_CCER register
	TIM3->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);

	TIM3->CR1 |= TIM_CR1_CEN;				// Start TIM3
}

int main(void){
	uint16_t duty_cycle = 0;
	uint16_t freq = 0;
	uint16_t pwm = 0;
	GCL_PWM_Input_Init();
	while (1) {
		freq = TIM3->CCR1;
		duty_cycle = TIM3->CCR2;
		pwm = duty_cycle/(freq/100);
		HAL_Delay(10);
	}
}

