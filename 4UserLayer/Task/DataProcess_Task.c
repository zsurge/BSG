/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : DataProcess_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年5月15日
  最近修改   :
  功能描述   : 对刷卡/QR/远程送过来的数据进行处理
  函数列表   :
  修改历史   :
  1.日    期   : 2020年5月15日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#define LOG_TAG    "DataProcess"
#include "elog.h"

#include "DataProcess_Task.h"
#include "CmdHandle.h"
#include "bsp_uart_fifo.h"
#include "jsonUtils.h"
#include "malloc.h"
#include "tool.h"
#include "bsp_time.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

#define DATAPROC_TASK_PRIO		(tskIDLE_PRIORITY + 6) 
#define DATAPROC_STK_SIZE 		(configMINIMAL_STACK_SIZE*12)

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *dataProcTaskName = "vDataProcTask"; 

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskDataProc = NULL;  
CMD_BUFF_STRU gCmd_buff = {0};


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
 
static void vTaskDataProcess(void *pvParameters);

void CreateDataProcessTask(void)
{
    xTaskCreate((TaskFunction_t )vTaskDataProcess,         
                (const char*    )dataProcTaskName,       
                (uint16_t       )DATAPROC_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )DATAPROC_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskDataProc);
}


static void vTaskDataProcess(void *pvParameters)
{    
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(30); /* 设置最大等待时间为100ms */ 
    int ret = 0;
    int len = 0;

    uint8_t openLeft[8] = { 0x02,0x00,0x07,0x01,0x06,0x4c,0x03,0x4D };    
    uint8_t openRight[8] = { 0x02,0x00,0x07,0x01,0x06,0x52,0x03,0x53 };
    uint8_t jsonbuff[256] = {0};

    
    CMD_BUFF_STRU *ptCmd = &gCmd_buff;
    READER_BUFF_STRU *ptMsg  = &gReaderRecvMsg;
    memset(&gReaderRecvMsg,0x00,sizeof(READER_BUFF_STRU));   
    memset(&gCmd_buff,0x00,sizeof(CMD_BUFF_STRU));       
    
    while (1)
    {

        if(gCardSortTimer.flag && gCardSortTimer.cardSortTimer == 0)
        {
            log_d("-----start sort-----\r\n");
            gCardSortTimer.flag = 0;
            sortLastPageCard();
        }
        
        xReturn = xQueueReceive( xCardIDQueue,    /* 消息队列的句柄 */
                                 (void *)&ptMsg,  /*这里获取的是结构体的地址 */
                                 xMaxBlockTime); /* 设置阻塞时间 */
        if(pdTRUE != xReturn)
        {  
            //没有接收到数据
            //队列满
            continue;
        }

        //消息接收成功，发送接收到的消息                
        log_d("cardid %02x,%02x,%02x,%02x,devid = %d,mode = %d\r\n",ptMsg->cardID[0],ptMsg->cardID[1],ptMsg->cardID[2],ptMsg->cardID[3],ptMsg->devID,ptMsg->mode);

        if(ptMsg->mode == DOWNLOAD_CARD_MODE)
        {
            ret = addCard(ptMsg->cardID,CARD_MODE);
            
            if(ret != 1)
            {
               //1.添加用户失败，重启，继续添加
               //2.排序
            }              
        }
        else if(ptMsg->mode == READMODE)
        {        
            ret = readHead(ptMsg->cardID, CARD_MODE);
            log_d("readHead = %d\r\n",ret);
            
            if(ret != NO_FIND_HEAD)
            {
                log_d("read card success\r\n");     

                ptCmd->cmd_len = 8;
                
                if(ptMsg->devID == 1)
                {
                    memcpy(ptCmd->cmd,openLeft,ptCmd->cmd_len);                
                }
                else
                {
                    memcpy(ptCmd->cmd,openRight,ptCmd->cmd_len); 
                } 

    			/* 使用消息队列实现指针变量的传递 */
    			if(xQueueSend(xCmdQueue,             /* 消息队列句柄 */
    						 (void *) &ptCmd,             /* 发送结构体指针变量ptReader的地址 */
    						 (TickType_t)30) != pdPASS )
    			{
                    xQueueReset(xCmdQueue);
                    DBG("send card2  queue is error!\r\n"); 
                    //发送卡号失败蜂鸣器提示
                    //或者是队列满                
                }
                else
                {
                    log_d("2 cardid %02x,%02x,%02x,%02x,devid = %d\r\n",ptMsg->cardID[0],ptMsg->cardID[1],ptMsg->cardID[2],ptMsg->cardID[3],ptMsg->devID);
                    //打包数据                
                    packetCard(ptMsg->cardID, jsonbuff);

                    //发送数据到MQTT服务器
                    len = strlen((const char*)jsonbuff);

                    len = mqttSendData(jsonbuff,len);
                    
                    log_d("send = %d\r\n",len);   

                    
                }
            }
            else
            {
                log_d("read card error: not find card\r\n");
            }  
        }
        
        
        /* 发送事件标志，表示任务正常运行 */        
        xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_4); 
        vTaskDelay(30); 

    }

}


