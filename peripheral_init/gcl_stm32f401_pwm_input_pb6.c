/* 			=== GCLibs ===
	STM32F4xx PWM Input Capture using CMSIS
	This code initializes PB6 pin as PWM Input based on TIM4
	
		DIFFERENCE FROM STM32F1xx series
	1. different GPIO init part
	2. Added TIM_CCER_CC1NP and TIM_CCER_CC2NP settings
*/	


void GCL_PWM_Input_Init_PB6(void){
	// TIM4 init
	TIM4->CR1 &= ~TIM_CR1_CEN;
	TIM4->SR &= ~TIM_SR_UIF;

	RCC->APB1RSTR |= RCC_APB1RSTR_TIM4RST;
	RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM4RST;
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

	// PB6 GPIO init
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	GPIOB->MODER |= GPIO_MODER_MODER6_1;		//Set GPIOA, PIN 6 as Input (MODER[13:12] = 00)
	GPIOB->AFR[0] |= (2<<24);					// configure alternate function AF2 (TIM3..TIM5). See Figure 26 of RM0090

	// TIM4 init
	TIM4->CR1 |= TIM_CR1_ARPE;
	TIM4->PSC = 7;					// Timer prescaler
	TIM4->ARR = 0xFFFF;					// count up to this value
	TIM4->CR1 &= ~TIM_CR1_UDIS;
	TIM4->EGR |= TIM_EGR_UG;
	TIM4->SMCR &= ~(TIM_SMCR_SMS | TIM_SMCR_TS | TIM_SMCR_ETF | TIM_SMCR_ETPS | TIM_SMCR_ECE | TIM_SMCR_ETP);
	TIM4->CR1 |= TIM_CR1_ARPE;
	while (!TIM4->SR & TIM_SR_UIF);

	TIM4->DIER &= ~TIM_DIER_TIE;				// 0: Trigger interrupt disabled.
	TIM4->DIER &= ~TIM_DIER_TDE;				// 0: Trigger DMA request disabled.


	// NOTE: CHANNEL 3 and 4 could not be configured as PWM input, only CH1 and CH2

	// Select the active input for TIMx_CCR1: write the CC1S bits to 01 in the TIMx_CCMR1 register (TI1 selected).
	TIM4->CCER &= ~TIM_CCER_CC1E;				// disable IC1 capture
	TIM4->CCMR1 &= ~TIM_CCMR1_CC1S_1;			// Bits 1:0 CC1S: Capture/Compare 1 selection /
	TIM4->CCMR1 |= TIM_CCMR1_CC1S_0;			// 01: CC1 channel is configured as input, IC1 is mapped on TI1

	// Select the active polarity for TI1FP1 (used both for capture in TIMx_CCR1 and counter clear): write the CC1P to ‘0’ (active on rising edge).
	TIM4->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP);				// RESET to capture on RISING edge
	TIM4->CCMR1 &= ~TIM_CCMR1_IC1PSC;			// IC1PSC: Input capture 1 prescaler / 00: no prescaler, capture is done each time an edge is detected on the capture input
	TIM4->CCMR1 &= ~TIM_CCMR1_IC1F;			// Bits 7:4 IC1F: Input capture 1 filter / 0000: No filter, sampling is done at f DTS

	// Select the active input for TIMx_CCR2: write the CC2S bits to 10 in the TIMx_CCMR1 register (TI1 selected).
	TIM4->CCER &= ~TIM_CCER_CC2E;				// disable IC2 capture
	TIM4->CCMR1 &= ~TIM_CCMR1_CC2S_0;			// CC2S: Capture/compare 2 selection
	TIM4->CCMR1 |= TIM_CCMR1_CC2S_1;			// 10: CC2 channel is configured as input, IC2 is mapped on TI1

	// Select the active polarity for TI1FP2 (used for capture in TIMx_CCR2): write the CC2P bit to ‘1’ (active on falling edge).
	TIM4->CCER |= TIM_CCER_CC2P;				// SET to capture on FALLING edge
	TIM4->CCER &= ~(TIM_CCER_CC2NP);
	TIM4->CCMR1 &= ~TIM_CCMR1_IC2PSC;			// IC2PSC[1:0]: Input capture 2 prescaler
	TIM4->CCMR1 &= ~TIM_CCMR1_IC2F;			// Bits 15:12 IC2F: Input capture 2 filter

	// Select the valid trigger input: write the TS bits to 101 in the TIMx_SMCR register (TI1FP1 selected).
	TIM4->SMCR = 0x00;
	TIM4->SMCR |= (TIM_SMCR_TS_2 | TIM_SMCR_TS_0);	// 101: Filtered Timer Input 1 (TI1FP1)

	// Configure the slave mode controller in reset mode: write the SMS bits to 100 in the TIMx_SMCR register.
	TIM4->SMCR |= TIM_SMCR_SMS_2;				// 100: Reset mode - Rising edge of the selected trigger input (TRGI) reinitializes the counter and generates an update of the registers.

	// Enable the captures: write the CC1E and CC2E bits to ‘1 in the TIMx_CCER register
	TIM4->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);

	TIM4->CR1 |= TIM_CR1_CEN;				// Start TIM3
}


uint16_t pwm_data[2] = {0, 0};
int main(void){
  GCL_PWM_Input_Init_PB6();
  while (1)
  {
	  pwm_data[0] = TIM4->CCR1;
	  pwm_data[1] = TIM4->CCR2;
	  HAL_Delay(100);
  }



