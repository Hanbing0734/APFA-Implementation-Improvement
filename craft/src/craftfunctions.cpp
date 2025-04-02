/*
 * @Author: Hanbing
 * @Date: 2024-09-29 09:17:45
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-24 17:21:40
 * @FilePath: /SAT/craft/src/craftfunctions.cpp
 * @Description:
 *
 * Copyright (c) 2024 by Hanbing, All Rights Reserved.
 */

#include "craftfunctions.h"
void print_state(const std::array<std::bitset<ELEMENT_SIZE>, 16> &state) {
    cout << hex;
    for (int i = 0; i < 16; i++)
        cout << state[i].to_ulong();
    cout << dec << endl;
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

void mix_columns(std::array<std::bitset<ELEMENT_SIZE>, 16> &state, Util &myUtil) {
    std::array<std::bitset<ELEMENT_SIZE>, 16> tmp;
    for (int column = 0; column < 4; column++) {
        for (int row = 0; row < 4; row++) {
            std::bitset<ELEMENT_SIZE> x = 0b0;
            for (int M_column = 0; M_column < 4; M_column++) {
                if (myUtil.M[row * 4 + M_column] == 1)
                    x ^= state[M_column * 4 + column];
            }
            tmp[row * 4 + column] = x;
        }
    }
    state = tmp;
}

void ksr_schedule(const std::array<std::array<std::bitset<ELEMENT_SIZE>, 16>, 2> &master_key,
                  const std::array<std::bitset<ELEMENT_SIZE>, 16> &tweakey,
                  std::array<std::array<std::bitset<ELEMENT_SIZE>, 16>, 4> &ksr,
                  Util &myUtil) {
    // string funcName(__func__);
    std::array<std::bitset<ELEMENT_SIZE>, 16> K0 = master_key[0];
    std::array<std::bitset<ELEMENT_SIZE>, 16> K1 = master_key[1];
    std::array<std::bitset<ELEMENT_SIZE>, 16> T = tweakey;
    // TK0
    for (size_t i = 0; i < 16; i++) {
        ksr[0][i] = K0[i] ^ T[i];
    }
    // TK1
    for (size_t i = 0; i < 16; i++) {
        ksr[1][i] = K1[i] ^ T[i];
    }
    // TK2
    for (size_t i = 0; i < 16; i++) {
        int permuted_index = myUtil.TWEAKEY_QT[i];
        ksr[2][i] = K0[i] ^ (T[permuted_index]);
    }
    // TK3
    for (size_t i = 0; i < 16; i++) {
        int permuted_index = myUtil.TWEAKEY_QT[i];
        ksr[3][i] = K1[i] ^ (T[permuted_index]);
    }
}

void craftEncrypt(const std::array<std::bitset<ELEMENT_SIZE>, 16> &plaint,
                  std::array<std::bitset<ELEMENT_SIZE>, 16> &cipher,
                  int have_error,
                  const std::array<std::array<std::bitset<ELEMENT_SIZE>, 16>, 4> &ksr,
                  Util &myUtil) {
    std::array<std::bitset<ELEMENT_SIZE>, 16> state = plaint;
    for (int RC = 0; RC < ROUNDS; RC++) {
        // 3.1  MC  0->1
        (void)mix_columns(state, myUtil);
        // 3.2  AC  1->2
        state[4] ^= myUtil.RoundConstants[RC][0];
        state[5] ^= myUtil.RoundConstants[RC][1];
        // 3.3  AK  2->3
        for (int j = 0; j < 16; j++)
            state[j] ^= ksr[RC % 4][j];
        if (RC != ROUNDS - 1) {
            // 3.4  PL  3->4
            std::array<std::bitset<ELEMENT_SIZE>, 16> tmp;
            for (int j = 0; j < 16; j++) {
                int ind = myUtil.shiftRows(j);
                tmp[j] = state[ind];
            }
            state = tmp;
            // 3.5  SB  4->0
            for (int j = 0; j < 16; j++) {
                unsigned ind = state[j].to_ulong();
                state[j] = myUtil.S(ind, have_error);
            }
        }
    }
    cipher = state;
}

// 1 初始tweakey约束    // 2 密钥编排
void genKsrConstraints(CMSat::SATSolver &solver,
                       ClauseNode nodeMasterkey[2][16][ELEMENT_SIZE],
                       ClauseNode nodeTweakey[16][ELEMENT_SIZE],
                       ClauseNode nodeKsr[4][16][ELEMENT_SIZE],
                       Util &myUtil, string &msg) {
    // 1.1 初始密钥约束
    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < 16; j++) {
            for (size_t item = 0; item < ELEMENT_SIZE; item++) {
                (void)nodeMasterkey[i][j][item].newNode(solver);
            }
        }
    }
    // 1.2 初始tweakey约束
    for (size_t j = 0; j < 16; j++) {
        for (size_t item = 0; item < ELEMENT_SIZE; item++) {
            (void)nodeTweakey[j][item].newNode(solver);
        }
    }
    // 2 密钥编排
    // 2.1 TK0
    for (size_t j = 0; j < 16; j++) {
        for (size_t item = 0; item < ELEMENT_SIZE; item++) {
            vector<unsigned> xor_vars;
            xor_vars.push_back(nodeKsr[0][j][item].newNode(solver));
            xor_vars.push_back(nodeMasterkey[0][j][item].getIndex());
            xor_vars.push_back(nodeTweakey[j][item].getIndex());
            (void)addXorClauses(solver, xor_vars, false, myUtil, msg);
        }
    }
    // 2.2 TK1
    for (size_t j = 0; j < 16; j++) {
        for (size_t item = 0; item < ELEMENT_SIZE; item++) {
            vector<unsigned> xor_vars;
            xor_vars.push_back(nodeKsr[1][j][item].newNode(solver));
            xor_vars.push_back(nodeMasterkey[1][j][item].getIndex());
            xor_vars.push_back(nodeTweakey[j][item].getIndex());
            (void)addXorClauses(solver, xor_vars, false, myUtil, msg);
        }
    }
    // 2.3 TK2
    for (size_t j = 0; j < 16; j++) {
        int permuted_index = myUtil.TWEAKEY_QT[j];
        for (size_t item = 0; item < ELEMENT_SIZE; item++) {
            vector<unsigned> xor_vars;
            xor_vars.push_back(nodeKsr[2][j][item].newNode(solver));
            xor_vars.push_back(nodeMasterkey[0][j][item].getIndex());
            xor_vars.push_back(nodeTweakey[permuted_index][item].getIndex());
            (void)addXorClauses(solver, xor_vars, false, myUtil, msg);
        }
    }
    // 2.4 TK3
    for (size_t j = 0; j < 16; j++) {
        int permuted_index = myUtil.TWEAKEY_QT[j];
        for (size_t item = 0; item < ELEMENT_SIZE; item++) {
            vector<unsigned> xor_vars;
            xor_vars.push_back(nodeKsr[3][j][item].newNode(solver));
            xor_vars.push_back(nodeMasterkey[1][j][item].getIndex());
            xor_vars.push_back(nodeTweakey[permuted_index][item].getIndex());
            (void)addXorClauses(solver, xor_vars, false, myUtil, msg);
        }
    }

    // for (size_t i = 0; i < 4; i++) {
    //     for (size_t j = 0; j < 16; j++) {
    //         int permuted_index = myUtil.TWEAKEY_QT[j];
    //         for (size_t item = 0; item < ELEMENT_SIZE; item++) {
    //             nodeKsr[i][j][item].newNode(solver);
    //         }
    //     }
    // }
}
// 3 加密约束
void genEncryptConstraints(CMSat::SATSolver &solver, int startRound,
                           ClauseNode nodeKsr[4][16][ELEMENT_SIZE],
                           ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                           Util &myUtil, string &msg) {
    for (int j = 0; j < 16; j++) {
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            nodeState[0][0][j][item].newNode(solver);
        }
    }
    for (int RC = startRound; RC < ROUNDS; RC++) {
        // 3.1  MC  0->1
        genMCConstraints(solver, RC, startRound, nodeState, myUtil, msg);
        // 3.2  AC  1->2
        genACConstraints(solver, RC, startRound, nodeState, myUtil, msg);
        // 3.3  AK  2->3
        genAKConstraints(solver, RC, startRound, nodeKsr, nodeState, myUtil, msg);
        if (RC != ROUNDS - 1) {
            // 3.4  PL  3->4
            genPLConstraints(RC, startRound, nodeState, myUtil);
            // 3.5  SB  4->0
            genSBConstraints(solver, RC, startRound, nodeState, myUtil, msg);
        }
    }
}

// 3.1  MC  0->1
void genMCConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg) {
    int r = RC - startRound;
    for (int column = 0; column < 4; column++) {
        // y0 = x0 ^ x2 ^ x3
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            vector<unsigned> xor_vars;
            xor_vars.push_back(nodeState[r][1][0 * 4 + column][item].newNode(solver));
            xor_vars.push_back(nodeState[r][0][0 * 4 + column][item].getIndex());
            xor_vars.push_back(nodeState[r][0][2 * 4 + column][item].getIndex());
            xor_vars.push_back(nodeState[r][0][3 * 4 + column][item].getIndex());
            (void)addXorClauses(solver, xor_vars, 0, myUtil, msg);
        }
        // y1 = x1 ^ x3
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            vector<unsigned> xor_vars;
            xor_vars.push_back(nodeState[r][1][1 * 4 + column][item].newNode(solver));
            xor_vars.push_back(nodeState[r][0][1 * 4 + column][item].getIndex());
            xor_vars.push_back(nodeState[r][0][3 * 4 + column][item].getIndex());
            (void)addXorClauses(solver, xor_vars, 0, myUtil, msg);
        }
        // y2 = x2
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            nodeState[r][1][2 * 4 + column][item] = nodeState[r][0][2 * 4 + column][item];
        }
        // y3 = x3
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            nodeState[r][1][3 * 4 + column][item] = nodeState[r][0][3 * 4 + column][item];
        }
    }
    // for (int column = 0; column < 4; column++) {
    //     for (int row = 0; row < 4; row++) {
    //         for (int item = 0; item < ELEMENT_SIZE; item++) {
    //             nodeState[r][1][row * 4 + column][item].newNode(solver);
    //             vector<unsigned> xor_vars;
    //             xor_vars.push_back(nodeState[r][1][row * 4 + column][item].getIndex());
    //             for (int M_column = 0; M_column < 4; M_column++) {
    //                 if (myUtil.M[row * 4 + M_column] == 1) {
    //                     xor_vars.push_back(nodeState[r][0][M_column * 4 + column][item].getIndex());
    //                 }
    //             }
    //             (void)addXorClauses(solver, xor_vars, 0, myUtil, msg);
    //         }
    //     }
    // }
}

// 3.2  AC  1->2
void genACConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg) {
    int r = RC - startRound;
    for (int j = 0; j < 16; j++) {
        if (j == 4 || j == 5) {
            for (int item = 0; item < ELEMENT_SIZE; item++) {
                if (myUtil.RoundConstants[RC][j - 4][item] == 1) {
                    vector<unsigned int> xor_vars;
                    nodeState[r][2][j][item].newNode(solver);
                    xor_vars.push_back(nodeState[r][2][j][item].getIndex());
                    xor_vars.push_back(nodeState[r][1][j][item].getIndex());
                    (void)addXorClauses(solver, xor_vars, true, myUtil, msg);
                } else
                    nodeState[r][2][j][item] = nodeState[r][1][j][item];
            }
        } else {
            for (int item = 0; item < ELEMENT_SIZE; item++) {
                nodeState[r][2][j][item] = nodeState[r][1][j][item];
            }
        }
    }
}

// 3.3  AK  2->3
void genAKConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeKsr[4][16][ELEMENT_SIZE],
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg) {
    int r = RC - startRound;
    for (int j = 0; j < 16; j++) {
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            nodeState[r][3][j][item].newNode(solver);
            vector<unsigned> xor_vars;
            xor_vars.push_back(nodeState[r][2][j][item].getIndex());
            xor_vars.push_back(nodeState[r][3][j][item].getIndex());
            xor_vars.push_back(nodeKsr[RC % 4][j][item].getIndex());
            (void)addXorClauses(solver, xor_vars, false, myUtil, msg);
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

// 3.5  SB  4->0
void genSBConstraints(CMSat::SATSolver &solver, int RC, int startRound,
                      ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE],
                      Util &myUtil, string &msg) {
    int r = RC - startRound;
    for (int j = 0; j < 16; j++) {
        for (int item = 0; item < ELEMENT_SIZE; item++) {
            nodeState[r + 1][0][j][item].newNode(solver);
        }
    }
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
                        clause.push_back(CMSat::Lit(nodeState[r][4][j][index].newClauseNode(solver, NODE_CHECK_USED), item.second));
                    } else {
                        clause.push_back(CMSat::Lit(nodeState[r + 1][0][j][index - ELEMENT_SIZE].newClauseNode(solver, NODE_CHECK_USED), item.second));
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
                        xor_vars.push_back(nodeState[r][4][j][0].getIndex());
                        break;
                    case 2:
                        xor_vars.push_back(nodeState[r][4][j][1].getIndex());
                        break;
                    case 4:
                        xor_vars.push_back(nodeState[r][4][j][2].getIndex());
                        break;
                    case 8:
                        xor_vars.push_back(nodeState[r][4][j][3].getIndex());
                        break;
                    default:
                        xor_vars.push_back(nodeIT[index_IT].newClauseNode(solver, NODE_CHECK_USED));

                        if (haveIT[index_IT] != true) {
                            for (auto item : words) {
                                vector<CMSat::Lit> lits_1;
                                lits_1.push_back(CMSat::Lit(nodeState[r][4][j][item].newClauseNode(solver, NODE_CHECK_USED), false));
                                lits_1.push_back(CMSat::Lit(nodeIT[index_IT].getIndex(), true));
                                (void)addClauses(solver, lits_1, myUtil, msg);
                            }
                            vector<CMSat::Lit> lits_2;
                            for (auto item : words) {
                                lits_2.push_back(CMSat::Lit(nodeState[r][4][j][item].newClauseNode(solver, NODE_CHECK_USED), true));
                            }
                            lits_2.push_back(CMSat::Lit(nodeIT[index_IT].getIndex(), false));
                            (void)addClauses(solver, lits_2, myUtil, msg);

                            haveIT[index_IT] = true;
                        }
                        break;
                    }
                }

                xor_vars.push_back(nodeState[r + 1][0][j][ir].newClauseNode(solver, NODE_CHECK_USED));
                (void)addXorClauses(solver, xor_vars, xor_const, myUtil, msg);
            }
        }
        // 若使用错误S盒，则过错误S盒的数据不存在被改写的原S盒数据 的约束
        if (ENCYPT_HAVEERROR == HAVE_ERROR) {
            bitset<SBOX_ITEM_SIZE> errSboxItem;
            myUtil.getSboxInErrorIndex(errSboxItem);
            vector<CMSat::Lit> lits;
            ClauseNode nodeD[ELEMENT_SIZE];
            for (int item = ELEMENT_SIZE - 1; item >= 0; item--) {
                vector<unsigned> xor_vars;
                xor_vars.push_back(nodeD[item].newClauseNode(solver, NODE_CHECK_USED));
                xor_vars.push_back(nodeState[r + 1][0][j][item].getIndex());
                (void)addXorClauses(solver, xor_vars, errSboxItem[item], myUtil, msg);

                lits.push_back(CMSat::Lit(nodeD[item].getIndex(), false));
            }
            (void)addClauses(solver, lits, myUtil, msg);
        }
    }
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
            lits.push_back(CMSat::Lit(nodeState[R - 1][3][j][item].getIndex(), !cipher[j][item]));
            (void)addClauses(solver, lits, myUtil, msg);
        }
    }
}
