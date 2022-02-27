#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "beep.h"
#include "lcd.h"
#include "key.h"
#include "malloc.h"
#include "w25qxx.h"
#include "RC522.h"
#include "spi.h"
#include "string.h"
#include "timer.h"
#include "main.h"
#include "exti.h"
// #include "sram.h"
// #include "usmart.h"
// #include "sdio_sdcard.h"
// #include "ff.h"
// #include "exfuns.h"
// #include "fontupd.h"
// #include "text.h"
// #include "rtc.h"

/** RFID接线方式
	PC10 -- CLK
	PC11 -- MISO
	PC12 -- MOSI
	PA4  -- NSS
	PA6  -- RST
*/
/**
 * @author DXR
 * @create 2021/12/2 13:56
 * <p>
 * 数据协议(未设置校验位)：
 * 0：报文头     1Byte  0x5B 即'['
 * 1：命令类型   1Byte  * @ $ = ( )
 * 2：数据段     xByte  x <= 200Byte
 * x+2：报文尾   1Byte  0x5D 即']'
 * <p>
 * 命令格式(第一个字节为命令类型，其后为数据段)：
 * 搜索ID：*ID
 * 查询用户名：@ID  返回对用ID的用户名
 * 查询余额：$ID    返回对应ID的余额
 * 设置余额：=ID,余额
 * 注册用户：(ID,用户名,余额
 * 数据段内容有多组 用 , 分隔
 * 如：[(ID,用户名,余额]  命令类型为:(  数据段为:ID,用户名,余额  即为注册用户
 * 删除用户数据：)ID
 * 以上命令发送后服务器处理成功会有相同命令类型的返回 否则返回 '~'+错误信息
 */
u8 adminID[ID_LEN + 1] = "7155135179";	// 管理员卡号(已将十六进制转成十进制) 用于删除用户的权限控制 可根据需求自行修改管理员卡号
u8 cardID[ID_LEN + 1];					// 存放普通用户ID卡号 +1是为了存放'\0'
u8 userName[NAME_LEN + 1];				// 用户名 +1是为了存放'\0'
u16 balance;							// 账户余额
u8 keyFun;								// 先初始化没有任何按键按下的状态
const u8 DATA_SIZE = ID_LEN + NAME_LEN; // 数据大小

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置系统中断优先级分组2
	delay_init(168);								//初始化延时函数
	LED_Init();										//初始化LED
	BEEP_Init();									//初始化蜂鸣器
	KEY_Init();										//初始化按键
	EXTIX_Init();									// 初始化外部中断 用于读取按键值
	W25QXX_Init();									//初始化W25Q128
	uart_init(115200);								//串口1初始化波特率为115200
	LCD_Init();										// LCD初始化
	RC522_SPI3_Init();
	MFRC522_Initializtion();
	POINT_COLOR = RED; // 设置字体为红色
	LCD_ShowString(30, 110, 200, 16, 16, "System loading...");
	BCSInit();	  // BusChargeSystem(BCS)初始化
	SysRunning(); // 运行入口
}

/**
 * @brief 运行入口
 */
void SysRunning(void)
{
	while (1)
	{
		LcdDesktop();
		switch (keyFun)
		{
		case KEY0_VALUE:
			LCD_Clear(BLUE);
			BCSRunning();	   //  系统运行
			keyFun = KEY_NULL; // 置为KEY_NULL -> 防止下次循环继续调用此功能
			break;
		case KEY1_VALUE:
			LCD_Clear(BLUE);
			ManageUser();	   // 管理用户
			keyFun = KEY_NULL; // 置为KEY_NULL -> 防止下次循环继续调用此功能
			break;
		case KEY2_VALUE:
			LCD_Clear(BLUE);
			BalanceRecharge(); // 余额充值
			keyFun = KEY_NULL;
			break;
		}
	}
}

/**
 * @brief 显示LCD桌面
 */
void LcdDesktop(void)
{
	// 系统信息
	LCD_DrawRectangle(20, 20, 220, 116); // 绘制一个外矩形框
	LCD_DrawRectangle(22, 22, 218, 114); // 绘制一个内矩形框
	LCD_ShowString(30, 30, 200, 16, 16, "BusChargeSystem(BCS)  ");
	LCD_ShowString(30, 50, 200, 16, 16, "   @Version: 2.0      ");
	LCD_ShowString(30, 70, 200, 16, 16, "   @Author: DXR       ");
	LCD_ShowString(30, 90, 200, 16, 16, "   @Date: 2021/12/1   ");
	// 桌面信息
	LCD_ShowString(20, 130, 200, 16, 16, "=========================");
	LCD_ShowString(20, 150, 200, 16, 16, "*       <Desktop>       *");
	LCD_ShowString(20, 170, 200, 16, 16, "=========================");
	LCD_ShowString(20, 190, 200, 16, 16, "*    KEY0 <Running>     *");
	LCD_ShowString(20, 210, 200, 16, 16, "*    KEY1 <Manage User> *");
	LCD_ShowString(20, 230, 200, 16, 16, "*    KEY2 <Recharge>    *");
	LCD_ShowString(20, 250, 200, 16, 16, "=========================");
}

void ConsumeTips()
{
	LCD_DrawRectangle(20, 130, 220, 230); // 绘制一个外矩形框
	LCD_DrawRectangle(22, 132, 218, 228); // 绘制一个内矩形框
	LCD_ShowString(30, 150, 200, 16, 16, "    ====(((())))====   ");
}

/**
 * @brief BusChargeSystem(BCS)运行
 */
void BCSRunning(void)
{
	u8 status;
	u8 tempCardID[ID_LEN] = {0x0};
	u8 consume = 0;
	LCD_Clear(BLUE);
	while (1)
	{
		ConsumeTips();
		status = ReadCard();
		if (status == MI_OK)
		{
			balance = GetBalance(); // 获取当前用户的余额
			/* 如果上一次刷卡的ID和这一次刷卡的ID不相同 那么就认为有新的用户正在消费 */
			if (IsEquals(cardID, tempCardID) == FALSE)
			{
				if (consume > 0)
				{
					/* 可以在这里写一个保存消费记录到服务器  */
				}
				consume = 0; // 则把消费金额置零
			}
			/* 余额必须大于0 才可以消费 */
			if (balance > 0)
			{
				LCD_ShowNum(75, 180, ++consume, 3, 24); // 显示消费金额
				LCD_ShowString(115, 180, 200, 16, 24, "RMB");
				/* 读卡成功提示 */
				OK_BEEP();
				OK_LED();
				BalancdInc(-1); // 余额-1
				LCD_Clear(BLUE);
				memcpy(tempCardID, cardID, ID_LEN); // 把这次消费的ID复制到临时存储区
			}
			else /* 余额不足 */
			{
				LCD_ShowString(40, 180, 200, 16, 16, "Error:Balance is null!");
				/* 读卡错误提示 */
				ERR_BEEP();
				ERR_LED();
				LCD_Clear(BLUE);
			}
		}
		/* 用户可强制退出运行模式 */
		if (keyFun == KEYUP_VALUE)
		{
			LCD_Clear(BLUE);
			LCD_ShowString(40, 110, 200, 16, 16, "Tips: Exiting...");
			// //ERR_BEEP();
			delay_ms(1000);
			LCD_Clear(BLUE);
			break;
		}
		// LCD_Clear(BLUE);
	}
}

/**
 * @brief 获取用户余额
 * @return u8 返回当前ID对应的用户余额
 */
u16 GetBalance()
{
	u16 bal = 0;
	u16 i;
	u8 buf[DATA_SIZE];
	sprintf((char *)buf, "[$%s]", cardID);
	printf(buf);	// 发送串口数据 [$ID]
	delay_ms(1000); // 延时是为了稳定通信
	while (USART_RX_STA & 0x8000 == 0)
	{
		/* 等待数据接收完成 */
	}
	USART_RX_STA = 0;
	if (USART_RX_BUF[1] == '$')
	{
		/* 将用户余额读取出来 */
		for (i = 2; USART_RX_BUF[i] != '.'; i++)
		{
			bal = bal * 10 + USART_RX_BUF[i] - 48;
		}
		return bal;
	}
	LCD_Clear(BLUE);
	LCD_ShowString(30, 110, 220, 16, 16, "Error:Not fuond user! ");
	ERR_BEEP();
	delay_ms(1000);
	LCD_Clear(BLUE);
	return 0;
}

/**
 * @brief 增加或减少当前用户余额
 * @param bal 可接收正值和负值 正值为充值 负值为扣费
 * @return 成功返回TRUE 返回FALSE
 */
Boolean BalancdInc(int bal)
{
	u8 sendBuf[DATA_SIZE];

	sprintf((char *)sendBuf, "[=%s,%d]", cardID, bal);
	printf(sendBuf); // 发送串口数据 [=ID,余额]
	delay_ms(1000);	 // 延时是为了稳定通信
	while (USART_RX_STA & 0x8000 == 0)
	{
		/* 等待数据接收完成 */
	}
	USART_RX_STA = 0;
	if (USART_RX_BUF[1] == '=')
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * @brief 显示充值菜单
 */
void RechargeMenu(void)
{
	LCD_ShowString(20, 130, 200, 16, 16, "=========================");
	LCD_ShowString(20, 150, 200, 16, 16, "*       <Recharge>      *");
	LCD_ShowString(20, 170, 200, 16, 16, "=========================");
	LCD_ShowString(20, 190, 200, 16, 16, "*      KEY1: 100 RMB    *");
	LCD_ShowString(20, 210, 200, 16, 16, "*      KEY2: 200 RMB    *");
	LCD_ShowString(20, 230, 200, 16, 16, "*      KEYUP: Exit      *");
	LCD_ShowString(20, 250, 200, 16, 16, "=========================");
}

/**
 * @brief 余额充值
 */
void BalanceRecharge(void)
{
	u8 status;
	keyFun = KEY_NULL; // 先初始化没有任何按键按下的状态
	LCD_Clear(BLUE);
	while (1)
	{
		RechargeMenu(); // 显示充值菜单
		/* KEY_UP 按下 则退出 */
		if (KEYUP_VALUE == keyFun)
		{
			LCD_Clear(BLUE);
			return;
		}

		if (keyFun == KEY1_VALUE || keyFun == KEY2_VALUE)
		{
			LCD_Clear(BLUE);
			ReadCardTips();		 // 提示读卡信息
			status = ReadCard(); // 读卡
			if (status == MI_OK)
			{
				/* 充值 100 或 200 RMB */
				if (BalancdInc(keyFun * 100) == TRUE)
				{
					LCD_Clear(BLUE);
					LCD_ShowString(30, 110, 220, 16, 16, "OK:Recharge succeed!");
					LCD_ShowNum(75, 180, keyFun * 100, 3, 24);
					OK_BEEP();
					LCD_ShowString(115, 180, 200, 16, 24, "RMB");
					delay_ms(2000);
					LCD_Clear(BLUE);
					keyFun = KEY_NULL;
				}
				else
				{
					LCD_Clear(BLUE);
					LCD_ShowString(30, 110, 220, 16, 16, "Error:Not fuond user! ");
					ERR_BEEP();
					delay_ms(1000);
					LCD_Clear(BLUE);
					keyFun = KEY_NULL;
				}
			}
			else
			{
				keyFun = KEY_NULL; // 退出
			}
		}
	}
}

/**
 * @brief 搜索用户卡的ID
 * @param cardID[u8*] 用户卡ID
 * @return Boolean 存在返回TRUE 否则返回FALSE
 */
Boolean SearchID(u8 *cardID)
{
	u8 buf[DATA_SIZE];
	sprintf((char *)buf, "[*%s]", cardID);
	printf(buf);	// 发送串口数据 [*ID]
	delay_ms(1000); // 延时是为了稳定通信
	while (USART_RX_STA & 0x8000 == 0)
	{
		/* 等待数据接收完成 */
	}
	if (USART_RX_BUF[1] == '*')
	{
		OK_LED();
		return TRUE;
	}
	ERR_LED();
	return FALSE;
}

/**
 * @brief 比较两个数组是否相等
 * @param arr1[u8*]
 * @param arr2[u8*]
 * @return Boolean 相等返回TRUE 否则返回FALSE
 */
Boolean IsEquals(u8 *arr1, u8 *arr2)
{
	u8 i;
	u8 ans = 0;
	u8 len = strlen((char *)arr1);
	if (len != strlen((char *)arr2))
	{
		return FALSE;
	}
	/* 通过异或运算校验每一位 */
	for (i = 0; i < len; i++)
	{
		ans |= arr1[i] ^ arr2[i];
	}
	/* 如果最终ans还是0 则说明两个数组相等返回TRUE 否则不相等返回FALSE */
	return ans == 0 ? TRUE : FALSE;
}

/**
 * @brief 读卡提示信息
 */
void ReadCardTips(void)
{
	// LCD_Clear(BLUE);
	LCD_ShowString(20, 130, 200, 16, 16, "=========================");
	LCD_ShowString(20, 150, 200, 16, 16, "*    ====(((())))====   *");
	LCD_ShowString(20, 170, 200, 16, 16, "*        Reading...     *");
	LCD_ShowString(20, 190, 200, 16, 16, "*       KEYUP:Exit      *");
	LCD_ShowString(20, 210, 200, 16, 16, "=========================");
}

/**
 * @brief RFID读卡
 * @return char 返回状态
 */
char ReadCard(void)
{
	char status;
	u8 cardIDTemp[4];
	u8 cardType[4];
	// ReadCardTips(); // 读卡提示信息
	while (1)
	{
		status = PcdRequest(0x52, cardType); // 复位应答
		status = PcdAnticoll(cardIDTemp);	 /*防冲撞*/
		status = PcdSelect(cardIDTemp);		 //选卡
		if (status == MI_OK)
		{
			break;
		}
		/* 可强制退出 */
		if (keyFun == KEYUP_VALUE)
		{
			// LCD_Clear(BLUE);
			return MI_ERR;
		}
	}
	/* 把读到的ID卡号转为一组十进制数组成的字符串存入cardID */
	sprintf((char *)cardID, "%d%d%d%d", cardIDTemp[0], cardIDTemp[1], cardIDTemp[2], cardIDTemp[3]);
	return MI_OK;
}

/**
 * @brief 管理用户菜单
 */
void ManageUserMenu(void)
{
	LCD_ShowString(20, 130, 200, 16, 16, "=========================");
	LCD_ShowString(20, 150, 200, 16, 16, "*      <Manage User>    *");
	LCD_ShowString(20, 170, 200, 16, 16, "=========================");
	LCD_ShowString(20, 190, 200, 16, 16, "*    KEY0: Add User     *");
	LCD_ShowString(20, 210, 200, 16, 16, "*    KEY1: Delete User  *");
	LCD_ShowString(20, 230, 200, 16, 16, "*    KEYUP: Exit        *");
	LCD_ShowString(20, 250, 200, 16, 16, "=========================");
}

/**
 * @brief 管理用户
 */
void ManageUser(void)
{
	u8 status;

	keyFun = KEY_NULL;

	while (1)
	{
		ManageUserMenu();
		/* KEY_UP 按下 则退出 */
		if (KEYUP_VALUE == keyFun)
		{
			// PcdHalt();
			LCD_Clear(BLUE);
			return;
		}
		/* 添加用户 -> 用户注册 */
		if (keyFun == KEY0_VALUE)
		{
			UserSignup();
			keyFun = KEY_NULL;
		}
		/* 删除用户 */
		if (keyFun == KEY1_VALUE)
		{
			LCD_Clear(BLUE);
			LCD_ShowString(20, 110, 200, 16, 16, "*      <Admin Login>    *");
			ReadCardTips();
			status = ReadCard(); // 读卡
			if (status == MI_OK)
			{
				/* 验证管理员 */
				if (IsEquals(cardID, adminID) == TRUE)
				{
					OK_BEEP();
					DeleteUser();
					keyFun = KEY_NULL;
				}
				else
				{
					LCD_Clear(BLUE);
					LCD_ShowString(30, 110, 220, 16, 16, "Error:Non-Administrator!");
					ERR_BEEP();
					delay_ms(1000);
					LCD_Clear(BLUE);
					keyFun = KEY_NULL;
				}
			}
			else
			{
				keyFun = KEY_NULL;
			}
		}
	}
}

/**
 * @brief 显示添加用户菜单
 */
void AddUserMenu(void)
{
	LCD_ShowString(20, 130, 200, 16, 16, "=========================");
	LCD_ShowString(20, 150, 200, 16, 16, "*       <Add User>      *");
	LCD_ShowString(20, 170, 200, 16, 16, "=========================");
	LCD_ShowString(20, 190, 200, 16, 16, "*    KEY0: Zhou Mingfa  *");
	LCD_ShowString(20, 210, 200, 16, 16, "*    KEY1: Xu Mengyan   *");
	LCD_ShowString(20, 230, 200, 16, 16, "*    KEY2: Fu Yiqing    *");
	LCD_ShowString(20, 250, 200, 16, 16, "*    KEYUP: Exit        *");
	LCD_ShowString(20, 270, 200, 16, 16, "=========================");
}

/**
 * @brief 注册
 */
void UserSignup(void)
{
	char status;
	u8 name[10] = "ZMFXMYFYQ";
	keyFun = KEY_NULL; // 先初始化没有任何按键按下的状态
	while (1)
	{
		AddUserMenu();
		ShowTime(DHM); // 显示时间

		/* KEY_UP 按下 则退出 */
		if (KEYUP_VALUE == keyFun)
		{
			// PcdHalt();
			LCD_Clear(BLUE);
			return;
		}

		/* 存在按键按下 且 按下的按键不是KEYUP */
		if ((KEY_NULL != keyFun) && (keyFun != KEYUP_VALUE))
		{
			LCD_Clear(BLUE);
			ReadCardTips();
			status = ReadCard(); // 读卡
			if (status == MI_OK)
			{
				/* 存入用户名 且 注册默认充值10RMB */
				snprintf((char *)userName, 4, "%s", &name[keyFun * 3]);
				balance = 10;
				AddUser(cardID);
				keyFun = KEY_NULL; // 执行一次注册功能 将按键置空
			}
			else
			{
				keyFun = KEY_NULL;
			}
		}
	}
}

/**
 * @brief 通过传入的用户卡ID添加用户
 * @param cardID
 */
void AddUser(u8 *cardID)
{
	/* 添加成功 */
	if (AddData(cardID) == TRUE)
	{
		LCD_Clear(BLUE);
		LCD_ShowString(20, 110, 200, 16, 16, "OK:Signup succeed!");
		LCD_ShowString(20, 140, 200, 16, 16, "Name:");
		LCD_ShowString(80, 135, 200, 16, 24, userName); // 显示用户名
		LCD_ShowString(20, 170, 200, 16, 16, "Balance:");
		LCD_ShowNum(80, 165, balance, 3, 24); // 显示余额
		LCD_ShowString(120, 165, 200, 16, 24, "RMB");

		OK_BEEP();
		delay_ms(3000);
		LCD_Clear(BLUE);
	}
	else /* 添加失败 */
	{
		LCD_Clear(BLUE);
		LCD_ShowString(20, 110, 200, 16, 16, "Error:Signup fail!");
		ERR_BEEP();
		delay_ms(1000);
		LCD_Clear(BLUE);
	}
}

/**
 * @brief 添加用户数据
 * @param cardID[u8*]
 * @return Boolean
 */
Boolean AddData(u8 *cardID)
{
	u8 sendBuf[DATA_SIZE], dataBuf[DATA_SIZE];
	sprintf((char *)dataBuf, "%s,%s,%d", cardID, userName, balance);
	sprintf((char *)sendBuf, "[(%s]", dataBuf);
	printf(sendBuf); // 发送串口数据 [(ID,用户名,余额]
	delay_ms(1000);	 // 延时是为了稳定通信
	while (USART_RX_STA & 0x8000 == 0)
	{
		/* 等待数据接收完成 */
	}
	USART_RX_STA = 0;
	if (USART_RX_BUF[1] == '(')
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * @brief 显示删除用户菜单
 *
 */
void DelUserMenu(void)
{
	LCD_ShowString(20, 130, 200, 16, 16, "=========================");
	LCD_ShowString(20, 150, 200, 16, 16, "*    <Please Select>    *");
	LCD_ShowString(20, 170, 200, 16, 16, "=========================");
	LCD_ShowString(20, 190, 200, 16, 16, "* KEY1: Delete a user   *");
	LCD_ShowString(20, 210, 200, 16, 16, "* KEYUP: Exit           *");
	LCD_ShowString(20, 230, 200, 16, 16, "=========================");
}

/**
 * @brief 删除用户
 */
void DeleteUser(void)
{
	char status;
	keyFun = KEY_NULL;
	LCD_Clear(BLUE);
	while (1)
	{
		DelUserMenu();
		if (KEYUP_VALUE == keyFun)
		{
			LCD_Clear(BLUE);
			return;
		}

		/* 按KEY1删除指定用户卡ID 需刷指定的卡进行删除操作 */
		while (keyFun == KEY1_VALUE)
		{
			LCD_Clear(BLUE);
			ReadCardTips();
			status = ReadCard(); // 刷卡
			if (status == MI_OK)
			{
				RemoveUser(cardID);
				keyFun = KEY_NULL;
			}
			else /* 读卡失败 */
			{
				keyFun = KEY_NULL; // 强制退出
			}
		}
	}
}

/**
 * @brief 通过传入的用户卡ID删除指定用户
 * @param cardID
 */
void RemoveUser(u8 *cardID)
{
	/* 删除成功 */
	if (RemoveData(cardID) == TRUE)
	{
		LCD_Clear(BLUE);
		LCD_ShowString(10, 110, 220, 16, 16, "OK:Delete user succeed!");
		OK_BEEP();
		delay_ms(1000);
		LCD_Clear(BLUE);
	}
	else /* 无此用户 删除失败 */
	{
		LCD_Clear(BLUE);
		LCD_ShowString(10, 110, 220, 16, 16, "Error:Not fuond user! ");
		ERR_BEEP();
		delay_ms(1000);
		LCD_Clear(BLUE);
	}
}

/**
 * @brief 删除指定用户数据
 * @param cardID[u8*]
 * @return Boolean
 */
Boolean RemoveData(u8 *cardID)
{
	u8 sendBuf[DATA_SIZE];
	sprintf((char *)sendBuf, "[)%s]", cardID);
	printf(sendBuf); // 发送串口数据 [)ID]
	delay_ms(1000);	 // 延时是为了稳定通信
	while (USART_RX_STA & 0x8000 == 0)
	{
		/* 等待数据接收完成 */
	}
	USART_RX_STA = 0;
	if (USART_RX_BUF[1] == ')')
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * @brief 返回给定模式的时间 或 显示时间
 * @param timeMode[u8] 选择的模式
 * @return u8 D返回日 H返回时、M返回分  DHM显示时间，无返回
 */
u8 ShowTime(u8 timeMode)
{
	RTC_TimeTypeDef TheTime; //时间
	RTC_DateTypeDef TheData; //日期
	u8 *TimeBuff;			 //时间存储

	RTC_GetTime(RTC_Format_BIN, &TheTime);
	RTC_GetDate(RTC_Format_BIN, &TheData);
	switch (timeMode)
	{
	case D:
		return TheData.RTC_Date;
	case H:
		return TheTime.RTC_Hours;
	case M:
		return TheTime.RTC_Minutes;
	case DHM:
		TimeBuff = malloc(40);
		sprintf((char *)TimeBuff, "%02d%s:%02d%s:%02d%s", TheData.RTC_Date, "D", TheTime.RTC_Hours, "H", TheTime.RTC_Minutes, "M");
		LCD_ShowString(130, 300, 200, 16, 16, TimeBuff);
		free(TimeBuff);
		break;
	}
	return 0;
}

/**
 * @brief BusChargeSystem(BCS)初始化
 */
void BCSInit(void)
{
	LCD_Clear(BLUE);   // 清屏为蓝色 即背景颜色
	BACK_COLOR = BLUE; // 设置背景颜色为蓝色
	POINT_COLOR = RED; // 设置字体为红色
	keyFun = KEY_NULL; // 先初始化没有任何按键按下的状态
}
