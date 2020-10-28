/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : OpenDoor_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月28日
  最近修改   :
  功能描述   : 跟电梯通讯的任务处理文件
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月28日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#define LOG_TAG    "CommTask"
#include "elog.h"

#include "OpenDoor_Task.h"
#include "bsp_dipSwitch.h"
#include "bsp_time.h"



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
 
#define OPENDOOR_TASK_PRIO		(tskIDLE_PRIORITY + 8) 
#define OPENDOOR_STK_SIZE 		(configMINIMAL_STACK_SIZE*2)


/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *OpenDoorTaskName = "vOpenDoorTask"; 

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskOpenDoor = NULL;  

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskOpenDoor(void *pvParameters);



void CreateOpenDoorTask(void)
{
    xTaskCreate((TaskFunction_t )vTaskOpenDoor,         
                (const char*    )OpenDoorTaskName,       
                (uint16_t       )OPENDOOR_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )OPENDOOR_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskOpenDoor);
}               



static void vTaskOpenDoor(void *pvParameters)
{ 
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(50); /* 设置最大等待时间为100ms */     

    uint32_t devID = 0;

    while(1)
    { 
        if(gOpenDoorTimer.flag == 1 && gOpenDoorTimer.outTimer == 0)
        {
            gOpenDoorTimer.flag = 0;
            log_d("close 1 door\r\n");
            SW1_HI();                    
        }

        if(gOpenDoor2Timer.flag ==1 && gOpenDoor2Timer.outTimer == 0)
        {
            gOpenDoor2Timer.flag = 0;
            log_d("close 2 door\r\n");
            SW2_HI();                    
        }        

        xReturn = xQueueReceive( xCmdQueue,    /* 消息队列的句柄 */
                                 (void *)&devID,  /*这里获取的是结构体的地址 */
                                 xMaxBlockTime); /* 设置阻塞时间 */
        if(pdTRUE == xReturn)
        {            
            log_d("recv id = %d\r\n",devID);
            
            if(devID == READER1)
            {            
                log_d("open 1 door\r\n");
                SW1_LOW();  
            }
            else if(devID == READER1)
            {
                log_d("open 2 door\r\n");
                SW2_LOW();
            }   
        }
        
        xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);   
        vTaskDelay(30); 
        
    }  
}



