#include "bsp_led.h" 
#include "string.h"


//初始化PF9和PF10为输出口.并使能这两个口的时钟		    
//LED IO初始化
void bsp_LED_Init(void)
{    	 
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_ALL_LED, ENABLE);//使能GPIOF时钟
    
      //GPIOFerr
      GPIO_InitStructure.GPIO_Pin = GPIO_PIN_ERRORLED;   //LED1对应IO口
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           //普通输出模式
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          //推挽输出
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;      //100MHz
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //上拉
      GPIO_Init(GPIO_PORT, &GPIO_InitStructure);         //初始化GPIO
      
      GPIO_SetBits(GPIO_PORT,GPIO_PIN_ERRORLED);
}


