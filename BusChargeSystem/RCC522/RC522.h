#ifndef _RC522_H
#define _RC522_H
#include "sys.h"

#define MYRC522_CS PAout(4)
#define MYRC522_RST PAout(6) //
u32 ReadID(void);
char MFRC522_Reset(void);
char PcdSelect(unsigned char *pSnr);                                                                      //选卡
char PcdWrite(unsigned char addr, unsigned char *pData);                                                  //写数据到M卡
char PcdAuthState(unsigned char auth_mode, unsigned char addr, unsigned char *pKey, unsigned char *pSnr); //验证密码
char PcdRead(unsigned char addr, unsigned char *pData);
void CalulateCRC(unsigned char *pIndata, unsigned char len, unsigned char *pOutData);
char PcdAnticoll(u8 *pSnr);
void ClearBitMask(unsigned char reg, unsigned char mask);
void MFRC522_Initializtion(void);
void AntennaOn(u8 off);
void WriteRawRC(unsigned char Address, unsigned char value);
unsigned char ReadRawRC(unsigned char Address);
void ClearBitMask(unsigned char reg, unsigned char mask);
void SetBitMask(unsigned char reg, unsigned char mask);
char PcdComMF522(u8 Command,
                 u8 *pIn,
                 u8 InLenByte,
                 u8 *pOut,
                 u8 *pOutLenBit);
char PcdRequest(unsigned char req_code, unsigned char *pTagType);
char PcdHalt(void);

//MF522命令字
/////////////////////////////////////////////////////////////////////
#define PCD_IDLE 0x00       //取消当前命令
#define PCD_AUTHENT 0x0E    //验证密钥
#define PCD_RECEIVE 0x08    //接收数据
#define PCD_TRANSMIT 0x04   //发送数据
#define PCD_TRANSCEIVE 0x0C //发送并接收数据
#define PCD_RESETPHASE 0x0F //复位
#define PCD_CALCCRC 0x03    //CRC计算

/////////////////////////////////////////////////////////////////////
//Mifare_One卡片命令字
/////////////////////////////////////////////////////////////////////
#define PICC_REQIDL 0x26    //寻天线区内未进入休眠状态
#define PICC_REQALL 0x52    //寻天线区内全部卡
#define PICC_ANTICOLL1 0x93 //防冲撞
#define PICC_ANTICOLL2 0x95 //防冲撞
#define PICC_AUTHENT1A 0x60 //验证A密钥
#define PICC_AUTHENT1B 0x61 //验证B密钥
#define PICC_READ 0x30      //读块
#define PICC_WRITE 0xA0     //写块
#define PICC_DECREMENT 0xC0 //扣款
#define PICC_INCREMENT 0xC1 //充值
#define PICC_RESTORE 0xC2   //调块数据到缓冲区
#define PICC_TRANSFER 0xB0  //保存缓冲区中数据
#define PICC_HALT 0x50      //休眠
// PAGE 0                										命令和状态
#define RFU00 0x00         //保留
#define CommandReg 0x01    //启动和停止命令的执行
#define ComIEnReg 0x02     //中断请求传输使能位
#define DivlEnReg 0x03     //中断请求传输使能位
#define ComIrqReg 0x04     //包含中断请求标志
#define DivIrqReg 0x05     //包含中断请求标志
#define ErrorReg 0x06      //错误标志位
#define Status1Reg 0x07    //传输状态标志位
#define Status2Reg 0x08    //包含接收和发送标志位
#define FIFODataReg 0x09   //64字节发送、接收fifo缓冲区
#define FIFOLevelReg 0x0A  //fifo缓冲区存储字节数
#define WaterLevelReg 0x0B //fifo上溢和下溢报警的等级
#define ControlReg 0x0C    //不同的控制寄存器
#define BitFramingReg 0x0D //面向位的帧的调节
#define CollReg 0x0E       //RF接口上检测到的第一个位冲突的位的位置
#define RFU0F 0x0F         //保留位
// PAGE 1     															命令
#define RFU10 0x10          //保留位
#define ModeReg 0x11        //定义发送和接收模式
#define TxModeReg 0x12      //定义发送数据速率
#define RxModeReg 0x13      //定义接收数据速率
#define TxControlReg 0x14   //天线驱动引脚1和2的逻辑特性
#define TxAutoReg 0x15      //天线驱动器的设置
#define TxSelReg 0x16       //选择天线驱动内部源
#define RxSelReg 0x17       //选择内部接收设置
#define RxThresholdReg 0x18 //选择位译码器的阈值
#define DemodReg 0x19       //定义解调器的设置
#define RFU1A 0x1A          //保留位
#define RFU1B 0x1B          //保留位
#define MifareReg 0x1C      //控制ISO mifare模式的106kbit/s的通信
#define RFU1D 0x1D          //保留位
#define RFU1E 0x1E          //保留位
#define SerialSpeedReg 0x1F //选择uart接口的速率
// PAGE 2    																CRG
#define RFU20 0x20         //保留位
#define CRCResultRegM 0x21 //显示CRC计算的最大值
#define CRCResultRegL 0x22 //显示CRC计算的最小值
#define RFU23 0x23         //保留位
#define ModWidthReg 0x24   //控制modwidth的设置
#define RFU25 0x25         //保留位
#define RFCfgReg 0x26      //配置接收增益
#define GsNReg 0x27
#define CWGsCfgReg 0x28
#define ModGsCfgReg 0x29
#define TModeReg 0x2A          //内部定时器设置
#define TPrescalerReg 0x2B     //
#define TReloadRegH 0x2C       //16位定时器重装载值高位
#define TReloadRegL 0x2D       //16位定时器重装载值地位
#define TCounterValueRegH 0x2E //显示定时器实际定时值高位
#define TCounterValueRegL 0x2F //显示定时器实际定时器地位
// PAGE 3      										//测试寄存器
#define RFU30 0x30           //保留位
#define TestSel1Reg 0x31     //常用测试信号配置
#define TestSel2Reg 0x32     //常用测试信号配置及PRBS控制
#define TestPinEnReg 0x33    //使能D1-D7引脚驱动
#define TestPinValueReg 0x34 //定义D1-D7的值，当他被用作I/O总线时
#define TestBusReg 0x35      //显示的内部测试总线的状态
#define AutoTestReg 0x36     //控制数字信号自测试
#define VersionReg 0x37      //显示版本
#define AnalogTestReg 0x38   //控制AUX1和AUX2引脚
#define TestDAC1Reg 0x39     //定义TestDAC1的测试值
#define TestDAC2Reg 0x3A     ////定义TestDAC2的测试值
#define TestADCReg 0x3B      //显示ADC I和Q的实际值
#define RFU3C 0x3C           //保留位
#define RFU3D 0x3D           //保留位
#define RFU3E 0x3E           //保留位
#define RFU3F 0x3F           //保留位

#define MAXRLEN 18
#define MI_OK 1
#define MI_NOTAGERR (-1)
#define MI_ERR 0
#endif
