#include "bitops/rot.h"
#include "doctest.h"
#include "sfc64.h"

#include <bitset>
#include <iostream>

#include <nmmintrin.h>

uint16_t mumx16(uint16_t a, uint16_t b) {
    auto m = static_cast<uint32_t>(a) * static_cast<uint32_t>(b);
    m ^= m >> 16;
    return static_cast<uint16_t>(m);
}

// 63.2199% coverage
uint32_t mumx32(uint32_t a, uint32_t b) {
    auto m = static_cast<uint64_t>(a) * static_cast<uint64_t>(b);
    m ^= m >> 32;
    return static_cast<uint32_t>(m);
}

// 63,21% coverage
inline uint32_t mumxmumxx2_32(uint32_t v, uint32_t a, uint32_t b) {
    return mumx32(mumx32(v, a), mumx32(v, b));
}

// 63.21% coverage:
inline uint32_t mumxmumxx3_32(uint32_t v, uint32_t , uint32_t b) {
    return (v ^ rotr(v, 13) ^ rotr(v ^ b, 23));
}

//////////////

// 48% coverage
inline uint16_t wyhash3_mix16(uint16_t v, uint16_t wyp0, uint16_t wyp1, uint16_t wyp4) {
    uint16_t a = static_cast<uint16_t>(v & 0x00ff);
    uint16_t b = static_cast<uint16_t>(v >> 8U);
    return mumx16(mumx16(a ^ wyp0, b ^ wyp1), UINT16_C(8) ^ wyp4);
}

// 48.63% coverage
inline uint32_t wyhash3_mix32(uint32_t v, uint32_t wyp0, uint32_t wyp1, uint32_t wyp4) {
    uint32_t a = static_cast<uint32_t>(v & 0x0000ffff);
    uint32_t b = static_cast<uint32_t>(v >> 16U);
    return mumx32(mumx32(a ^ wyp0, b ^ wyp1), UINT16_C(8) ^ wyp4);
}

// 39.34% coverage
inline uint32_t wyhash3_rand(uint32_t v, uint32_t wyp0) {
    return mumx32(v ^ wyp0, v);
}

//////

// 100% coverage
inline uint32_t fmix32(uint32_t h) noexcept {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

///////

// 74.69% coverage
inline uint32_t lemire_stronglyuniversal32(uint32_t x, uint32_t k1, uint32_t k2, uint32_t k3,
                                           uint32_t k4, uint32_t k5, uint32_t k6) noexcept {
    uint32_t lo = x & UINT32_C(0x0000ffff);
    uint32_t hi = x >> 16;

    uint32_t r1 = (k1 * lo + k2 * hi + k3) >> 16;
    uint32_t r2 = (k4 * lo + k5 * hi + k6) >> 16;
    return (r1 << 16) | r2;
}

// 100% coverage
inline uint32_t crc32(uint32_t v) noexcept {
    return static_cast<uint32_t>(_mm_crc32_u64(0, v));
}

// 100% coverage
inline uint32_t xorshift(uint32_t h) noexcept {
    h ^= h >> 2;
    return h;
}

// 100% coverage
inline uint32_t xorshift2(uint32_t h, uint32_t a) noexcept {
    return a ^ h ^ (h >> 17);
}

inline uint32_t rotrxx(uint32_t x) noexcept {
    return x ^ rotr(x, 25) ^ rotr(x, 13);
}

// 50% coverage
inline uint32_t rotrx(uint32_t x) noexcept {
    return x ^ rotr(x, 25);
}

TEST_CASE("coverage") {
    std::cout << std::hex << rotr(0xe7037ed1a0b428db , 47) << std::endl;
    
    // can't allocate bitset on the stack => segfault
    static constexpr size_t Size = UINT64_C(1) << 32;
    auto bits = new std::bitset<Size>();

    sfc64 rng;
    auto k1 = static_cast<uint32_t>(rng() | 1);
    auto k2 = static_cast<uint32_t>(rng() | 1);
#if 0    
    auto k3 = static_cast<uint16_t>(rng() | 1);

    auto k4 = static_cast<uint32_t>(rng() | 1);
    auto k5 = static_cast<uint32_t>(rng() | 1);
    auto k6 = static_cast<uint16_t>(rng() | 1);
#endif

    for (size_t i = 0; i < Size; ++i) {
        // bits->set(mumx32(i, k1));
        // bits->set(wyhash3_mix32(i, k1, k2, k3));
        // bits->set(fmix32(i));
        // bits->set(lemire_stronglyuniversal32(i, k1, k2, k3, k4, k5, k6));
        // bits->set(xorshift2(i, k1));
        bits->set(mumxmumxx3_32(i, k1, k2));
        // bits->set(wyhash3_rand(i, k1));
    }

    auto ratio = (100.0 * static_cast<double>(bits->count()) / static_cast<double>(Size));
    std::cout << ratio << "% coverage (" << bits->count() << " of " << Size << ")" << std::endl;
}
