#include "main.h"
unsigned char maled_CC[10]= {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7f,0x6F}; // Display 0->9
// ----Private Variables---
uint16_t tram,chuc,dv =0;
char state_button=0;
char state_button_b=0;
char state_button_c=0;
char state_button_PWM=0;
unsigned int D=0;


void clock_74HC594() //PB0: CLK
{
	GPIOB->ODR |= (1<<0);//LOGIC 1 
	GPIOB->ODR &= ~(1<<0);//logic 0
}
void lath_74HC595()// PB1: Lath
{
	GPIOB->ODR |= (1<<1);//LOGIC 1
	GPIOB->ODR &= ~(1<<1);//logic 0
}
void data_74HC594 (uint8_t n) //PB2: Data
{
	for(char i=0;i<=7;i++)
	{
		if ((n & (1<<(7-i))) ==0) 
		{	
			GPIOB->ODR &= ~(1<<3);
		}
		else GPIOB->ODR |= (1<<3); 
		clock_74HC594();
	}
}

// -------Display on LED 7-segment---------
void LED_PutNum(uint8_t dv, uint8_t chuc, uint8_t tram)
{
	data_74HC594(maled_CC[dv]);
	data_74HC594(maled_CC[chuc]);
	data_74HC594(maled_CC[tram]);
	
	lath_74HC595();	
}

//-----Configuration of using 4 buttons (4 pins in input mode) and 3 pin in output mode (use IC74HC595)------
void Buttons_IC74HC595_init (void)
{
	RCC->APB2ENR |= (1<<2)|(1<<3);//clock for GPIOA,GPIOB
	
	GPIOB->CRL = 0x00;//Clear
	GPIOB-> CRL |= (1<<(4*0)) | (1<<(4*1)) | (1<<(4*3));//Output Push Pull PB0, PB1, PB3
	
	GPIOA->CRL = 0x00;//Clear
	GPIOA->CRL |=(8<<(4*2)) | (8<<(4*3)) | (8<<(4*4)) | (8<<(4*5));  //Input PULL Up/Down PA1,PA2,PA3
	GPIOA->ODR &= ~(1<<2) | ~(1<<3) | ~(1<<4) | ~(1<<5); //Input PULL down PA1,PA2,PA3
}

//-----Configuration of Timer 2 for PWM------
void TIM2_PWM_Init (uint16_t psc, uint16_t arr)
{
	RCC->APB1ENR |= (1<<0); //clock for TIM2
	RCC->APB2ENR |= (1<<2); // clock for GPIOA
	GPIOA->CRL |= (0xB<<4);// AF ouput push-pull for PA1
	
	TIM2->PSC = psc-1;
	TIM2->ARR = arr-1;
	
	TIM2->CCER |= TIM_CCER_CC2E; // PWM mode 1
	
	TIM2->CCMR1 |=(6<<TIM_CCMR1_OC2M_Pos);
	
	TIM2->CCR2=0;
	TIM2->CR1 |= (1<<0);
}
//-----Change value of duty cycle----
void TIM2_PWM_SetDuty (uint16_t duty)
{
	TIM2->CCR2=duty;
}

int main(void)
{
	Buttons_IC74HC595_init();
	TIM2_PWM_Init(8,1000);
	while (1)
  	{
		if (((GPIOA->IDR & (1<<2)) == (1<<2)) && (state_button ==0)) 
		{
			state_button =1; 
			tram++;
			if(tram>9) tram=0;
		}
		else if (((GPIOA->IDR & (1<<2)) == 0) && (state_button ==1))
		{
			state_button =0; 
		}
		if (((GPIOA->IDR & (1<<3)) == (1<<3)) && (state_button_b ==0)) 
		{
			state_button_b =1; 
			chuc++;
			if(chuc>9) chuc=0;
		}
		else if (((GPIOA->IDR & (1<<3)) == 0) && (state_button_b ==1))
		{
			state_button_b =0; 
		}
		if (((GPIOA->IDR & (1<<4)) == (1<<4)) && (state_button_c ==0)) 
		{
			state_button_c =1;
			dv++;
			if(dv>9) dv=0;
		}
		else if (((GPIOA->IDR & (1<<4)) == 0) && (state_button_c ==1))
		{
			state_button_c =0;
		}
		
			
		LED_PutNum(dv,chuc,tram);
			
			
		if (((GPIOA->IDR & (1<<5)) == (1<<5)) && (state_button_PWM ==0)) 
		{
			state_button_PWM =1; 
			D=tram*100+chuc*10+dv;
			TIM2_PWM_SetDuty(D);
		}
		else if (((GPIOA->IDR & (1<<5)) == 0) && (state_button_PWM ==1))
		{
			state_button_PWM =0; 
		}
	}
}
