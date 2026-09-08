# AES-128 from FIPS 197

This is an implementation of AES-128 encryption and decryption in C++, written
directly from [NIST FIPS 197-upd1](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.197-upd1.pdf)
without reference to existing cryptographic libraries.

## What this acheives

Most AES implementations hardcode the S-box as a 256-byte table, this attempts to derive it.
Using multiplicative inverse in GF(2^8) followed by the affine transform 
because the point of the exercise is the field arithmetic, not the table.

## Status

- [ ] GF(2^8) multiplication and multiplicative inverse
- [ ] S-box derivation and inverse S-box
- [ ] Key expansion
- [ ] SubBytes / ShiftRows / MixColumns / AddRoundKey
- [ ] Inverse cipher
- [ ] Verification against FIPS 197 Appendix C test vectors
- [ ] Constant-time verification

## Build

```
g++ -std=c++17 -O2 -o aes src/*.cpp
./aes --test
```
