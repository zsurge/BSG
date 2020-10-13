/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : Led_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��2��25��
  ����޸�   :
  ��������   : LED�ƿ���
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��2��25��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#define LOG_TAG    "led"
#include "elog.h"

#include "led_task.h"
//#include "spi_flash.h" 
#include "bsp_led.h"
#include "bsp_beep.h"
#include "bsp_dipSwitch.h"


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define LED_TASK_PRIO	    (tskIDLE_PRIORITY)
#define LED_STK_SIZE 		(configMINIMAL_STACK_SIZE)

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *ledTaskName = "vLedTask";      //LED��������


/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskLed = NULL;      //LED��


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskLed(void *pvParameters);

void CreateLedTask(void)
{
    //����LED����
    xTaskCreate((TaskFunction_t )vTaskLed,         
                (const char*    )ledTaskName,       
                (uint16_t       )LED_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )LED_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskLed);
}


//LED������ 
static void vTaskLed(void *pvParameters)
{ 

//    uint32_t iTime1=0,iTime2  =  0;    
//    char wbuf[256] = {"hello world"};
//    char rbuf[256] = {0};
//    uint16_t i = 0;
    
//    Sound2(300);


//    for(i=0;i<256;i++)
//    {
//        if(i % 2 == 0)
//        {
//          wbuf[i] = 0x55;
//        }
//        else
//        {
//            wbuf[i] = 0xBB;        
//        }
//    }

//        SPI_FLASH_BufferRead(rbuf,1000,255);

//        dbh("1 rx", rbuf, 255);

//        iTime1  =  xTaskGetTickCount();
//        SPI_FLASH_SectorErase(1000,1000);
//        iTime2 = xTaskGetTickCount();   /* ���½���ʱ�� */
//        log_d ( "SPI_FLASH_SectorErase��use %d ms\r\n",iTime2 - iTime1 );   

//        SPI_FLASH_BufferRead(rbuf,1000,255);

//        dbh("2 rx", rbuf, 255);        

//        SPI_FLASH_BufferWrite(wbuf,1000,255);        

//        SPI_FLASH_BufferRead(rbuf,1000,255);

//        dbh("3 rx", rbuf, 255);

//        iTime1  =  xTaskGetTickCount();
//        SPI_FLASH_BulkErase();
//        iTime2 = xTaskGetTickCount();   /* ���½���ʱ�� */
//        log_d ( "SPI_FLASH_BulkErase��use %d ms\r\n",iTime2 - iTime1 );           
//        
//        SPI_FLASH_BufferRead(rbuf,1000,255);
//        
//        dbh("4 rx", rbuf, 255);

        



    while(1)
    {  

//        SPI_FLASH_BufferRead(rbuf,1000,255);
//        dbh("rx", rbuf, 255);

        LEDERROR = !LEDERROR;

//        SW2_LOW();

		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_0);  
        vTaskDelay(1000); 

//        SW2_HI();
//        vTaskDelay(1000);    
    }
} 


