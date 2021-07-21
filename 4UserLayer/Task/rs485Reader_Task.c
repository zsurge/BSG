/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : rs485Reader_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年9月29日
  最近修改   :
  功能描述   : 485读卡器任务
  函数列表   :
  修改历史   :
  1.日    期   : 2020年9月29日
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
#include "rs485Reader_Task.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
 
#define RS485READER_TASK_PRIO		(tskIDLE_PRIORITY + 6)
#define RS485READER_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)


#define UNFINISHED		        	    0x00
#define FINISHED          	 			0x55

#define STARTREAD		        	    0x00
#define ENDREAD         	 			0xAA

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *rS485ReaderTaskName = "vRS485ReadTask";  

typedef struct FROMREADER
{
    uint8_t rxBuff[64];               //接收字节数    
    uint8_t rxStatus;                   //接收状态
    uint16_t rxCnt;                     //接收字节数
}FROMREADER_STRU;

static FROMREADER_STRU gReaderData;


/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskRs485Reader = NULL;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskRs485Reader(void *pvParameters);
static  uint8_t parseReader(void);


void CreateRs485ReaderTask(void)
{
    //读取条码数据并处理
    xTaskCreate((TaskFunction_t )vTaskRs485Reader,     
                (const char*    )rS485ReaderTaskName,   
                (uint16_t       )RS485READER_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )RS485READER_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskRs485Reader);
}


static void vTaskRs485Reader(void *pvParameters)
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
        
            if(parseReader() == FINISHED)
            {      
                
                if(gReaderData.rxCnt > sizeof(buf))
                {   
                    memset(&gReaderData,0x00,sizeof(gReaderData));
                    continue;
                }
                
                memcpy(buf,gReaderData.rxBuff,gReaderData.rxCnt-2);  
                memset(&gReaderData,0x00,sizeof(gReaderData));
                
                if(strstr_t((const char*)buf,(const char*)"CARD") == NULL)
                {
                    continue;
                }
                
                memcpy(cardAsc,buf + 9,8);
                log_d("cardAsc = %s\r\n",cardAsc);
                
                asc2bcd(cardBcd,cardAsc, 8, 1);
                log_d("cardid %02x,%02x,%02x,%02x\r\n",cardBcd[0],cardBcd[1],cardBcd[2],cardBcd[3]);                
                cardBcd[0] = 0x00;
                
                ptReaderBuf->devID = READER1; 
                ptReaderBuf->mode = READMODE;
                memcpy(ptReaderBuf->cardID,cardBcd,sizeof(cardBcd)); 

    			/* 使用消息队列实现指针变量的传递 */
    			if(xQueueSend(xCardIDQueue,             /* 消息队列句柄 */
    						 (void *) &ptReaderBuf,             /* 发送结构体指针变量ptReader的地址 */
    						 (TickType_t)10) != pdPASS )
    			{
    //                xQueueReset(xCardIDQueue); 删除该句，为了防止在下发数据的时候刷卡
                    DBG("send card1  queue is error!\r\n"); 
                    //发送卡号失败蜂鸣器提示
                    //或者是队列满                
                } 
            
            }
            
            /* 发送事件标志，表示任务正常运行 */        
            xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_2);  
            vTaskDelay(100); 
        }

}

static uint8_t parseReader(void)
{
    uint8_t ch = 0;   
    
    while(1)
    {    
        //读取485数据，若是没读到，退出，再重读
        if(!RS485_Recv(COM3,&ch,1))
        {            
            return UNFINISHED;
        }
        
        //读取缓冲区数据到BUFF
        gReaderData.rxBuff[gReaderData.rxCnt++] = ch;
        
//        log_d("ch = %c,gReaderData.rxBuff = %c \r\n",ch,gReaderData.rxBuff[gReaderData.rxCnt-1]);
         
        //最后一个字节为回车，或者总长度为510后，结束读取
        if(gReaderData.rxBuff[gReaderData.rxCnt-1] == 0x0A || gReaderData.rxCnt >=64)
        {   
            
           if(gReaderData.rxBuff[gReaderData.rxCnt-1] == 0x0A)
           {
//                dbh("gReaderData.rxBuff", (char*)gReaderData.rxBuff, gReaderData.rxCnt);  
//                log_d("gReaderData.rxBuff = %s,len = %d\r\n",gReaderData.rxBuff,gReaderData.rxCnt-1);
                return FINISHED;
           }

            //若没读到最后一个字节，但是总长到位后，清空，重读缓冲区
            memset(&gReaderData,0xFF,sizeof(FROMREADER_STRU));
        }
        
    }   

   
}


