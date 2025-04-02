
#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <vector>

using namespace std;

#define KEY_TEST "10001011101111001111001101111100101011001000011111000100001100111010100101100010010011110011111001000111011010001100111110111000"

int main(int argc, char const *argv[]) {
    string key_test(KEY_TEST);

    cout << key_test << endl;

    bitset<128> master_key;  // 主密钥
    int ind = 0;
    for (int index = 0; index < 128; index++) {
        // master_key[index] = 0;
        master_key[index] = key_test[ind++] - '0';
    }

    for (int index = 0; index < 128; index++) {
        cout << master_key[index];
    }
    cout << endl;
    return 0;
}
