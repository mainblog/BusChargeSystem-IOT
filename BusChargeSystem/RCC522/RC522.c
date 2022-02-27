#include "RC522.h"
#include "spi.h"
#include "delay.h"
#include "sys.h"

u8 my_test;
void MFRC522_Initializtion(void)
{
    u8 teststate;
    teststate = MFRC522_Reset();
    //Timer: TPrescaler*TreloadVal/6.78MHz = 0xD3E*0x32/6.78=25ms
    //	WriteRawRC(TModeReg,0x8D);				//TAuto=1???????,?????????4????????4?
    //	//Write_MFRC522(TModeReg,0x1D);				//TAutoRestart=1???????,0x0D3E?0.5ms?????//test
    //	WriteRawRC(TPrescalerReg,0x3E); 	//??????8?
    //	WriteRawRC(TReloadRegL,0x32);		//?????8?
    //	WriteRawRC(TReloadRegH,0x00);		//?????8?
    //	WriteRawRC(TxAutoReg,0x40); 			//100%ASK
    //	WriteRawRC(ModeReg,0x3D); 				//CRC???0x6363
    //	WriteRawRC(CommandReg,0x00);			//??MFRC522
    //	//Write_MFRC522(RFCfgReg, 0x7F);    //RxGain = 48dB???????
    //	MYRC522_CS = 0;
    AntennaOn(1);
    AntennaOn(0);
    ClearBitMask(Status2Reg, 0x08);
    WriteRawRC(ModeReg, 0x3D);   //3F
    WriteRawRC(RxSelReg, 0x86);  //84
    WriteRawRC(RFCfgReg, 0x7F);  //4F
    WriteRawRC(TReloadRegL, 30); //tmoLength);// TReloadVal = 'h6a =tmoLength(dec)
    WriteRawRC(TReloadRegH, 0);
    WriteRawRC(TModeReg, 0x8D);
    WriteRawRC(TPrescalerReg, 0x3E);
    //		  teststate = ReadRawRC(TPrescalerReg);
    teststate = ReadRawRC(TModeReg);
    delay_ms(5);
    AntennaOn(1);
    teststate = ReadRawRC(TModeReg);
    teststate = ReadRawRC(TPrescalerReg);
    //	 MYRC522_CS = 1;

    delay_ms(10);
}

/////////////////////////////////////////////////////////////////////
//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdHalt(void)
{
    char status;
    u8 unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    return MI_OK;
}
char MFRC522_Reset(void)
{
    //		MYRC522_RST=0;
    //    delay_ms(10);
    //    MYRC522_RST=1;
    //    delay_ms(10);  //0x01     / 0x0F //??
    //    WriteRawRC(CommandReg,PCD_RESETPHASE);//
    //    delay_ms(200);
    //                //0x11
    //    WriteRawRC(ModeReg,0x3D); //?Mifare???,CRC???0x6363
    //	delay_ms(5);
    //					//0x2D
    //    WriteRawRC(TReloadRegL,30);
    //	delay_ms(5);
    //					//0x2C
    //    WriteRawRC(TReloadRegH,0);
    //	delay_ms(5);
    //					// 0x2A
    //    WriteRawRC(TModeReg,0x8D);
    //	delay_ms(5);
    //					// 0x2B
    //    WriteRawRC(TPrescalerReg,0x3E);
    //	delay_ms(5);
    //					//0x15
    //    WriteRawRC(TxAutoReg,0x40);

    MYRC522_RST = 1;
    delay_ms(10);
    MYRC522_RST = 0;
    delay_ms(10);
    MYRC522_RST = 1;
    delay_ms(10);
    WriteRawRC(CommandReg, PCD_RESETPHASE);
    WriteRawRC(CommandReg, PCD_RESETPHASE);
    delay_ms(200);
    WriteRawRC(ModeReg, 0x3D); //和Mifare卡通讯，CRC初始值0x6363
    delay_ms(5);
    WriteRawRC(TReloadRegL, 30);
    delay_ms(5);
    WriteRawRC(TReloadRegH, 0);
    delay_ms(5);
    WriteRawRC(TModeReg, 0x8D);
    delay_ms(5);
    WriteRawRC(RFCfgReg, 0x7F); //4F
    delay_ms(5);
    WriteRawRC(TPrescalerReg, 0x3E);
    delay_ms(5);
    WriteRawRC(TxAutoReg, 0x40); //必须要
    return MI_OK;
}

//开天线  1开启，0关闭
void AntennaOn(u8 off)
{
    u8 i;
    if (off & 1)
    {
        i = ReadRawRC(TxControlReg);
        if (!(i & 0x03))
        {
            SetBitMask(TxControlReg, 0x03);
        }
    }
    else
    {
        ClearBitMask(TxControlReg, 0x03);
    }
}

u32 ReadID(void)
{
    u8 RC522ID = 0;
    u8 addr = 0;

    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //使能GPIOF时钟

    //初始化蜂鸣器对应引脚GPIOF8
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      //普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;     //下拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);             //初始化GPIO

    //GPIO_ResetBits(GPIOA,GPIO_Pin_1);  //蜂鸣器对应引脚GPIOF8拉低，
    //WriteRawRC(0x01,0x0f);  //软件重启
    WriteRawRC(CommandReg, PCD_RESETPHASE);

    WriteRawRC(ModeReg, 0x3D); //和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TReloadRegL, 30);
    WriteRawRC(TReloadRegH, 0);
    WriteRawRC(TModeReg, 0x8D);
    WriteRawRC(TPrescalerReg, 0x3E);
    WriteRawRC(TxAutoReg, 0x40);

    RC522ID = ReadRawRC(ModeReg);
    RC522ID = ReadRawRC(0x00);
}
void WriteRawRC(unsigned char Address, unsigned char value) //写寄存器
{
    unsigned char i, ucAddr;
    ucAddr = ((Address << 1) & 0x7E);
    MYRC522_CS = 0;

    SPI3_ReadWriteByte(ucAddr);

    SPI3_ReadWriteByte(value);
    MYRC522_CS = 1;
    delay_ms(5);
}

unsigned char ReadRawRC(unsigned char Address)
{
    u8 ucAddr;
    u8 ucResult = 0;
    MYRC522_CS = 0;
    ;
    ucAddr = ((Address << 1) & 0x7E) | 0x80;

    SPI3_ReadWriteByte(ucAddr);
    ucResult = SPI3_ReadWriteByte(0);
    MYRC522_CS = 1;
    return ucResult;
}

//功    能：寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//          pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//返    回: 成功返回MI_OK

char PcdRequest(unsigned char req_code, unsigned char *pTagType)
{
    char status;
    u8 unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
    //  unsigned char xTest ;
    ClearBitMask(Status2Reg, 0x08);
    WriteRawRC(BitFramingReg, 0x07);

    //  xTest = ReadRawRC(BitFramingReg);
    //  if(xTest == 0x07 )
    //   { LED_GREEN  =0 ;}
    // else {LED_GREEN =1 ;while(1){}}
    SetBitMask(TxControlReg, 0x03);

    ucComMF522Buf[0] = req_code;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);
    //     if(status  == MI_OK )
    //   { LED_GREEN  =0 ;}
    //   else {LED_GREEN =1 ;}
    if ((status == MI_OK) && (unLen == 0x10))
    {
        *pTagType = ucComMF522Buf[0];
        *(pTagType + 1) = ucComMF522Buf[1];
    }
    else
    {
        status = MI_ERR;
    }

    return status;
    //	char   status;
    //u8   unLen;
    //	u8   ucComMF522Buf[MAXRLEN];

    //	ClearBitMask(Status2Reg,0x08);
    //	WriteRawRC(BitFramingReg,0x07);
    //	SetBitMask(TxControlReg,0x03);
    //
    //	ucComMF522Buf[0] = req_code;

    //	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);

    //	if ((status == MI_OK) && (unLen == 0x10))
    //	{
    //		*pTagType     = ucComMF522Buf[0];
    //		*(pTagType+1) = ucComMF522Buf[1];
    //	}
    //	else
    //	{   status = MI_ERR;   }
    //
    //	return status;
}
//
void ClearBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask); // clear bit mask
}

void SetBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp | mask); // set bit mask
}

/////////////////////////////////////////////////////////////////////
//功    能：读取M1卡一块数据
//参数说明: addr[IN]：块地址
//          pData[OUT]：读出的数据，16字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdRead(unsigned char addr, unsigned char *pData)
{
    char status;
    u8 unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    if ((status == MI_OK) && (unLen == 0x90))
    //   {   memcpy(pData, ucComMF522Buf, 16);   }
    {
        for (i = 0; i < 16; i++)
        {
            *(pData + i) = ucComMF522Buf[i];
        }
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}
/////////////////////////////////////////////////////////////////////
//功    能：写数据到M1卡一块
//参数说明: addr[IN]：块地址
//          pData[IN]：写入的数据，16字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdWrite(unsigned char addr, unsigned char *pData)
{
    char status;
    u8 unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, pData, 16);
        for (i = 0; i < 16; i++)
        {
            ucComMF522Buf[i] = *(pData + i);
        }
        CalulateCRC(ucComMF522Buf, 16, &ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, &unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：验证卡片密码
//参数说明: auth_mode[IN]: 密码验证模式
//                 0x60 = 验证A密钥
//                 0x61 = 验证B密钥
//          addr[IN]：块地址
//          pKey[IN]：密码
//          pSnr[IN]：卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdAuthState(unsigned char auth_mode, unsigned char addr, unsigned char *pKey, unsigned char *pSnr)
{
    char status;
    u8 unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i = 0; i < 6; i++)
    {
        ucComMF522Buf[i + 2] = *(pKey + i);
    }
    for (i = 0; i < 4; i++)
    {
        ucComMF522Buf[i + 8] = *(pSnr + i);
    }
    //   memcpy(&ucComMF522Buf[2], pKey, 6);
    //   memcpy(&ucComMF522Buf[8], pSnr, 4);

    status = PcdComMF522(PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {
        status = MI_ERR;
    }

    return status;
}
/////////////////////////////////////////////////////////////////////
//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]:RC522命令字
//          pInData[IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOutData[OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
/////////////////////////////////////////////////////////////////////
char PcdComMF522(u8 Command,
                 u8 *pIn,
                 u8 InLenByte,
                 u8 *pOut,
                 u8 *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    u32 i;
    switch (Command)
    {
    case PCD_AUTHENT:
        irqEn = 0x12;
        waitFor = 0x10;
        break;
    case PCD_TRANSCEIVE:
        irqEn = 0x77;
        waitFor = 0x30;
        break;
    default:
        break;
    }

    WriteRawRC(ComIEnReg, irqEn | 0x80);
    ClearBitMask(ComIrqReg, 0x80);

    WriteRawRC(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);

    for (i = 0; i < InLenByte; i++)
    {
        WriteRawRC(FIFODataReg, pIn[i]);
    }

    //		my_test = ReadRawRC(FIFOLevelReg);   //查有多少个字节存储在fifo中
    WriteRawRC(CommandReg, Command);

    if (Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg, 0x80);
    } //开始传送

    //i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
    i = 100000;
    do
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitFor));
    ClearBitMask(BitFramingReg, 0x80);

    if (i != 0)
    {
        if (!(ReadRawRC(ErrorReg) & 0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }
            if (Command == PCD_TRANSCEIVE)
            {
                n = ReadRawRC(FIFOLevelReg);
                lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits)
                {
                    *pOutLenBit = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *pOutLenBit = n * 8;
                }
                if (n == 0)
                {
                    n = 1;
                }
                if (n > MAXRLEN)
                {
                    n = MAXRLEN;
                }
                for (i = 0; i < n; i++)
                {
                    pOut[i] = ReadRawRC(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }
    }

    SetBitMask(ControlReg, 0x80); // stop timer now
    WriteRawRC(CommandReg, PCD_IDLE);
    return status;
}
/////////////////////////////////////////////////////////////////////
//用MF522计算CRC16函数
/////////////////////////////////////////////////////////////////////
void CalulateCRC(unsigned char *pIndata, unsigned char len, unsigned char *pOutData)
{
    unsigned char i, n;
    ClearBitMask(DivIrqReg, 0x04);
    WriteRawRC(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);
    for (i = 0; i < len; i++)
    {
        WriteRawRC(FIFODataReg, *(pIndata + i));
    }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x04));
    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);
}
/////////////////////////////////////////////////////////////////////
//功    能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdSelect(unsigned char *pSnr)
{
    char status;
    unsigned char i;
    u8 unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i = 0; i < 4; i++)
    {
        ucComMF522Buf[i + 2] = *(pSnr + i);
        ucComMF522Buf[6] ^= *(pSnr + i);
    }
    CalulateCRC(ucComMF522Buf, 7, &ucComMF522Buf[7]);

    ClearBitMask(Status2Reg, 0x08);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);

    if ((status == MI_OK) && (unLen == 0x18))
    {
        status = MI_OK;
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：防冲撞
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdAnticoll(u8 *pSnr)
{
    char status;
    u8 i, snr_check = 0;
    u8 unLen;
    u8 ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg, 0x08);
    WriteRawRC(BitFramingReg, 0x00);
    ClearBitMask(CollReg, 0x80);

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);

    if (status == MI_OK)
    {
        for (i = 0; i < 4; i++)
        {
            *(pSnr + i) = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i])
        {
            status = MI_ERR;
        }
    }

    SetBitMask(CollReg, 0x80);
    return status;
}
