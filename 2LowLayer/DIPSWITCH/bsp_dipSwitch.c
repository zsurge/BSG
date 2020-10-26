/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : bsp_dipSwitch.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年12月23日
  最近修改   :
  功能描述   : 拨码开关驱动
  函数列表   :
  修改历史   :
  1.日    期   : 2019年12月23日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "bsp_dipSwitch.h"
#include "tool.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
void bsp_dipswitch_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOG, ENABLE );	//使能GPIOG时钟


	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_DIP0|GPIO_PIN_DIP1|GPIO_PIN_DIP2|GPIO_PIN_DIP3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;			//普通输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		    //上拉
	GPIO_Init ( GPIO_PORT_DIPSWITCH, &GPIO_InitStructure );		//初始化GPIO

	GPIO_SetBits ( GPIO_PORT_DIPSWITCH,GPIO_PIN_DIP0|GPIO_PIN_DIP1|GPIO_PIN_DIP2|GPIO_PIN_DIP3);

}

//拨码开关，拨到ON时，该bit置1
int16_t bsp_dipswitch_read(void)
{
    static int16_t value = 0;

    if(DIP0 == 1)
    {
        CLRBIT(value, 0);
    }
    else
    {
        SETBIT(value, 0);
    }

    if(DIP1 == 1)
    {
        CLRBIT(value, 1);
    }
    else
    {
        SETBIT(value, 1);
    }

    if(DIP2 == 1)
    {
        CLRBIT(value, 2);
    }
    else
    {
        SETBIT(value, 2);
    }

    if(DIP3 == 1)
    {
        CLRBIT(value, 3);
    }
    else
    {
        SETBIT(value, 3);
    }    

    return value;
}


void bsp_sw_init(void)
{
     GPIO_InitTypeDef  GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG|RCC_AHB1Periph_GPIOD, ENABLE);//使能GPIOG,GPIOD时钟
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOG, &GPIO_InitStructure);//初始化GPIOE2,3,4
    
    GPIO_SetBits(GPIOG, GPIO_Pin_2);
       
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;//WK_UP对应引脚PA0
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;//下拉
    GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIOA0

    GPIO_SetBits(GPIOD, GPIO_Pin_15);
}

