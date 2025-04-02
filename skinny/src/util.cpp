/*
 * @Author: Hanbing
 * @Date: 2024-09-23 01:53:12
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-12 14:43:35
 * @FilePath: /SAT/skinny/src/util.cpp
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

#include "util.h"
Util::Util() {}

/**
 * @description:
 * @param {int} sbox_mode
 * @param {string} &msg
 * @return {*}
 */
Util::Util(int sbox_mode, std::string &msg) {
    (void)this->init_RoundConstants();
    (void)this->injectError();
    switch (sbox_mode) {
    case ANF_MODE:
        this->genAnfFromSbox(NO_ERROR, msg);
        this->genAnfFromSbox(HAVE_ERROR, msg);
        this->printAnf(NO_ERROR);
        this->printAnf(HAVE_ERROR);
        break;

    case CNF_MODE:
        this->genCnfFromSbox(NO_ERROR, msg);
        this->genCnfFromSbox(HAVE_ERROR, msg);
        this->printCnf(NO_ERROR);
        this->printCnf(HAVE_ERROR);
        break;

    default:
        break;
    }
}

void Util::init_RoundConstants() {
    RoundConstants[0] = 0b00001;
    for (int r = 0; r < ROUNDS - 1; r++) {
        std::bitset<6> shifted_x = (RoundConstants[r] << 1);                    // x 左移 1 位
        std::bitset<6> bit_5 = (RoundConstants[r] >> 5) & std::bitset<6>(0x1);  // 取 x 的x5（从右开始计数，0 索引）
        std::bitset<6> bit_4 = (RoundConstants[r] >> 4) & std::bitset<6>(0x1);  // 取 x 的x4

        RoundConstants[r + 1] = shifted_x ^ bit_5 ^ bit_4 ^ std::bitset<6>(0x1);
    }
}

void Util::injectError() {
    this->setSboxErrorIndex(SBOX_ERROR_INDEX);
    this->setSboxErrorPosition(SBOX_ERROR_POSITION);
    copy(begin(sbox), end(sbox), begin(e_sbox));
    e_sbox[sbox_error_index].flip(sbox_error_position);
}

int Util::genAnfFromSbox(int haveError, string &msg) {
    string funcName(__func__);
    if (haveError == NO_ERROR) {  // 正确S盒的ANF
        ifstream fin("./IO/in/ANF/ANF.txt");
        if (!fin.good()) {
            msg = funcName + ":\t open ./IO/in/ANF/ANF.txt fail.!!";
            return 1;
        } else {
            msg = funcName + ":\t open ./IO/in/ANF/ANF.txt success.!!";
        }
        for (int i = 3; i >= 0; --i) {
            // for (int i = 0; i < 4; i++) {
            string str;
            getline(fin, str);
            stringstream input_stream(str);
            // x0 x1 x2 x3 1
            while (input_stream >> str) {
                int j = 0;
                int coefficient = 0;
                while (isdigit(str[j])) {
                    char c = str[j++];
                    coefficient = coefficient * 10 + (c - '0');
                }
                vector<int> words;
                if (str[0] == '+' || (coefficient % 2 == 0 && coefficient != 0)) {  // 偶数次消去 '+'跳过
                    continue;
                } else if (str[0] == '1') {
                    words.push_back(4);
                    anf_sbox[i].push_back(words);
                    continue;
                } else {
                    while (j < static_cast<int>(str.length())) {
                        while (str[j] == 'x' || str[j] == '*')
                            j++;
                        coefficient = 0;
                        while (isdigit(str[j])) {
                            char c = str[j++];
                            coefficient = coefficient * 10 + (c - '0');
                        }
                        words.push_back(coefficient);
                    }
                    anf_sbox[i].push_back(words);
                    continue;
                }
            }
        }
        fin.close();
    }
    if (haveError == HAVE_ERROR) {  // 带错误S盒的ANF
        string filePath = "./IO/in/ANF/ERRANF_" + to_string(this->sbox_error_index) + "_" + to_string(this->sbox_error_position) + ".txt";
        ifstream fin(filePath.c_str());
        if (!fin.good()) {
            msg = funcName + ":\t open " + filePath + " fail.!!";
            return 1;
        } else {
            msg = funcName + ":\t open " + filePath + " success.!!";
        }
        // stringstream input_stream;
        // input_stream << fin.rdbuf();
        for (int i = 3; i >= 0; --i) {
            // for (int i = 0; i < 4; i++) {
            string str;
            getline(fin, str);
            stringstream input_stream(str);
            // x0 x1 x2 x3 1
            while (input_stream >> str) {
                int j = 0;
                int ans = 0;
                while (isdigit(str[j])) {
                    char c = str[j++];
                    ans = ans * 10 + (c - '0');
                }
                vector<int> words;
                if (str[0] == '+' || (ans % 2 == 0 && ans != 0)) {  // 偶数次消去 '+'跳过
                    continue;
                } else if (str[0] == '1') {
                    words.push_back(4);
                    anf_err_sbox[i].push_back(words);
                    continue;
                } else {
                    while (j < static_cast<int>(str.length())) {
                        while (str[j] == 'x' || str[j] == '*')
                            j++;
                        ans = 0;
                        while (isdigit(str[j])) {
                            char c = str[j++];
                            ans = ans * 10 + (c - '0');
                            // ans = ans * 10 + (3 - c + '0');
                        }
                        words.push_back(ans);
                    }
                    anf_err_sbox[i].push_back(words);
                    continue;
                }
            }
        }
        fin.close();
    }
    return 0;
}

int Util::genCnfFromSbox(int haveError, string &msg) {
    string funcName(__func__);
    if (haveError == NO_ERROR) {
        ifstream fin("./IO/in/CNF/CNF.txt");
        if (!fin.good()) {
            msg = funcName + ":\t open ./IO/in/CNF/CNF.txt fail.!!";
            return 1;
        } else {
            msg = funcName + ":\t open ./IO/in/CNF/CNF.txt success.!!";
        }
        stringstream input_stream;
        input_stream << fin.rdbuf();
        fin.close();
        string str;
        getline(input_stream, str);
        int start = 0;
        while (start < static_cast<int>(str.length())) {
            int l = str.find_first_of('(', start);
            int r = str.find_first_of(')', start);
            if (l == static_cast<int>(string::npos)) {
                break;
            }
            string s = str.substr(l + 1, r - l - 1);
            start = r + 1;
            vector<pair<int, bool>> cnf_clause;
            for (int i = 0; i < static_cast<int>(s.length()); i++) {
                if (s[i] >= '0' && s[i] <= '7') {
                    if (s[i + 1] == '\'') {
                        cnf_clause.push_back(make_pair(s[i] - '0', true));
                    } else {
                        cnf_clause.push_back(make_pair(s[i] - '0', false));
                    }
                }
            }
            cnf_sbox.push_back(cnf_clause);
        }
    }
    if (haveError == HAVE_ERROR) {
        string filePath = "./IO/in/CNF/ERRCNF_" + to_string(this->sbox_error_index) + "_" + to_string(this->sbox_error_position) + ".txt";
        ifstream fin(filePath.c_str());
        if (!fin.good()) {
            msg = funcName + ":\t open " + filePath + " fail.!!";
            return 1;
        } else {
            msg = funcName + ":\t open " + filePath + " success.!!";
        }
        stringstream input_stream;
        input_stream << fin.rdbuf();
        fin.close();
        string str;
        getline(input_stream, str);
        int start = 0;
        while (start < static_cast<int>(str.length())) {
            int l = str.find_first_of('(', start);
            int r = str.find_first_of(')', start);
            if (l == static_cast<int>(string::npos)) {
                break;
            }
            string s = str.substr(l + 1, r - l - 1);
            start = r + 1;
            vector<pair<int, bool>> cnf_clause;
            for (int i = 0; i < static_cast<int>(s.length()); i++) {
                if (s[i] >= '0' && s[i] <= '7') {
                    if (s[i + 1] == '\'') {
                        cnf_clause.push_back(make_pair(s[i] - '0', true));
                    } else {
                        cnf_clause.push_back(make_pair(s[i] - '0', false));
                    }
                }
            }
            cnf_err_sbox.push_back(cnf_clause);
        }
    }
    return 0;
}

void Util::printAnf(int haveError) {
    if (haveError == NO_ERROR) {
        cout << "sbox:\t" << hex;
        for (int i = 0; i < 16; i++) {
            cout << sbox[i] << ' ';
        }
        cout << dec << endl;
        for (int i = 3; i >= 0; i--) {
            cout << "y" << i << " = ";
            for (auto words : anf_sbox[i]) {
                for (auto item : words) {
                    cout << item;
                }
                cout << "  ";
            }
            cout << endl;
        }
    }
    if (haveError == HAVE_ERROR) {
        cout << "e_sbox:\t" << hex;
        for (int i = 0; i < 16; i++) {
            cout << e_sbox[i] << ' ';
        }
        cout << dec << endl;
        for (int i = 3; i >= 0; i--) {
            cout << "y" << i << "\' = ";
            for (auto words : anf_err_sbox[i]) {
                for (auto item : words) {
                    cout << item;
                }
                cout << "  ";
            }
            cout << endl;
        }
    }
}

void Util::printCnf(int haveError) {
    if (haveError == NO_ERROR) {
        cout << "sbox:\t" << hex;
        for (int i = 0; i < 16; i++) {
            cout << sbox[i] << ' ';
        }
        cout << dec << endl;
        cout << "8 = ";
        for (vector<pair<int, bool>> a : cnf_sbox) {
            cout << '(';
            for (int i = 0; i < static_cast<int>(a.size()); i++) {
                if (i > 0)
                    cout << '+';
                cout << a[i].first;
                if (a[i].second)
                    cout << '\'';
            }
            cout << ')';
        }
        cout << endl;
    }
    if (haveError == HAVE_ERROR) {
        cout << "e_sbox:\t" << hex;
        for (int i = 0; i < 16; i++) {
            cout << e_sbox[i] << ' ';
        }
        cout << dec << endl;
        cout << "8 = ";
        for (vector<pair<int, bool>> a : cnf_err_sbox) {
            cout << '(';
            for (int i = 0; i < static_cast<int>(a.size()); i++) {
                if (i > 0)
                    cout << '+';
                cout << a[i].first;
                if (a[i].second)
                    cout << '\'';
            }
            cout << ')';
        }
        cout << endl;
    }
}

int Util::shiftRows(int index) {
    return this->ShiftRowsP[index];
}

bitset<4> Util::S(int index, int have_error) {
    if (have_error == NO_ERROR) {
        return this->sbox[index];
    } else {  // (have_error == HAVE_ERROR) {
        return this->e_sbox[index];
    }
}

int Util::getAnf(int index, std::vector<std::vector<int>> &anfSbox, int have_error, std::string &msg) {
    string funcName(__func__);
    if (index < 0 || index >= ELEMENT_SIZE) {
        msg = funcName + ":\tindex should in 0~3";
        return 1;
    }
    switch (have_error) {
    case NO_ERROR:
        if (this->anf_sbox[index].empty()) {
            msg = funcName + ":\t anf_sbox[" + to_string(index) + "] is empty!";
            return 1;
        }
        anfSbox = this->anf_sbox[index];
        break;

    case HAVE_ERROR:
        if (this->anf_err_sbox[index].empty()) {
            msg = funcName + ":\t anf_err_sbox[" + to_string(index) + "] is empty!";
            return 1;
        }
        anfSbox = this->anf_err_sbox[index];
        break;

    default:
        break;
    }
    return 0;
}

int Util::getCnf(std::vector<std::vector<std::pair<int, bool>>> &cnfSbox, int have_error, std::string &msg) {
    string funcName(__func__);
    switch (have_error) {
    case NO_ERROR:
        if (this->cnf_sbox.empty()) {
            msg = funcName + ":\t cnf_sbox is empty!";
            return 1;
        }
        cnfSbox = this->cnf_sbox;
        break;
    case HAVE_ERROR:
        if (this->cnf_err_sbox.empty()) {
            msg = funcName + ":\t cnf_err_sbox is empty!";
            return 1;
        }
        cnfSbox = this->cnf_err_sbox;

        break;
    default:
        break;
    }
    return 0;
}

int Util::getNumClauses() { return numClauses; }
void Util::setNumClauses(int numClauses_) { numClauses = numClauses_; }
void Util::addClauses() { numClauses++; }
int Util::getSboxErrorIndex() { return sbox_error_index; }
void Util::setSboxErrorIndex(int getSboxErrorIndex) { sbox_error_index = getSboxErrorIndex; }
void Util::getSboxInErrorIndex(std::bitset<4> &V) { V = this->sbox[sbox_error_index]; }
int Util::getSboxErrorPosition() { return sbox_error_position; }
void Util::setSboxErrorPosition(int sboxErrorPosition) { sbox_error_position = sboxErrorPosition; }
/*
 */