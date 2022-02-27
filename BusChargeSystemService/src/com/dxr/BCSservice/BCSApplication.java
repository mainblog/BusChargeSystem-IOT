package com.dxr.BCSservice;

import java.io.IOException;

/**
 * @author DXR
 * @create 2021/12/6 1:44
 */
public class BCSApplication {

    public static void main(String[] args) {
        try {
            TCPServer tcpServer = new TCPServer();
            Thread thread = new Thread(tcpServer);
            thread.start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}