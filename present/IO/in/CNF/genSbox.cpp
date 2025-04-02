/*
 * @Author: Hanbing0734
 * @Date: 2024-09-27 10:38:08
 * @LastEditors: Hanbing
 * @LastEditTime: 2024-10-11 09:50:36
 * @FilePath: /SAT/present/IO/in/CNF/genSbox.cpp
 * @Description:
 *
 * Copyright (c) 2024 by Hanbing0734, All Rights Reserved.
 */
#include <algorithm>
#include <array>
#include <bitset>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

// using std::bitset;
using namespace std;

#define ELEMENT_SIZE 4
#define SBOX_SIZE (ELEMENT_SIZE * ELEMENT_SIZE)
#define BLOCK_SIZE 64

void getSboxcsv(const std::array<std::bitset<ELEMENT_SIZE>, SBOX_SIZE> &sbox);
void injectError(const std::array<std::bitset<ELEMENT_SIZE>, SBOX_SIZE> &sbox, int error_index, int error_position);

int main(int argc, char const *argv[]) {
// 设置控制台的编码为UTF-8（在Windows上）
#ifdef _WIN32
    system("chcp 65001");
#endif

    const std::array<std::bitset<ELEMENT_SIZE>, SBOX_SIZE> sbox = {0xC, 0x5, 0x6, 0xB, 0x9, 0x0, 0xA, 0xD, 0x3, 0xE, 0xF, 0x8, 0x4, 0x7, 0x1, 0x2};
    int error_index, error_position;
    getSboxcsv(sbox);
    for (int i = 0; i < SBOX_SIZE; i++) {
        for (int j = 0; j < ELEMENT_SIZE; j++) {
            injectError(sbox, i, j);
        }
    }

    return 0;
}

void getSboxcsv(const std::array<std::bitset<ELEMENT_SIZE>, SBOX_SIZE> &sbox) {
    std::string directoryPath = "./sbox_csv";
    string filePath = directoryPath + "/sbox.csv";
    // 创建目录
    // std::filesystem::create_directory(directoryPath);
    ofstream fout(filePath, ios::out);  // 以文本模式打开data.txt
    for (int i = ELEMENT_SIZE - 1; i >= 0; i--) {
        fout << i << ',';
    }
    for (int i = ELEMENT_SIZE - 1; i >= 0; i--) {
        fout << i + ELEMENT_SIZE << ',';
    }
    fout << "," << ELEMENT_SIZE + ELEMENT_SIZE << endl;

    for (int index = 0; index < SBOX_SIZE; index++) {
        bitset<ELEMENT_SIZE> pos = index;
        for (int i = ELEMENT_SIZE - 1; i >= 0; i--) {
            fout << pos[i] << ',';
        }
        for (int i = ELEMENT_SIZE - 1; i >= 0; i--) {
            fout << sbox[index][i] << ',';
        }
        fout << ",1" << endl;
    }
    fout.close();
    return;
}
void injectError(const std::array<std::bitset<ELEMENT_SIZE>, SBOX_SIZE> &sbox, int error_index, int error_position) {
    std::array<std::bitset<ELEMENT_SIZE>, SBOX_SIZE> e_sbox = sbox;
    e_sbox[error_index].flip(error_position);

    // string filePath = "./ERRCNF_" + to_string(error_index) + "_" + to_string(error_position) + ".txt";
    // ofstream file(filePath, ios::out);  // 以文本模式打开data.txt
    // file.close();

    std::string directoryPath = "./sbox_csv";
    string filePath = directoryPath + "/esbox_" + to_string(error_index) + "_" + to_string(error_position) + ".csv";
    // 创建目录
    // std::filesystem::create_directory(directoryPath);
    ofstream fout(filePath, ios::out);  // 以文本模式打开data.txt
    for (int i = ELEMENT_SIZE - 1; i >= 0; i--) {
        fout << i << ',';
    }
    for (int i = ELEMENT_SIZE - 1; i >= 0; i--) {
        fout << i + ELEMENT_SIZE << ',';
    }
    fout << "," << ELEMENT_SIZE + ELEMENT_SIZE << endl;

    for (int index = 0; index < SBOX_SIZE; index++) {
        bitset<ELEMENT_SIZE> pos = index;
        for (int i = ELEMENT_SIZE - 1; i >= 0; i--) {
            fout << pos[i] << ',';
        }
        for (int i = ELEMENT_SIZE - 1; i >= 0; i--) {
            fout << e_sbox[index][i] << ',';
        }
        fout << ",1" << endl;
    }
    fout.close();
    return;
}
