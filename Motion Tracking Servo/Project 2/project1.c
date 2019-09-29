//#define RCC_AHB2ENR (*((volatile unsigned long *) 0x4002104C))
//#define GPIOB_MODER (*((volatile unsigned long *) 0x48000400)) 
//#define GPIOB_BSRR (*((volatile unsigned long *) 0x48000418))

//#define LED_ON (1 << 3) 
//#define LED_OFF (1 << (3+16)) 

//int main(){ 	
//	int i; 
//	RCC_AHB2ENR |= (1 << 1); 
//	GPIOB_MODER &= ~(3<<(2*3)); 
//	GPIOB_MODER |= 1<<(2*3); 
//	while(1){ 
//		GPIOB_BSRR |= LED_ON; 
//		for(i=0; i<100000; i++); 
//		GPIOB_BSRR |= LED_OFF; 
//		for(i=0; i<100000; i++); 
//		} 
//	}





//#define RCC_AHB1ENR (*((volatile unsigned long *) 0x4002104C))  // FIX THE ADDRESS
	
#define GPIOA_PUPDR (*((volatile unsigned long *) 0x4800000C))  //GPIOA pullup pulldown Register
#define ADC_CFGR (*((volatile unsigned long *) 0x5004000C))      //ADC configuration register
#define GPIOB_MODER (*((volatile unsigned long *) 0x48000400))  //GPIOB Mode Register
#define RCC_AHB2ENR (*((volatile unsigned long *) 0x4002104C))  //GPIO Enable Register
#define RCC_APB1ENR (*((volatile unsigned long *) 0x40021058))  //Peripheral Clock Enable Register
#define RCC_APB2ENR (*((volatile unsigned long *) 0x40021060))  //Peripheral Clock Enable Register
#define GPIOB_BSRR (*((volatile unsigned long *) 0x48000418))   //GPIOB Output Bit set/reset Register
#define GPIOB_IDR (*((volatile unsigned long *) 0x48000410))    //GPIOB Input Data Register
#define GPIOB_AFRL (*((volatile unsigned long *) 0x48000420))    //GPIOB Alt Function
#define ADC_ISR (*((volatile unsigned long *) 0x50040000))      //ADC Interrupt and Status Register
#define ADC_IER (*((volatile unsigned long *) 0x50040004))      //ADC Interrupt Enable Register
#define ADC_CR (*((volatile unsigned long *) 0x50040008))       //ADC Control Register
#define ADC_SQR1 (*((volatile unsigned long *) 0x50040030))     //ADC Regular Sequence Register
#define ADC_DR (*((volatile unsigned long *) 0x50040040))       //ADC Data Register
#define ADC_CCR (*((volatile unsigned long *) 0x50040308))      //ADC Common Control Register
#define TIM2_PSC (*((volatile unsigned long *) 0x40000028))     //Timer 2 Prescaler
#define TIM2_ARR (*((volatile unsigned long *) 0x4000002c))
#define TIM2_CCMR1 (*((volatile unsigned long *) 0x40000018))
#define TIM2_CCR1 (*((volatile unsigned long *) 0x40000034))
#define TIM2_CCR2 (*((volatile unsigned long *) 0x40000038)) 
#define TIM2_CR1 (*((volatile unsigned long *) 0x40000000)) //auto reload register
#define TIM2_CCER (*((volatile unsigned long *) 0x40000020)) //TIMx capture/compare enable register
#define TIM2_EGR (*((volatile unsigned long *) 0x40000014)) //event generation register


#define LED_ON (1 << 3)
#define LED_OFF (1 << (3+16))

void ADC_Initialize(void);
void GPIO_Initialize(void);
unsigned int value, i;


int main(){
	GPIO_Initialize();
	ADC_Initialize();
	
	while(1){
		ADC_CR |= 1<<2; //start the ADC conversion
		while((ADC_ISR & 4) == 0);//checks the end of conversion bit
		value = ADC_DR & 0xFFF; //to isolate the 12 bits that we have, maximum resolution
		TIM2_CCR2 = value*4;
		for(i = 0; i<10000;i++);
	}
}

void GPIO_Initialize(void){
	RCC_AHB2ENR |= (1<<1); //GPIO B clock enable
	RCC_APB1ENR |= (1<<0); //enable timer 2 clock
	GPIOB_MODER &= ~(3<<(2*3));//clear out the mode register
	GPIOB_MODER |= (2<<(2*3));//sets PB3 to AF
	GPIOB_AFRL |= (1<<(4*3)); //sets Port pin 3 to alternate function 1, it is 4 bits wide though
	TIM2_CR1 |= (1<<7); // auto reload register Buffer
	TIM2_PSC = 0; //sets the prescaler that you woud like to have; runs at full speed
	TIM2_ARR = 16000; //this is so the clock is every milisecond
	
	TIM2_CCMR1 |= 0x6800; //sets the pwm mode with the 6 and sets the correct enable preload bit with the 8
	TIM2_CCER |= (1<<4); //enable the CCR
	TIM2_CCR2 = 1000; //preload that is not zero so we can tell if something is working
	TIM2_EGR |= 1; //allows for update events to occur

	
	TIM2_CR1 |= 1; //clock enable to allow for the clock to start ticking
}

void ADC_Initialize(){
	RCC_AHB2ENR|=(1<<0); //enable the clock for GPIOA
	RCC_AHB2ENR|=(1<<13); //enable the ADC clock
	//mode defaults to ADC
	GPIOA_PUPDR &= ~(3<<2*1); //PA1 no pull up or pull down
	
	ADC_CR &= ~(1<<29); //this is to clear the deep power mode bit
	//this must be done first
	ADC_CR|= (1<<28); //this turns on the voltage regulator
	for(i = 0;i<10000;i++);//delay so the voltage regulator and deep power mode can reach equilibrium, needs to be about 20 microseconds
	ADC_CCR |= (1<<22); //enable the internal reference
	ADC_CCR|=(1<<16); //enable the high speed clock
	
	ADC_ISR |= 1;//used as a flag to make sure that it is ready to be used
	ADC_CR |= 1; //enable ADC
	while((ADC_ISR &&1)==0);//checks to see when the ADC is ready to be used
	ADC_SQR1 |= (6<<6); //channel 6 is selected for the ADC sequence
	//if we wanted to do a longer sequence than just 1 that is set in the first four bits of ADC_SQR1
	ADC_CFGR |= (1<< 16); //discontinuous run mode enabledd
}
