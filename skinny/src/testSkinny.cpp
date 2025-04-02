/*
 * @Author: Hanbing
 * @Date: 2024-09-14 07:52:04
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-24 10:19:09
 * @FilePath: /SAT/skinny/src/testSkinny.cpp
 * @Description:
 *
 * Copyright (c) 2024 by Hanbing, All Rights Reserved.
 */

#include <cryptominisat5/cryptominisat.h>
#ifndef SKINNY_MODE
#define SKINNY_MODE 64
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
#include "params.h"
#include "skinnyfunctions.h"
#include "util.h"
using namespace std;

void test_skinny(Util &myUtil);

int runCryptominisat(CMSat::SATSolver &solver,
                     const std::array<std::bitset<ELEMENT_SIZE>, 16> tweakeys_initial[3],
                     Util &myUtil);

int testCryptominisat(CMSat::SATSolver &solver,
                      const ClauseNode nodeTweakeyInit[TWEAKEY_MODE][16][ELEMENT_SIZE],
                      const std::array<std::bitset<ELEMENT_SIZE>, 16> tweakeys_initial[3]);
int mainSAT(int test_or_run, int numThreads, int startRound, int num_cipher,
            const std::array<std::bitset<ELEMENT_SIZE>, 16> tweakeys_initial[3],
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

    string msg("");
    Util myUtil = Util(SBOX_MODE, msg);

    myUtil.setNumClauses(0);
    cout << "Max time :\t" << SAT_MAX_TIME << endl;
    cout << "SKINNY_MODE:\t" << SKINNY_MODE << endl;
    cout << "TWEAKEY_MODE:\t" << TWEAKEY_MODE << endl;
    cout << "SAT_MODE:\t" << SAT_MODE << endl;
    cout << "\tSBOX_MODE:\t" << SBOX_MODE << endl;
    cout << "\tENCYPT_HAVEERROR:\t" << ENCYPT_HAVEERROR << endl;

    cout << "AnalysesRounds:\t" << numAnalysesRounds << endl;
    cout << "MinCiphers:\t" << numMinCipher << endl;
    cout << "MaxCiphers:\t" << numMaxCipher << endl;
    cout << "Threads:\t" << numThreads << endl;

    // (void)test_skinny(myUtil);

    std::array<std::bitset<ELEMENT_SIZE>, 16> tweakeys[3];
    string key_test(KEY_TEST);
    srand(time(0));
    int ind = 0;
    for (int i = 0; i < TWEAKEY_MODE; i++) {
        for (int j = 0; j < 16; j++) {
            // tweakeys[i][j] = std::bitset<ELEMENT_SIZE>(rand() & (int)(pow(2, ELEMENT_SIZE) - 1));
            for (int o = 0; o < ELEMENT_SIZE; o++) {
                tweakeys[i][j][o] = key_test[ind++] - '0';
            }
        }
    }
    std::array<std::bitset<ELEMENT_SIZE>, 16 / 2> ksr[ROUNDS];  // 轮密钥
    (void)tweakey_schedule(tweakeys, ksr, myUtil);
    cout << "---------------------------------------------------------------------------------------------------" << endl;

    std::array<std::bitset<ELEMENT_SIZE>, 16> plaints[MAX_CIPHERS + 10];                                  // 明文
    std::array<std::bitset<ELEMENT_SIZE>, 16> ciphers[MAX_CIPHERS + 10], errorCiphers[MAX_CIPHERS + 10];  // 密文

    for (int num_cipher = 1; num_cipher <= numMaxCipher; num_cipher++) {
        // 预处理 打表
        for (int j = 0; j < 16; j++) {
            plaints[num_cipher][j] = std::bitset<ELEMENT_SIZE>(rand() & (int)(pow(2, ELEMENT_SIZE) - 1));
        }
        (void)skinnyEncrypt(plaints[num_cipher], ciphers[num_cipher], NO_ERROR, ksr, myUtil);
        (void)skinnyEncrypt(plaints[num_cipher], errorCiphers[num_cipher], HAVE_ERROR, ksr, myUtil);
    }
    for (int num_cipher = 1; num_cipher <= numMaxCipher; num_cipher++) {
        if (num_cipher < numMinCipher) {
            continue;
        }
        int num_solution = mainSAT(TEST, numThreads, startRound, num_cipher, tweakeys, errorCiphers, myUtil, msg);
        if (num_solution == NONE_SOLUTION) {
            cout << "\ttest: 无解！" << endl;
            break;
        }
        int numResults = mainSAT(RUN, numThreads, startRound, num_cipher, tweakeys, errorCiphers, myUtil, msg);
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
            const std::array<std::bitset<ELEMENT_SIZE>, 16> tweakeys_initial[3],
            const std::array<std::bitset<ELEMENT_SIZE>, BLOCK_SIZE / ELEMENT_SIZE> errorCiphers[MAX_CIPHERS + 10],
            Util &myUtil, string &msg) {
    // 主体
    myUtil.setNumClauses(0);
    CMSat::SATSolver solver;
    // solver.set_num_threads(numThreads);
    ClauseNode nodeTweakeyInit[TWEAKEY_MODE][16][ELEMENT_SIZE];
    ClauseNode nodeKsr[ROUNDS][16 / 2][ELEMENT_SIZE];
    (void)genKsrConstraints(solver, nodeTweakeyInit, nodeKsr, myUtil, msg);
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
        return testCryptominisat(solver, nodeTweakeyInit, tweakeys_initial);
    } else if (test_or_run == RUN) {
        // solver.simplify();  // 简化
        return runCryptominisat(solver, tweakeys_initial, myUtil);
    } else
        return 0;
}

int testCryptominisat(CMSat::SATSolver &solver,
                      const ClauseNode nodeTweakeyInit[TWEAKEY_MODE][16][ELEMENT_SIZE],
                      const std::array<std::bitset<ELEMENT_SIZE>, 16> tweakeys_initial[3]) {
    // 计算时间
    clock_t pro_start;

    // addTestConstraints 添加等于正确tweakey的约束，测试能否求解
    // CMSat::SATSolver solver2;
    // CMSat::copy_solver_to_solver(&solver, &solver2);
    for (int i = 0; i < TWEAKEY_MODE; i++) {
        for (int j = 0; j < 16; j++) {
            for (int item = 0; item < ELEMENT_SIZE; item++) {
                vector<CMSat::Lit> assumptions;
                assumptions.push_back(CMSat::Lit(nodeTweakeyInit[i][j][item].getIndex(), !tweakeys_initial[i][j][item]));
                solver.add_clause(assumptions);
            }
        }
    }
    pro_start = clock();
    solver.set_max_time(SAT_MAX_TIME);
    CMSat::lbool ret = solver.solve();
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

int runCryptominisat(CMSat::SATSolver &solver,
                     const std::array<std::bitset<ELEMENT_SIZE>, 16> tweakeys_initial[3],
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
        string re;
        for (int i = 0; i < KEY_SIZE; i++) {
            // re += '0' + solver.get_model()[i].getValue();
            re += '1' - solver.get_model()[i].getValue();
        }
        string master_key;
        for (int i = 0; i < TWEAKEY_MODE; i++) {
            for (int j = 0; j < 16; j++) {
                for (int item = 0; item < ELEMENT_SIZE; item++) {
                    master_key += '0' + tweakeys_initial[i][j][item];
                }
            }
        }
        if (re.compare(master_key) == 0) {
            cout << "解得主密钥:  " << numResults << "\t" << re << endl;
            // indexResult = numResults;
        }
        // cout << numResults << " " << re << endl;

        // Banning found solution
        vector<CMSat::Lit> ban_solution;
        for (uint32_t var = 0; var < KEY_SIZE; var++) {
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

void test_skinny(Util &myUtil) {
    std::array<std::bitset<ELEMENT_SIZE>, 16> tweakeys[3];
    std::array<std::bitset<ELEMENT_SIZE>, 16 / 2> ksr[ROUNDS];  // 轮密钥

    std::array<std::bitset<ELEMENT_SIZE>, 16> plaint;           // 明文
    std::array<std::bitset<ELEMENT_SIZE>, 16> cipher, cipher2;  // 密文

    string tk1_str = "0b98acadc7fdf93d";
    string tk2_str = "cc507116dc9c7307";
    string tk3_str = "ca30711dfce3733e";
    string plain_str = "0000000000000000";
    string test_cipher = "4c3e726936232477";
    bool reversed = false;
    convert_hexstr_to_bitsetarray(tk1_str, tweakeys[0], reversed);
    convert_hexstr_to_bitsetarray(tk2_str, tweakeys[1], reversed);
    convert_hexstr_to_bitsetarray(tk3_str, tweakeys[2], reversed);
    convert_hexstr_to_bitsetarray(plain_str, plaint, reversed);
    convert_hexstr_to_bitsetarray(test_cipher, cipher2, reversed);
    (void)tweakey_schedule(tweakeys, ksr, myUtil);
    // print_state(tweakeys[0]);
    // print_state(tweakeys[1]);
    // print_state(tweakeys[2]);
    // for (int r = 0; r < ROUNDS; r++) {
    //     print_ksr(ksr[r]);
    // }
    cout << "plaint:\t";
    print_state(plaint);
    skinnyEncrypt(plaint, cipher, NO_ERROR, ksr, myUtil);
    cout << "cipher:\t";
    print_state(cipher);
    cout << "cipher2:\t";
    print_state(cipher2);
}

/*
 */