//Abdullah Aljandali
// EE454 Project 3
#include "stml432kc.h"
#include "string.h"

#define RIGHT 1<<4
#define UP 1<<5
#define DOWN 1<<11
#define LEFT 1<<8
#define SUBMIT 1<<1
#define MAX_SELECTION 3
#define MIN_SELECTION 0



void USART1_Init();
void USART1_Read( char* buffer,  long nBytes);
void USART1_Write( char* buffer,  long nBytes);
void GPIO_Initialize(void);
void ADC_Initialize(void);

int red = 140;
int green = 187;
int blue = 217;
 long k,i,j;
int a,b,c,d;

int main(){
	int currentPWM = 0;//pwm value
	int currentConv=0;
	int lastConv=0;
	int lastError=0;
	int currentError=0;
	int values[4];//Target,P,I,D
	char names[4];// 'T' 'P' 'I' 'D'
	int currentSelection =0;
	int p,i,d;
  char message[32];
	char commandMode[1];
	char redBrightness[1];
	char greenBrightness[1];
	char blueBrightness[1];
  char temp[12];
	int pressed[4] = {0,0,0,0}; // UP, RIGHT, DOWN, LEFT
	redBrightness[0] = red;
	greenBrightness[0] = green;
	blueBrightness[0] = blue;
	commandMode[0] = 124;

	values[0] = 0; //Target
	values[1] = 0; //P
	values[2] = 0; //I
	values[3] = 0;//D
	names[0] = 'T';
	names[1] = 'P';
	names[2] = 'I';
	names[3] = 'D';
	for(i=0; i<60000;i++);

	GPIO_Initialize();
	USART1_Init();
	ADC_Initialize();

	USART1_Write(commandMode,1);
	USART1_Write(redBrightness,1);

	USART1_Write(commandMode,1);
	USART1_Write(greenBrightness,1);

	USART1_Write(commandMode,1);
	USART1_Write(blueBrightness,1);

	for(i=0;i<32; i++)
		message[i] = ' ';
	USART1_Write(message,32);
	USART1_Write(message,32);
	USART1_Write(message,32);
	USART1_Write(message,32);

	while (((GPIOB_IDR & SUBMIT) >> SUBMIT) != 0){
		//check if buttons are unpressed
		if(((GPIOB_IDR & RIGHT)>>RIGHT)!=0){
 			pressed[1] = 0;
		}

		if(((GPIOA_IDR & LEFT)>>LEFT)!=0){
			pressed[3] = 0;
		}


		//check if buttons are pressed
		if(((GPIOB_IDR & RIGHT)>>RIGHT)==0 && pressed[1] == 0)
		{
			currentSelection++;
			if(currentSelection > MAX_SELECTION)
				currentSelection = MIN_SELECTION;
			pressed[1] = 1;
		}

		if(((GPIOB_IDR & UP)>>UP)==0)
			values[currentSelection] = values[currentSelection] + 1;


		if(((GPIOA_IDR & DOWN)>>DOWN)==0)
			values[currentSelection] = values[currentSelection] - 1;


		if(((GPIOA_IDR & LEFT)>>LEFT)==0 && pressed[3] == 0)
		{
			currentSelection--;
			if(currentSelection < MIN_SELECTION)
				currentSelection = MAX_SELECTION;
			pressed[3] = 1;
		}


		sprintf(message,"%c: %d",names[currentSelection],values[currentSelection]);

		for(i= strlen(message); i<32; i++)
			message[i] = ' ';

		USART1_Write(message,32);
	}


		sprintf(message,"T: %d  P: %d  I: %d  D: %d",values[0],values[1],values[2],values[3]);

		for(i= strlen(message); i<32; i++)
			message[i] = ' ';

		USART1_Write(message,32);

	while(1){
		lastError = currentError;

		ADC_CR |= 1<<2; //starts conversion on ADC`
		while((ADC_ISR & 4) == 0);//checks EOC (End Of Conversion) bit
		currentConv = ADC_DR & 0xFFF; //isolates bits to our max resolution of 12
		currentError = values[3]-currentConv;

		p = currentError;
		i = 0;
		d = 20 * (lastError-currentError)/1000;
		currentPWM = currentPWM + values[0]*(currentError)+values[1]*i + values[2] * d;

		TIM2_CCR2 = currentPWM;
	}
}

void GPIO_Initialize(void){

	RCC_AHB2ENR|=(1<<0); //enable GPIOA clock
	RCC_AHB2ENR |= (1<<1); //enable GPIOB clock

	// GPIO initialization for PWM
	RCC_APB1ENR |= (1<<0); //enable timer 2 clock
	GPIOB_MODER &= ~(3<<(2*3));//clear out PB3 mode register
	GPIOB_MODER |= (2<<(2*3));//set PB3 to AF
	GPIOB_AFRL |= (1<<(4*3)); //sets PB3 to Alternate Function 1
	TIM2_CR1 |= (1<<7); // auto reload register Buffer enable
	TIM2_PSC = 1; //sets the prescaler
	TIM2_ARR = 40000; //so the clock is every 4 miliseconds
	TIM2_CCMR1 |= 0x6800; //sets to pwm mode with the 6 and sets the enable preload bit with the 8
	TIM2_CCER |= (1<<4); //enable the CCR
	TIM2_CCR2 = 0; //preload to see if it is working
	TIM2_EGR |= 1; //update events can occur
	TIM2_CR1 |= 1; //makes clock to start ticking
	//GPIOB_OTYPER &= (~0x1<<3);


	//GPIO initilization for buttons
	//moder = 00 for input
	//pupdr = 01 for pull up

	//PB4 for right
	GPIOB_MODER &= ~(3 << (4*2));
	GPIOB_PUPDR &= ~(3 << (4*2));
	GPIOB_PUPDR |= (1 << (4*2));

	// PB5 for up
	GPIOB_MODER &= ~(3 << (5*2));
	GPIOB_PUPDR &= ~(3 << (5*2));
	GPIOB_PUPDR |= (1 << (5*2));

	// PA11 for down
	GPIOA_MODER &= ~(3 << (11*2));
	GPIOA_PUPDR &= ~(3 << (11*2));
	GPIOA_PUPDR |= (1 << (11*2));

	// PA8 for left
	GPIOA_MODER &= ~(3 << (8*2));
	GPIOA_PUPDR &= ~(3 << (8*2));
	GPIOA_PUPDR |= (1 << (8*2));

	// PB1 for submit
	GPIOB_MODER &= ~(3 << (1*2));
	GPIOB_PUPDR &= ~(3 << (1*2));
	GPIOB_PUPDR |= (1 << (1*2));

	//GPIO initialization for USART1

	// enable GPIO clock and configure the TX pin  and the RX pin as:
	// alternate function, high speed, push-pull, pull-up
	//PB.6 = AF7 (USART1_TX), PB.7 = AF7(USART1_RX), see appendix I

	//00 = input, 01 = output, 10 = alternate function, 11 = analog
	GPIOB_MODER &= ~(0xF << (2*6)); // clear mode bits for pin 6 and 7
	GPIOB_MODER |= 0xA << (2*6); //select alternate function mode

	//alternative function 7 = USART 1
	//Appendix I shows all alternate functions
	GPIOB_AFRL |= 0x77 << (4*6); // set pin 6 and 7 to AF7

	//GPIOA speed: 00 = low speed, 01 = medium speed,
  //								 10 = fast speed,11 = high speed
	GPIOB_OSPEEDR |= 0xF << (2*6);

	//GPIO push-pull 00 = no pull-up/pull-down, 01 - pull-up (01)
	//							 10 = pull down, 11 = reserved
	GPIOB_PUPDR &= ~(0xF<<(2*6));
	GPIOB_PUPDR |= 0x5 << (2*6); // select pullup

	//GPIO output type: 0 = push-pull, 1 = open drain
	GPIOB_OTYPER &= ~(0x3<<6);
}

void ADC_Initialize(){
	RCC_AHB2ENR|=(1<<13); //enable ADC clock
	//mode defaults to ADC
	GPIOA_PUPDR &= ~(3<<2*0); //PA0 no pull up or pull down

	ADC_CR &= ~(1<<29); //clears the deep power mode bit
	//do this first
	ADC_CR|= (1<<28); //turn on voltage regulator
	for(i = 0;i<10000;i++);//delay for voltage regulator and deep power mode to reach equilibrium, need about 20 microseconds
	ADC_CCR |= (1<<22); //enable internal reference
	ADC_CCR|=(1<<16); //enable high speed clock

	ADC_ISR |= 1;//used as a flag to ensure adc is ready to be used
	ADC_CR |= 1; //enable ADC
	while((ADC_ISR &&1)==0);//checks for when ADC is ready to be used
	ADC_SQR1 &= ~(0xF);// tells adc to use 1 channel to convert
	ADC_SQR1 |= (5<<6); //channel 5 is selected for the ADC sequence PA0

	//ADC_CFGR |= (1<< 16); //discontinuous run mode enabledd
	//ADC_CFGR |= (1<<17);//discontinuous run mode needs to know how many bits we are using
}


void USART1_Init(){
	int i;

	RCC_APB2ENR |= 1 << 14; // USART clock enable

	//disable USART
	USART1_CR1 &= ~USART_CR1_UE;

	//set data length to 8bits
	// 00 = 8 data bits, 01 = 9 data bits, 10 = 7 data bits

	USART1_CR1 &= ~USART_CR1_M0;
	USART1_CR1 &= ~USART_CR1_M1;

	//select 1 stop bit
	//00 = 1 stop bit	   01 = 0.5 stop bit
	//10 = 2 stop bits   11 = 1.5 stop bit

	USART1_CR2 &= ~USART_CR2_STOP;

	// set parity control as a no parity
	// 0 = no parity,
	// 1 = parity enabled (then, program PS bit to select Even or Odd parity)
	USART1_CR1 &= USART_CR1_PCE;

	// Oversample by 16
	// 0 = oversampling by 16, 1 = oversampling by 8
	USART1_CR1 &= ~USART_CR1_OVER8;

	// set Baud rate to 9600 using APB frequency (4MHz)
	// see example 1 in section 22.1.1
	USART1_BRR = 0x1A1;

	// enable transmission and reception
	USART1_CR1 |= (USART_CR1_TE | USART_CR1_RE);

	// enable USART
	USART1_CR1 |= USART_CR1_UE;

	// verify that USART is ready for transmission
	// TEACK: tramsmit enable acknowledge flag. HArdware sets or resets iot.
	k=0;
	while( ((USART1_ISR & USART_ISR_TEACK) == 0) && k <1000000 )
		k++;
	// verify that USART is ready for reception
	// REACK: Receive enable acknowledge flag. HArdware sets or resets it.
	k =0;
	while(((USART1_ISR &USART_ISR_REACK) ==0) && k <1000000)
		k++;
	for(i =0; i<50000; i++);
}

void USART1_Read( char* buffer,  long nBytes){
	int i;

	for(i =0; i< nBytes; i++){
		k=0;
		while( (!(USART1_ISR & USART_ISR_RXNE)) && k <1000000) // wait until hardware sets RXNE
			k++;
		buffer[i] = USART1_RDR;								 // Reading RDR clears RXNE
	}
}
void USART1_Write( char* buffer,  long nBytes){
	int i;

	for(i=0; i< nBytes; i++){
		k=0;
		while( (!(USART1_ISR & USART_ISR_TXE)) && k <1000000) // wait until hardware sets TXE
			k++;
		USART1_TDR = buffer[i] & 0xFF;       // writing to TDR clears TXE flag
	}

	// wait until TX bit is set. TC is set by hardware and cleared by software.
	k=0;
	while( (!(USART1_ISR & USART_ISR_TC)) && k <1000000)	// TC: Transmission complete flag
		k++;
	k=0;

	// writing 1 to the TCCF bit in ICR clears the TC bit in ISR
	USART1_ICR |= USART_ICR_TTCF; //TCCF: Transmission complete clear flag
}
