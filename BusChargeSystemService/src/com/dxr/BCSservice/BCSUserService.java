package com.dxr.BCSservice;

import java.math.BigDecimal;
import java.sql.*;
import java.util.Date;
import java.text.SimpleDateFormat;


/**
 * @author DXR
 * @create 2021/12/2 14:34
 */
public class BCSUserService implements BCSOperator {

    private BigDecimal ID;
    private String name;
    private char sex;
    private int age;
    private double balance;

    public BCSUserService(BigDecimal ID) {
        initUserData(ID);
    }

    /**
     * 初始化用户数据
     * <p>
     * 此方法从数据库中获取对应用户ID的数据并对实例进行初始化
     * <p>
     * 此方法应由构造器调用
     *
     * @param ID 用户ID
     */
    private void initUserData(BigDecimal ID) {
        Connection conn = null;
        ResultSet rs = null;
        PreparedStatement ps = null;
        try {
            conn = JdbcUtilsSing.getInstance().getConnection();
            String sql = "select * from user_tb where id = ?;";
            ps = conn.prepareStatement(sql);
            ps.setBigDecimal(1, ID);
            rs = ps.executeQuery();
            if (rs.next()) {
                this.ID = rs.getBigDecimal("id");
                this.name = rs.getString("name");
                this.sex = rs.getString("name").toCharArray()[0];
                this.age = rs.getInt("age");
                this.balance = rs.getDouble("balance");
            }
        } catch (SQLException e) {
            e.printStackTrace();
        } finally {
            JdbcUtilsSing.getInstance().free(rs, ps, conn);
        }
    }

    /**
     * 插入用户数据
     * <p>
     * 将用户的相应数据插入到数据库的用户表中
     *
     * @param ID      用户ID
     * @param name    用户名
     * @param sex     性别
     * @param age     年龄
     * @param balance 余额
     * @return 插入成功返回true 否则返回false
     */
    private boolean insertUserData(BigDecimal ID, String name, char sex, int age, Double balance) {
        Connection conn = null;
        PreparedStatement ps = null;
        try {
            // 连接数据库
            System.out.println("连接数据库...");
            conn = JdbcUtilsSing.getInstance().getConnection();

            // 获取本地时间 并格式化
            Date date = new Date();
            SimpleDateFormat ft = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss");
            // 插入数据到用户表user_tb
            String sql = "insert into user_tb(id,name,sex,age,balance,signup_time) values(?,?,?,?,?,?);";
            ps = conn.prepareStatement(sql);
            ps.setBigDecimal(1, ID);
            ps.setString(2, name);
            ps.setString(3, Character.toString(sex));
            ps.setInt(4, age);
            ps.setDouble(5, balance);
            ps.setString(6, ft.format(date));
            ps.execute();

//            printUserTable(); // 打印查询表结果
        } catch (SQLException e) {
            // JDBC错误
            e.printStackTrace();
            return false;
        } finally {
            JdbcUtilsSing.getInstance().free(null, ps, conn);
        }
        return true;
    }

    /**
     * 删除用户数据
     * <p>
     * 从数据库中删除对应用户ID的数据
     *
     * @param ID 用户ID
     * @return 删除成功返回true 否则返回false
     */
    private boolean deleteUserData(BigDecimal ID) {
        Connection conn = null;
        PreparedStatement ps = null;
        try {
            // 连接数据库
            System.out.println("连接数据库...");
            conn = JdbcUtilsSing.getInstance().getConnection();
            // 删除用户语句
            String sql = "delete from user_tb where id = ?;";
            ps = conn.prepareStatement(sql);
            ps.setBigDecimal(1, ID);
            ps.executeUpdate();

//            printUserTable(); // 打印查询表结果
        } catch (SQLException e) {
            // JDBC错误
            e.printStackTrace();
            return false;
        } finally {
            JdbcUtilsSing.getInstance().free(null, ps, conn);
        }
        return true;
    }

    /**
     * 更新用户余额
     * <p>
     * 在数据库中更新对应用户ID的余额
     *
     * @param ID 用户ID
     * @param balance 余额
     * @return 更新成功返回true 否则返回false
     */
    private boolean updateUserBalance(BigDecimal ID, double balance) {
        Connection conn = null;
        PreparedStatement ps = null;
        try {
            // 连接数据库
            System.out.println("连接数据库...");
            conn = JdbcUtilsSing.getInstance().getConnection();
            // 删除用户语句
            String sql = "update user_tb set balance = ? where id = ?;";
            ps = conn.prepareStatement(sql);
//            ps.setString(1,"balance");
            ps.setDouble(1, balance);
            ps.setBigDecimal(2, ID);
            ps.executeUpdate();

//            printUserTable(); // 打印查询表结果
        } catch (SQLException e) {
            // JDBC错误
            e.printStackTrace();
            return false;
        } finally {
            JdbcUtilsSing.getInstance().free(null, ps, conn);
        }
        return true;
    }

    @Override
    public boolean addUser(BigDecimal ID, String username, double balance) {
        return addUser(ID, username, '男', 0, balance);
    }

    @Override
    public boolean addUser(BigDecimal ID, String username, char sex, int age, double balance) {
        this.ID = ID;
        this.name = username;
        this.sex = sex;
        this.age = age;
        this.balance = balance;
        return insertUserData(ID, username, sex, age, balance);
    }

    @Override
    public boolean deleteUser() {
        return deleteUserData(this.ID);
    }

    @Override
    public boolean setBalance(double balance) {
        this.balance = balance;
        return updateUserBalance(this.ID, this.balance);
    }

    @Override
    public BigDecimal getID() {
        return ID;
    }

    @Override
    public String getName() {
        return this.name;
    }

    @Override
    public double getBalance() {
        return this.balance;
    }

    public char getSex() {
        return sex;
    }

    public int getAge() {
        return age;
    }

//    /**
//     * 打印出查询user_tb的结果(用于测试)
//     */
//    private void printUserTable() {
//        Connection conn = null;
//        PreparedStatement ps = null;
//        ResultSet rs = null;
//        try {
//            conn = JdbcUtilsSing.getInstance().getConnection();
//            ps = conn.prepareStatement("select * from user_tb");
//            rs = ps.executeQuery();
//            // 展开结果集数据库
//            while (rs.next()) {
//                // 通过字段检索
//                BigDecimal id = rs.getBigDecimal("id");
//                String username = rs.getString("name");
//                String sex = rs.getString("sex");
//                int age = rs.getInt("age");
//                double balance = rs.getDouble("balance");
//                String signupTime = rs.getString("signup_time");
//                System.out.println("ID:" + id);
//                System.out.println("用户名:" + username);
//                System.out.println("余额:" + balance);
//                System.out.println("注册时间:" + signupTime);
//            }
//        } catch (SQLException e) {
//            e.printStackTrace();
//        } finally {
//            // 释放资源
//            JdbcUtilsSing.getInstance().free(rs, ps, conn);
//        }
//    }
}
