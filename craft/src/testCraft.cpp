/*
 * @Author: Hanbing
 * @Date: 2024-09-14 07:52:04
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-25 19:01:14
 * @FilePath: /SAT/craft/src/testCraft.cpp
 * @Description:
 *
 * Copyright (c) 2024 by Hanbing, All Rights Reserved.
 */

#include <cryptominisat5/cryptominisat.h>
#ifndef CRAFT_MODE
#define CRAFT_MODE 64
#endif
#ifndef TWEAKEY_MODE
#define TWEAKEY_MODE 2
#endif

#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

#include "clausenode.h"
#include "craftfunctions.h"
#include "params.h"
#include "util.h"
using namespace std;

int runCryptominisat(CMSat::SATSolver &solver,
                     const std::array<std::array<std::bitset<ELEMENT_SIZE>, 16>, 2> &master_key,
                     const std::array<std::bitset<ELEMENT_SIZE>, 16> &tweakey,
                     Util &myUtil);

int testCryptominisat(CMSat::SATSolver &solver,
                      const ClauseNode nodeMasterkey[2][16][ELEMENT_SIZE],
                      ClauseNode nodeTweakey[16][ELEMENT_SIZE],
                      const std::array<std::array<std::bitset<ELEMENT_SIZE>, 16>, 2> &master_key,
                      std::array<std::bitset<ELEMENT_SIZE>, 16> tweakey);
int mainSAT(int test_or_run, int numThreads, int startRound, int num_cipher,
            const std::array<std::array<std::bitset<ELEMENT_SIZE>, 16>, 2> &master_key,
            const std::array<std::bitset<ELEMENT_SIZE>, 16> &tweakey,
            const std::array<std::bitset<ELEMENT_SIZE>, BLOCK_SIZE / ELEMENT_SIZE> errorCiphers[MAX_CIPHERS + 10],
            Util &myUtil, string &msg);
int main(int argc, char const *argv[]) {
    int numAnalysesRounds = 5;
    int numMinCipher = MIN_CIPHERS;
    int numMaxCipher = MAX_CIPHERS;
    int numThreads = THREADS;
    int startRound;  // 哪一轮开始分析
    if (argc > 1)
        numThreads = atoi(argv[1]);
    if (argc > 2)
        numAnalysesRounds = atoi(argv[2]);
    if (argc > 3)
        numMinCipher = atoi(argv[3]);
    if (argc > 4)
        numMaxCipher = atoi(argv[4]);
    startRound = ROUNDS - numAnalysesRounds;

    std::string msg("");
    Util myUtil = Util(SBOX_MODE, msg);

    myUtil.setNumClauses(0);
    cout << "Max time :\t" << SAT_MAX_TIME << endl;
    cout << "SAT_MODE:\t" << SAT_MODE << endl;
    cout << "\tSBOX_MODE:\t" << SBOX_MODE << endl;
    cout << "\tENCYPT_HAVEERROR:\t" << ENCYPT_HAVEERROR << endl;

    cout << "AnalysesRounds:\t" << numAnalysesRounds << endl;
    cout << "MinCiphers:\t" << numMinCipher << endl;
    cout << "MaxCiphers:\t" << numMaxCipher << endl;
    cout << "Threads:\t" << numThreads << endl;

    srand(time(0));
    std::array<std::array<std::bitset<ELEMENT_SIZE>, 16>, 2> master_key;
    string key_test(KEY_TEST);
    int ind = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 16; j++) {
            // master_key[i][j] = std::bitset<ELEMENT_SIZE>(rand() & (int)(pow(2, ELEMENT_SIZE) - 1));
            for (int o = 0; o < ELEMENT_SIZE; o++) {
                master_key[i][j][o] = key_test[ind++] - '0';
            }
        }
    }
    std::array<std::bitset<ELEMENT_SIZE>, 16> tweakey;
    for (int j = 0; j < 16; j++) {
        // tweakey[j] = std::bitset<ELEMENT_SIZE>(rand() & (int)(pow(2, ELEMENT_SIZE) - 1));
        for (int o = 0; o < ELEMENT_SIZE; o++) {
            tweakey[j][o] = key_test[ind++] - '0';
        }
    }
    std::array<std::array<std::bitset<ELEMENT_SIZE>, 16>, 4> ksr;  // 轮密钥
    (void)ksr_schedule(master_key, tweakey, ksr, myUtil);
    cout << "---------------------------------------------------------------------------------------------------" << endl;

    std::array<std::bitset<ELEMENT_SIZE>, 16> plaints[MAX_CIPHERS + 10];                                  // 明文
    std::array<std::bitset<ELEMENT_SIZE>, 16> ciphers[MAX_CIPHERS + 10], errorCiphers[MAX_CIPHERS + 10];  // 密文

    for (int num_cipher = 1; num_cipher <= numMaxCipher; num_cipher++) {
        // 预处理 打表
        for (int j = 0; j < 16; j++) {
            plaints[num_cipher][j] = std::bitset<ELEMENT_SIZE>(rand() & (int)(pow(2, ELEMENT_SIZE) - 1));
        }
        (void)craftEncrypt(plaints[num_cipher], ciphers[num_cipher], NO_ERROR, ksr, myUtil);
        (void)craftEncrypt(plaints[num_cipher], errorCiphers[num_cipher], HAVE_ERROR, ksr, myUtil);
    }
    // 主体
    for (int num_cipher = 1; num_cipher <= numMaxCipher; num_cipher++) {
        if (num_cipher < numMinCipher) {  // 类唯密文分析，只收集后几轮的信息
            continue;
        }
        int num_solution = mainSAT(TEST, numThreads, startRound, num_cipher, master_key, tweakey, errorCiphers, myUtil, msg);
        if (num_solution == NONE_SOLUTION) {
            cout << "\ttest: 无解！" << endl;
            break;
        }
        // solver.simplify();  // 简化
        int numResults = mainSAT(RUN, numThreads, startRound, num_cipher, master_key, tweakey, errorCiphers, myUtil, msg);
        if (numResults == 1) {
            cout << "---------------------------------------------------------------------------------------------------" << endl;
            cout << "求得唯一解！" << endl;
            cout << "numAnalysesRounds:\t" << numAnalysesRounds << endl;
            cout << "startRound:\t" << startRound << endl;
            break;
        }
    }
    cout << "---------------------------------------------------------------------------------------------------" << endl;
}

int mainSAT(int test_or_run, int numThreads, int startRound, int num_cipher,
            const std::array<std::array<std::bitset<ELEMENT_SIZE>, 16>, 2> &master_key,
            const std::array<std::bitset<ELEMENT_SIZE>, 16> &tweakey,
            const std::array<std::bitset<ELEMENT_SIZE>, BLOCK_SIZE / ELEMENT_SIZE> errorCiphers[MAX_CIPHERS + 10],
            Util &myUtil, string &msg) {
    myUtil.setNumClauses(0);
    CMSat::SATSolver solver;
    solver.set_num_threads(numThreads);
    ClauseNode nodeMasterkey[2][16][ELEMENT_SIZE];
    ClauseNode nodeTweakey[16][ELEMENT_SIZE];
    ClauseNode nodeKsr[4][16][ELEMENT_SIZE];
    (void)genKsrConstraints(solver, nodeMasterkey, nodeTweakey, nodeKsr, myUtil, msg);
    for (int cipher_index = 1; cipher_index <= num_cipher; cipher_index++) {
        std::array<std::bitset<ELEMENT_SIZE>, 16> errorCipher = errorCiphers[cipher_index];
        ClauseNode nodeState[ROUNDS][5][16][ELEMENT_SIZE];  // 向solver中添加加密过程变量的索引
        (void)genEncryptConstraints(solver, startRound, nodeKsr, nodeState, myUtil, msg);
        (void)genCipherConstraints(solver, startRound, errorCipher, nodeState, myUtil, msg);
    }
    if (test_or_run == TEST) {
        cout << "num_cipher:\t" << num_cipher << endl;
        cout << "\tTotal vars:\t" << solver.nVars() << endl;
        cout << "\tTotal clauses:\t" << myUtil.getNumClauses() << endl;
        return testCryptominisat(solver, nodeMasterkey, nodeTweakey, master_key, tweakey);
    } else if (test_or_run == RUN) {
        // solver.simplify();  // 简化
        return runCryptominisat(solver, master_key, tweakey, myUtil);
    } else
        return 0;
}

int runCryptominisat(CMSat::SATSolver &solver,
                     const std::array<std::array<std::bitset<ELEMENT_SIZE>, 16>, 2> &master_key,
                     const std::array<std::bitset<ELEMENT_SIZE>, 16> &tweakey,
                     Util &myUtil) {
    // cout << __func__ << endl;
    // 计算时间
    clock_t pro_start;
    pro_start = clock();

    int numResults = 0;
    // int indexResult = -1;
    bool tooManyResults = false;
    while (true) {
        double totalTime = (double)(clock() - pro_start) / CLOCKS_PER_SEC;
        // solver.set_max_time(SAT_MAX_TIME - totalTime);
        CMSat::lbool ret = solver.solve();
        if (ret != CMSat::l_True) {
            // cout << "----------------------------------------------------求解完毕: " << indexResult << "------------------------------------------------" << endl;
            break;
        }

        ++numResults;
        if (numResults >= pow(2, MAX_RESULTS_LOG2)) {
            tooManyResults = true;
            break;
        }
        // 数据处理
        string MK;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 16; j++) {
                for (size_t item = 0; item < ELEMENT_SIZE; item++) {
                    MK += '0' + master_key[i][j][item];
                }
            }
        }
        string TK;
        for (int j = 0; j < 16; j++) {
            for (size_t item = 0; item < ELEMENT_SIZE; item++) {
                TK += '0' + tweakey[j][item];
            }
        }
        string key = MK + TK;
        string re;
        for (int i = 0; i < KEY_SIZE + TWEAKEY_SIZE; i++) {
            // re += '0' + solver.get_model()[i].getValue();
            re += '1' - solver.get_model()[i].getValue();
        }
        // cout << numResults << "\t" << re << endl;
        if (re.compare(key) == 0) {
            cout << "解得 mater_key 与 tweakey:\t" << numResults << endl;
            cout << "mater_key:\t" << MK << endl;
            cout << "tweakey:\t" << TK << endl;
        }

        // Banning found solution
        vector<CMSat::Lit> ban_solution;
        for (uint32_t var = 0; var < KEY_SIZE + TWEAKEY_SIZE; var++) {
            if (solver.get_model()[var] != CMSat::l_Undef) {
                ban_solution.push_back(CMSat::Lit(var, (solver.get_model()[var] == CMSat::l_True) ? true : false));
            }
            // ban_solution.push_back(Lit(var, !solver.get_model()[var].getValue()));
        }
        string msg;
        (void)addClauses(solver, ban_solution, myUtil, msg);
    }
    // double totalTime = (double)(clock() - pro_start) / CLOCKS_PER_SEC;
    // if (totalTime > SAT_MAX_TIME) {
    //     cout << "\tNot solved within " << SAT_MAX_TIME << " seconds, Have " << numResults << " results." << endl;
    //     numResults = 0;
    // } else
    if (tooManyResults) {
        cout << "\tHave more than " << numResults << " results, spent " << clock() - pro_start << " clocks." << endl;
        // numResults = -1;
    } else {
        cout << "\tHave " << numResults << " results, spent " << clock() - pro_start << " clocks." << endl;
    }
    return numResults;
}

int testCryptominisat(CMSat::SATSolver &solver,
                      const ClauseNode nodeMasterkey[2][16][ELEMENT_SIZE],
                      ClauseNode nodeTweakey[16][ELEMENT_SIZE],
                      const std::array<std::array<std::bitset<ELEMENT_SIZE>, 16>, 2> &master_key,
                      std::array<std::bitset<ELEMENT_SIZE>, 16> tweakey) {
    // 计算时间
    clock_t pro_start;

    // addTestConstraints 添加等于正确密钥的约束，测试能否求解

    // 1.1 初始密钥约束
    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < 16; j++) {
            for (size_t item = 0; item < ELEMENT_SIZE; item++) {
                vector<CMSat::Lit> assumptions;
                assumptions.push_back(CMSat::Lit(nodeMasterkey[i][j][item].getIndex(), !master_key[i][j][item]));
                solver.add_clause(assumptions);
            }
        }
    }
    // 1.2 初始tweakey约束
    for (size_t j = 0; j < 16; j++) {
        for (size_t item = 0; item < ELEMENT_SIZE; item++) {
            vector<CMSat::Lit> assumptions;
            assumptions.push_back(CMSat::Lit(nodeTweakey[j][item].getIndex(), !tweakey[j][item]));
            solver.add_clause(assumptions);
        }
    }
    pro_start = clock();
    solver.set_max_time(SAT_MAX_TIME);
    CMSat::lbool ret = solver.solve();
    // solver.remove_and_clean_all();
    // double totalTime = (double)(clock() - pro_start) / CLOCKS_PER_SEC;
    cout << "\tTest == masterkey, spend " << clock() - pro_start << " clocks." << endl;
    if (ret != CMSat::l_True) {
        return NONE_SOLUTION;
    }
    // else {
    //     // addTestConstraints 添加不等于正确密钥的约束，若有解，说明解不唯一
    //     pro_start = clock();
    //     for (int i = KEY_SIZE - 1; i >= 0; i--) {
    //         vector<CMSat::Lit> assumptions;
    //         assumptions.push_back(CMSat::Lit(nodeKsr[0][i].getIndex(), master_key[i]));
    //         ret = solver.solve(&assumptions);
    //         if (ret == CMSat::l_True) {
    //             cout << "\tTest != masterkey, spend " << clock() - pro_start << " clocks." << endl;
    //             return MULTI_SOLUTION;
    //         }
    //     }
    // }
    return HAVE_SOLUTION;
}

/*
 */