/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : rs485Reader2_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2021��7��21��
  ����޸�   :
  ��������   : 485������2����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2021��7��21��
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
#include "rs485Reader2_Task.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
 
#define RS485READER2_TASK_PRIO		(tskIDLE_PRIORITY + 5)
#define RS485READER2_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)


#define UNFINISHED		        	    0x00
#define FINISHED          	 			0x55

#define STARTREAD		        	    0x00
#define ENDREAD         	 			0xAA

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *rS485Reader2TaskName = "vRS485Read2Task";  

typedef struct FROMREADER
{
    uint8_t rxBuff[64];               //�����ֽ���    
    uint8_t rxStatus;                   //����״̬
    uint16_t rxCnt;                     //�����ֽ���
}FROMREADER2_STRU;

static FROMREADER2_STRU gReader2Data;


/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskRs485Reader2 = NULL;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskRs485Reader2(void *pvParameters);
static  uint8_t parseReader2(void);


void CreateRs485Reader2Task(void)
{
    //��ȡ�������ݲ�����
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
            /* ���� */
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

    			/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    			if(xQueueSend(xCardIDQueue,             /* ��Ϣ���о�� */
    						 (void *) &ptReaderBuf,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
    						 (TickType_t)20) != pdPASS )
    			{
    //                xQueueReset(xCardIDQueue); ɾ���þ䣬Ϊ�˷�ֹ���·����ݵ�ʱ��ˢ��
                    DBG("send card1  queue is error!\r\n"); 
                    //���Ϳ���ʧ�ܷ�������ʾ
                    //�����Ƕ�����                
                } 
            
            }
            
            /* �����¼���־����ʾ������������ */        
            xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_5);  
            vTaskDelay(80); 
        }

}

static uint8_t parseReader2(void)
{
    uint8_t ch = 0;   
    
    while(1)
    {    
        //��ȡ485���ݣ�����û�������˳������ض�
        if(!RS485_Recv(COM2,&ch,1))
        {            
            return UNFINISHED;
        }
        
        //��ȡ���������ݵ�BUFF
        gReader2Data.rxBuff[gReader2Data.rxCnt++] = ch;
        
//        log_d("ch = %c,gReaderData.rxBuff = %c \r\n",ch,gReaderData.rxBuff[gReaderData.rxCnt-1]);
         
        //���һ���ֽ�Ϊ�س��������ܳ���Ϊ510�󣬽�����ȡ
        if(gReader2Data.rxBuff[gReader2Data.rxCnt-1] == 0x0A || gReader2Data.rxCnt >=64)
        {   
            
           if(gReader2Data.rxBuff[gReader2Data.rxCnt-1] == 0x0A)
           {
//                dbh("gReaderData.rxBuff", (char*)gReaderData.rxBuff, gReaderData.rxCnt);  
//                log_d("gReaderData.rxBuff = %s,len = %d\r\n",gReaderData.rxBuff,gReaderData.rxCnt-1);
                return FINISHED;
           }

            //��û�������һ���ֽڣ������ܳ���λ����գ��ض�������
            memset(&gReader2Data,0xFF,sizeof(FROMREADER2_STRU));
        }
        
    }   

   
}


