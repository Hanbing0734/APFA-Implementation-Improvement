/*
 * @Author: Hanbing
 * @Date: 2024-09-29 09:17:45
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-12 17:11:06
 * @FilePath: /SAT/skinny/src/skinnyfunctions.cpp
 * @Description:
 *
 * Copyright (c) 2024 by Hanbing, All Rights Reserved.
 */

#ifndef SKINNY_MODE
#define SKINNY_MODE 64
#endif
#ifndef TWEAKEY_MODE
#define TWEAKEY_MODE 2
#endif

#include "skinnyfunctions.h"

void print_state(const std::array<std::bitset<ELEMENT_SIZE>, 16> &state) {
    cout << hex;
    for (int i = 0; i < 16; i++)
        cout << state[i].to_ulong();
    cout << dec << endl;
}

void print_ksr(const std::array<std::bitset<ELEMENT_SIZE>, 16 / 2> &state) {
    cout << hex;
    for (int i = 0; i < 16 / 2; i++)
        cout << state[i].to_ulong();
    cout << dec << endl;
}

void convert_hexstr_to_bitsetarray(string hex_str,
                                   std::array<std::bitset<ELEMENT_SIZE>, 16> &bitset_array,
                                   bool reversed = false) {
    if (reversed == true) {
        for (int i = 0; i < 16; i++) {
            int ans = stoi(hex_str.substr(i, 1), nullptr, 16);
            cout << ans;
            bitset_array[15 - i] = std::bitset<ELEMENT_SIZE>(stoi(hex_str.substr(i, 1), nullptr, 16));
        }

    } else {
        for (int i = 0; i < 16; i++) {
            int ans = stoi(hex_str.substr(i, 1), nullptr, 16);
            cout << ans;
            bitset_array[i] = std::bitset<ELEMENT_SIZE>(stoi(hex_str.substr(i, 1), nullptr, 16));
        }
    }
    cout << endl;
}

std::bitset<ELEMENT_SIZE> tweak_lfsr(const std::bitset<ELEMENT_SIZE> &x, int tkx) {
    std::bitset<ELEMENT_SIZE> result;
    if (tkx == 1) {
        result = x;
    } else if (tkx == 2) {
#if SKINNY_MODE == 64
        // x3 x2 x1 x0 -> x2 x1 x0 x3x2
        std::bitset<ELEMENT_SIZE> shifted_x = (x << 1);                               // x 左移 1 位
        std::bitset<ELEMENT_SIZE> bit_3 = (x >> 3) & std::bitset<ELEMENT_SIZE>(0x1);  // 取 x 的x3（从右开始计数，0 索引）
        std::bitset<ELEMENT_SIZE> bit_2 = (x >> 2) & std::bitset<ELEMENT_SIZE>(0x1);  // 取 x 的x0
        result = shifted_x ^ bit_3 ^ bit_2;
#elif SKINNY_MODE == 128
        // x7 x6 x5 x4 x3 x2 x1 x0 -> x6 x5 x4 x3 x2 x1 x0 x7x5
        std::bitset<ELEMENT_SIZE> shifted_x = (x << 1);                               // x 左移 1 位
        std::bitset<ELEMENT_SIZE> bit_7 = (x >> 7) & std::bitset<ELEMENT_SIZE>(0x1);  // 取 x 的x6（从右开始计数，0 索引）
        std::bitset<ELEMENT_SIZE> bit_5 = (x >> 5) & std::bitset<ELEMENT_SIZE>(0x1);  // 取 x 的x4
        result = shifted_x ^ bit_7 ^ bit_5;
#endif
    } else {
#if SKINNY_MODE == 64
        // x3 x2 x1 x0 -> x0x3 x3 x2 x1
        std::bitset<ELEMENT_SIZE> shifted_x = (x >> 1);                               // x 右移 1 位
        std::bitset<ELEMENT_SIZE> bit_3 = (x >> 3) & std::bitset<ELEMENT_SIZE>(0x1);  // 取 x 的 x3 （从右开始计数，0 索引）
        std::bitset<ELEMENT_SIZE> bit_0 = (x >> 0) & std::bitset<ELEMENT_SIZE>(0x1);  // 取 x 的 x0
        bit_3 = bit_3 << (ELEMENT_SIZE - 1);
        bit_0 = bit_0 << (ELEMENT_SIZE - 1);
        result = shifted_x ^ bit_3 ^ bit_0;
#elif SKINNY_MODE == 128
        // x7 x6 x5 x4 x3 x2 x1 x0 -> x0x6 x7 x6 x5 x4 x3 x2 x1
        std::bitset<ELEMENT_SIZE> shifted_x = (x >> 1);                               // x 右移 1 位
        std::bitset<ELEMENT_SIZE> bit_6 = (x >> 6) & std::bitset<ELEMENT_SIZE>(0x1);  // 取 x 的 x6 （从右开始计数，0 索引）
        std::bitset<ELEMENT_SIZE> bit_0 = (x >> 0) & std::bitset<ELEMENT_SIZE>(0x1);  // 取 x 的 x0
        bit_6 = bit_6 << (ELEMENT_SIZE - 1);
        bit_0 = bit_0 << (ELEMENT_SIZE - 1);
        result = shifted_x ^ bit_6 ^ bit_0;
#endif
    }
    return result;
}

void mix_columns(std::array<std::bitset<ELEMENT_SIZE>, 16> &state) {
    for (size_t j = 0; j < 4; j++) {
        std::bitset<ELEMENT_SIZE> tmp;
        state[j + 4 * 1] ^= state[j + 4 * 2];
        state[j + 4 * 2] ^= state[j + 4 * 0];
        state[j + 4 * 3] ^= state[j + 4 * 2];
        tmp = state[j + 4 * 3];
        state[j + 4 * 3] = state[j + 4 * 2];
        state[j + 4 * 2] = state[j + 4 * 1];
        state[j + 4 * 1] = state[j + 4 * 0];
        state[j + 4 * 0] = tmp;
    }
}

void tweakey_schedule(const std::array<std::bitset<ELEMENT_SIZE>, 16> tweakeys_initial[3],
                      std::array<std::bitset<ELEMENT_SIZE>, 16 / 2> ksr[ROUNDS],
                      Util &myUtil) {
    // string funcName(__func__);
    std::array<std::bitset<ELEMENT_SIZE>, 16> tkx[3];
    for (int j = 0; j < TWEAKEY_MODE; j++) {
        tkx[j] = tweakeys_initial[j];
    }
    // 初始化
    for (size_t i = 0; i < 16 / 2; i++) {
        for (int j = 0; j < TWEAKEY_MODE; j++) {
            ksr[0][i] ^= tkx[j][i];
        }
    }
    for (int r = 0; r < ROUNDS - 1; r++) {
        // 密钥编排
        std::array<std::bitset<ELEMENT_SIZE>, 16> tkx_permuted[3];
        // PT
        for (size_t i = 0; i < 16; i++) {
            int permuted_index = myUtil.TWEAKEY_QT[i];
            for (int j = 0; j < TWEAKEY_MODE; j++) {
                tkx_permuted[j][i] ^= tkx[j][permuted_index];
            }
        }
        // 前两行进行LFSR   TK1 或 TK1^TK2 或 TK1^TK2^TK3
        for (size_t i = 0; i < 16; i++) {
            for (int j = 0; j < TWEAKEY_MODE; j++) {
                if (i < 8) {  // 前两行
                    tkx[j][i] = tweak_lfsr(tkx_permuted[j][i], j + 1);
                } else {
                    tkx[j][i] = tkx_permuted[j][i];
                }
            }
        }

        // 更新ksr
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < TWEAKEY_MODE; j++) {
                ksr[r + 1][i] ^= tkx[j][i];
            }
        }
    }
}

void skinnyEncrypt(const std::array<std::bitset<ELEMENT_SIZE>, 16> &plaint,
                   std::array<std::bitset<ELEMENT_SIZE>, 16> &cipher,
                   int have_error,
                   const std::array<std::bitset<ELEMENT_SIZE>, 16 / 2> ksr[ROUNDS],
                   Util &myUtil) {
    std::array<std::bitset<ELEMENT_SIZE>, 16> state = plaint;
    for (int RC = 0; RC < ROUNDS; RC++) {
        // 3.1  SB  0->1
        for (int j = 0; j < 16; j++) {
            unsigned ind = state[j].to_ulong();
            state[j] = myUtil.S(ind, have_error);
        }
        // 3.2  AC  1->2
        myUtil.bitset_xor(state[0], 0, myUtil.RoundConstants[RC], 0, 4);
        myUtil.bitset_xor(state[4], 0, myUtil.RoundConstants[RC], 4, 2);
        state[8] ^= std::bitset<ELEMENT_SIZE>(0x2);
        // 3.3  AK  2->3
        for (int j = 0; j < 16 / 2; j++)
            state[j] ^= ksr[RC][j];
        // 3.4  shiftRows  3->4
        std::array<std::bitset<ELEMENT_SIZE>, 16> tmp;
        for (int j = 0; j < 16; j++) {
            int ind = myUtil.shiftRows(j);
            tmp[j] = state[ind];
        }
        state = tmp;
        // 3.5  MC  4->0
        (void)mix_columns(state);
    }
    cipher = state;
    // cout << "cipher:\t\t" << hex << cipher.to_ulong() << dec << endl;
}

void addClauses(CMSat::SATSolver &solver,
                const vector<CMSat::Lit> &lits,
                Util &myUtil, string &msg) {
    if (!solver.add_clause(lits)) {
        string funcName(__func__);
        msg = funcName + ":\t solver add_clause error!";
    }
    myUtil.addClauses();
}

void addXorClauses(CMSat::SATSolver &solver,
                   const vector<unsigned int> &vars,
                   bool rhs, Util &myUtil, string &msg) {
    if (!solver.add_xor_clause(vars, rhs)) {
        string funcName(__func__);
        msg = funcName + ":\t solver add_clause error!";
    }
    myUtil.addClauses();
}

// 1 初始tweakey约束    // 2 密钥编排
void genKsrConstraints(CMSat::SATSolver &solver,
                       ClauseNode nodeTweakeyInit[TWEAKEY_MODE][16][ELEMENT_SIZE],
                       ClauseNode nodeKsr[ROUNDS][16 / 2][ELEMENT_SIZE],
                       Util &myUtil, string &msg) {
    ClauseNode nodeTweakey[TWEAKEY_MODE][16][ELEMENT_SIZE];
    // 1.1 原始Tweakey约束
    for (int i = 0; i < TWEAKEY_MODE; i++) {
        for (int j = 0; j < 16; j++) {
            for (int item = 0; item < ELEMENT_SIZE; item++) {
                nodeTweakeyInit[i][j][item].newClauseNode(solver, NODE_NOT_CHECK_USED);
                nodeTweakey[i][j][item] = nodeTweakeyInit[i][j][item];
            }
        }
    }
    // 1.2 原始ksr约束
    for (int j = 0; j < 16 / 2; j++) {
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            vector<unsigned> xor_vars;
            xor_vars.push_back(nodeKsr[0][j][item].newClauseNode(solver, NODE_NOT_CHECK_USED));
            for (int i = 0; i < TWEAKEY_MODE; i++) {
                xor_vars.push_back(nodeTweakey[i][j][item].getIndex());
            }
            (void)addXorClauses(solver, xor_vars, 0, myUtil, msg);
        }
    }

    // 2 密钥编排
    for (int r = 0; r < ROUNDS - 1; r++) {
        // 2.1 PT
        ClauseNode nodePermutedTweakey[TWEAKEY_MODE][16][ELEMENT_SIZE];
        for (int i = 0; i < TWEAKEY_MODE; i++) {
            for (int j = 0; j < 16; j++) {
                int permuted_index = myUtil.TWEAKEY_QT[j];
                for (int item = 0; item < ELEMENT_SIZE; item++) {
                    nodePermutedTweakey[i][j][item] = nodeTweakey[i][permuted_index][item];
                }
            }
        }

        // 2.2 前两行进行LFSR   TK1 或 TK1^TK2 或 TK1^TK2^TK3
        for (int j = 0; j < 16; j++) {
            for (int i = 0; i < TWEAKEY_MODE; i++) {
                if (i >= 1 && j < 16 / 2) {  // tk2和tk3 的前两行
#if TWEAKEY_MODE >= 2
                    if (i == 1) {  // tk2
#if SKINNY_MODE == 64
                        // x3 x2 x1 x0 -> x2 x1 x0 x3x2
                        for (int item = 1; item < ELEMENT_SIZE; item++) {
                            nodeTweakey[i][j][item] = nodePermutedTweakey[i][j][item - 1];
                        }
                        nodeTweakey[i][j][0].newClauseNode(solver, NODE_NOT_CHECK_USED);
                        vector<unsigned> xor_vars;
                        xor_vars.push_back(nodeTweakey[i][j][0].getIndex());
                        xor_vars.push_back(nodePermutedTweakey[i][j][3].getIndex());
                        xor_vars.push_back(nodePermutedTweakey[i][j][2].getIndex());
                        (void)addXorClauses(solver, xor_vars, 0, myUtil, msg);
#elif SKINNY_MODE == 128
                        // x7 x6 x5 x4 x3 x2 x1 x0 -> x6 x5 x4 x3 x2 x1 x0 x7x5
                        for (int item = 1; item < ELEMENT_SIZE; item++) {
                            nodeTweakey[i][j][item] = nodePermutedTweakey[i][j][item - 1];
                        }
                        nodeTweakey[i][j][0].newClauseNode(solver, NODE_NOT_CHECK_USED);
                        vector<unsigned> xor_vars;
                        xor_vars.push_back(nodeTweakey[i][j][0].getIndex());
                        xor_vars.push_back(nodePermutedTweakey[i][j][7].getIndex());
                        xor_vars.push_back(nodePermutedTweakey[i][j][5].getIndex());
                        (void)addXorClauses(solver, xor_vars, 0, myUtil, msg);
#endif
                    }
#endif
#if TWEAKEY_MODE >= 3
                    if (i == 2) {  // tk3
#if SKINNY_MODE == 64
                        // x3 x2 x1 x0 -> x0x3 x3 x2 x1
                        for (int item = 1; item < ELEMENT_SIZE; item++) {
                            nodeTweakey[i][j][item - 1] = nodePermutedTweakey[i][j][item];
                        }
                        nodeTweakey[i][j][ELEMENT_SIZE - 1].newClauseNode(solver, NODE_NOT_CHECK_USED);
                        vector<unsigned> xor_vars;
                        xor_vars.push_back(nodeTweakey[i][j][ELEMENT_SIZE - 1].getIndex());
                        xor_vars.push_back(nodePermutedTweakey[i][j][3].getIndex());
                        xor_vars.push_back(nodePermutedTweakey[i][j][0].getIndex());
                        (void)addXorClauses(solver, xor_vars, 0, myUtil, msg);
#elif SKINNY_MODE == 128
                        // x7 x6 x5 x4 x3 x2 x1 x0 -> x0x6 x7 x6 x5 x4 x3 x2 x1
                        for (int item = 1; item < ELEMENT_SIZE; item++) {
                            nodeTweakey[i][j][item - 1] = nodePermutedTweakey[i][j][item];
                        }
                        nodeTweakey[i][j][ELEMENT_SIZE - 1].newClauseNode(solver, NODE_NOT_CHECK_USED);
                        vector<unsigned> xor_vars;
                        xor_vars.push_back(nodeTweakey[i][j][ELEMENT_SIZE - 1].getIndex());
                        xor_vars.push_back(nodePermutedTweakey[i][j][6].getIndex());
                        xor_vars.push_back(nodePermutedTweakey[i][j][0].getIndex());
                        (void)addXorClauses(solver, xor_vars, 0, myUtil, msg);
#endif
                    }
#endif
                } else {  // 其他
                    for (int item = 0; item < ELEMENT_SIZE; item++) {
                        nodeTweakey[i][j][item] = nodePermutedTweakey[i][j][item];
                    }
                }
            }
        }
        // 2.3 更新nodeKsr
        for (int j = 0; j < 16 / 2; j++) {
            for (int item = 0; item < ELEMENT_SIZE; item++) {
                vector<unsigned> xor_vars;
                xor_vars.push_back(nodeKsr[r + 1][j][item].newClauseNode(solver, NODE_NOT_CHECK_USED));
                for (int i = 0; i < TWEAKEY_MODE; i++) {
                    xor_vars.push_back(nodeTweakey[i][j][item].getIndex());
                }
                (void)addXorClauses(solver, xor_vars, 0, myUtil, msg);
            }
        }
    }
}

// 3 加密约束
void genEncryptConstraints(CMSat::SATSolver &solver, int startRound,
                           ClauseNode nodeKsr[ROUNDS][16 / 2][ELEMENT_SIZE],
                           ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                           Util &myUtil, string &msg) {
    for (int j = 0; j < 16; j++) {
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            nodeState[0][0][j][item].newClauseNode(solver, NODE_NOT_CHECK_USED);
        }
    }
    for (int RC = startRound; RC < ROUNDS; RC++) {
        // 3.1  SB  0->1
        genSBConstraints(solver, RC, startRound, nodeState, myUtil, msg);
        // 3.2  AC  1->2
        genACConstraints(solver, RC, startRound, nodeState, myUtil, msg);
        // 3.3  AK  2->3
        genAKConstraints(solver, RC, startRound, nodeKsr, nodeState, myUtil, msg);
        // 3.4  PL  3->4
        genPLConstraints(RC, startRound, nodeState, myUtil);
        // 3.5  MC  4->0
        genMCConstraints(solver, RC, startRound, nodeState, myUtil, msg);
    }
}

// 3.1  SB  0->1
void genSBConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg) {
    int r = RC - startRound;
    for (int j = 0; j < 16; j++) {
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            nodeState[r][1][j][item].newClauseNode(solver, NODE_NOT_CHECK_USED);
        }
    }
    bitset<SBOX_ITEM_SIZE> errSboxItem;
    myUtil.getSboxInErrorIndex(errSboxItem);
    for (int j = 0; j < 16; j++) {
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
                    if (index < ELEMENT_SIZE) {
                        clause.push_back(CMSat::Lit(nodeState[r][0][j][index].newClauseNode(solver, NODE_CHECK_USED), item.second));
                    } else {
                        clause.push_back(CMSat::Lit(nodeState[r][1][j][index - ELEMENT_SIZE].newClauseNode(solver, NODE_CHECK_USED), item.second));
                    }
                }
                (void)addClauses(solver, clause, myUtil, msg);
            }
        }
        // ANF
        if (SBOX_MODE == ANF_MODE) {
            //      X: x3|x2|x1|x0    Y: y3|y2|y1|y0       比如 S[0]=0xC -->  S[0000]=1100
            //      存储的数据按照下标表示 比如anf_sbox[1][2]指的是错误S盒转化成anf的y2
            const int index_size = pow(2, ELEMENT_SIZE);
            ClauseNode nodeIT[index_size];
            bool haveIT[index_size + 5] = {false};
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
                        if (item == ELEMENT_SIZE) {
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
                        xor_vars.push_back(nodeState[r][0][j][0].getIndex());
                        break;
                    case 2:
                        xor_vars.push_back(nodeState[r][0][j][1].getIndex());
                        break;
                    case 4:
                        xor_vars.push_back(nodeState[r][0][j][2].getIndex());
                        break;
                    case 8:
                        xor_vars.push_back(nodeState[r][0][j][3].getIndex());
                        break;
                    default:
                        xor_vars.push_back(nodeIT[index_IT].newClauseNode(solver, NODE_CHECK_USED));

                        if (haveIT[index_IT] != true) {
                            for (auto item : words) {
                                vector<CMSat::Lit> lits_1;
                                lits_1.push_back(CMSat::Lit(nodeState[r][0][j][item].newClauseNode(solver, NODE_CHECK_USED), false));
                                lits_1.push_back(CMSat::Lit(nodeIT[index_IT].getIndex(), true));
                                (void)addClauses(solver, lits_1, myUtil, msg);
                            }
                            vector<CMSat::Lit> lits_2;
                            for (auto item : words) {
                                lits_2.push_back(CMSat::Lit(nodeState[r][0][j][item].newClauseNode(solver, NODE_CHECK_USED), true));
                            }
                            lits_2.push_back(CMSat::Lit(nodeIT[index_IT].getIndex(), false));
                            (void)addClauses(solver, lits_2, myUtil, msg);

                            haveIT[index_IT] = true;
                        }
                        break;
                    }
                }
                xor_vars.push_back(nodeState[r][1][j][ir].newClauseNode(solver, NODE_CHECK_USED));
                (void)addXorClauses(solver, xor_vars, xor_const, myUtil, msg);
            }
        }
        // 若使用错误S盒，则过错误S盒的数据不存在被改写的原S盒数据 的约束
        if (ENCYPT_HAVEERROR == HAVE_ERROR) {
            vector<CMSat::Lit> lits;
            ClauseNode nodeD[ELEMENT_SIZE];
            for (int item = ELEMENT_SIZE - 1; item >= 0; item--) {
                vector<unsigned> xor_vars;
                xor_vars.push_back(nodeD[item].newClauseNode(solver, NODE_CHECK_USED));
                xor_vars.push_back(nodeState[r][1][j][item].getIndex());
                (void)addXorClauses(solver, xor_vars, errSboxItem[item], myUtil, msg);

                lits.push_back(CMSat::Lit(nodeD[item].getIndex(), false));
            }
            (void)addClauses(solver, lits, myUtil, msg);
        }
    }
}

// 3.2  AC  1->2
void genACConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg) {
    int r = RC - startRound;
    for (int j = 0; j < 16; j++) {
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            if (j == 0 && item < 4) {
                if (myUtil.RoundConstants[RC][item] == 1) {
                    vector<unsigned int> xor_vars;
                    nodeState[r][2][j][item].newClauseNode(solver, NODE_NOT_CHECK_USED);
                    xor_vars.push_back(nodeState[r][2][j][item].getIndex());
                    xor_vars.push_back(nodeState[r][1][j][item].getIndex());
                    (void)addXorClauses(solver, xor_vars, true, myUtil, msg);
                } else
                    nodeState[r][2][j][item] = nodeState[r][1][j][item];
            } else if (j == 4 && item < 2) {
                if (myUtil.RoundConstants[RC][item + 4] == 1) {
                    vector<unsigned int> xor_vars;
                    nodeState[r][2][j][item].newClauseNode(solver, NODE_NOT_CHECK_USED);
                    xor_vars.push_back(nodeState[r][2][j][item].getIndex());
                    xor_vars.push_back(nodeState[r][1][j][item].getIndex());
                    (void)addXorClauses(solver, xor_vars, true, myUtil, msg);
                } else
                    nodeState[r][2][j][item] = nodeState[r][1][j][item];
            } else if (j == 8 && item == 1) {
                vector<unsigned int> xor_vars;
                nodeState[r][2][j][item].newClauseNode(solver, NODE_NOT_CHECK_USED);
                xor_vars.push_back(nodeState[r][2][j][item].getIndex());
                xor_vars.push_back(nodeState[r][1][j][item].getIndex());
                (void)addXorClauses(solver, xor_vars, true, myUtil, msg);
            } else {
                nodeState[r][2][j][item] = nodeState[r][1][j][item];
            }
        }
    }
}

// 3.3  AK  2->3
void genAKConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeKsr[ROUNDS][16 / 2][ELEMENT_SIZE],
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg) {
    int r = RC - startRound;
    for (int j = 0; j < 16; j++) {
        if (j < 16 / 2) {
            for (int item = 0; item < ELEMENT_SIZE; item++) {
                nodeState[r][3][j][item].newClauseNode(solver, NODE_NOT_CHECK_USED);
                vector<unsigned> xor_vars;
                xor_vars.push_back(nodeState[r][2][j][item].getIndex());
                xor_vars.push_back(nodeState[r][3][j][item].getIndex());
                xor_vars.push_back(nodeKsr[RC][j][item].getIndex());
                (void)addXorClauses(solver, xor_vars, false, myUtil, msg);
            }
        } else {
            for (int item = 0; item < ELEMENT_SIZE; item++) {
                nodeState[r][3][j][item] = nodeState[r][2][j][item];
            }
        }
    }
}

// 3.4  PL  3->4
void genPLConstraints(int RC, int startRound,
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil) {
    int r = RC - startRound;
    for (int j = 0; j < 16; j++) {
        int ind = myUtil.shiftRows(j);
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            nodeState[r][4][j][item] = nodeState[r][3][ind][item];
        }
    }
}

// 3.5  MC  4->0
void genMCConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg) {
    int r = RC - startRound;
    for (int column = 0; column < 4; column++) {
        // y0 = x0 ^ x2 ^ x3
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            vector<unsigned> xor_vars;
            xor_vars.push_back(nodeState[r + 1][0][0 * 4 + column][item].newClauseNode(solver, NODE_NOT_CHECK_USED));
            xor_vars.push_back(nodeState[r][4][0 * 4 + column][item].getIndex());
            xor_vars.push_back(nodeState[r][4][2 * 4 + column][item].getIndex());
            xor_vars.push_back(nodeState[r][4][3 * 4 + column][item].getIndex());
            (void)addXorClauses(solver, xor_vars, 0, myUtil, msg);
        }
        // y1 = x0
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            nodeState[r + 1][0][1 * 4 + column][item] = nodeState[r][4][0 * 4 + column][item];
        }
        // y2 = x1 ^ x2
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            vector<unsigned> xor_vars;
            xor_vars.push_back(nodeState[r + 1][0][2 * 4 + column][item].newClauseNode(solver, NODE_NOT_CHECK_USED));
            xor_vars.push_back(nodeState[r][4][1 * 4 + column][item].getIndex());
            xor_vars.push_back(nodeState[r][4][2 * 4 + column][item].getIndex());
            (void)addXorClauses(solver, xor_vars, 0, myUtil, msg);
        }
        // y3 = x0 ^ x2
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            vector<unsigned> xor_vars;
            xor_vars.push_back(nodeState[r + 1][0][3 * 4 + column][item].newClauseNode(solver, NODE_NOT_CHECK_USED));
            xor_vars.push_back(nodeState[r][4][0 * 4 + column][item].getIndex());
            xor_vars.push_back(nodeState[r][4][2 * 4 + column][item].getIndex());
            (void)addXorClauses(solver, xor_vars, 0, myUtil, msg);
        }
    }
    // for (int column = 0; column < 4; column++) {
    //     for (int row = 0; row < 4; row++) {
    //         for (int item = 0; item < ELEMENT_SIZE; item++) {
    //             nodeState[r + 1][0][row * 4 + column][item].newClauseNode(solver, NODE_NOT_CHECK_USED);
    //             vector<unsigned> xor_vars;
    //             xor_vars.push_back(nodeState[r + 1][0][row * 4 + column][item].getIndex());
    //             for (int M_column = 0; M_column < 4; M_column++) {
    //                 if (myUtil.M[row * 4 + M_column] == 1) {
    //                     xor_vars.push_back(nodeState[r][4][M_column * 4 + column][item].getIndex());
    //                 }
    //             }
    //             (void)addXorClauses(solver, xor_vars, 0, myUtil, msg);
    //         }
    //     }
    // }
}

// 4 添加密文的约束
void genCipherConstraints(CMSat::SATSolver &solver, int startRound,
                          std::array<std::bitset<ELEMENT_SIZE>, 16> cipher,
                          ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                          Util &myUtil, string &msg) {
    int R = ROUNDS - startRound;
    for (int j = 0; j < 16; j++) {
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            vector<CMSat::Lit> lits;
            lits.push_back(CMSat::Lit(nodeState[R][0][j][item].getIndex(), !cipher[j][item]));
            (void)addClauses(solver, lits, myUtil, msg);
        }
    }
}