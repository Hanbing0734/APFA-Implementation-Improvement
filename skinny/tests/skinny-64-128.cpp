/*
 * C++ implementation of Skinny-64-128
 * Date: Jan 10, 2020
 * Author: Hosein Hadipour
 * Contact: hsn.hadipour@gmail.com
 */
// Skinny-64-128: 36 rounds

#include <stdint.h>
#include <stdio.h>

#include <iostream>
#include <string>

using namespace std;

// 4-bit Sbox
const uint8_t S[16] = {0xc, 0x6, 0x9, 0x0, 0x1, 0xa, 0x2, 0xb, 0x3, 0x8, 0x5, 0xd, 0x4, 0xe, 0x7, 0xf};
// 4-bit Sbox Inverse
const uint8_t Sinv[16] = {0x3, 0x4, 0x6, 0x8, 0xc, 0xa, 0x1, 0xe, 0x9, 0x2, 0x5, 0x7, 0x0, 0xb, 0xd, 0xf};
// Permutation
const uint8_t P[16] = {0x0, 0x1, 0x2, 0x3, 0x7, 0x4, 0x5, 0x6, 0xa, 0xb, 0x8, 0x9, 0xd, 0xe, 0xf, 0xc};
const uint8_t Pinv[16] = {0x0, 0x1, 0x2, 0x3, 0x5, 0x6, 0x7, 0x4, 0xa, 0xb, 0x8, 0x9, 0xf, 0xc, 0xd, 0xe};
// Tweakey Permutation
const uint8_t Q[16] = {0x9, 0xf, 0x8, 0xd, 0xa, 0xe, 0xc, 0xb, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};
// const uint8_t Qinv[16] = {0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x2, 0x0, 0x4, 0x7, 0x6, 0x3, 0x5, 0x1};
// Round Constants
const uint8_t RC[36] = {0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3E, 0x3D, 0x3B, 0x37, 0x2F,
                        0x1E, 0x3C, 0x39, 0x33, 0x27, 0x0E, 0x1D, 0x3A, 0x35, 0x2B,
                        0x16, 0x2C, 0x18, 0x30, 0x21, 0x02, 0x05, 0x0B, 0x17, 0x2E,
                        0x1C, 0x38, 0x31, 0x23, 0x06, 0x0D};

void print_state(uint8_t state[16]);
void convert_hexstr_to_statearray(string hex_str, uint8_t int_array[16], bool reversed);
uint8_t tweak_tk2_lfsr(uint8_t x);
void mix_columns(uint8_t state[16]);
void inv_mix_columns(uint8_t state[16]);
void tweakey_schedule(int rounds, uint8_t tk1[][16], uint8_t tk2[][16], uint8_t round_tweakey[][8]);
void enc(int R, uint8_t plaintext[16], uint8_t ciphertext[16], uint8_t tk[][8]);
void dec(int R, uint8_t plaintext[16], uint8_t ciphertext[16], uint8_t tk[][8]);

void print_state(uint8_t state[16]) {
    for (int i = 0; i < 16; i++)
        printf("%01x", state[i]);
    printf("\n");
}

void print_ksr(uint8_t state[8]) {
    for (int i = 0; i < 8; i++)
        printf("%01x", state[i]);
    printf("\n");
}
void convert_hexstr_to_statearray(string hex_str, uint8_t int_array[16], bool reversed = false) {
    if (reversed == true)
        for (size_t i = 0; i < 16; i++)
            int_array[15 - i] = static_cast<uint8_t>(stoi(hex_str.substr(i, 1), 0, 16) & 0xf);
    else
        for (size_t i = 0; i < 16; i++)
            int_array[i] = static_cast<uint8_t>(stoi(hex_str.substr(i, 1), 0, 16) & 0xf);
}

uint8_t tweak_tk2_lfsr(uint8_t x) {
    x = (x << 1) ^ ((x >> 3) & 0x1) ^ ((x >> 2) & 0x1);
    x = x & 0xf;
    return x;
}

void mix_columns(uint8_t state[16]) {
    uint8_t tmp;
    for (uint8_t j = 0; j < 4; j++) {
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

void inv_mix_columns(uint8_t state[16]) {
    uint8_t tmp;
    for (uint8_t j = 0; j < 4; j++) {
        tmp = state[j + 4 * 3];
        state[j + 4 * 3] = state[j + 4 * 0];
        state[j + 4 * 0] = state[j + 4 * 1];
        state[j + 4 * 1] = state[j + 4 * 2];
        state[j + 4 * 2] = tmp;
        state[j + 4 * 3] ^= state[j + 4 * 2];
        state[j + 4 * 2] ^= state[j + 4 * 0];
        state[j + 4 * 1] ^= state[j + 4 * 2];
    }
}

void tweakey_schedule(int rounds, uint8_t tk1[][16], uint8_t tk2[][16], uint8_t round_tweakey[][8]) {
    // Declare tweakey after permutation
    uint8_t tkp1[rounds - 1][16];
    uint8_t tkp2[rounds - 1][16];
    for (uint8_t i = 0; i < 16; i++)
        tk1[0][i] = (tk1[0][i] & 0xf);
    for (uint8_t i = 0; i < 16; i++)
        tk2[0][i] = (tk2[0][i] & 0xf);
    for (uint8_t i = 0; i < 8; i++)
        round_tweakey[0][i] = (tk1[0][i] ^ tk2[0][i]);
    for (int r = 1; r < rounds; r++) {
        // Apply tweakey permutation on TK1 and TK2
        for (int i = 0; i < 16; i++) {
            tkp1[r - 1][i] = tk1[r - 1][Q[i]];
            tkp2[r - 1][i] = tk2[r - 1][Q[i]];
        }
        // Apply LFSR on two upper rows of TK2
        for (int i = 0; i < 16; i++) {
            // LFSRs are not performed on TK1 at all
            tk1[r][i] = tkp1[r - 1][i];
            if (i < 8) {
                tk2[r][i] = tweak_tk2_lfsr(tkp2[r - 1][i]);
            } else {
                tk2[r][i] = tkp2[r - 1][i];
            }
        }
        // Update round tweakeys
        for (int i = 0; i < 8; i++)
            round_tweakey[r][i] = (tk1[r][i] ^ tk2[r][i]);
        // printf("\ntweakeys: ");
        // print_state(round_tweakey[r]);
    }
}

void enc(int R, uint8_t plaintext[16], uint8_t ciphertext[16], uint8_t tk[][8]) {
    for (uint8_t i = 0; i < 16; i++) {
        ciphertext[i] = plaintext[i] & 0xf;
    }
    for (uint8_t r = 0; r < R; r++) {
        // SBox
        for (uint8_t i = 0; i < 16; i++)
            ciphertext[i] = S[ciphertext[i]];
        // Add constants (constants only affects on three upper cells of the first column)
        ciphertext[0] ^= (RC[r] & 0xf);
        ciphertext[4] ^= ((RC[r] >> 4) & 0x3);
        ciphertext[8] ^= 0x2;
        // Add round tweakey (tweakey only exclusive-ored with two upper rows of the state)
        for (uint8_t i = 0; i < 8; i++)
            ciphertext[i] ^= tk[r][i];
        // Permute nibbles
        uint8_t temp[16];
        for (uint8_t i = 0; i < 16; i++)
            temp[i] = ciphertext[i];
        for (uint8_t i = 0; i < 16; i++)
            ciphertext[i] = temp[P[i]];
        // MixColumn
        mix_columns(ciphertext);
        // Print state
        // printf("\nR%02d : ", r + 1);
        // print_state(ciphertext);
    }
}

void dec(int R, uint8_t plaintext[16], uint8_t ciphertext[16], uint8_t tk[][8]) {
    for (uint8_t i = 0; i < 16; i++) {
        plaintext[i] = ciphertext[i] & 0xf;
    }
    int ind;
    uint8_t temp[16];
    for (int r = 0; r < R; r++) {
        // MixColumn inverse
        inv_mix_columns(plaintext);
        // Permute nibble inverse
        for (uint8_t i = 0; i < 16; i++)
            temp[i] = plaintext[i];
        for (uint8_t i = 0; i < 16; i++)
            plaintext[i] = temp[Pinv[i]];
        // temp[P[i]] = plaintext[i];
        //  Add tweakey
        ind = R - r - 1;
        for (uint8_t i = 0; i < 8; i++)
            plaintext[i] ^= tk[ind][i];
        // Add constants
        plaintext[0] ^= (RC[ind] & 0xf);
        plaintext[4] ^= ((RC[ind] >> 4) & 0x3);
        plaintext[8] ^= 0x2;
        // SBox inverse
        for (uint8_t i = 0; i < 16; i++)
            plaintext[i] = Sinv[plaintext[i]];
        // Print state
        // printf("\nR%02d : ", r + 1);
        // print_state(plaintext);
    }
}

int main() {
    uint8_t plaintext[16];
    uint8_t ciphertext[16];
    int R = 36;
    uint8_t tk1[R][16];
    uint8_t tk2[R][16];
    uint8_t rtk[R][8];
    uint8_t tweakey1[16];
    uint8_t tweakey2[16];
    // Test vectors
    string tk1_str = "0b98acadc7fdf93d";
    string tk2_str = "cc507116dc9c7307";
    string plain_str = "0000000000000000";
    string cipher_str = "4c3e726936232477";
    bool reversed = false;
    convert_hexstr_to_statearray(tk1_str, tweakey1, reversed);
    convert_hexstr_to_statearray(tk2_str, tweakey2, reversed);
    convert_hexstr_to_statearray(plain_str, plaintext, reversed);
    for (uint8_t i = 0; i < 16; i++) {
        tk1[0][i] = tweakey1[i];
        tk2[0][i] = tweakey2[i];
    }
    tweakey_schedule(R, tk1, tk2, rtk);
    for (int r = 0; r < R; r++) {
        print_ksr(rtk[r]);
    }
    printf("%-30s", "plaintext before encryption:");
    print_state(plaintext);
    enc(R, plaintext, ciphertext, rtk);
    printf("%-30s", "ciphertext:");
    print_state(ciphertext);
    printf("%-30s", "expected ciphertext:");
    printf("%s\n", cipher_str.c_str());
    dec(R, plaintext, ciphertext, rtk);
    printf("%-30s", "plaintext after decryption:");
    print_state(plaintext);
    printf("Press Enter to exit ...\n");
    getchar();
    return 0;
}