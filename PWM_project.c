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

//-----Configuration of using 3 pin in output mode (use IC74HC595)------
void IC74HC595_init (void)
{
	RCC->APB2ENR |= (1<<3);//clock for GPIOB
	
	GPIOB->CRL = 0x00;//Clear
	GPIOB-> CRL |= (1<<(4*0)) | (1<<(4*1)) | (1<<(4*3));//Output Push Pull PB0, PB1, PB3
}

//-----Configuration of Timer 2 for PWM------
void TIM2_PWM_Init (uint16_t psc, uint16_t arr)
{
	RCC->APB1ENR |= (1<<0); //clock for TIM2
	RCC->APB2ENR |= (1<<2); // clock for GPIOA
	
	GPIOA->CRL = 0x00;//Clear
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
//------------------------EXTI for 4 buttons-------
void Button_EXTI ()
{
	RCC->APB2ENR |= (1<<0) | (1<<2);
	
	
	GPIOA->CRL |=(8<<(4*2)) | (8<<(4*3)) | (8<<(4*4)) | (8<<(4*5));  //Input PULL Up/Down PA1,PA2,PA3
	GPIOA->ODR &= ~(1<<2) | ~(1<<3) | ~(1<<4) | ~(1<<5); //Input PULL down PA1,PA2,PA3
	
	AFIO->EXTICR[0] &= ~(1<<8); //LINE 2
	AFIO->EXTICR[0] &= ~(1<<12); //LINE 3
	AFIO->EXTICR[1] &= ~(1<<0); //LINE 4
	AFIO->EXTICR[1] &= ~(1<<4); //LINE 5
	
	EXTI->IMR |= (1<<2) | (1<<3) | (1<<4) | (1<<5);
	
	EXTI->RTSR |= (1<<2) | (1<<3) | (1<<4) | (1<<5);
	EXTI->FTSR &= ~(1<<2) | ~(1<<3) | ~(1<<4) | ~(1<<5);
	
	NVIC->ISER[0] |= (1<<8) | (1<<9) | (1<<10) | (1<<23);
}


int main(void)
{
	//Configuration for 3 pin output to use IC 74HC595
	Buttons_IC74HC595_init();
	
	//Configuration for Timer 2 to use PWM at pin PA1
	TIM2_PWM_Init(8,1000);
	
	//Configuration for 4 buttons using EXTI
	Button_EXTI();
	
	while (1)
  	{ 
		LED_PutNum(dv,chuc,tram);
	}
}

//--------Interrupt vector---------
void EXTI2_IRQHandler (void)
{
	tram++;
	if(tram>9) tram=0;
	EXTI->PR |= (1<<2);
}

void EXTI3_IRQHandler (void)
{
	chuc++;
	if(chuc>9) chuc=0;
	EXTI->PR |= (1<<3);
}

void EXTI4_IRQHandler (void)
{
	dv++;
	if(dv>9) dv=0;
	EXTI->PR |= (1<<4);
}

void EXTI9_5_IRQHandler (void)
{
	if (EXTI->PR & (1<<5))
	{
		D=tram*100+chuc*10+dv;
		TIM2_PWM_SetDuty(D);
	}
	EXTI->PR |= (1<<5);
}
