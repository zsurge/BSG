/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : rs485Reader2_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2021年7月21日
  最近修改   :
  功能描述   : 485读卡器2任务
  函数列表   :
  修改历史   :
  1.日    期   : 2021年7月21日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

#define LOG_TAG    "BarCode"
#include "elog.h"

#include "tool.h"
#include "CmdHandle.h"
#include "bsp_uart_fifo.h"
#include "rs485Reader2_Task.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
 
#define RS485READER2_TASK_PRIO		(tskIDLE_PRIORITY + 5)
#define RS485READER2_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)


#define UNFINISHED		        	    0x00
#define FINISHED          	 			0x55

#define STARTREAD		        	    0x00
#define ENDREAD         	 			0xAA

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *rS485Reader2TaskName = "vRS485Read2Task";  

typedef struct FROMREADER
{
    uint8_t rxBuff[64];               //接收字节数    
    uint8_t rxStatus;                   //接收状态
    uint16_t rxCnt;                     //接收字节数
}FROMREADER2_STRU;

static FROMREADER2_STRU gReader2Data;


/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskRs485Reader2 = NULL;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskRs485Reader2(void *pvParameters);
static  uint8_t parseReader2(void);


void CreateRs485Reader2Task(void)
{
    //读取条码数据并处理
    xTaskCreate((TaskFunction_t )vTaskRs485Reader2,     
                (const char*    )rS485Reader2TaskName,   
                (uint16_t       )RS485READER2_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )RS485READER2_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskRs485Reader2);
}


static void vTaskRs485Reader2(void *pvParameters)
{ 
        uint8_t buf[64] = {0};  
        uint8_t cardAsc[9] = {0};
        uint8_t cardBcd[4] = {0};
        
        READER_BUFF_STRU *ptReaderBuf = &gReaderMsg;     

        while(1)
        { 
            /* 清零 */
            ptReaderBuf->devID = 0; 
            ptReaderBuf->mode = 0;
            memset(ptReaderBuf->cardID,0x00,sizeof(ptReaderBuf->cardID));  
            
            memset(buf,0x00,sizeof(buf));
            memset(cardAsc,0x00,sizeof(cardAsc));
            memset(cardBcd,0x00,sizeof(cardBcd));
        
            if(parseReader2() == FINISHED)
            {      
                
                if(gReader2Data.rxCnt > sizeof(buf))
                {   
                    memset(&gReader2Data,0x00,sizeof(gReader2Data));
                    continue;
                }
                
                memcpy(buf,gReader2Data.rxBuff,gReader2Data.rxCnt-2);  
                memset(&gReader2Data,0x00,sizeof(gReader2Data));
                
                if(strstr_t((const char*)buf,(const char*)"CARD") == NULL)
                {
                    continue;
                }
                
                memcpy(cardAsc,buf + 9,8);
                log_d("cardAsc = %s\r\n",cardAsc);
                
                asc2bcd(cardBcd,cardAsc, 8, 1);
                log_d("cardid %02x,%02x,%02x,%02x\r\n",cardBcd[0],cardBcd[1],cardBcd[2],cardBcd[3]);                
                cardBcd[0] = 0x00;
                
                ptReaderBuf->devID = READER2; 
                ptReaderBuf->mode = READMODE;
                memcpy(ptReaderBuf->cardID,cardBcd,sizeof(cardBcd)); 

    			/* 使用消息队列实现指针变量的传递 */
    			if(xQueueSend(xCardIDQueue,             /* 消息队列句柄 */
    						 (void *) &ptReaderBuf,             /* 发送结构体指针变量ptReader的地址 */
    						 (TickType_t)20) != pdPASS )
    			{
    //                xQueueReset(xCardIDQueue); 删除该句，为了防止在下发数据的时候刷卡
                    DBG("send card1  queue is error!\r\n"); 
                    //发送卡号失败蜂鸣器提示
                    //或者是队列满                
                } 
            
            }
            
            /* 发送事件标志，表示任务正常运行 */        
            xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_5);  
            vTaskDelay(80); 
        }

}

static uint8_t parseReader2(void)
{
    uint8_t ch = 0;   
    
    while(1)
    {    
        //读取485数据，若是没读到，退出，再重读
        if(!RS485_Recv(COM2,&ch,1))
        {            
            return UNFINISHED;
        }
        
        //读取缓冲区数据到BUFF
        gReader2Data.rxBuff[gReader2Data.rxCnt++] = ch;
        
//        log_d("ch = %c,gReaderData.rxBuff = %c \r\n",ch,gReaderData.rxBuff[gReaderData.rxCnt-1]);
         
        //最后一个字节为回车，或者总长度为510后，结束读取
        if(gReader2Data.rxBuff[gReader2Data.rxCnt-1] == 0x0A || gReader2Data.rxCnt >=64)
        {   
            
           if(gReader2Data.rxBuff[gReader2Data.rxCnt-1] == 0x0A)
           {
//                dbh("gReaderData.rxBuff", (char*)gReaderData.rxBuff, gReaderData.rxCnt);  
//                log_d("gReaderData.rxBuff = %s,len = %d\r\n",gReaderData.rxBuff,gReaderData.rxCnt-1);
                return FINISHED;
           }

            //若没读到最后一个字节，但是总长到位后，清空，重读缓冲区
            memset(&gReader2Data,0xFF,sizeof(FROMREADER2_STRU));
        }
        
    }   

   
}


