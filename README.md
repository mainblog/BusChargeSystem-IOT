# BusChargeSystem(IOT)

## 项目介绍

基于 STM32F4+ESP8266+RFID 的公交车收费系统(物联网版)

做这个小小的项目过程中参考了很多正点原子的资料，很多东西都是刚开始学习，所以只会以最简单的方式实现一个物联网项目。

项目示意图：
![项目通信图](./image/signal_communication.jpg)

## 1. 功能列表

| 功能 | 介绍 |
| :--- | ---- |
| 添加用户 | 刷卡识别需要添加的用户 |
| 删除用户 | 刷卡识别需要删除的用户 |
| 刷卡消费 | 每次刷卡消费金额为1元，同一用户可连续刷卡消费 |
| 余额充值 | 通过按键选择充值金额并刷卡识别完成充值 |

## 2. 技术栈

### 2.1 环境

| 环境        | 版本          |
| :---------- | ------------- |
| 操作系统    | Windows/Linux |
| KeilMDK-ARM | 5.35          |
| Arduino     | 1.8.15        |
| Jdk         | 1.8           |
| Mysql       | 8.0           |

### 2.2 硬件端

- STM32F4
- ESP8266
- RFID 模块

### 2.3 服务端

- Java
- Mysql

## 3. 项目开发

### 3.1 硬件端开发

+ .\BusChargeSystem\为STM32F4开发板的代码，请使用Keil uVision5集成开发工具打开\USER\BusChargeSystem.uvprojx文件
+ .\esp8266\为esp8266开发板的代码，请使用Arduino开发工具打开

### 3.2 硬件接线方式

| STM32F4 | RFID 模块 |
| :-------- | --------- |
| 3.3V      | VCC       |
| GND       | GND       |
| PC10      | CLK       |
| PC11      | MISO      |
| PC12      | MOSI      |
| PA4       | NSS       |
| PA6       | RST       |

| STM32F4 | ESP8266 |
| :-------- | ------------ |
| 3.3V      | VCC          |
| GND       | GND          |
| PA9(TX)   | RX           |
| PA10(RX)  | TX           |

### 3.3 服务端开发

+ .\sql\为数据库脚本，安装mysql，导入bus_charge_system.sql脚本
+ 数据库名称为：bus_charge_system
+ 数据库默认账号：BCSAdmin 密码：bJXhYu7e8EH4mzNT
+ .\BusChargeSystem\为服务端代码，请使用IntelliJ IDEA打开
+ 启动服务端程序，默认端口为8888

### 3.4 数据库表

表名：user_tb

| 字段名      | 类型     | 注释     |
| :---------- | -------- | -------- |
| id          | int      | RFID模块读卡的卡号 |
| name        | varchar  | 用户名   |
| sex         | char     |         |
| age         | int      |         |
| balance     | double   | 用户余额 |
| signup_time | datetime | 注册时间 |

## 4. 项目部署

### 4.1 硬件端部署

STM32:
+ 使用Keil uVision5集成开发工具打开.\BusChargeSystem\USER\目录下的BusChargeSystem.uvprojx文件并编译
+ 编译后会在.\BusChargeSystem\OBJ\目录下生成BusChargeSystem.hex文件，通过FlyMcu工具将此文件下载到开发板上 或 使用DAP仿真器

ESP8266:
+ 用Arduino开发工具打开.\esp8266\目录下的esp8266.ino文件并将程序下载到开发板上

### 4.2 服务端部署

+ 将.\BusChargeSystemService目录下的Java程序打包成Jar包并上传到云服务器
+ 在云服务器上将8888端口加入安全组
+ 在终端执行Jar包，命令如下：
  ```
  java -jar BusChargeSystemService.jar
  ```

## 5. 其他

**！！！此项目仅用于学习与分享 不会用于其它任何用途！！！**

**版权方：**

正点原子@ALIENTEK

广州市星翼电子科技有限公司