#ifndef DES_CHAT_DESOP_H
#define DES_CHAT_DESOP_H

#include <cstdint>

class DesOp {
private:
    uint8_t key[8] = {0};
    uint8_t subKeys[16][6] = {0};

    static const uint8_t IP[64];
    static const uint8_t IP_INV[64];
    static const uint8_t E[48];
    static const uint8_t S[8][4][16];
    static const uint8_t P[32];
    static const uint8_t PC1[2][28];
    static const uint8_t LS[16];
    static const uint8_t PC2[48];

    void GenerateSubKeys();
    void F(uint8_t* R, uint8_t* subKey, uint8_t* result);
    void DES(uint8_t* plainText_byte, uint8_t* cipherText_byte, bool isEncrypt);

    static void Xor(uint8_t* a, uint8_t* b, int length);
    static void Copy(uint8_t* a, uint8_t* b, int length);
    static void ByteToBit(uint8_t* byte, uint8_t* bit, int length);
    static void BitToByte(uint8_t* bit, uint8_t* byte, int length);

public:
    DesOp();
    ~DesOp() = default;
    void SetKey(const char* key);
    void Encrypt(char* plainText, int plainTextLength, char*& cipherText, int& cipherTextLength);
    void Decrypt(char* cipherText, int cipherTextLength, char*& plainText, int& plainTextLength);
};

#endif