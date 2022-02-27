package com.dxr.BCSservice;

import java.sql.*;

/**
 * JDBC单例模式(饿汉式)
 *
 * @author DXR
 * @create 2021/12/7 15:31
 */
public final class JdbcUtilsSing {
    // MySQL 8.0 以上版本 - JDBC 驱动名及数据库 URL
    private static final String JDBC_DRIVER = "com.mysql.cj.jdbc.Driver";
    private static final String DB_URL = "jdbc:mysql://127.0.0.1:3306/bus_charge_system?characterEncoding=utf8&useSSL=false&allowPublicKeyRetrieval=true&serverTimezone=UTC";
    // 数据库的用户名与密码，需要根据自己的设置
    private static final String USER = "BCSAdmin";
    private static final String PASS = "123456"; // bJXhYu7e8EH4mzNT

    private static final JdbcUtilsSing instance = new JdbcUtilsSing();

    // 注册 JDBC 驱动
    static {
        try {
            Class.forName(JDBC_DRIVER);
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
    }

    public JdbcUtilsSing() {
    }

    /**
     * 获取实例
     *
     * @return 返回一个JdbcUtilsSing的实例
     */
    public static JdbcUtilsSing getInstance() {
        return instance;
    }

    /**
     * 建立连接
     *
     * @return 返回一个Connection实例
     */
    public Connection getConnection() {
        Connection conn = null;
        try {
            conn = DriverManager.getConnection(DB_URL, USER, PASS);
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return conn;
    }

    /**
     * 释放资源
     *
     * @param rs   ResultSet
     * @param st   Statement
     * @param conn Connection
     */
    public void free(ResultSet rs, Statement st, Connection conn) {
        // 释放ResultSet
        try {
            if (rs != null)
                rs.close();
        } catch (SQLException e) {
            e.printStackTrace();
        } finally {
            // 释放Statement
            try {
                if (st != null)
                    st.close();
            } catch (SQLException e) {
                e.printStackTrace();
            } finally {
                // 释放Connection
                try {
                    if (conn != null)
                        conn.close();
                } catch (SQLException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
