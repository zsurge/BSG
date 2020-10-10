/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : main.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年7月9日
  最近修改   :
  功能描述   : 主程序模块
  函数列表   :
  修改历史   :
  1.日    期   : 2019年7月9日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#define LOG_TAG    "main"
#include "elog.h"

#include "def.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
//任务优先级 
#define APPCREATE_TASK_PRIO		(tskIDLE_PRIORITY)
#define APPCREATE_STK_SIZE 		(configMINIMAL_STACK_SIZE*16)

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
const char *AppCreateTaskName = "vAppCreateTask"; 

//任务句柄
static TaskHandle_t xHandleTaskAppCreate = NULL;     

SemaphoreHandle_t gxMutex = NULL;
EventGroupHandle_t xCreatedEventGroup = NULL;
QueueHandle_t xCmdQueue = NULL; 
QueueHandle_t xCardIDQueue = NULL;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void AppTaskCreate(void);
static void AppObjCreate (void);
static void App_Printf(char *format, ...);
static void EasyLogInit(void);


int main(void)
{   
    //硬件初始化
    bsp_Init();  

    EasyLogInit();      

	/* 创建任务通信机制 */
	AppObjCreate();

    
    //创建AppTaskCreate任务
    xTaskCreate((TaskFunction_t )AppTaskCreate,     
                (const char*    )AppCreateTaskName,   
                (uint16_t       )APPCREATE_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )APPCREATE_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskAppCreate);   

    
    /* 启动调度，开始执行任务 */
    vTaskStartScheduler();    
    
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{
    //进入临界区
    taskENTER_CRITICAL();    

    //网卡初始化
    StartEthernet();   

    //握手
    CreateHandShakeTask();

    //LED灯
    CreateLedTask();

    //跟控制板通讯
    CreateCommTask();
    
//    //按键
//    CreateKeyTask();

    //读卡器
//    CreateReaderTask();
    CreateRs485ReaderTask();


    //卡数据处理
    CreateDataProcessTask();

    //MQTT通讯
    CreateMqttTask();

    //看门狗
//    CreateWatchDogTask();

    //删除本身
    vTaskDelete(xHandleTaskAppCreate); //删除AppTaskCreate任务

    //退出临界区
    taskEXIT_CRITICAL();   

}




/*
*********************************************************************************************************
*	函 数 名: AppObjCreate
*	功能说明: 创建任务通信机制
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppObjCreate (void)
{
	/* 创建事件标志组 */
	xCreatedEventGroup = xEventGroupCreate();
	
	if(xCreatedEventGroup == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
        App_Printf("创建事件标志组失败\r\n");
    }

	/* 创建互斥信号量 */
    gxMutex = xSemaphoreCreateMutex();
	
	if(gxMutex == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
        App_Printf("创建互斥信号量失败\r\n");
    }    

    //创消息队列，存放刷卡或卡号下发数据
    xCardIDQueue = xQueueCreate((UBaseType_t ) CARD_QUEUE_LEN,/* 消息队列的长度 */
                              (UBaseType_t ) sizeof(READER_BUFF_STRU *));/* 消息的大小 */
    if(xCardIDQueue == NULL)
    {
        App_Printf("create xCardIDQueue error!\r\n");
    }
    
    
    xCmdQueue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* 消息队列的长度 */
                              (UBaseType_t ) sizeof(CMD_BUFF_STRU *));/* 消息的大小 */
    if(xCmdQueue == NULL)
    {
        App_Printf("create xCmdQueue error!\r\n");
    }
   



}


/*
*********************************************************************************************************
*	函 数 名: App_Printf
*	功能说明: 线程安全的printf方式		  			  
*	形    参: 同printf的参数。
*             在C中，当无法列出传递函数的所有实参的类型和数目时,可以用省略号指定参数表
*	返 回 值: 无
*********************************************************************************************************
*/
static void  App_Printf(char *format, ...)
{
    char  buf_str[512 + 1];
    va_list   v_args;


    va_start(v_args, format);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) format,
                                  v_args);
    va_end(v_args);

	/* 互斥信号量 */
	xSemaphoreTake(gxMutex, portMAX_DELAY);

    printf("%s", buf_str);

   	xSemaphoreGive(gxMutex);
}


static void EasyLogInit(void)
{
    /* initialize EasyLogger */
     elog_init();
     /* set EasyLogger log format */
     elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
     elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_ALL & ~ELOG_FMT_TIME);
     elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_TIME);
     elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_TIME);

     
     /* start EasyLogger */
     elog_start();  
}




