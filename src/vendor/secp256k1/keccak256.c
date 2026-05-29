// keccak256.c
// Keccak-256 implementation (Ethereum-compatible)
// This is the pre-FIPS Keccak, not the final SHA-3 standard

#include "keccak256.h"
#include <stdlib.h>

#define KECCAK_ROUNDS 24

// Rotation constants
static const unsigned int keccakf_rotc[24] = {
    1,  3,  6,  10, 15, 21, 28, 36, 45, 55, 2,  14,
    27, 41, 56, 8,  25, 43, 62, 18, 39, 61, 20, 44
};

// Pi lane constants
static const unsigned int keccakf_piln[24] = {
    10, 7,  11, 17, 18, 3, 5,  16, 8,  21, 24, 4,
    15, 23, 19, 13, 12, 2, 20, 14, 22, 9,  6,  1
};

// Round constants
static const uint64_t keccakf_rndc[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
    0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

// Rotate left
static inline uint64_t rotl64(uint64_t x, unsigned int n) {
    return (x << n) | (x >> (64 - n));
}

// Keccak-f[1600] permutation
static void keccakf(uint64_t state[25]) {
    uint64_t t, bc[5];

    for (int round = 0; round < KECCAK_ROUNDS; round++) {
        // Theta
        for (int i = 0; i < 5; i++) {
            bc[i] = state[i] ^ state[i + 5] ^ state[i + 10] ^ state[i + 15] ^ state[i + 20];
        }

        for (int i = 0; i < 5; i++) {
            t = bc[(i + 4) % 5] ^ rotl64(bc[(i + 1) % 5], 1);
            for (int j = 0; j < 25; j += 5) {
                state[j + i] ^= t;
            }
        }

        // Rho Pi
        t = state[1];
        for (int i = 0; i < 24; i++) {
            int j = keccakf_piln[i];
            bc[0] = state[j];
            state[j] = rotl64(t, keccakf_rotc[i]);
            t = bc[0];
        }

        // Chi
        for (int j = 0; j < 25; j += 5) {
            for (int i = 0; i < 5; i++) {
                bc[i] = state[j + i];
            }
            for (int i = 0; i < 5; i++) {
                state[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
            }
        }

        // Iota
        state[0] ^= keccakf_rndc[round];
    }
}

// Main Keccak-256 function
void keccak256(const uint8_t* input, size_t inputLen, uint8_t* output) {
    uint64_t state[25] = {0};
    size_t rate = 136; // 1088 bits = 136 bytes (for Keccak-256)
    size_t blockSize = 0;
    const uint8_t* pos = input;
    size_t remaining = inputLen;

    // Absorb phase
    while (remaining >= rate) {
        for (size_t i = 0; i < rate / 8; i++) {
            state[i] ^= ((uint64_t*)pos)[i];
        }
        keccakf(state);
        pos += rate;
        remaining -= rate;
    }

    // Pad last block
    uint8_t temp[200] = {0};
    memcpy(temp, pos, remaining);
    temp[remaining] = 0x01; // Keccak padding (not SHA-3 0x06!)
    temp[rate - 1] |= 0x80;

    for (size_t i = 0; i < rate / 8; i++) {
        state[i] ^= ((uint64_t*)temp)[i];
    }
    keccakf(state);

    // Squeeze phase (output 32 bytes)
    memcpy(output, state, 32);
}

// Convenience allocator
uint8_t* keccak256_alloc(const uint8_t* input, size_t inputLen) {
    uint8_t* output = (uint8_t*)malloc(32);
    if (output) {
        keccak256(input, inputLen, output);
    }
    return output;
}
