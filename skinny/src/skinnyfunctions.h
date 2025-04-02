/*
 * @Author: Hanbing
 * @Date: 2024-10-08 08:45:39
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-12 14:38:40
 * @FilePath: /SAT/skinny/src/skinnyfunctions.h
 * @Description:
 *
 * Copyright (c) 2024 by Hanbing, All Rights Reserved.
 */

#ifndef _SKINNY_SRC_SKINNYFUNCTIONS_H
#define _SKINNY_SRC_SKINNYFUNCTIONS_H

#include <cryptominisat5/cryptominisat.h>

#include <algorithm>
#include <array>
#include <bitset>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

#include "clausenode.h"
#include "util.h"
// 全局参数
#include "params.h"
using namespace std;

void print_state(const std::array<std::bitset<ELEMENT_SIZE>, 16> &state);

void print_ksr(const std::array<std::bitset<ELEMENT_SIZE>, 16 / 2> &state);

void convert_hexstr_to_bitsetarray(string hex_str,
                                   std::array<std::bitset<ELEMENT_SIZE>, 16> &bitset_array,
                                   bool reversed);

std::bitset<ELEMENT_SIZE> tweak_lfsr(const std::bitset<ELEMENT_SIZE> &x, int tkx);

void mix_columns(std::array<std::bitset<ELEMENT_SIZE>, 16> &state);

void tweakey_schedule(const std::array<std::bitset<ELEMENT_SIZE>, 16> tweakeys_initial[3],
                      std::array<std::bitset<ELEMENT_SIZE>, 16 / 2> ksr[ROUNDS],
                      Util &myUtil);

void skinnyEncrypt(const std::array<std::bitset<ELEMENT_SIZE>, 16> &plaint,
                   std::array<std::bitset<ELEMENT_SIZE>, 16> &cipher,
                   int have_error,
                   const std::array<std::bitset<ELEMENT_SIZE>, 16 / 2> ksr[ROUNDS],
                   Util &myUtil);

void addClauses(CMSat::SATSolver &solver,
                const vector<CMSat::Lit> &lits,
                Util &myUtil, string &msg);

void addXorClauses(CMSat::SATSolver &solver,
                   const vector<unsigned int> &vars,
                   bool rhs, Util &myUtil, string &msg);

// 1 初始tweakey约束    2 密钥编排
void genKsrConstraints(CMSat::SATSolver &solver,
                       ClauseNode nodeTweakeyInit[TWEAKEY_MODE][16][ELEMENT_SIZE],
                       ClauseNode nodeKsr[ROUNDS][16 / 2][ELEMENT_SIZE],
                       Util &myUtil, string &msg);

// 3 加密约束
void genEncryptConstraints(CMSat::SATSolver &solver, int startRound,
                           ClauseNode nodeKsr[ROUNDS][16 / 2][ELEMENT_SIZE],
                           ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                           Util &myUtil, string &msg);

// 3.1  SB  0->1
void genSBConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg);

// 3.2  AC  1->2
void genACConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg);

// 3.3  AK  2->3
void genAKConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeKsr[ROUNDS][16 / 2][ELEMENT_SIZE],
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg);

// 3.4  shiftRows  3->4
void genPLConstraints(int RC, int startRound,
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil);

// 3.5  MC  4->0
void genMCConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg);

// 4 添加错误密文的约束
void genCipherConstraints(CMSat::SATSolver &solver, int startRound,
                          std::array<std::bitset<ELEMENT_SIZE>, 16> cipher,
                          ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                          Util &myUtil, string &msg);
#endif