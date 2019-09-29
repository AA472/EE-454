#define LED_ON (1 << 3)
#define LED_OFF (1 << (3+16))
void ADC_Initialize(void);
void GPIO_Initialize(void);
void Search(void);
void Ambient(void);
unsigned int i;
int totalAngle, value1, value2;
int ambient1 = 0;
int ambient2 = 0;
int rotation = 0;

int main(){

 GPIO_Initialize();
 ADC_Initialize();
 totalAngle = 3000; // starts around the middle
 while(1){
  ADC_CR |= 1<<2; //start the ADC conversion
  while((ADC_ISR & 4) == 0);//checks the end of conversion bit
  value1 = ADC_DR & 0xFFF; //to isolate the 12 bits that we have, maximum resolution
  while((ADC_ISR & 4) == 0);//checks the end of conversion bit
  value2 = ADC_DR & 0xFFF; //to isolate the 12 bits that we have, maximum resolution

  if(value1-value2>-250&&value1-value2<250){
   totalAngle=totalAngle;
  }else if(value1>value2&&totalAngle<5000){
   totalAngle+=100;
  }else if(value2>value1&&totalAngle>1000){
   totalAngle-=100;
  }
  TIM2_CCR2 = totalAngle;

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
 TIM2_PSC = 1; //sets the prescaler to 4 so we can get 20ms (5*4ms)
 TIM2_ARR = 40000; //this is so the clock is every 4 miliseconds
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
 GPIOA_PUPDR &= ~(3<<2*0); //PA0 no pull up or pull down
 ADC_CR &= ~(1<<29); //this is to clear the deep power mode bit
 //this must be done first
 ADC_CR|= (1<<28); //this turns on the voltage regulator
 for(i = 0;i<10000;i++);//delay so the voltage regulator and deep power mode can reach equilibrium, needs to be about 20 microseconds
 ADC_CCR |= (1<<22); //enable the internal reference
 ADC_CCR|=(1<<16); //enable the high speed clock

 ADC_ISR |= 1;//used as a flag to make sure that it is ready to be used
 ADC_CR |= 1; //enable ADC
 while((ADC_ISR &&1)==0);//checks to see when the ADC is ready to be used
 ADC_SQR1 |= 1;
 ADC_SQR1 |= (5<<6); //channel 6 is selected for the ADC sequence
 ADC_SQR1 |= (6<<12); //channel 6 is selected for the ADC sequence
 //if we wanted to do a longer sequence than just 1 that is set in the first four bits of ADC_SQR1
 ADC_CFGR |= (1<< 16); //discontinuous run mode enabled
 ADC_CFGR |= (1<<17);
}
