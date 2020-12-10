/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : rs485Reader_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��9��29��
  ����޸�   :
  ��������   : 485����������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��9��29��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/

#define LOG_TAG    "BarCode"
#include "elog.h"

#include "tool.h"
#include "CmdHandle.h"
#include "bsp_uart_fifo.h"
#include "rs485Reader_Task.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
 
#define RS485READER_TASK_PRIO		(tskIDLE_PRIORITY + 7)
#define RS485READER_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)


#define UNFINISHED		        	    0x00
#define FINISHED          	 			0x55

#define STARTREAD		        	    0x00
#define ENDREAD         	 			0xAA

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *rS485ReaderTaskName = "vRS485ReadTask";  

typedef struct FROMREADER
{
    uint8_t rxBuff[512];               //�����ֽ���    
    uint8_t rxStatus;                   //����״̬
    uint16_t rxCnt;                     //�����ֽ���
}FROMREADER_STRU;

static FROMREADER_STRU gReaderData;


/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskRs485Reader = NULL;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskRs485Reader(void *pvParameters);
static  uint8_t parseReader(void);


void CreateRs485ReaderTask(void)
{
    //��ȡ�������ݲ�����
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
            /* ���� */
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

    			/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    			if(xQueueSend(xCardIDQueue,             /* ��Ϣ���о�� */
    						 (void *) &ptReaderBuf,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
    						 (TickType_t)10) != pdPASS )
    			{
    //                xQueueReset(xCardIDQueue); ɾ���þ䣬Ϊ�˷�ֹ���·����ݵ�ʱ��ˢ��
                    DBG("send card1  queue is error!\r\n"); 
                    //���Ϳ���ʧ�ܷ�������ʾ
                    //�����Ƕ�����                
                } 
            
            }
            
            /* �����¼���־����ʾ������������ */        
            xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_2);  
            vTaskDelay(100); 
        }

}

static uint8_t parseReader(void)
{
    uint8_t ch = 0;   
    
    while(1)
    {    
        //��ȡ485���ݣ�����û�������˳������ض�
        if(!RS485_Recv(COM3,&ch,1))
        {            
            return UNFINISHED;
        }
        
        //��ȡ���������ݵ�BUFF
        gReaderData.rxBuff[gReaderData.rxCnt++] = ch;
        
//        log_d("ch = %c,gReaderData.rxBuff = %c \r\n",ch,gReaderData.rxBuff[gReaderData.rxCnt-1]);
         
        //���һ���ֽ�Ϊ�س��������ܳ���Ϊ510�󣬽�����ȡ
        if(gReaderData.rxBuff[gReaderData.rxCnt-1] == 0x0A || gReaderData.rxCnt >=510)
        {   
            
           if(gReaderData.rxBuff[gReaderData.rxCnt-1] == 0x0A)
           {
//                dbh("gReaderData.rxBuff", (char*)gReaderData.rxBuff, gReaderData.rxCnt);  
//                log_d("gReaderData.rxBuff = %s,len = %d\r\n",gReaderData.rxBuff,gReaderData.rxCnt-1);
                return FINISHED;
           }

            //��û�������һ���ֽڣ������ܳ���λ����գ��ض�������
            memset(&gReaderData,0xFF,sizeof(FROMREADER_STRU));
        }
        
    }   

   
}


