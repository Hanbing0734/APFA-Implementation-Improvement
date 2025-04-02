/*
 * @Author: Hanbing
 * @Date: 2024-09-23 01:51:15
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-12 09:03:51
 * @FilePath: /SAT/present/src/util.h
 * @Description: 相关参数与函数、通用工具
 *
 * Copyright (c) 2024 by Hanbing, All Rights Reserved.
 */
#ifndef _PRESENT_SRC_UTIL_H
#define _PRESENT_SRC_UTIL_H

#include <algorithm>
#include <bitset>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "params.h"
using namespace std;

class Util {
private:
    int sbox_error_index;
    int sbox_error_position;
    const bitset<SBOX_ITEM_SIZE> sbox[16] = {0xC, 0x5, 0x6, 0xB, 0x9, 0x0, 0xA, 0xD,
                                             0x3, 0xE, 0xF, 0x8, 0x4, 0x7, 0x1, 0x2};
    bitset<SBOX_ITEM_SIZE> e_sbox[16];
    vector<vector<int>> anf_sbox[ELEMENT_SIZE];
    vector<vector<int>> anf_err_sbox[ELEMENT_SIZE];
    vector<vector<pair<int, bool>>> cnf_sbox;
    vector<vector<pair<int, bool>>> cnf_err_sbox;
    int P[BLOCK_SIZE];  // PL置换表
    int numClauses;

public:
    Util();

    Util(int sbox_mode, string &msg);

    /**
     * @description: 注入错误，S盒的一位bit上进行翻转
     * @return {*}
     */
    void injectError();

    /**
     * @description: 读入S盒的Anf格式数据，数据采用matlab转化
     * @param {int} haveError
     * @param {string} msg
     * @return {*}
     */
    int genAnfFromSbox(int haveError, string &msg);

    /**
     * @description: 读入S盒的Cnf格式数据，数据采用Logic Friday产生
     * @param {int} haveError
     * @param {string} msg
     * @return {*}
     */
    int genCnfFromSbox(int haveError, string &msg);

    /**
     * @description: PL置换表
     * @param {string} msg
     * @return {*}
     */
    int init_P(string &masg);

    /**
     * @description: 验证输出S盒的Anf
     * @param {int} haveError
     * @return {*}
     */
    void printAnf(int haveError);

    /**
     * @description: 验证输出S盒的Cnf
     * @param {int} haveError
     * @return {*}
     */
    void printCnf(int haveError);

    /**
     * @description: PL变换
     * @param {int} index
     * @return {*}
     */
    int PL(int index);

    /**
     * @description: S盒变换
     * @param {int} index
     * @param {int} have_error
     * @return {*}
     */
    bitset<4> S(int index, int have_error);

    /**
     * @description: 获取S盒的Anf格式表示，按index获取对应位置
     * @param {int} index
     * @param {vector<vector<int>>} &anfSbox
     * @param {int} have_error
     * @param {string} &msg
     * @return {*}
     */
    int getAnf(int index, vector<vector<int>> &anfSbox, int have_error, string &msg);

    /**
     * @description: 获取S盒的Cnf格式表示
     * @param {vector<vector<pair<int, bool>>>} &cnfSbox
     * @param {string} &msg
     * @return {*}
     */
    int getCnf(vector<vector<pair<int, bool>>> &cnfSbox, int have_error, string &msg);

    int getNumClauses();
    void setNumClauses(int);
    void addClauses();
    int getSboxErrorIndex();
    void setSboxErrorIndex(int);
    void getSboxInErrorIndex(bitset<4> &V);

    int getSboxErrorPosition();
    void setSboxErrorPosition(int);
};

#endif  // _PRESENT_SRC_UTIL_H