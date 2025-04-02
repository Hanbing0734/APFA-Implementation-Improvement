/*
 * @Author: Hanbing
 * @Date: 2024-09-29 09:17:45
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-12 17:11:29
 * @FilePath: /SAT/present/src/present80functions.cpp
 * @Description:
 *
 * Copyright (c) 2024 by Hanbing, All Rights Reserved.
 */

#ifndef PRESENT_MODE
#define PRESENT_MODE 80
#endif
#include "presentfunctions.h"

void keySchedule(const bitset<KEY_SIZE> &master_key, int have_error, bitset<KEY_SIZE> ksr[ROUNDS], Util &myUtil) {
    // string funcName(__func__);
    ksr[0] = master_key;
    for (int r = 0; r < ROUNDS - 1; r++) {
        bitset<KEY_SIZE> k;
        for (int i = 0; i < KEY_SIZE; i++)
            k[i] = ksr[r][(i + 19) % KEY_SIZE];
        // k = (k << 61) | (k >> 19);
        bitset<4> s;
        int index = (k[KEY_SIZE - 1] << 3) + (k[KEY_SIZE - 2] << 2) + (k[KEY_SIZE - 3] << 1) + k[KEY_SIZE - 4];
        s = myUtil.S(index, have_error);
        for (int i = KEY_SIZE - 1; i >= KEY_SIZE - 4; i--)
            k[i] = s[i - (KEY_SIZE - 4)];
        bitset<5> rc(r + 1);
        for (int i = 19; i >= 15; i--)
            k[i] = k[i] ^ rc[i - 15];
        ksr[r + 1] = k;
    }
}

void presentEncrypt(const bitset<BLOCK_SIZE> &plaint, bitset<BLOCK_SIZE> &cipher, int have_error, const bitset<KEY_SIZE> ksr[ROUNDS], Util &myUtil) {
    // AK(state, 0);
    bitset<BLOCK_SIZE> state = plaint;
    for (int i = 0; i < BLOCK_SIZE; i++)
        state[BLOCK_SIZE - 1 - i] = state[BLOCK_SIZE - 1 - i] ^ ksr[0][KEY_SIZE - 1 - i];
    for (int RC = 1; RC < ROUNDS; RC++) {
        // SB(state, RC);
        for (int i = 0; i < BLOCK_SIZE; i += ELEMENT_SIZE) {
            int ind = (state[i + 3] << 3) + (state[i + 2] << 2) + (state[i + 1] << 1) + state[i];
            bitset<4> s = myUtil.S(ind, have_error);
            for (int j = 0; j < ELEMENT_SIZE; j++)
                state[i + j] = s[j];
        }
        // PL(state, RC);
        bitset<BLOCK_SIZE> tmp(0);
        for (int i = 0; i < BLOCK_SIZE; i++)
            tmp[myUtil.PL(i)] = state[i];
        state = tmp;
        // AK(state, RC);
        for (int i = 0; i < BLOCK_SIZE; i++)
            state[BLOCK_SIZE - 1 - i] = state[BLOCK_SIZE - 1 - i] ^ ksr[RC][KEY_SIZE - 1 - i];
    }
    cipher = state;
    // cout << "cipher:\t\t" << hex << cipher.to_ulong() << dec << endl;
}

void addClauses(CMSat::SATSolver &solver, const vector<CMSat::Lit> &lits, Util &myUtil, string &msg) {
    if (!solver.add_clause(lits)) {
        string funcName(__func__);
        msg = funcName + ":\t solver add_clause error!";
    }
    myUtil.addClauses();
}

void addXorClauses(CMSat::SATSolver &solver, const vector<unsigned int> &vars, bool rhs, Util &myUtil, string &msg) {
    if (!solver.add_xor_clause(vars, rhs)) {
        string funcName(__func__);
        msg = funcName + ":\t solver add_clause error!";
    }
    myUtil.addClauses();
}

void genKsrConstraints(CMSat::SATSolver &solver, ClauseNode nodeKsr[][KEY_SIZE], Util &myUtil, string &msg) {
    // 1 主密钥约束
    for (int i = KEY_SIZE - 1; i >= 0; i--) {
        nodeKsr[0][i].newClauseNode(solver, NODE_NOT_CHECK_USED);
    }

    // 2 轮密钥生成过程中的约束
    for (int r = 0; r < ROUNDS - 1; r++) {
        // 2.1 循环左移61位   下一轮的第i对应上一轮的i-19
        //      bitset大端存储 r[i] == r + 1 [(i +61) % KEY_SIZE]
        //              -- > r + 1 [i]==r[(i - 61 + KEY_SIZE) % KEY_SIZE]
        for (int i = KEY_SIZE - 1; i >= 0; i--) {
            nodeKsr[r + 1][i] = nodeKsr[r][(i - 61 + KEY_SIZE) % KEY_SIZE];
        }
        // 2.2 循环左移61位之后的最高半字节79~76进行字节代换
        //       X: x3|x2|x1|x0    Y: y3|y2|y1|y0       比如 S[0]=0xC -->  S[0000]=1100
        //       存储的数据按照下标表示 比如anf_sbox[0][2]指的是正确S盒转化成anf的y2
        bitset<SBOX_ITEM_SIZE> errSboxItem;
        myUtil.getSboxInErrorIndex(errSboxItem);
        for (int i = KEY_SIZE - (1 * ELEMENT_SIZE); i >= KEY_SIZE - (1 * ELEMENT_SIZE); i -= ELEMENT_SIZE) {
            // 2.2.1
            if (SBOX_MODE == CNF_MODE) {
                int iR = i;  // 79~76
                for (int index = iR; index < iR + ELEMENT_SIZE; index++) {
                    (void)nodeKsr[r + 1][index].newClauseNode(solver, NODE_NOT_CHECK_USED);
                }
                int iL = (iR - 61 + KEY_SIZE) % KEY_SIZE;  // 18~15 // iL -> iR
                vector<vector<pair<int, bool>>> cnf_box;
                if (myUtil.getCnf(cnf_box, KSR_HAVEERROR, msg)) {
                    return;
                }
                for (auto words : cnf_box) {
                    vector<CMSat::Lit> clause;
                    for (auto item : words) {
                        int index = item.first;
                        if (index < ELEMENT_SIZE) {
                            clause.push_back(CMSat::Lit(nodeKsr[r][iL + index].newClauseNode(solver, NODE_CHECK_USED), item.second));
                        } else {
                            clause.push_back(CMSat::Lit(nodeKsr[r + 1][iR + (index - ELEMENT_SIZE)].newClauseNode(solver, NODE_CHECK_USED), item.second));
                        }
                    }
                    (void)addClauses(solver, clause, myUtil, msg);
                }
            }
            // 2.2.2 ANF
            if (SBOX_MODE == ANF_MODE) {
                ClauseNode nodeIT[16];
                bool haveIT[20] = {false};
                int iL = (i - 61 + KEY_SIZE) % KEY_SIZE;
                for (int iR = i + 3; iR >= i; iR--) {
                    vector<unsigned> xor_vars;
                    int xor_const = 0;
                    vector<vector<int>> anfOfIndex;
                    if (myUtil.getAnf(iR - i, anfOfIndex, KSR_HAVEERROR, msg)) {
                        return;
                    }
                    for (auto words : anfOfIndex) {
                        int index_IT = 0;
                        for (int item : words) {
                            if (item == 4) {
                                index_IT = 0;
                                break;
                            }
                            index_IT += pow(2, item);
                        }

                        switch (index_IT) {
                        case 0:
                            xor_const ^= 1;
                            break;
                        case 1:
                            xor_vars.push_back(nodeKsr[r][iL + 0].getIndex());
                            break;
                        case 2:
                            xor_vars.push_back(nodeKsr[r][iL + 1].getIndex());
                            break;
                        case 4:
                            xor_vars.push_back(nodeKsr[r][iL + 2].getIndex());
                            break;
                        case 8:
                            xor_vars.push_back(nodeKsr[r][iL + 3].getIndex());
                            break;
                        default:
                            xor_vars.push_back(nodeIT[index_IT].newClauseNode(solver, NODE_CHECK_USED));

                            if (haveIT[index_IT] != true) {
                                for (auto item : words) {
                                    vector<CMSat::Lit> lits_1;
                                    lits_1.push_back(CMSat::Lit(nodeKsr[r][iL + item].newClauseNode(solver, NODE_CHECK_USED), false));
                                    lits_1.push_back(CMSat::Lit(nodeIT[index_IT].getIndex(), true));
                                    (void)addClauses(solver, lits_1, myUtil, msg);
                                }
                                vector<CMSat::Lit> lits_2;
                                for (auto item : words) {
                                    lits_2.push_back(CMSat::Lit(nodeKsr[r][iL + item].newClauseNode(solver, NODE_CHECK_USED), true));
                                }
                                lits_2.push_back(CMSat::Lit(nodeIT[index_IT].getIndex(), false));
                                (void)addClauses(solver, lits_2, myUtil, msg);

                                haveIT[index_IT] = true;
                            }
                            break;
                        }
                    }
                    xor_vars.push_back(nodeKsr[r + 1][iR].newClauseNode(solver, NODE_NOT_CHECK_USED));
                    (void)addXorClauses(solver, xor_vars, xor_const, myUtil, msg);
                }
            }

            // 2.2.3 若密钥使用错误S盒，则过错误S盒的数据不存在被改写的原S盒数据 的约束
            if (KSR_HAVEERROR == HAVE_ERROR) {
                vector<CMSat::Lit> lits;
                ClauseNode nodeD[ELEMENT_SIZE];
                for (int j = ELEMENT_SIZE - 1; j >= 0; j--) {
                    vector<unsigned> xor_vars;
                    xor_vars.push_back(nodeD[j].newClauseNode(solver, NODE_CHECK_USED));
                    xor_vars.push_back(nodeKsr[r + 1][i + j].getIndex());
                    (void)addXorClauses(solver, xor_vars, errSboxItem[j], myUtil, msg);

                    lits.push_back(CMSat::Lit(nodeD[j].getIndex(), false));
                }
                (void)addClauses(solver, lits, myUtil, msg);
            }
        }
        // 2.3 19~15 5位与轮数计数器异或
        bitset<5> rc(r + 1);
        for (int i = 19; i >= 15; i--) {
            // k[i] = k[i] ^ rc[i - 15];
            if (!rc[i - 15])
                continue;
            vector<unsigned> xor_vars;
            xor_vars.push_back(nodeKsr[r][(i - 61 + KEY_SIZE) % KEY_SIZE].getIndex());
            // nodeKsr[r + 1][i].newClauseNode(solver, NODE_NOT_CHECK_USED);
            xor_vars.push_back(nodeKsr[r + 1][i].newClauseNode(solver, NODE_NOT_CHECK_USED));
            (void)addXorClauses(solver, xor_vars, true, myUtil, msg);
        }
    }
}

void genErrorCipherConstraints(CMSat::SATSolver &solver, int startRound, bitset<BLOCK_SIZE> errorCipher, ClauseNode nodeState[][3][BLOCK_SIZE], Util &myUtil, string &msg) {
    // 4 添加错误密文的约束
    for (int i = BLOCK_SIZE - 1; i >= 0; i--) {
        vector<CMSat::Lit> lits;
        lits.push_back(CMSat::Lit(nodeState[ROUNDS - startRound][0][i].newClauseNode(solver, NODE_CHECK_USED), !errorCipher[i]));
        (void)addClauses(solver, lits, myUtil, msg);
    }
}

void genEncryptConstraints(CMSat::SATSolver &solver, int startRound, ClauseNode nodeKsr[][KEY_SIZE], ClauseNode nodeState[][3][BLOCK_SIZE], Util &myUtil, string &msg) {
    // 3 加密约束
    for (int RC = startRound; RC < ROUNDS; RC++) {
        if (RC != startRound) {
            // 3.1 0->1
            genSBConstraints(solver, RC, startRound, nodeState, myUtil, msg);
            // 3.2 1->2
            genPLConstraints(RC, startRound, nodeState, myUtil);
        }
        // 3.3 2->0
        genAKConstraints(solver, RC, startRound, nodeKsr, nodeState, myUtil, msg);
    }
}

void genSBConstraints(CMSat::SATSolver &solver, int RC, int startRound, ClauseNode nodeState[][3][BLOCK_SIZE], Util &myUtil, string &msg) {
    bitset<4> errSboxItem;
    myUtil.getSboxInErrorIndex(errSboxItem);
    // 3.1
    // 0 -> 1
    for (int i = BLOCK_SIZE - ELEMENT_SIZE; i >= 0; i -= ELEMENT_SIZE) {
        // CNF
        if (SBOX_MODE == CNF_MODE) {
            vector<vector<pair<int, bool>>> cnf_box;
            if (myUtil.getCnf(cnf_box, ENCYPT_HAVEERROR, msg)) {
                return;
            }
            for (auto words : cnf_box) {
                vector<CMSat::Lit> clause;
                for (auto item : words) {
                    int index = item.first;
                    if (index < 4) {
                        clause.push_back(CMSat::Lit(nodeState[RC - startRound][0][i + index].newClauseNode(solver, NODE_CHECK_USED), item.second));
                    } else {
                        clause.push_back(CMSat::Lit(nodeState[RC - startRound][1][i + (index - 4)].newClauseNode(solver, NODE_CHECK_USED), item.second));
                    }
                }
                (void)addClauses(solver, clause, myUtil, msg);
            }
        }
        // ANF
        if (SBOX_MODE == ANF_MODE) {
            //      X: x3|x2|x1|x0    Y: y3|y2|y1|y0       比如 S[0]=0xC -->  S[0000]=1100
            //      存储的数据按照下标表示 比如anf_sbox[1][2]指的是错误S盒转化成anf的y2

            ClauseNode nodeIT[16];
            bool haveIT[20] = {false};
            for (int ir = ELEMENT_SIZE - 1; ir >= 0; ir--) {
                vector<vector<int>> anfOfIndex;
                if (myUtil.getAnf(ir, anfOfIndex, ENCYPT_HAVEERROR, msg)) {
                    return;
                }
                vector<unsigned> xor_vars;
                int xor_const = 0;
                for (auto words : anfOfIndex) {
                    int index_IT = 0;
                    for (auto item : words) {
                        if (item == 4) {
                            index_IT = 0;
                            break;
                        }
                        index_IT += pow(2, item);
                    }
                    switch (index_IT) {
                    case 0:
                        xor_const ^= 1;
                        break;
                    case 1:
                        xor_vars.push_back(nodeState[RC - startRound][0][i + 0].getIndex());
                        break;
                    case 2:
                        xor_vars.push_back(nodeState[RC - startRound][0][i + 1].getIndex());
                        break;
                    case 4:
                        xor_vars.push_back(nodeState[RC - startRound][0][i + 2].getIndex());
                        break;
                    case 8:
                        xor_vars.push_back(nodeState[RC - startRound][0][i + 3].getIndex());
                        break;
                    default:
                        xor_vars.push_back(nodeIT[index_IT].newClauseNode(solver, NODE_CHECK_USED));

                        if (haveIT[index_IT] != true) {
                            for (auto item : words) {
                                vector<CMSat::Lit> lits_1;
                                lits_1.push_back(CMSat::Lit(nodeState[RC - startRound][0][i + item].newClauseNode(solver, NODE_CHECK_USED), false));
                                lits_1.push_back(CMSat::Lit(nodeIT[index_IT].getIndex(), true));
                                (void)addClauses(solver, lits_1, myUtil, msg);
                            }
                            vector<CMSat::Lit> lits_2;
                            for (auto item : words) {
                                lits_2.push_back(CMSat::Lit(nodeState[RC - startRound][0][i + item].newClauseNode(solver, NODE_CHECK_USED), true));
                            }
                            lits_2.push_back(CMSat::Lit(nodeIT[index_IT].getIndex(), false));
                            (void)addClauses(solver, lits_2, myUtil, msg);

                            haveIT[index_IT] = true;
                        }
                        break;
                    }
                }
                xor_vars.push_back(nodeState[RC - startRound][1][i + ir].newClauseNode(solver, NODE_CHECK_USED));
                (void)addXorClauses(solver, xor_vars, xor_const, myUtil, msg);
            }
        }
        // 若使用错误S盒，则过错误S盒的数据不存在被改写的原S盒数据 的约束
        if (ENCYPT_HAVEERROR == HAVE_ERROR) {
            vector<CMSat::Lit> lits;
            ClauseNode nodeD[ELEMENT_SIZE];
            for (int j = ELEMENT_SIZE - 1; j >= 0; j--) {
                vector<unsigned> xor_vars;
                xor_vars.push_back(nodeD[j].newClauseNode(solver, NODE_CHECK_USED));
                xor_vars.push_back(nodeState[RC - startRound][1][i + j].getIndex());
                (void)addXorClauses(solver, xor_vars, errSboxItem[j], myUtil, msg);

                lits.push_back(CMSat::Lit(nodeD[j].getIndex(), false));
            }
            (void)addClauses(solver, lits, myUtil, msg);
        }
    }
}

void genPLConstraints(int RC, int startRound, ClauseNode nodeState[][3][BLOCK_SIZE], Util &myUtil) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        nodeState[RC - startRound][2][myUtil.PL(i)] = nodeState[RC - startRound][1][i];
    }
}

void genAKConstraints(CMSat::SATSolver &solver, int RC, int startRound, ClauseNode nodeKsr[][KEY_SIZE], ClauseNode nodeState[][3][BLOCK_SIZE], Util &myUtil, string &msg) {
    for (int i = BLOCK_SIZE - 1; i >= 0; i--) {
        vector<unsigned> xor_vars;
        xor_vars.push_back(nodeState[RC - startRound][2][i].newClauseNode(solver, NODE_CHECK_USED));
        xor_vars.push_back(nodeState[RC + 1 - startRound][0][i].newClauseNode(solver, NODE_CHECK_USED));
        xor_vars.push_back(nodeKsr[RC][i + 16].getIndex());
        (void)addXorClauses(solver, xor_vars, false, myUtil, msg);
    }
}
