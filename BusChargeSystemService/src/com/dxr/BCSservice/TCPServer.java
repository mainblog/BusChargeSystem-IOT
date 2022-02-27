package com.dxr.BCSservice;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.math.BigDecimal;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Arrays;

/**
 * @author DXR
 * @create 2021/12/2 13:56
 */
public class TCPServer implements Runnable {
    // 监听的端口
    private static final int PORT = 8888;
    // 缓冲区的大小 200Byte
    private static final int BUF_SIZE = 200;
    // 缓冲区
    private byte[] buf;
    // 缓冲区数组索引
    private int bufIndex = 0;
    // 开始接收字节标志位
    private boolean startByte = false;

    private final ServerSocket serverSocket;
    private Socket socket;

    public TCPServer() throws IOException {
        serverSocket = new ServerSocket(PORT);
    }

    /**
     * 解析数据并执行相应的操作
     * <p>
     * 数据协议(未设置校验位)：
     * 0：报文头     1Byte  0x5B 即'['
     * 1：命令类型   1Byte  * @ $ = ( )
     * 2：数据段     xByte  x <= (200 - 3)Byte
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
     *
     * @throws Exception 抛出 命令类型错误 用户不存在等错误数据异常
     */
    private void parseData() throws Exception {
        // 第2位开始到length-1是数据段
        byte[] data = Arrays.copyOfRange(buf, 2, buf.length - 1);
        // 拆分数据
        String[] dataArray = new String(data).split(",");
        // 获取ID  每个数据段的首位都是用户ID
        BigDecimal ID = new BigDecimal(dataArray[0]);
        BCSOperator bcsUser = new BCSUserService(ID);
        byte cmd = buf[1]; // 命令类型
        switch (cmd) {
            case '*': // 查询用户ID是否存在
                if (bcsUser.getID() != null) {
                    sendDataToClient('*', "用户存在！");
                } else {
                    throw new Exception("用户不存在！");
                }
                break;
            case '@': // 获取用户名
                if (bcsUser.getID() != null) {
                    sendDataToClient('@', bcsUser.getName());
                } else {
                    throw new Exception("用户不存在！");
                }
                break;
            case '$': // 获取用户余额
                if (bcsUser.getID() != null) {
                    sendDataToClient('$', Integer.toString((int) bcsUser.getBalance()));
                } else {
                    throw new Exception("用户不存在！");
                }
                break;
            case '=': // 设置用户余额
                if (bcsUser.getID() != null) {
                    // 获取数据段中的余额
                    double balanceIcn = Double.parseDouble(dataArray[1]);
                    if (bcsUser.setBalance(bcsUser.getBalance() + balanceIcn))
                        sendDataToClient('=', "设置余额成功！");
                } else {
                    throw new Exception("用户不存在！");
                }
                break;
            case '(': // 注册用户
                if (bcsUser.getID() == null) {
                    String name = dataArray[1]; // 获取用户名
                    double balance = Double.parseDouble(dataArray[2]); // 获取注册默认充值的余额
                    if (bcsUser.addUser(ID, name, balance))
                        sendDataToClient('(', "注册成功！");
                } else {
                    throw new Exception("用户已存在！");
                }
                break;
            case ')': // 删除用户
                if (bcsUser.getID() != null) {
                    if (bcsUser.deleteUser())
                        sendDataToClient(')', "删除成功！");
                } else {
                    throw new Exception("用户不存在！");
                }
                break;
            default:
                throw new Exception("命令类型错误！");
        }
    }

    /**
     * 发送数据到客户端
     *
     * @param cmd  命令类型
     * @param data 数据段
     * @throws IOException 抛出输出流的IO异常
     */
    private void sendDataToClient(char cmd, String data) throws IOException {
        OutputStream outputStream = socket.getOutputStream();
        String sendData = "[" + cmd + data + "]";
        outputStream.write(sendData.getBytes());
        outputStream.flush();
        System.out.print("返回的数据为：");
        System.out.write(sendData.getBytes());
        System.out.println();
    }

    @Override
    public void run() {
        while (true) {
            try {
                System.out.println("正在监听的端口：" + serverSocket.getLocalPort() + "\n等待连接... ");
                socket = serverSocket.accept();
                System.out.println("连接成功==>>客户端主机地址为：" + socket.getRemoteSocketAddress());

                // 通过socket.getInputStream() 读取
                InputStream inputStream = socket.getInputStream();
                // IO读取
                byte data = (byte) inputStream.read(); // 读取第一个字节
                int dataLen = 0;
                if (data == 0x5B) { // 接收到第一个字节为报文头
                    dataLen = inputStream.available(); // 剩余的可用数据长度
                    if (dataLen + 1 > BUF_SIZE) {
                        System.out.println("接收到的数据长度：" + dataLen);
                        sendDataToClient('~', "Error,data len:" + dataLen);
                        continue;
                    }
                    buf = new byte[dataLen + 1];
                    bufIndex = 0;
                    buf[bufIndex++] = data;
                    startByte = true;
                }
                while (dataLen != 0) {
                    data = (byte) inputStream.read();
                    if (startByte) {
                        buf[bufIndex++] = data; // 存储数据
                    }
                    if (data == 0x5D && startByte) { // 接收到报文尾
                        bufIndex |= 0x8000; // 接收到一个完整的数据包的标志
                        startByte = false;
                    }
                    --dataLen;
                }

                if ((bufIndex & 0x8000) != 0) {
                    try {
                        System.out.println("接收到的数据长度：" + buf.length);
                        System.out.print("数据为：");
                        System.out.write(buf);
                        System.out.println();
                        parseData(); // 解析数据并执行相关操作
                    } catch (Exception e) {
                        /* 向客户端发送异常信息 */
                        sendDataToClient('~', e.getMessage());
                        e.printStackTrace();
                    }
                    bufIndex = 0;
                }

            } catch (IOException e) {
                e.printStackTrace();
                break;
            } finally {
                try {
                    if (socket != null) {
                        /* socket关闭也会把输入输出流一起关闭 */
                        socket.close();
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
