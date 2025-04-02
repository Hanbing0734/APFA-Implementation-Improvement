
#ifndef _PRESENT_SRC_PRESENTFUNCTIONS_H
#define _PRESENT_SRC_PRESENTFUNCTIONS_H

#include <cryptominisat5/cryptominisat.h>

#include <algorithm>
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

/**
 * @description: 编排轮密钥
 * @param {bitset<KEY_SIZE>} &master_key
 * @param {bitset<KEY_SIZE>} ksr
 * @param {Util} &myUtil
 * @return {*}
 */
void keySchedule(const bitset<KEY_SIZE> &master_key, int have_error, bitset<KEY_SIZE> ksr[ROUNDS], Util &myUtil);

/**
 * @description: 加密
 * @param {bitset<BLOCK_SIZE>} &plaint
 * @param {bitset<BLOCK_SIZE>} &cipher
 * @param {int} have_error
 * @param {bitset<KEY_SIZE>} ksr
 * @param {Util} &myUtil
 * @return {*}
 */
void presentEncrypt(const bitset<BLOCK_SIZE> &plaint, bitset<BLOCK_SIZE> &cipher, int have_error, const bitset<KEY_SIZE> ksr[ROUNDS], Util &myUtil);

/**
 * @description: 向solver求解器添加 | 语句，真(0),假(1)，要求一个每个Lit为l_true(0)
 * @param {SATSolver} &solver
 * @param {vector<CMSat::Lit>} &lits
 * @param {Util} &myUtil
 * @param {string} &msg
 * @return {*}
 */
void addClauses(CMSat::SATSolver &solver, const vector<CMSat::Lit> &lits, Util &myUtil, string &msg);

/**
 * @description: 向solver求解器添加 xor 语句，异或结果为l_true(0) : l_false(1)
 * @param {SATSolver} &solver
 * @param {vector<unsigned int>} &vars
 * @param {bool} rhs
 * @param {Util} &myUtil
 * @param {string} &msg
 * @return {*}
 */
void addXorClauses(CMSat::SATSolver &solver, const vector<unsigned int> &vars, bool rhs, Util &myUtil, string &msg);

/**
 * @description: 向solver求解器添加密钥生成的约束
 * @param {SATSolver} &solver
 * @param {ClauseNode} nodeKsr
 * @param {Util} &myUtil
 * @param {string} &msg
 * @return {*}
 */
void genKsrConstraints(CMSat::SATSolver &solver, ClauseNode nodeKsr[][KEY_SIZE], Util &myUtil, string &msg);

/**
 * @description: 过S盒的约束
 * @param {SATSolver} &solver
 * @param {int} RC
 * @param {int} startRound
 * @param {ClauseNode} nodeState
 * @param {Util} &myUtil
 * @param {string} &msg
 * @return {*}
 */
void genSBConstraints(CMSat::SATSolver &solver, int RC, int startRound, ClauseNode nodeState[][3][BLOCK_SIZE], Util &myUtil, string &msg);

/**
 * @description: PL线性变换的约束
 * @param {int} RC
 * @param {int} startRound
 * @param {ClauseNode} nodeState
 * @param {Util} &myUtil
 * @return {*}
 */
void genPLConstraints(int RC, int startRound, ClauseNode nodeState[][3][BLOCK_SIZE], Util &myUtil);

/**
 * @description: 异或轮密钥的约束
 * @param {SATSolver} &solver
 * @param {int} RC
 * @param {int} startRound
 * @param {ClauseNode} nodeKsr
 * @param {ClauseNode} nodeState
 * @param {Util} &myUtil
 * @param {string} &msg
 * @return {*}
 */
void genAKConstraints(CMSat::SATSolver &solver, int RC, int startRound, ClauseNode nodeKsr[][KEY_SIZE], ClauseNode nodeState[][3][BLOCK_SIZE], Util &myUtil, string &msg);

/**
 * @description: 向solver求解器添加加密状态的约束
 * @param {SATSolver} &solver
 * @param {int} startRound
 * @param {ClauseNode} nodeKsr
 * @param {ClauseNode} nodeState
 * @param {Util} &myUtil
 * @param {string} &msg
 * @return {*}
 */
void genEncryptConstraints(CMSat::SATSolver &solver, int startRound, ClauseNode nodeKsr[][KEY_SIZE], ClauseNode nodeState[][3][BLOCK_SIZE], Util &myUtil, string &msg);

/**
 * @description: 添加错误密文的约束
 * @param {SATSolver} &solver
 * @param {int} startRound
 * @param {bitset<BLOCK_SIZE>} errorCipher
 * @param {ClauseNode} nodeState
 * @param {Util} &myUtil
 * @param {string} &msg
 * @return {*}
 */
void genErrorCipherConstraints(CMSat::SATSolver &solver, int startRound, bitset<BLOCK_SIZE> errorCipher, ClauseNode nodeState[][3][BLOCK_SIZE], Util &myUtil, string &msg);

#endif