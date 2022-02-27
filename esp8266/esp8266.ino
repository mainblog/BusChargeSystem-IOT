#include <Arduino.h>
#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "DXR 7400"
#define STAPSK "zxcvbnm.."
#define BUF_SIZE 200
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

// const char *host = "192.168.137.1";
const char *host = "120.25.221.146";
const uint16_t port = 8888;

void setup()
{
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);    // STA模式 即连接模式
  WiFi.begin(ssid, password); // 连接到wifi网络

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // Serial.flush();
}

char recvBuf[BUF_SIZE];
char data;
u16 bufIndex = 0;
bool startByte = false;

/**
   @brief 向服务器发送数据
*/
void sendData(char *buf)
{
  WiFiClient client;
  while (!client.connect(host, port))
  {
    Serial.write("connection failed\r\n");
    delay(2000);
  }
  if (client.connected())
  {
    client.write(buf);
  }
  /* 等待服务器返回数据 */
  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    /* 10秒内服务器都没有回复响应 则超时关闭连接 */
    if (millis() - timeout > 10000)
    {
      Serial.write(">>> Client Timeout !\r\n");
      client.stop();
      return;
    }
  }

  int len = client.available();
  if (len > BUF_SIZE) // 服务器返回的数据长度超出缓存区大小
  {
    while (client.read() >= 0)
    {
    }
    Serial.write("len error\r\n");
    client.stop(); // 关闭连接
    return;
  }
  // Serial.println(len);
  char *sendBuf = (char *)malloc(sizeof(char) * len + 1);
  while (client.available())
  {
    // 从服务器获取数据
    char data = (char)client.read();
    if (data == 0x5B)
    {
      memset(sendBuf, 0, len + 1);
      bufIndex = 0;
      startByte = true;
    }
    if (startByte)
    {
      sendBuf[bufIndex++] = data; // 存储数据
    }
    if (data == 0x5D && startByte)
    {
      bufIndex |= 0x8000; // 接收到数户据的标志
      startByte = false;
    }
  }
  if (bufIndex & 0x8000)
  {
    Serial.write(sendBuf); // 向串口发送数据
    Serial.write("\r\n");
  }
  free(sendBuf);
  client.stop(); // 关闭连接
}

void loop()
{
  while (Serial.available())
  {
    data = (char)Serial.read();
    if (data == 0x5B)
    {
      memset(recvBuf, 0, BUF_SIZE);
      bufIndex = 0;
      startByte = true;
    }
    if (startByte)
    {
      recvBuf[bufIndex++] = data; // 存储数据
    }
    if (data == 0x5D && startByte)
    {
      bufIndex |= 0x8000; // 接收到数户据的标志
      startByte = false;
    }
  }
  if (bufIndex & 0x8000)
  {
    sendData(recvBuf);
    bufIndex = 0;
  }
}
