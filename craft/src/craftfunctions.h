/*
 * @Author: Hanbing
 * @Date: 2024-10-08 08:45:39
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-24 09:42:39
 * @FilePath: /SAT/craft/src/craftfunctions.h
 * @Description:
 *
 * Copyright (c) 2024 by Hanbing, All Rights Reserved.
 */

#ifndef _CRAFT_SRC_CRAFTFUNCTIONS_H
#define _CRAFT_SRC_CRAFTFUNCTIONS_H

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
void addClauses(CMSat::SATSolver &solver,
                const vector<CMSat::Lit> &lits,
                Util &myUtil, string &msg);
void addXorClauses(CMSat::SATSolver &solver,
                   const vector<unsigned int> &vars,
                   bool rhs, Util &myUtil, string &msg);
void mix_columns(std::array<std::bitset<ELEMENT_SIZE>, 16> &state, Util &myUtil);
void ksr_schedule(const std::array<std::array<std::bitset<ELEMENT_SIZE>, 16>, 2> &master_key,
                  const std::array<std::bitset<ELEMENT_SIZE>, 16> &tweakey,
                  std::array<std::array<std::bitset<ELEMENT_SIZE>, 16>, 4> &ksr,
                  Util &myUtil);
void craftEncrypt(const std::array<std::bitset<ELEMENT_SIZE>, 16> &plaint,
                  std::array<std::bitset<ELEMENT_SIZE>, 16> &cipher,
                  int have_error,
                  const std::array<std::array<std::bitset<ELEMENT_SIZE>, 16>, 4> &ksr,
                  Util &myUtil);

// 1 初始tweakey约束    // 2 密钥编排
void genKsrConstraints(CMSat::SATSolver &solver,
                       ClauseNode nodeMasterkey[2][16][ELEMENT_SIZE],
                       ClauseNode nodeTweakey[16][ELEMENT_SIZE],
                       ClauseNode nodeKsr[4][16][ELEMENT_SIZE],
                       Util &myUtil, string &msg);
// 3 加密约束
void genEncryptConstraints(CMSat::SATSolver &solver, int startRound,
                           ClauseNode nodeKsr[4][16][ELEMENT_SIZE],
                           ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                           Util &myUtil, string &msg);
// 3.1  MC  0->1
void genMCConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg);
// 3.2  AC  1->2
void genACConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg);
// 3.3  AK  2->3
void genAKConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeKsr[4][16][ELEMENT_SIZE],
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg);
// 3.4  PL  3->4
void genPLConstraints(int RC, int startRound,
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil);
// 3.5  SB  4->0
void genSBConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg);
// 4 添加密文的约束
void genCipherConstraints(CMSat::SATSolver &solver, int startRound,
                          std::array<std::bitset<ELEMENT_SIZE>, 16> cipher,
                          ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                          Util &myUtil, string &msg);
#endif  // _CRAFT_SRC_CRAFTFUNCTIONS_H