#include <gpio.h>
#include <rcc.h>

void SystemInit(void);

void SystemInit(void)
{
}

int main()
{
	RCC.APB2_ENR.BITS.IOPB = 1;
	GPIO_Mode(&GPIOB, 1UL << 12, GPIO_MODE_OUTPUT_PUSHPULL_10MHz);
	while(1)
	{
		GPIOB.ODR.BITS.b12 = !GPIOB.ODR.BITS.b12;
		for(unsigned int i = 0; i < 500000; i++);
	}
}
