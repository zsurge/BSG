

/* Includes ------------------------------------------------------------------*/
#include "spi_flash.h"

#define SPI_FLASH_PageSize              256
#define SPI_FLASH_PerWritePageSize      256


void sFLASH_LowLevel_DeInit(void);
void sFLASH_LowLevel_Init(void); 


void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u32 NumByteToWrite);

static void sFLASH_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  DeInitializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void sFLASH_DeInit(void)
{
  sFLASH_LowLevel_DeInit();
}

/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void sFLASH_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;

  sFLASH_LowLevel_Init();
    
  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  /*!< SPI configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;

  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(sFLASH_SPI, &SPI_InitStructure);

  /*!< Enable the sFLASH_SPI  */
  SPI_Cmd(sFLASH_SPI, ENABLE);
}

///////////////////////////////////////////////////////////////////
void sFLASH_WriteStatuR(uint8_t byte)
{
   /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "RDID " instruction */
  sFLASH_SendByte(0x01);

  sFLASH_SendByte(byte);


  sFLASH_CS_HIGH();



}
/**
  * @brief  Reads FLASH identification.
  * @param  None
  * @retval FLASH identification
  */
uint32_t sFLASH_ReadStatuR(void)
{
  uint8_t Temp = 0;

  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "RDID " instruction */
  sFLASH_SendByte(0x05);

  /*!< Read a byte from the FLASH */
  Temp= sFLASH_SendByte(sFLASH_DUMMY_BYTE);


  sFLASH_CS_HIGH();


   
  return Temp;
}



/**
  * @brief  Reads a byte from the SPI Flash.
  * @note   This function must be used only if the Start_Read_Sequence function
  *         has been previously called.
  * @param  None
  * @retval Byte Read from the SPI Flash.
  */
uint8_t sFLASH_ReadByte(void)
{
  return (sFLASH_SendByte(sFLASH_DUMMY_BYTE));
}

/**
  * @brief  Sends a byte through the SPI interface and return the byte received
  *         from the SPI bus.
  * @param  byte: byte to send.
  * @retval The value of the received byte.
  */
uint8_t sFLASH_SendByte(uint8_t byte)
{
  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(sFLASH_SPI, byte);

  /*!< Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /*!< Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(sFLASH_SPI);
}



/**
  * @brief  Enables the write access to the FLASH.
  * @param  None
  * @retval None
  */
void sFLASH_WriteEnable(void)
{
  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "Write Enable" instruction */
  sFLASH_SendByte(sFLASH_CMD_WREN);

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();
}

/**
  * @brief  Polls the status of the Write In Progress (WIP) flag in the FLASH's
  *         status register and loop until write opertaion has completed.
  * @param  None
  * @retval None
  */
void sFLASH_WaitForWriteEnd(void)
{
  uint8_t flashstatus = 0;

  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "Read Status Register" instruction */
  sFLASH_SendByte(sFLASH_CMD_RDSR);

  /*!< Loop as long as the memory is busy with a write cycle */
  do
  {
    /*!< Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    flashstatus = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

  }
  while ((flashstatus & sFLASH_WIP_FLAG) == SET); /* Write in progress */

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();
}

/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void sFLASH_LowLevel_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< Enable the SPI clock */
  sFLASH_SPI_CLK_INIT(sFLASH_SPI_CLK, ENABLE);

  /*!< Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd(sFLASH_SPI_SCK_GPIO_CLK | sFLASH_SPI_MISO_GPIO_CLK | 
                         sFLASH_SPI_MOSI_GPIO_CLK | sFLASH_CS_GPIO_CLK, ENABLE);
  
  /*!< SPI pins configuration *************************************************/

  /*!< Connect SPI pins to AF5 */  
  GPIO_PinAFConfig(sFLASH_SPI_SCK_GPIO_PORT, sFLASH_SPI_SCK_SOURCE, sFLASH_SPI_SCK_AF);
  GPIO_PinAFConfig(sFLASH_SPI_MISO_GPIO_PORT, sFLASH_SPI_MISO_SOURCE, sFLASH_SPI_MISO_AF);
  GPIO_PinAFConfig(sFLASH_SPI_MOSI_GPIO_PORT, sFLASH_SPI_MOSI_SOURCE, sFLASH_SPI_MOSI_AF);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
        
  /*!< SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = sFLASH_SPI_SCK_PIN;
  GPIO_Init(sFLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /*!< SPI MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  sFLASH_SPI_MOSI_PIN;
  GPIO_Init(sFLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /*!< SPI MISO pin configuration */
  GPIO_InitStructure.GPIO_Pin =  sFLASH_SPI_MISO_PIN;
  GPIO_Init(sFLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure sFLASH Card CS pin in output pushpull mode ********************/
  GPIO_InitStructure.GPIO_Pin = sFLASH_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(sFLASH_CS_GPIO_PORT, &GPIO_InitStructure);
}
///////////////////////////////////////////////////////////////////////////////////////
void sFLASH_Write(uint32_t WriteAddr,uint8_t Wdata)
{
  /*!< Enable the write access to the FLASH */
  sFLASH_WriteEnable();

  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();
  /*!< Send "Write to Memory " instruction */
  sFLASH_SendByte(sFLASH_CMD_WRITE);
/*!< Send the 24-bit address of the address to read from -------------------*/
  /*!< Send ReadAddr high nibble address byte */
  sFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /*!< Send ReadAddr medium nibble address byte */
  sFLASH_SendByte((WriteAddr& 0xFF00) >> 8);
  /*!< Send ReadAddr low nibble address byte */
  sFLASH_SendByte(WriteAddr & 0xFF);

  /*!< while there is data to be written on the FLASH */
    sFLASH_SendByte(Wdata);

    //////////////////////////////////////////////////
    
    ////////////////////////////////////////////////
  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  /*!< Wait the end of Flash writing */
  sFLASH_WaitForWriteEnd();
}
///////////////////////////
 uint8_t sFLASH_Read(uint32_t ReadAddr)
{
   uint8_t  mm=0;
  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "Read from Memory " instruction */
  sFLASH_SendByte(sFLASH_CMD_READ);

/*!< Send the 24-bit address of the address to read from -------------------*/
  /*!< Send ReadAddr high nibble address byte */
  sFLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /*!< Send ReadAddr medium nibble address byte */
  sFLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /*!< Send ReadAddr low nibble address byte */
  sFLASH_SendByte(ReadAddr & 0xFF);

 // while (NumByteToRead--) /*!< while there is data to be read */
 // {
   
    mm = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

 // }

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();
  return mm;
}
//////////////////////////////////////////////////////////////////////////////////////
/**
  * @brief  DeInitializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void sFLASH_LowLevel_DeInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< Disable the sFLASH_SPI  ************************************************/
  SPI_Cmd(sFLASH_SPI, DISABLE);
  
  /*!< DeInitializes the sFLASH_SPI *******************************************/
  SPI_I2S_DeInit(sFLASH_SPI);
  
  /*!< sFLASH_SPI Periph clock disable ****************************************/
  sFLASH_SPI_CLK_INIT(sFLASH_SPI_CLK, DISABLE);
      
  /*!< Configure all pins used by the SPI as input floating *******************/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  GPIO_InitStructure.GPIO_Pin = sFLASH_SPI_SCK_PIN;
  GPIO_Init(sFLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = sFLASH_SPI_MISO_PIN;
  GPIO_Init(sFLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = sFLASH_SPI_MOSI_PIN;
  GPIO_Init(sFLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = sFLASH_CS_PIN;
  GPIO_Init(sFLASH_CS_GPIO_PORT, &GPIO_InitStructure);
}  	

#if 0

/*
*********************************************************************************************************
*   函 数 名: sf_PageWrite
*   功能说明: 向一个page内写入若干字节。字节个数不能超出页面大小（4K)
*   形    参:     _pBuf : 数据源缓冲区；
*               _uiWriteAddr ：目标区域首地址
*               _usSize ：数据个数，不能超过页面大小
*   返 回 值: 无
*********************************************************************************************************
*/
void sFLASH_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
    uint32_t i, j;

    for (j = 0; j < _usSize / 256; j++)
    {
        sf_WriteEnable();                               /* 发送写使能命令 */

        SF_CS_LOW();                                    /* 使能片选 */
        sf_SendByte(0x02);                              /* 发送AAI命令(地址自动增加编程) */
        sf_SendByte((_uiWriteAddr & 0xFF0000) >> 16);   /* 发送扇区地址的高8bit */
        sf_SendByte((_uiWriteAddr & 0xFF00) >> 8);      /* 发送扇区地址中间8bit */
        sf_SendByte(_uiWriteAddr & 0xFF);               /* 发送扇区地址低8bit */

        for (i = 0; i < 256; i++)
        {
            sf_SendByte(*_pBuf++);                  /* 发送数据 */
        }

        SF_CS_HIGH();                               /* 禁止片选 */

        sf_WaitForWriteEnd();                       /* 等待串行Flash内部写操作完成 */

        _uiWriteAddr += 256;
    }

    /* 进入写保护状态 */
    SF_CS_LOW();
    sf_SendByte(CMD_DISWR);
    SF_CS_HIGH();

    sf_WaitForWriteEnd();                           /* 等待串行Flash内部写操作完成 */
    
}
    
/*
*********************************************************************************************************
*   函 数 名: sf_ReadBuffer
*   功能说明: 连续读取若干字节。字节个数不能超出芯片容量。
*   形    参:     _pBuf : 数据源缓冲区；
*               _uiReadAddr ：首地址
*               _usSize ：数据个数, 可以大于PAGE_SIZE,但是不能超出芯片总容量
*   返 回 值: 无
*********************************************************************************************************
*/
void sFLASH_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
    /* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
    if ((_uiSize == 0) ||(_uiReadAddr + _uiSize) > g_tSF.TotalSize)
    {
        return;
    }

    /* 擦除扇区操作 */
    SF_CS_LOW();                                    /* 使能片选 */
    sf_SendByte(CMD_READ);                          /* 发送读命令 */
    sf_SendByte((_uiReadAddr & 0xFF0000) >> 16);    /* 发送扇区地址的高8bit */
    sf_SendByte((_uiReadAddr & 0xFF00) >> 8);       /* 发送扇区地址中间8bit */
    sf_SendByte(_uiReadAddr & 0xFF);                /* 发送扇区地址低8bit */
    while (_uiSize--)
    {
        *_pBuf++ = sf_SendByte(DUMMY_BYTE);         /* 读一个字节并存储到pBuf，读完后指针自加1 */
    }
    SF_CS_HIGH();                                   /* 禁能片选 */
}

 /*
    *********************************************************************************************************
    *   函 数 名: sf_AutoWritePage
    *   功能说明: 写1个PAGE并校验,如果不正确则再重写两次。本函数自动完成擦除操作。
    *   形    参:     _pBuf : 数据源缓冲区；
    *               _uiWriteAddr ：目标区域首地址
    *               _usSize ：数据个数，不能超过页面大小
    *   返 回 值: 0 : 错误， 1 ： 成功
    *********************************************************************************************************
    */
static uint8_t sf_AutoWritePage(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen)
{
    uint16_t i;
    uint16_t j;                 /* 用于延时 */
    uint32_t uiFirstAddr;       /* 扇区首址 */
    uint8_t ucNeedErase;        /* 1表示需要擦除 */
    uint8_t cRet;

//        uint8_t s_spiBuf[4*1024] = {0}; 


    /* 长度为0时不继续操作,直接认为成功 */
    if (_usWrLen == 0)
    {
        return 1;
    }

    /* 如果偏移地址超过芯片容量则退出 */
    if (_uiWrAddr >= g_tSF.TotalSize)
    {
        return 0;
    }

    /* 如果数据长度大于扇区容量，则退出 */
    if (_usWrLen > g_tSF.PageSize)
    {
        return 0;
    }

    /* 如果FLASH中的数据没有变化,则不写FLASH */
    bsp_sf_ReadBuffer(s_spiBuf, _uiWrAddr, _usWrLen);
    if (memcmp(s_spiBuf, _ucpSrc, _usWrLen) == 0)
    {
        return 1;
    }

    /* 判断是否需要先擦除扇区 */
    /* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
    ucNeedErase = 0;
    if (sf_NeedErase(s_spiBuf, _ucpSrc, _usWrLen))
    {
        ucNeedErase = 1;
    }

    uiFirstAddr = _uiWrAddr & (~(g_tSF.PageSize - 1));

    if (_usWrLen == g_tSF.PageSize)     /* 整个扇区都改写 */
    {
        for (i = 0; i < g_tSF.PageSize; i++)
        {
            s_spiBuf[i] = _ucpSrc[i];
        }
    }
    else                        /* 改写部分数据 */
    {
        /* 先将整个扇区的数据读出 */
        bsp_sf_ReadBuffer(s_spiBuf, uiFirstAddr, g_tSF.PageSize);

        /* 再用新数据覆盖 */
        i = _uiWrAddr & (g_tSF.PageSize - 1);
        memcpy(&s_spiBuf[i], _ucpSrc, _usWrLen);
    }

    /* 写完之后进行校验，如果不正确则重写，最多3次 */
    cRet = 0;
    for (i = 0; i < 3; i++)
    {

        /* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
        if (ucNeedErase == 1)
        {
            bsp_sf_EraseSector(uiFirstAddr);        /* 擦除1个扇区 */
        }

        /* 编程一个PAGE */
        bsp_sf_PageWrite(s_spiBuf, uiFirstAddr, g_tSF.PageSize);

        if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
        {
            cRet = 1;
            break;
        }
        else
        {
            if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
            {
                cRet = 1;
                break;
            }

            /* 失败后延迟一段时间再重试 */
            for (j = 0; j < 10000; j++);
        }
    }

    return cRet;
}

    

/*
*********************************************************************************************************
*   函 数 名: sf_WriteBuffer
*   功能说明: 写1个扇区并校验,如果不正确则再重写两次。本函数自动完成擦除操作。
*   形    参:     _pBuf : 数据源缓冲区；
*               _uiWrAddr ：目标区域首地址
*               _usSize ：数据个数，不能超过页面大小
*   返 回 值: 1 : 成功， 0 ： 失败
*********************************************************************************************************
*/
uint8_t bsp_sf_WriteBuffer(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize)
{
    uint16_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

    /*mod运算求余，若writeAddr是SPI_FLASH_PageSize整数倍，运算结果Addr值为0*/
    Addr = _uiWriteAddr % g_tSF.PageSize;
    
    /*差count个数据值，刚好可以对齐到页地址*/
    count = g_tSF.PageSize - Addr;

    /*计算出要写多少整数页*/
    NumOfPage =  _usWriteSize / g_tSF.PageSize;

    /*mod运算求余，计算出剩余不满一页的字节数*/
    NumOfSingle = _usWriteSize % g_tSF.PageSize;

    if (Addr == 0) /* 起始地址是页面首地址  */
    {
        if (NumOfPage == 0) /* 数据长度小于页面大小 */
        {
            if (sf_AutoWritePage(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
            {
                return 0;
            }
        }
        else    /* 数据长度大于等于页面大小 */
        {
            while (NumOfPage--)
            {
                if (sf_AutoWritePage(_pBuf, _uiWriteAddr, g_tSF.PageSize) == 0)
                {
                    return 0;
                }
                _uiWriteAddr +=  g_tSF.PageSize;
                _pBuf += g_tSF.PageSize;
            }
            if (sf_AutoWritePage(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
            {
                return 0;
            }
        }
    }
    else  /* 起始地址不是页面首地址  */
    {
        if (NumOfPage == 0) /* 数据长度小于页面大小 */
        {
            if (NumOfSingle > count) /* (_usWriteSize + _uiWriteAddr) > SPI_FLASH_PAGESIZE */
            {
                temp = NumOfSingle - count;

                if (sf_AutoWritePage(_pBuf, _uiWriteAddr, count) == 0)
                {
                    return 0;
                }

                _uiWriteAddr +=  count;
                _pBuf += count;

                if (sf_AutoWritePage(_pBuf, _uiWriteAddr, temp) == 0)
                {
                    return 0;
                }
            }
            else
            {
                if (sf_AutoWritePage(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
                {
                    return 0;
                }
            }
        }
        else    /* 数据长度大于等于页面大小 */
        {
            _usWriteSize -= count;
            NumOfPage =  _usWriteSize / g_tSF.PageSize;
            NumOfSingle = _usWriteSize % g_tSF.PageSize;

            if (sf_AutoWritePage(_pBuf, _uiWriteAddr, count) == 0)
            {
                return 0;
            }

            _uiWriteAddr +=  count;
            _pBuf += count;

            while (NumOfPage--)
            {
                if (sf_AutoWritePage(_pBuf, _uiWriteAddr, g_tSF.PageSize) == 0)
                {
                    return 0;
                }
                _uiWriteAddr +=  g_tSF.PageSize;
                _pBuf += g_tSF.PageSize;
            }

            if (NumOfSingle != 0)
            {
                if (sf_AutoWritePage(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
                {
                    return 0;
                }
            }
        }
    }
    return 1;   /* 成功 */
}


#endif


#if  1
 /**
  * @brief  对FLASH按页写入数据，调用本函数写入数据前需要先擦除扇区
  * @param	pBuffer，要写入数据的指针
  * @param WriteAddr，写入地址
  * @param  NumByteToWrite，写入数据长度，必须小于等于SPI_FLASH_PerWritePageSize
  * @retval 无
  */
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u32 NumByteToWrite)
{
  /* 发送FLASH写使能命令 */
  sFLASH_WriteEnable();

  /* 选择FLASH: CS低电平 */
  sFLASH_CS_LOW();
  /* 写页写指令*/
  sFLASH_SendByte(sFLASH_CMD_WRITE);
  /*发送写地址的高位*/
  sFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /*发送写地址的中位*/
  sFLASH_SendByte((WriteAddr & 0xFF00) >> 8);
  /*发送写地址的低位*/
  sFLASH_SendByte(WriteAddr & 0xFF);

  if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
  {
     NumByteToWrite = SPI_FLASH_PerWritePageSize;
  }

  /* 写入数据*/
  while (NumByteToWrite--)
  {
    /* 发送当前要写入的字节数据 */
    sFLASH_SendByte(*pBuffer);
    /* 指向下一字节数据 */
    pBuffer++;
  }

  /* 停止信号 FLASH: CS 高电平 */
  sFLASH_CS_HIGH();

  /* 等待写入完毕*/
  sFLASH_WaitForWriteEnd();
}


 /**
  * @brief  对FLASH写入数据，调用本函数写入数据前需要先擦除扇区
  * @param	pBuffer，要写入数据的指针
  * @param  WriteAddr，写入地址
  * @param  NumByteToWrite，写入数据长度
  * @retval 无
  */
void SPI_FLASH_BufferWrite(void *wBuf, u32 WriteAddr, u32 NumByteToWrite)
{
    
  u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
  
  uint8_t *pBuffer = (uint8_t *)wBuf;
	
	/*mod运算求余，若writeAddr是SPI_FLASH_PageSize整数倍，运算结果Addr值为0*/
  Addr = WriteAddr % SPI_FLASH_PageSize;
	
	/*差count个数据值，刚好可以对齐到页地址*/
  count = SPI_FLASH_PageSize - Addr;	
	/*计算出要写多少整数页*/
  NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
	/*mod运算求余，计算出剩余不满一页的字节数*/
  NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

	 /* Addr=0,则WriteAddr 刚好按页对齐 aligned  */
  if (Addr == 0) 
  {
		/* NumByteToWrite < SPI_FLASH_PageSize */
    if (NumOfPage == 0) 
    {
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
			/*先把整数页都写了*/
      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }
			
			/*若有多余的不满一页的数据，把它写完*/
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
    }
  }
	/* 若地址与 SPI_FLASH_PageSize 不对齐  */
  else 
  {
		/* NumByteToWrite < SPI_FLASH_PageSize */
    if (NumOfPage == 0) 
    {
			/*当前页剩余的count个位置比NumOfSingle小，写不完*/
      if (NumOfSingle > count) 
      {
        temp = NumOfSingle - count;
				
				/*先写满当前页*/
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;
				
				/*再写剩余的数据*/
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
      }
      else /*当前页剩余的count个位置能写完NumOfSingle个数据*/
      {				
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
			/*地址不对齐多出的count分开处理，不加入这个运算*/
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
      NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

      SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;
			
			/*把整数页都写了*/
      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }
			/*若有多余的不满一页的数据，把它写完*/
      if (NumOfSingle != 0)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

 /**
  * @brief  读取FLASH数据
  * @param 	pBuffer，存储读出数据的指针
  * @param   ReadAddr，读取地址
  * @param   NumByteToRead，读取数据长度
  * @retval 无
  */
void SPI_FLASH_BufferRead(void *rBuf, u32 ReadAddr, u32 NumByteToRead)
{

  uint8_t *pBuffer = (uint8_t *)rBuf; 
    
  /* 选择FLASH: CS低电平 */
  sFLASH_CS_LOW();

  /* 发送 读 指令 */
  sFLASH_SendByte(sFLASH_CMD_READ);

  /* 发送 读 地址高位 */
  sFLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* 发送 读 地址中位 */
  sFLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* 发送 读 地址低位 */
  sFLASH_SendByte(ReadAddr & 0xFF);
  
	/* 读取数据 */
  while (NumByteToRead--)
  {
    /* 读取一个字节*/
    *pBuffer = sFLASH_SendByte(sFLASH_DUMMY_BYTE);
    /* 指向下一个字节缓冲区 */
    pBuffer++;
  }

  /* 停止信号 FLASH: CS 高电平 */
  sFLASH_CS_HIGH();
}


 /**
  * @brief  擦除FLASH扇区
  * @param  SectorAddr：要擦除的扇区地址
  * @retval 无
  */
void SPI_FLASH_SectorErase(u32 WriteAddr,u32 NumByteToCls)
{

    /* 发送FLASH写使能命令 */
    sFLASH_WriteEnable();
  
    /* 选择FLASH: CS低电平 */
    sFLASH_CS_LOW();
    /* 写页写指令*/
    sFLASH_SendByte(sFLASH_CMD_WRITE);
    /*发送写地址的高位*/
    sFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
    /*发送写地址的中位*/
    sFLASH_SendByte((WriteAddr & 0xFF00) >> 8);
    /*发送写地址的低位*/
    sFLASH_SendByte(WriteAddr & 0xFF);
  
    /* 写入数据*/
    while (NumByteToCls--)
    {
      /* 发送当前要写入的字节数据 */
      sFLASH_SendByte(0x00);
    }
  
    /* 停止信号 FLASH: CS 高电平 */
    sFLASH_CS_HIGH();
  
    /* 等待写入完毕*/
    sFLASH_WaitForWriteEnd();


}



 /**
  * @brief  擦除FLASH扇区，整片擦除
  * @param  无
  * @retval 无
  */
void SPI_FLASH_BulkErase(void)
{
  uint32_t NumByteToCls = 131072;
  /* 发送FLASH写使能命令 */
  sFLASH_WriteEnable();
  
  /* 选择FLASH: CS低电平 */
  sFLASH_CS_LOW();
  /* 写页写指令*/
  sFLASH_SendByte(sFLASH_CMD_WRITE);
  /*发送写地址的高位*/
  sFLASH_SendByte((0 & 0xFF0000) >> 16);
  /*发送写地址的中位*/
  sFLASH_SendByte((0 & 0xFF00) >> 8);
  /*发送写地址的低位*/
  sFLASH_SendByte(0 & 0xFF);
  
  /* 写入数据*/
  while (NumByteToCls--)
  {
    /* 发送当前要写入的字节数据 */
    sFLASH_SendByte(0x00);
  }
  
  /* 停止信号 FLASH: CS 高电平 */
  sFLASH_CS_HIGH();
  
  /* 等待写入完毕*/
  sFLASH_WaitForWriteEnd();

}




#endif /* #if 0 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
