/*
 * @Author: Hanbing
 * @Date: 2024-09-23 01:51:15
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-12 14:51:07
 * @FilePath: /SAT/CRAFT/src/util.h
 * @Description: 相关参数与函数、通用工具
 *
 * Copyright (c) 2024 by Hanbing, All Rights Reserved.
 */
#ifndef _CRAFT_SRC_UTIL_H
#define _CRAFT_SRC_UTIL_H

#include <algorithm>
#include <array>
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
    std::bitset<ELEMENT_SIZE> sbox[ELEMENT_SIZE * ELEMENT_SIZE] = {0xc, 0xa, 0xd, 0x3, 0xe, 0xb, 0xf, 0x7, 0x8, 0x9, 0x1, 0x5, 0x0, 0x2, 0x4, 0x6};
    std::bitset<ELEMENT_SIZE> e_sbox[ELEMENT_SIZE * ELEMENT_SIZE];
    std::vector<std::vector<int>> anf_sbox[ELEMENT_SIZE];
    std::vector<std::vector<int>> anf_err_sbox[ELEMENT_SIZE];
    std::vector<std::vector<std::pair<int, bool>>> cnf_sbox;
    std::vector<std::vector<std::pair<int, bool>>> cnf_err_sbox;
    int numClauses;

public:
    const std::array<int, 16> TWEAKEY_QT = {12, 10, 15, 5, 14, 8, 9, 2, 11, 3, 7, 4, 6, 0, 1, 13};
    std::array<std::array<std::bitset<ELEMENT_SIZE>, 2>, ROUNDS> RoundConstants;  // Permutation
    const int ShiftRowsP[16] = {15, 12, 13, 14, 10, 9, 8, 11, 6, 5, 4, 7, 1, 2, 3, 0};
    const int M[16] = {1, 0, 1, 1,
                       0, 1, 0, 1,
                       0, 0, 1, 0,
                       0, 0, 0, 1};
    Util();

    Util(int sbox_mode, std::string &msg);

    void init_RoundConstants();

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
    int shiftRows(int index);

    /**
     * @description: S盒变换
     * @param {int} index
     * @param {int} have_error
     * @return {*}
     */
    bitset<SBOX_ITEM_SIZE> S(int index, int have_error);

    /**
     * @description: 获取S盒的Anf格式表示，按index获取对应位置
     * @param {int} index
     * @param {vector<vector<int>>} &anfSbox
     * @param {int} have_error
     * @param {string} &msg
     * @return {*}
     */
    int getAnf(int index, std::vector<std::vector<int>> &anfSbox, int have_error, std::string &msg);

    /**
     * @description: 获取S盒的Cnf格式表示
     * @param {vector<vector<pair<int, bool>>>} &cnfSbox
     * @param {string} &msg
     * @return {*}
     */
    int getCnf(std::vector<std::vector<std::pair<int, bool>>> &cnfSbox, int have_error, std::string &msg);

    int getNumClauses();
    void setNumClauses(int);
    void addClauses();
    int getSboxErrorIndex();
    void setSboxErrorIndex(int);
    void getSboxInErrorIndex(std::bitset<SBOX_ITEM_SIZE> &V);

    int getSboxErrorPosition();
    void setSboxErrorPosition(int);
};

#endif  // _CRAFT_SRC_UTIL_H