#include <bits/stdc++.h>
using namespace std;
bitset<4> s_box[16] = {0xC, 0x5, 0x6, 0xB, 0x9, 0x0, 0xA, 0xD, 0x3, 0xE, 0xF, 0x8, 0x4, 0x7, 0x1, 0x2};
bitset<4> inv_s_box[16] = {0x5, 0xE, 0xF, 0x8, 0xC, 0x1, 0x2, 0xD, 0xB, 0x4, 0x6, 0x3, 0x0, 0x7, 0x9, 0xA};
void addRoundKey(bitset<64> &state, const bitset<80> key) {
    for (int i = 0; i < 64; i++)
        state[63 - i] = state[63 - i] ^ key[79 - i];
}
void SubByte(bitset<64> &state) {
    for (int i = 0; i < 64; i += 4) {
        bitset<4> s = s_box[state[i] + (state[i + 1] << 1) + (state[i + 2] << 2) + (state[i + 3] << 3)];
        state[i] = s[0];
        state[i + 1] = s[1];
        state[i + 2] = s[2];
        state[i + 3] = s[3];
    }
}
void InvSubByte(bitset<64> &state) {
    for (int i = 0; i < 64; i += 4) {
        bitset<4> s = inv_s_box[state[i] + (state[i + 1] << 1) + (state[i + 2] << 2) + (state[i + 3] << 3)];
        state[i] = s[0];
        state[i + 1] = s[1];
        state[i + 2] = s[2];
        state[i + 3] = s[3];
    }
}
void PSub(bitset<64> &state) {
    bitset<64> tmp(0);
    for (int i = 0; i < 63; i++)
        tmp[i * 16 % 63] = state[i];
    tmp[63] = state[63];
    state = tmp;
}
void InvPSub(bitset<64> &state) {
    bitset<64> tmp(0);
    for (int i = 0; i < 63; i++)
        tmp[i] = state[i * 16 % 63];
    tmp[63] = state[63];
    state = tmp;
}
void keyUpdate(bitset<80> &key, bitset<5> rc) {
    key = (key << 61) | (key >> 19);
    bitset<4> s = s_box[(key[79] << 3) + (key[78] << 2) + (key[77] << 1) + key[76]];
    key[79] = s[3];
    key[78] = s[2];
    key[77] = s[1];
    key[76] = s[0];
    key[19] = key[19] ^ rc[4];
    key[18] = key[18] ^ rc[3];
    key[17] = key[17] ^ rc[2];
    key[16] = key[16] ^ rc[1];
    key[15] = key[15] ^ rc[0];
}
bitset<64> encrypt(bitset<64> state, bitset<80> key) {
    for (int RC = 1; RC < 32; RC++) {
        addRoundKey(state, key);
        SubByte(state);
        PSub(state);
        keyUpdate(key, RC);
    }
    addRoundKey(state, key);
    return state;
}
bitset<64> decrypt(bitset<64> state, bitset<80> key) {
    vector<bitset<80> > k;
    k.push_back(key);
    for (int i = 1; i < 32; i++) {
        keyUpdate(key, i);
        k.push_back(key);
    }
    for (int i = 31; i >= 1; i--) {
        addRoundKey(state, k[i]);
        InvPSub(state);
        InvSubByte(state);
    }
    addRoundKey(state, k[0]);
    return state;
}
int main() {
    bitset<64> plain = 0;
    bitset<80> key = 0;
    bitset<64> cipher = encrypt(plain, key);
    cout << "cipher:\t" << cipher << endl;
    cout << "cipher:\t" << hex << cipher.to_ulong() << dec << endl;
    // cout<<decrypt(cipher,key)<<endl;
    return 0;
}
// 5579c1387b228445
