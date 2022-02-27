package com.dxr.BCSservice;

import java.math.BigDecimal;

/**
 * BusChargeSystem(BCS)
 * 相关操作接口定义
 *
 * @author DXR
 * @create 2021/12/6 0:15
 */
public interface BCSOperator {

    /**
     * 获取用户ID
     * @return 返回用户ID
     */
    BigDecimal getID();

    /**
     * 获取用户名
     *
     * @return 返回用户名
     */
    String getName();

    /**
     * 获取用户余额
     *
     * @return 返回用户余额
     */
    double getBalance();

    /**
     * 充值用户余额
     *
     * @param balance 需要充值的余额
     * @return 充值成功返回true 否则返回false
     */
    boolean setBalance(double balance);

    /**
     * 添加用户
     *
     * @param ID       用户的ID
     * @param username 用户名
     * @param balance  用户余额
     * @return 添加用户成功返回true 否则返回false
     */
    boolean addUser(BigDecimal ID, String username, double balance);

    /**
     * 添加用户
     * @param ID 用户的ID
     * @param username 用户名
     * @param sex 性别
     * @param age 年龄
     * @param balance 余额
     * @return 添加用户成功返回true 否则返回false
     */
    boolean addUser(BigDecimal ID, String username,char sex,int age, double balance);

    /**
     * 删除用户
     *
     * @return 删除成功返回true 否则返回false
     */
    boolean deleteUser();
}
