void GCL_Encoder_EXTI_Init(void){
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->PUPDR |= (5<<2);				        // PA1 and PA2 pins used for encoder, pull up
	GPIOA->OSPEEDR |= (5<<2);				// Medium speed for PA1 and PA2
/*	Hardware interrupt selection
	To configure a line as interrupt source, use the following procedure:
	- Configure the corresponding mask bit in the EXTI_IMR register.
	- Configure the Trigger Selection bits of the Interrupt line (EXTI_RTSR and EXTI_FTSR)
	- Configure the enable and mask bits that control the NVIC IRQ channel mapped to the
	EXTI so that an interrupt coming from one of the EXTI line can be correctly
	acknowledged. */

	RCC->APB1ENR |= RCC_APB2ENR_SYSCFGCOMPEN;		// enable SYSCFG
	SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI1;		// PA1 as EXTI input
	EXTI->IMR |= EXTI_IMR_MR1;				// disable interrupt mask for input
	EXTI->FTSR |= EXTI_FTSR_TR1;				// Line1: Falling trigger enabled (for Event and Interrupt) for input line.
	NVIC_SetPriority(EXTI0_1_IRQn, 1);
	NVIC_EnableIRQ(EXTI0_1_IRQn);
}

uint16_t gcl_encoder_val = 100;				        // encoder value

void EXTI0_1_IRQHandler(void){
	if ((EXTI->PR) & 2) {					// if interrupt came from Line1
		EXTI->PR |= 2;					// clear Pending bit
		NVIC->ICPR[0U] |= 2;			
		if ((GPIOA->IDR) & (1<<2)) {			
			gcl_encoder_val++;
		}
		else {
			gcl_encoder_val--;
		}

	}
}

