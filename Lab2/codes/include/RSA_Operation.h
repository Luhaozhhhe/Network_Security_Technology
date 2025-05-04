// RSA操作，用于生成密钥、加解密
#ifndef ENCCHAT_RSA_H
#define ENCCHAT_RSA_H

#include <cstdint>
#include <random>

// 定义128位整型（注意：不是所有编译器都支持）
#define uint128_t __uint128_t
#define int128_t __int128_t

class RSA {
private:
    uint64_t p;     // 素数 p
    uint64_t q;     // 素数 q
    uint64_t n;     // 模数 n = p * q
    uint64_t phi;   // 欧拉函数 φ(n) = (p-1)*(q-1)
    uint64_t e;     // 公钥指数 e
    uint64_t d;     // 私钥指数 d

    // 模幂运算：计算 (base^exp) mod mod
    static uint64_t ModExp(uint64_t base, uint64_t exp, uint64_t mod);
    // 模逆运算：计算 a 关于模 m 的逆元
    static uint64_t ModInv(uint64_t a, uint64_t m);
    // Miller-Rabin 素性测试：检测 n 是否为质数，默认测试轮数为50次
    static bool MillerRabin(uint64_t n, int round = 50);

public:
    RSA();    // 构造函数，初始化密钥各项为0
    ~RSA();   // 析构函数

    // 生成公钥和私钥，返回true表示生成成功
    bool GenerateKey();

    // 获取公钥 e
    inline uint64_t GetPublicKey() { return e; };

    // 获取模数 n
    inline uint64_t GetModulus() { return n; };

    // 静态加密函数：使用公钥 e 对明文进行加密
    static uint64_t Encrypt(uint32_t plainText, uint64_t e, uint64_t n);

    // 使用私钥解密密文，返回解密后的明文
    uint32_t Decrypt(uint64_t cipherText);
    
    // 打印当前配置（密钥、模数等）
    void PrintConfig();
};

#endif