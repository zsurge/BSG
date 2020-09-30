#ifndef __BSP_LED_H
#define __BSP_LED_H
#include "sys.h"


//LED端口定义
#define RCC_ALL_LED     (RCC_AHB1Periph_GPIOG)

#define GPIO_PORT          GPIOG

#define GPIO_PIN_ERRORLED   GPIO_Pin_3






//LED端口定义
#define LEDERROR PGout(3)	





void bsp_LED_Init(void);//初始化		 	



//void bsp_LedToggle(uint8_t _no);
#endif
