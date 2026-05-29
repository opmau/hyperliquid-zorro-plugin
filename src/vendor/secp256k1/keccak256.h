// keccak256.h
// Standalone Keccak-256 implementation for Ethereum-compatible hashing
// Based on the standard Keccak specification (SHA-3 finalist)
// Note: Ethereum uses Keccak, not the final SHA-3 standard

#ifndef KECCAK256_H
#define KECCAK256_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// Keccak-256 hash function (Ethereum-compatible)
// Outputs 32 bytes
void keccak256(const uint8_t* input, size_t inputLen, uint8_t* output);

// Convenience function that returns hash as a new allocated buffer
// Caller must free the returned buffer
uint8_t* keccak256_alloc(const uint8_t* input, size_t inputLen);

#ifdef __cplusplus
}
#endif

#endif // KECCAK256_H
