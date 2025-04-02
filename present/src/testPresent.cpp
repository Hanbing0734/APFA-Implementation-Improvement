/*
 * @Author: Hanbing
 * @Date: 2024-09-14 07:52:04
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-24 09:28:23
 * @FilePath: /SAT/present/src/testPresent.cpp
 * @Description:
 *
 * Copyright (c) 2024 by Hanbing, All Rights Reserved.
 */

#include <cryptominisat5/cryptominisat.h>

#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

#include "clausenode.h"
#include "params.h"
#include "presentfunctions.h"
#include "util.h"
using namespace std;

int runCryptominisat(CMSat::SATSolver &solver,
                     const std::bitset<KEY_SIZE> &master_key, Util &myUtil);

int testCryptominisat(CMSat::SATSolver &solver, ClauseNode nodeKsr[][KEY_SIZE], const bitset<KEY_SIZE> &master_key);

int mainSAT(int test_or_run, int numThreads, int startRound, int num_cipher,
            const std::bitset<KEY_SIZE> &master_key,
            const std::bitset<BLOCK_SIZE> errorCiphers[MAX_CIPHERS + 10],
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

    cout << "Max time :\t" << SAT_MAX_TIME << endl;
    myUtil.setNumClauses(0);
    cout << "PRESENT_MODE:\t" << PRESENT_MODE << endl;
    cout << "SAT_MODE:\t" << SAT_MODE << endl;
    cout << "\tSBOX_MODE:\t" << SBOX_MODE << endl;
    cout << "\tKSR_HAVEERROR:\t" << KSR_HAVEERROR << endl;
    cout << "\tENCYPT_HAVEERROR:\t" << ENCYPT_HAVEERROR << endl;

    cout << "AnalysesRounds:\t" << numAnalysesRounds << endl;
    cout << "MinCiphers:\t" << numMinCipher << endl;
    cout << "MaxCiphers:\t" << numMaxCipher << endl;
    cout << "Threads:\t" << numThreads << endl;

    string key_test(KEY_TEST);
    bitset<KEY_SIZE> master_key;  // 主密钥
    srand(time(0));
    int ind = 0;
    for (int index = 0; index < KEY_SIZE; index++) {
        // master_key[index] = 0;
        master_key[index] = key_test[ind++] - '0';
        // master_key[index] = rand() & 1;
    }
    cout << "Master_key:\t" << master_key << endl;
    // cout << "Master_key:\t" << master_key.to_string() << endl;
    // master_key = (master_key << (KEY_SIZE - 19)) | (master_key >> 19);
    bitset<KEY_SIZE> ksr[ROUNDS];  // 轮密钥
    (void)keySchedule(master_key, KSR_HAVEERROR, ksr, myUtil);
    cout << "---------------------------------------------------------------------------------------------------" << endl;

    bitset<BLOCK_SIZE> plaints[MAX_CIPHERS + 10];                                  // 明文
    bitset<BLOCK_SIZE> ciphers[MAX_CIPHERS + 10], errorCiphers[MAX_CIPHERS + 10];  // 密文
    plaints[0] = 0b1;
    for (int num_cipher = 1; num_cipher <= numMaxCipher; num_cipher++) {
        // 预处理 打表
        // plaints[num_cipher] = (plaints[num_cipher - 1] << 1) | (plaints[num_cipher - 1] >> (BLOCK_SIZE - 1));
        // cout << plaints[num_cipher] << endl;
        for (int j = 0; j < BLOCK_SIZE; j++) {
            plaints[num_cipher][j] = rand() & 1;
        }
        (void)presentEncrypt(plaints[num_cipher], ciphers[num_cipher], NO_ERROR, ksr, myUtil);
        (void)presentEncrypt(plaints[num_cipher], errorCiphers[num_cipher], HAVE_ERROR, ksr, myUtil);
    }
    for (int num_cipher = 1; num_cipher <= numMaxCipher; num_cipher++) {
        if (num_cipher < numMinCipher) {
            continue;
        }
        int num_solution = mainSAT(TEST, numThreads, startRound, num_cipher, master_key, errorCiphers, myUtil, msg);
        if (num_solution == NONE_SOLUTION) {
            cout << "\ttest: 无解！" << endl;
            break;
        }
        int numResults = mainSAT(RUN, numThreads, startRound, num_cipher, master_key, errorCiphers, myUtil, msg);
        if (numResults == 1) {
            cout << "---------------------------------------------------------------------------------------------------" << endl;
            cout << "求得唯一解！" << endl;
            cout << "numAnalysesRounds:\t" << numAnalysesRounds << endl;
            cout << "startRound:\t" << startRound << endl;
            break;
        }
    }

    cout << "---------------------------------------------------------------------------------------------------" << endl;
    return 0;
}

int mainSAT(int test_or_run, int numThreads, int startRound, int num_cipher,
            const std::bitset<KEY_SIZE> &master_key,
            const std::bitset<BLOCK_SIZE> errorCiphers[MAX_CIPHERS + 10],
            Util &myUtil, string &msg) {
    // 主体
    myUtil.setNumClauses(0);
    CMSat::SATSolver solver;
    solver.set_num_threads(numThreads);
    ClauseNode nodeKsr[ROUNDS][KEY_SIZE];  // 向solver中添加 密钥变量的索引
    genKsrConstraints(solver, nodeKsr, myUtil, msg);
    for (int cipher_index = 1; cipher_index <= num_cipher; cipher_index++) {
        bitset<BLOCK_SIZE> errorCipher = errorCiphers[cipher_index];
        ClauseNode nodeState[ROUNDS][3][BLOCK_SIZE];  // 向solver中添加加密过程变量的索引
        genEncryptConstraints(solver, startRound, nodeKsr, nodeState, myUtil, msg);
        genErrorCipherConstraints(solver, startRound, errorCipher, nodeState, myUtil, msg);
    }
    if (test_or_run == TEST) {
        cout << "num_cipher:\t" << num_cipher << endl;
        cout << "\tTotal vars:\t" << solver.nVars() << endl;
        cout << "\tTotal clauses:\t" << myUtil.getNumClauses() << endl;
        return testCryptominisat(solver, nodeKsr, master_key);
    } else if (test_or_run == RUN) {
        // solver.simplify();  // 简化
        return runCryptominisat(solver, master_key, myUtil);
    } else
        return 0;
}

int runCryptominisat(CMSat::SATSolver &solver,
                     const std::bitset<KEY_SIZE> &master_key, Util &myUtil) {
    // cout << __func__ << endl;
    // 计算时间
    clock_t pro_start;
    pro_start = clock();

    int numResults = 0;
    // int indexResult = -1;
    bool tooManyResults = false;
    while (true) {
        double totalTime = (double)(clock() - pro_start) / CLOCKS_PER_SEC;
        solver.set_max_time(SAT_MAX_TIME - totalTime);
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
        if (re.compare(master_key.to_string()) == 0) {
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
    double totalTime = (double)(clock() - pro_start) / CLOCKS_PER_SEC;
    if (totalTime > SAT_MAX_TIME) {
        cout << "\tNot solved within " << SAT_MAX_TIME << " seconds, Have " << numResults << " results." << endl;
        numResults = 0;
    } else if (tooManyResults) {
        cout << "\tHave more than " << numResults << " results, spent " << clock() - pro_start << " clocks." << endl;
        // numResults = -1;
    } else {
        cout << "\tHave " << numResults << " results, spent " << clock() - pro_start << " clocks." << endl;
    }
    return numResults;
}

int testCryptominisat(CMSat::SATSolver &solver, ClauseNode nodeKsr[][KEY_SIZE], const bitset<KEY_SIZE> &master_key) {
    // 计算时间
    clock_t pro_start;

    // addTestConstraints 添加等于正确密钥的约束，测试能否求解
    for (int i = KEY_SIZE - 1; i >= 0; i--) {
        vector<CMSat::Lit> assumptions;
        assumptions.push_back(CMSat::Lit(nodeKsr[0][i].getIndex(), !master_key[i]));
        solver.add_clause(assumptions);
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

/*
 */