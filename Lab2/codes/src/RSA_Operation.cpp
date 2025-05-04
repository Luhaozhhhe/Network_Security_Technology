#include "RSA_Operation.h"
#include <cassert>
#include <iostream>

uint64_t RSA::ModExp(uint64_t base, uint64_t exp, uint64_t mod) {
    base = base % mod;
    uint64_t idx = (1LL << 63);
    while (!(exp & idx)) {
        idx >>= 1;
    }

    uint128_t result = 1;
    while (idx) {
        result = (uint128_t)((result * result) % mod);
        if (exp & idx) {
            result = (uint128_t)((result * base) % mod);
        }
        idx >>= 1;
    }

    return (uint64_t)result;
}

uint64_t RSA::ModInv(uint64_t a, uint64_t m) {

    assert(a < m);

    int128_t r0 = m, r = a;
    int128_t q = -1;
    int128_t s0 = 1, s = 0;
    int128_t t0 = 0, t = 1;

    while (r0 % r) {
        int128_t tmp = r0;
        r0 = r;
        r = tmp % r0;

        q = tmp / r0;

        tmp = s0;
        s0 = s;
        s = tmp - s0 * q;

        tmp = t0;
        t0 = t;
        t = tmp - t0 * q;
    }

    if (r == 1) {
        if (s < 0)
            s += a;
        if (t < 0)
            t += m;
        return t;
    }

    return 0;
}

bool RSA::MillerRabin(uint64_t n, int round) {
    if (n == 2) return true;
    if (n < 2 || (n & 1) == 0) return false;

    uint64_t m = n - 1;
    uint64_t k = 0;
    while ((m & 1) == 0) {
        m >>= 1;
        k++;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(2, n - 1);

    while (round--) {
        uint64_t a = dis(gen);
        uint64_t b = ModExp(a, m, n);
        if (b == 1 || b == n - 1) continue;

        for (int i = 0; i < k; i++) {
            b = ModExp(b, 2, n);
            if ((b == n - 1) && (i < k - 1)) {
                b = 1;
                break;
            }
            if (b == 1) return false;
        }

        if (b != 1) return false;
    }
    return true;
}


RSA::RSA() {
    p = 0; q = 0; n = 0; phi = 0; e = 0; d = 0;
}

RSA::~RSA() = default;

bool RSA::GenerateKey() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0x20000000, 0xFFFFFFFF);

    const int MAX_ROUND = 50;
    do {
        p = dis(gen);
    } while (!MillerRabin(p, MAX_ROUND));

    do {
        q = dis(gen);
    } while (!MillerRabin(q, MAX_ROUND));

    n = p * q;
    phi = (p - 1) * (q - 1);

    e = 65537;
    while (e < phi) {

        if ((phi % e != 0) && (MillerRabin(e, MAX_ROUND))) {
            break;
        }
        e += 2;
    }

    if (e >= phi) {

        p = 0; q = 0; n = 0; phi = 0; e = 0; d = 0;
        return false;
    }

    d = ModInv(e, phi);
    return true;
}

uint64_t RSA::Encrypt(uint32_t plainText, uint64_t e, uint64_t n) {
    return ModExp((uint64_t)plainText, e, n);
}

uint32_t RSA::Decrypt(uint64_t cipherText) {
    return (uint32_t)ModExp(cipherText, d, n);
}

// 打印RSA的具体配置信息
void RSA::PrintConfig() {
    std::cout << "RSA 配置数据：" << std::endl;
    std::cout << "p (素数): " << p << std::endl;
    std::cout << "q (素数): " << q << std::endl;
    std::cout << "n (模数): " << n << std::endl;
    std::cout << "phi: " << phi << std::endl;
    std::cout << "e (公钥指数): " << e << std::endl;
    std::cout << "d (私钥指数): " << d << std::endl;
}