#include "DES_Operation.h"
#include <random>
#include <cstdint>

// Initial Permutation
const uint8_t DesOp::IP[64] = {
    58, 50, 42, 34, 26, 18, 10, 2,
    60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6,
    64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17, 9, 1,
    59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5,
    63, 55, 47, 39, 31, 23, 15, 7
};

// Inverse Initial Permutation
const uint8_t DesOp::IP_INV[64] = {
    40, 8, 48, 16, 56, 24, 64, 32,
    39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30,
    37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28,
    35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26,
    33, 1, 41, 9, 49, 17, 57, 25
};

// Expansion Permutation (E-Box)
const uint8_t DesOp::E[48] = {
    32, 1, 2, 3, 4, 5,
    4, 5, 6, 7, 8, 9,
    8, 9, 10, 11, 12, 13,
    12, 13, 14, 15, 16, 17,
    16, 17, 18, 19, 20, 21,
    20, 21, 22, 23, 24, 25,
    24, 25, 26, 27, 28, 29,
    28, 29, 30, 31, 32, 1
};

// S-Boxes
const uint8_t DesOp::S[8][4][16] = {
    {
        {14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7},
        {0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8},
        {4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0},
        {15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13}
    },
    {
        {15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10},
        {3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5},
        {0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15},
        {13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9}
    },
    {
        {10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8},
        {13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1},
        {13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7},
        {1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12}
    },
    {
        {7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15},
        {13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9},
        {10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4},
        {3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14}
    },
    {
        {2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9},
        {14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6},
        {4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14},
        {11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3}
    },
    {
        {12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11},
        {10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8},
        {9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6},
        {4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13}
    },
    {
        {4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1},
        {13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6},
        {1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2},
        {6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12}
    },
    {
        {13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7},
        {1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2},
        {7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8},
        {2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11}
    }
};

// Permutation (P-Box)
const uint8_t DesOp::P[32] = {
    16, 7, 20, 21, 29, 12, 28, 17,
    1, 15, 23, 26, 5, 18, 31, 10,
    2, 8, 24, 14, 32, 27, 3, 9,
    19, 13, 30, 6, 22, 11, 4, 25
};

// Permuted Choice 1
const uint8_t DesOp::PC1[2][28] = {
    {
        57, 49, 41, 33, 25, 17, 9,
        1, 58, 50, 42, 34, 26, 18,
        10, 2, 59, 51, 43, 35, 27,
        19, 11, 3, 60, 52, 44, 36
    },
    {
        63, 55, 47, 39, 31, 23, 15,
        7, 62, 54, 46, 38, 30, 22,
        14, 6, 61, 53, 45, 37, 29,
        21, 13, 5, 28, 20, 12, 4
    }
};

// Left Shifts
const uint8_t DesOp::LS[16] = {
    1, 1, 2, 2, 2, 2, 2, 2,
    1, 2, 2, 2, 2, 2, 2, 1
};

// Permuted Choice 2
const uint8_t DesOp::PC2[48] = {
    14, 17, 11, 24, 1, 5, 3, 28,
    15, 6, 21, 10, 23, 19, 12, 4,
    26, 8, 16, 7, 27, 20, 13, 2,
    41, 52, 31, 37, 47, 55, 30, 40,
    51, 45, 33, 48, 44, 49, 39, 56,
    34, 53, 46, 42, 50, 36, 29, 32
};

void DesOp::Xor(uint8_t* a, uint8_t* b, int length) {
    for (int i = 0; i < length; i++) {
        a[i] ^= b[i];
    }
}

void DesOp::Copy(uint8_t* a, uint8_t* b, int length) {
    for (int i = 0; i < length; i++) {
        a[i] = b[i];
    }
}

void DesOp::ByteToBit(uint8_t* byte, uint8_t* bit, int length) {
    for (int i = 0; i < length; i++) {
        for (int j = 0; j < 8; j++) {
            bit[i * 8 + j] = (byte[i] >> (7 - j)) & 0x01;
        }
    }
}

void DesOp::BitToByte(uint8_t* bit, uint8_t* byte, int length) {
    for (int i = 0; i < length; i++) {
        byte[i] = 0;
        for (int j = 0; j < 8; j++) {
            byte[i] |= bit[i * 8 + j] << (7 - j);
        }
    }
}

DesOp::DesOp() {}


void DesOp::RandomGenKey() {
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<unsigned int> dist(0, 255);

    for (int i = 0; i < 8; i++) {
        key[i] = (uint8_t)dist(engine);
    }
    GenerateSubKeys();
}

uint8_t* DesOp::GetKey() {
    auto* key_copy = new uint8_t[8];
    Copy(key_copy, key, 8);
    return key_copy;
}


void DesOp::SetKey(const char* key) {
    for (int i = 0; i < 8; i++) {
        this->key[i] = key[i];
    }
    GenerateSubKeys();
}

void DesOp::GenerateSubKeys() {
    uint8_t key_bit[64];
    ByteToBit(key, key_bit, 8);
    uint8_t key56L[28], key56R[28];
    for (int i = 0; i < 28; i++) {
        key56L[i] = key_bit[PC1[0][i] - 1];
        key56R[i] = key_bit[PC1[1][i] - 1];
    }

    uint8_t subKey[48];
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < LS[i]; j++) {
            uint8_t tempL = key56L[0], tempR = key56R[0];
            for (int k = 0; k < 27; k++) {
                key56L[k] = key56L[k + 1];
                key56R[k] = key56R[k + 1];
            }
            key56L[27] = tempL;
            key56R[27] = tempR;
        }

        int index;
        for (int j = 0; j < 48; j++) {
            index = PC2[j] - 1;
            if (index < 28) {
                subKey[j] = key56L[index];
            } else {
                subKey[j] = key56R[index - 28];
            }
        }
        BitToByte(subKey, subKeys[i], 6);
    }
}

void DesOp::F(uint8_t* R, uint8_t* subKey, uint8_t* result) {
    uint8_t R_exp[48];
    for (int i = 0; i < 48; i++) {
        R_exp[i] = R[E[i] - 1];
    }

    uint8_t subKey_bit[48];
    ByteToBit(subKey, subKey_bit, 6);
    Xor(R_exp, subKey_bit, 48);

    uint8_t S_out[32];
    for (int i = 0; i < 8; i++) {
        int row = R_exp[i * 6] * 2 + R_exp[i * 6 + 5];
        int col = (R_exp[i * 6 + 1] << 3) + (R_exp[i * 6 + 2] << 2) + (R_exp[i * 6 + 3] << 1) + R_exp[i * 6 + 4];
        uint8_t val = S[i][row][col];
        for (int j = 0; j < 4; j++) {
            S_out[i * 4 + j] = (val >> (3 - j)) & 0x01;
        }
    }

    for (int i = 0; i < 32; i++) {
        result[i] = S_out[P[i] - 1];
    }
}

void DesOp::DES(uint8_t* plainText_byte, uint8_t* cipherText_byte, bool isEncrypt) {
    uint8_t plainText[64], cipherText[64];
    ByteToBit(plainText_byte, plainText, 8);
    ByteToBit(cipherText_byte, cipherText, 8);

    uint8_t plainText_bit[64];
    for (int i = 0; i < 64; i++) {
        plainText_bit[i] = plainText[IP[i] - 1];
    }

    uint8_t L[32], R[32];
    for (int i = 0; i < 32; i++) {
        L[i] = plainText_bit[i];
        R[i] = plainText_bit[i + 32];
    }

    uint8_t temp[32];
    for (int i = 0; i < 16; i++) {
        Copy(temp, R, 32);
        F(R, subKeys[isEncrypt ? i : (15 - i)], R);
        Xor(R, L, 32);
        Copy(L, temp, 32);
    }

    for (int i = 0; i < 32; i++) {
        plainText_bit[i] = R[i];
        plainText_bit[i + 32] = L[i];
    }

    for (int i = 0; i < 64; i++) {
        cipherText[i] = plainText_bit[IP_INV[i] - 1];
    }

    BitToByte(plainText, plainText_byte, 8);
    BitToByte(cipherText, cipherText_byte, 8);
}

void DesOp::Encrypt(char* plainText, int plainTextLength, char*& cipherText, int& cipherTextLength) {
    int padding = 8 - plainTextLength % 8;
    cipherTextLength = plainTextLength + padding;
    cipherText = new char[cipherTextLength];
    for (int i = 0; i < plainTextLength; i++) {
        cipherText[i] = plainText[i];
    }
    for (int i = plainTextLength; i < cipherTextLength; i++) {
        cipherText[i] = padding;
    }

    uint8_t plainTextBlock[8], cipherTextBlock[8];
    for (int i = 0; i < cipherTextLength; i += 8) {
        for (int j = 0; j < 8; j++) {
            plainTextBlock[j] = cipherText[i + j];
        }
        DES(plainTextBlock, cipherTextBlock, true);
        for (int j = 0; j < 8; j++) {
            cipherText[i + j] = cipherTextBlock[j];
        }
    }
}

void DesOp::Decrypt(char* cipherText, int cipherTextLength, char*& plainText, int& plainTextLength) {
    uint8_t plainTextBlock[8], cipherTextBlock[8];
    for (int i = 0; i < cipherTextLength; i += 8) {
        for (int j = 0; j < 8; j++) {
            cipherTextBlock[j] = cipherText[i + j];
        }
        DES(cipherTextBlock, plainTextBlock, false);
        for (int j = 0; j < 8; j++) {
            cipherText[i + j] = plainTextBlock[j];
        }
    }

    int padding = cipherText[cipherTextLength - 1];
    plainTextLength = cipherTextLength - padding;
    plainText = new char[plainTextLength + 1];
    for (int i = 0; i < plainTextLength; i++) {
        plainText[i] = cipherText[i];
    }
    plainText[plainTextLength] = '\0';
}