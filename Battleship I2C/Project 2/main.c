#include "stml432kc.h"
//Abdullah Aljandali
//EE454 Project 2
//4.4.2019

//I2C1 procedures
void I2C1_Init(); 
void I2C1_Stop();
void I2C1_WaitLineIdle();
void I2C1_Start (unsigned long SlaveAddress, char size, char direction);
//I2C3 procedures
void I2C3_Init();
void I2C3_Stop();
void I2C3_WaitLineIdle();
void I2C3_Start (unsigned long SlaveAddress, char size, char direction);

void blink_green(int num_times);
void blink_red(int num_times);
void blink_blue(int num_times);

void GPIOA_Initialize(void);
int shift1();
int shift2();


//battleship game functions
void ask_for_placement();
int check_for_winner();
char player1_guess();
char player2_guess();


int check_for_hit1(int location);
int check_for_hit2(int location);


//global variables
char sentData[15];
char receivedData[15];
char ship1_location[8];
char ship2_location[8];
int result;
char player1_hits = 0;
char player2_hits = 0;
void win();
void selectionMade();
int main(){
	char turn=0;
	char guess1 =0;
	char guess2 =0;
	char winner = 0;
	GPIOA_Initialize(); //initialize GPIO
	I2C1_Init(); //initialize I2C1
	I2C3_Init(); //initialize I2C3
	sentData[0] = 0;


	blink_red(3);
	blink_green(3);
	blink_blue(3);

	
	while(1){
		
	 ask_for_placement();

		while(1){
			turn = (turn + 1)%2; //starts with 1 and toggles between 1 and 2
			
			if(turn == 1){
				guess1 = player1_guess();
			
			sentData[0] = guess1;
			I2C1_Start(0x53,1, 0);
			I2C1_WaitLineIdle();
			I2C3_Start(0xAA,1, 0);
			
			}
			else{
				guess2 = player2_guess();

			sentData[0] = guess2;
			I2C3_Start(0xAA,1, 0);
			I2C3_WaitLineIdle();
			I2C1_Start(0x53,1, 0);
			}

			if(check_for_winner() != 0){
				winner = check_for_winner();
				win();
				break;
			}	
		}
	}
	return 0;
}

void win(){
	int i,j;
	for(i=0; i<4;i++){
		GPIOA_ODR |= 1 << 11; //Turn on PA11 
		GPIOB_ODR |= 1 << 3; //Turn on PB3 
		GPIOB_ODR |= 1 << 5; //Turn on PB5 

		for(j=0; j<100000; j++); //short software delay
	  GPIOA_ODR&= ~(1<<11);  //Turn off PA11 
		GPIOA_ODR&= ~(1<<3);  //Turn off PB3 
		GPIOA_ODR&= ~(1<<5);  //Turn off PB5
		
		for(j=0; j<100000; j++); //short software delay
	} 
}
void selectionMade(){
	blink_blue(3);
}
int check_for_hit1(int location){
	if(ship2_location[location] == 1){
		ship2_location[location] = 2;
		return 10;
	}
	else 
		return 60;
	
}
int check_for_hit2(int location){
	if(ship1_location[location] == 1){
		ship1_location[location] = 2;
		return 10;
	}
	else 
		return 60;
}
int shift1(){
	int shift =0;
	shift+= 1 * ((GPIOA_IDR & (1<<0)) >> 0);
	shift+= 2 * ((GPIOA_IDR & (1<<1)) >> 1);
	shift+= 4 * ((GPIOA_IDR & (1<<2)) >> 2);
	return shift;
}

int shift2(){
	int shift =0;
	shift+= 1 * ((GPIOA_IDR & (1<<4)) >> 4);
	shift+= 2 * ((GPIOA_IDR & (1<<5)) >> 5);
	shift+= 4 * ((GPIOA_IDR & (1<<6)) >> 6);
	return shift;
}
char player1_guess(){
	int player1_guess =0;

//player 1 guesses
	while( ((GPIOA_IDR & (1<<3)) >> 3) != 1 );

	player1_guess = shift1();

	return player1_guess;
}
char player2_guess(){
	int player2_guess =0;
	while( ((GPIOA_IDR & (1<<8)) >> 8) != 1 );
	
	player2_guess = shift2();		
	
	return player2_guess;
}
int check_for_winner(){
	int i, player1_hits=0, player2_hits=0;
	for(i=0;i<=7; i++){	
		if(ship1_location[i] == 2)
			player2_hits++;
		if(ship2_location[i] == 2)
			player1_hits++;
	}
	
	if(player1_hits == 3)
		return 1;
	else if(player2_hits == 3)
		return 2;
	else
		return 0;
}
void ask_for_placement(){
	int shift=0,i;
	for(i=0;i<=7; i++){	
		ship1_location[i] = 0;
		ship2_location[i] = 0;
	}
	
	//ask placement from player1
	while( ((GPIOA_IDR & (1<<3)) >> 3) != 1 );
	
	shift = shift1();
	if (shift > 5) shift = 5;
	selectionMade();

	for(i=shift;i<shift+3; i++){	
		ship1_location[i] = 1;
	}
	//ask placement from player2
	shift=0;

	while( ((GPIOA_IDR & (1<<8)) >>8) != 1 );

	
	shift = shift2();
	if (shift > 5) shift = 5;
	selectionMade();

	for(i=shift;i<shift+3; i++){	
		ship2_location[i] = 1;
	}
}

void I2C1_Init(){
	int OwnAddress = 0xAA;
	
	//enable the I2C clock and select SYSCLK as the Source
	RCC_APB1ENR |= 1 << 21; //I2C1 clock enable 
	
	//I2C CR1 Configuration
	//when the I2C is disabled (PE = 0), the I2C performs a software reset
	I2C1_CR1 &=~ I2C_CR1_PE; // Disable I2C
	I2C1_CR1 &= ~I2C_CR1_ANOFF;  // 0: Analog noise filter enabled
	I2C1_CR1 &= ~I2C_CR1_DNF; //0000: Digital filter disabled
	I2C1_CR1 |= I2C_CR1_ERRIE; //Errors interrupt enable
	I2C1_CR1 &= ~I2C_CR1_NOSTRETCH; // Enable clock stretching
	I2C1_CR1 |= I2C_CR1_ADDRIE; //ADDR interrupt enable
	I2C1_CR1 |= I2C_CR1_TXIE; //TX interrupt enable
	I2C1_CR1 |= I2C_CR1_RXIE; //RX interrupt enable
	I2C1_CR1 |= I2C_CR1_TCIE; //TC interrupt enable

	//I2C TIMINGR Configuration
	I2C1_TIMINGR = 0;
	//sysTimer = 80MHz, presc = 7, 800MHz/1+7 = 10 MHz
	I2C1_TIMINGR &= ~ I2C_TIMINGER_PRESC // clear the prescaler
	I2C1_TIMINGR |= 7<<28; // set clock prescaler to 7
	I2C1_TIMINGR |= 49; // SCLL: SCL Low Period(master mode) > 4.7us
	I2C1_TIMINGR |= 49<<8; //SCLH: SCL high period(master mode) > 4.0us
	I2C1_TIMINGR |= 14<<20; //SCLDEL: Data setup time >100us
	I2C1_TIMINGR |= 15<<16; //SDADEL: Data hold time > 1.25 us
	
	//I2C Own address 1 register (I2C_OAR1)
	I2C1_OAR1 &= ~I2C_OAR1_OA1EN;
	I2C1_OAR1 = I2C_OAR1_OA1EN | OwnAddress; // 7bit own address
	I2C1_OAR2 &= ~I2C_OAR2_OA2EN; //Disable own address 2
	
	//I2C CR2 Configuration
	I2C1_CR2 &= ~I2C_CR2_ADD10; // 0 = 7-bit mode, 1 = 10-bit mode
	I2C1_CR2 |= I2C_CR2_AUTOEND;// Enable the auto end
	I2C1_CR2 |= I2C_CR2_NACK; // for slave mode: set NACK
	I2C1_CR1 |= I2C_CR1_PE; // peripheral enable
}

void I2C1_Start (unsigned long SlaveAddress, char size, char direction){
	//direction = 0: master requests a write transfer
	//direction = 1: master requests a read transfer
			
	unsigned long tmpreg = I2C1_CR2;
	tmpreg &= (unsigned long) ~ ((unsigned long)		(I2C_CR2_SADD | I2C_CR2_NBYTES |
																									 I2C_CR2_RELOAD| I2C_CR2_AUTOEND|
																									 I2C_CR2_RD_WRN| I2C_CR2_START  |
																									 I2C_CR2_STOP));
	if(direction == READ_FROM_SLAVE)
		tmpreg |= I2C_CR2_RD_WRN; //Read from slave
	else
		tmpreg &= ~I2C_CR2_RD_WRN; //write to slave
			
	tmpreg |= (unsigned long)(((unsigned long) SlaveAddress & I2C_CR2_SADD) |
															 (((unsigned long) size << 16) & I2C_CR2_NBYTES));
			
	tmpreg |= I2C_CR2_START;
	I2C1_CR2 = tmpreg;
}
		
		
void I2C1_Stop(){
	//master: generate stop bit after the current byte has been transferred
	I2C1_CR2 |= I2C_CR2_STOP;
			
	//wait until STOPF flag is reset
	while( (I2C1_ISR & I2C_ISR_STOPF) == 0);
			
	I2C1_ICR |= I2C_ICR_STOPCF; //write 1 to clear stopf flag
}
		
void I2C1_WaitLineIdle(){
	//wait until I2C bus is ready
	while( (I2C1_ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY); //if busy, wait
}
		
void I2C3_Init(){
	int OwnAddress = 0x53;
	
	//enable the I2C clock and select SYSCLK as the Source
	RCC_APB1ENR |= 1 << 23; //I2C3 clock enable 

	
	//I2C CR1 Configuration
	//when the I2C is disabled (PE = 0), the I2C performs a software reset
	I2C3_CR1 &=~ I2C_CR1_PE; // Disable I2C
	I2C3_CR1 &= ~I2C_CR1_ANOFF;  // 0: Analog noise filter enabled
	I2C3_CR1 &= ~I2C_CR1_DNF; //0000: Digital filter disabled
	I2C3_CR1 |= I2C_CR1_ERRIE; //Errors interrupt enable
	//I2C1_CR1 &= ~I2C_CR1_SMBUS; // SMBus Mode: 0 = I2C mode; 1 = SMBus mode
	I2C3_CR1 &= ~I2C_CR1_NOSTRETCH; // Enable clock stretching
	I2C3_CR1 |= I2C_CR1_ADDRIE; //ADDR interrupt enable
	I2C3_CR1 |= I2C_CR1_TXIE; //TX interrupt enable
	I2C3_CR1 |= I2C_CR1_RXIE; //RX interrupt enable
	I2C3_CR1 |= I2C_CR1_TCIE; //TC interrupt enable
	
	//I2C TIMINGR Configuration
	I2C3_TIMINGR = 0;
	//sysTimer = 80MHz, presc = 7, 800MHz/1+7 = 10 MHz
	I2C3_TIMINGR &= ~ I2C_TIMINGER_PRESC // clear the prescaler
	I2C3_TIMINGR |= 7<<28; // set clock prescaler to 7
	I2C3_TIMINGR |= 49; // SCLL: SCL Low Period(master mode) > 4.7us
	I2C3_TIMINGR |= 49<<8; //SCLH: SCL high period(master mode) > 4.0us
	I2C3_TIMINGR |= 14<<20; //SCLDEL: Data setup time >100us
	I2C3_TIMINGR |= 15<<16; //SDADEL: Data hold time > 1.25 us
	
	//I2C Own address 1 register (I2C_OAR1)
	I2C3_OAR1 &= ~I2C_OAR1_OA1EN;
	I2C3_OAR1 = I2C_OAR1_OA1EN | OwnAddress; // 7bit own address
	I2C3_OAR2 &= ~I2C_OAR2_OA2EN; //Disable own address 2
	
	//I2C CR2 Configuration
	I2C3_CR2 &= ~I2C_CR2_ADD10; // 0 = 7-bit mode, 1 = 10-bit mode
	I2C3_CR2 |= I2C_CR2_AUTOEND;// Enable the auto end
	I2C3_CR2 |= I2C_CR2_NACK; // for slave mode: set NACK
	I2C3_CR1 |= I2C_CR1_PE; // enable I2C1
}

void I2C3_Start (unsigned long SlaveAddress, char size, char direction){
	//direction = 0: master requests a write transfer
	//direction = 1: master requests a read transfer
			
	unsigned long tmpreg = I2C3_CR2;
	tmpreg &= (unsigned long) ~ ((unsigned long)		(I2C_CR2_SADD | I2C_CR2_NBYTES |
																									 I2C_CR2_RELOAD| I2C_CR2_AUTOEND|
																									 I2C_CR2_RD_WRN| I2C_CR2_START  |
																									 I2C_CR2_STOP));
	if(direction == READ_FROM_SLAVE)
		tmpreg |= I2C_CR2_RD_WRN; //Read from slave
	else
		tmpreg &= ~I2C_CR2_RD_WRN; //write to slave
			
	tmpreg |= (unsigned long)(((unsigned long) SlaveAddress & I2C_CR2_SADD) |
															 (((unsigned long) size << 16) & I2C_CR2_NBYTES));
			
	tmpreg |= I2C_CR2_START;
	I2C3_CR2 = tmpreg;
}
		
		
void I2C3_Stop(){
	//master: generate stop bit after the current byte has been transferred
	I2C3_CR2 |= I2C_CR2_STOP;
			
	//wait until STOPF flag is reset
	while( (I2C3_ISR & I2C_ISR_STOPF) == 0);
			
	I2C3_ICR |= I2C_ICR_STOPCF; //write 1 to clear stopf flag
}
		
void I2C3_WaitLineIdle(){
	//wait until I2C bus is ready
	while( (I2C3_ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY); //if busy, wait
}
	
void I2C1_EV_IRQHandler(){


	//why am i in here
	char direction; // read or write?
	char Recieved_Address; 
		

	if (I2C1_ISR & I2C_ISR_ADDR) // check to see if slave is called
	{			
		
		I2C1_ICR|=I2C_ISR_ADDR; 
		Recieved_Address = ((I2C1_ISR & I2C_ISR_ADDCODE) >> 17);
		direction = ((I2C1_ISR & I2C_ISR_DIR) >> 16);  // means either read or write
		
		if (direction == 1) // read
		{
			//slave writes to master
			//that is I2C1 to I2C3
			//the interrupt will turn off automatically when the data is read out of the TXDR
			I2C1_TXDR = I2C1_TXDR - (I2C1_TXDR & 0xFF) + sentData[0];
	  }
		else
		{
		
			//master writes to slave
			//you don't need to do anything in this case
			//you will just receive data automatically
		}
	}
	else if (I2C1_ISR & I2C_ISR_TXIS) {
		//TXDR is empty and needs to be reloaded
		
			I2C1_TXDR = I2C1_TXDR - (I2C1_TXDR & 0xFF) + sentData[0];
	}

	else if (I2C1_ISR & I2C_ISR_RXNE) {
		//This bit is set by hardware when the received data is copied into the I2C_RXDR register, and is
		//ready to be read. It is cleared when I2C_RXDR is read.
		receivedData[0] = I2C1_RXDR;
		if(receivedData[0] >= 0 && receivedData[0] <= 7){
				sentData[0] = check_for_hit1(receivedData[0]);
		}
		else if(receivedData[0] == 10)
		{
			player1_hits +=1;
			blink_green(3);
		}
		else if(receivedData[0] == 60)
		{
			blink_red(3);
		}	;
	}else if (I2C1_ISR & I2C_ISR_TC){
		//if transfer is complete, send stop bit
		I2C1_Stop();
	}
}

void I2C3_EV_IRQHandler(){
	//why am i in here
	char direction;
	char Recieved_Address;

	if (I2C3_ISR & I2C_ISR_ADDR) // check to see if slave is called
	{			
		
		I2C3_ICR|=I2C_ISR_ADDR; 
		Recieved_Address = ((I2C3_ISR & I2C_ISR_ADDCODE) >> 17);
		direction = ((I2C3_ISR & I2C_ISR_DIR) >> 16);  // means either read or write
		
		if (direction == 1) // read
		{
			//slave writes to master
			//that is I2C1 to I2C3
			//the interrupt will turn off automatically when the data is read out of the TXDR
			I2C3_TXDR = I2C3_TXDR - (I2C3_TXDR & 0xFF) + sentData[0];
	  }
		else
		{
			//master writes to slave
			//you don't need to do anything in this case
			//you will just receive data automatically
		}
	}
	else if (I2C3_ISR & I2C_ISR_TXIS) {
			I2C3_TXDR = I2C3_TXDR - (I2C3_TXDR & 0xFF) + sentData[0];
	}
	
	else if (I2C3_ISR & I2C_ISR_RXNE) {
		receivedData[0] = I2C3_RXDR;
		if(receivedData[0] >= 0 && receivedData[0] <= 7){
				sentData[0] = check_for_hit2(receivedData[0]);
		}
		else if(receivedData[0] == 10)
		{
			blink_green(3);
			player1_hits +=1;
		}
		
		else if(receivedData[0] == 60)
		{
				blink_red(3);
		}
		
		//This bit is set by hardware when the received data is copied into the I2C_RXDR register, and is
		//ready to be read. It is cleared when I2C_RXDR is read.
	}else if (I2C3_ISR & I2C_ISR_TC){
		I2C3_Stop();
	}
}
void GPIOA_Initialize(void){
	
	// I2C1
	RCC_AHB2ENR |= 1; // enable port A clock
	GPIOA_MODER &= ~(3<<(2*9)); // clear PA9
	GPIOA_MODER &= ~(3<<(2*10)); // clear PA10
	GPIOA_MODER |= 2<<(2*9); // PA9 alt function
	GPIOA_MODER |= 2<<(2*10); // PA10 alt function
	GPIOA_AFRH |= 4 << (4* (9-8)); // PA9 I2C1_SCL
	GPIOA_AFRH |= 4 << (4* (10-8)); // PA10 I2C1_SDA
	GPIOA_OTYPER |= 1<<9; //PA9 open drain
	GPIOA_OTYPER |= 1<<10; //PA10 open drain
	
	// I2C3
	RCC_AHB2ENR |= 1<<1; // enable port B clock
	GPIOA_MODER &= ~(3<<(2*7)); // clear PA7
	GPIOB_MODER &= ~(3<<(2*4)); // clear PB4
	GPIOA_MODER |= 2<<(2*7); // PA7 alt function
	GPIOB_MODER |= 2<<(2*4); // PB4 alt function
	GPIOA_AFRL |= 4 << (4* (7)); // PA7 I2C3_SCL
	GPIOB_AFRL	|= 4 << (4* (4)); // PB4 I2C3_SDA
	GPIOA_OTYPER |= 1<<7; //PA7 open drain
	GPIOB_OTYPER |= 1<<4; //PB4 open drain
	GPIOB_PUPDR &= ~(3<<2*4);
	

	//I2C interrupt
	NVIC_ISER0 |= (1<<31); //I2C1 interrupt initialize
	NVIC_ISER2 |= (1<<8); //I2C3 interrupt initialize
	
	//Green LED
	GPIOB_MODER &= ~(3<<(2*3)); //clear PB3 
  GPIOB_MODER |= 1<<(2*3); //PB3 output
	
	
	//Red LED PA11
	GPIOA_MODER &= ~(3<<(11*2)); 	//clear PA11
	GPIOA_MODER |= (1<< 2*11);    //pin 11

	//blue LED PB5
	GPIOB_MODER &= ~(3<<(5*2)); 	//clear PB5
	GPIOB_MODER |= (1<< 2*5);    //pin 5
	
	//User input
	GPIOA_MODER &= ~(0xFFF);	//PA0-PA6 
	GPIOA_MODER &= ~(3<<(8*2)); 	//PA8 

	//set input pins to pulldown
	GPIOA_PUPDR &= ~(0xFFF);	//PA0-PA6 
	GPIOA_PUPDR &= ~(3<<(8*2)); 	//PA8  
	GPIOA_PUPDR |= 0xAAA;
	GPIOA_PUPDR |= (2<<(8*2));
}
	
void blink_green(int num_times){
	int i,j;
	for(i=0; i<num_times;i++){
		GPIOB_BSRR |= LED_ON; //Turn on LED 
		for(j=0; j<100000; j++); //short software delay
			GPIOB_BSRR |= LED_OFF;  //Turn off LED 
		for(j=0; j<100000; j++); //short software delay
	} 
}
void blink_blue(int num_times){
	int i,j;
	for(i=0; i<num_times;i++){
		GPIOA_ODR |= 1 << 11; //Turn on PA11 
		for(j=0; j<100000; j++); //short software delay
			GPIOA_ODR&= ~(1<<11);  //Turn off PA11 
		for(j=0; j<100000; j++); //short software delay
	} 
}

void blink_red(int num_times){
	int i,j;
	for(i=0; i<num_times;i++){
		GPIOB_ODR |= 1 << 5; //Turn on PA11 
		for(j=0; j<100000; j++); //short software delay
			GPIOB_ODR&= ~(1<<5);  //Turn off PA11 
		for(j=0; j<100000; j++); //short software delay
	} 
}