# AES-128 from FIPS 197

This is an implementation of AES-128 encryption and decryption in C, written
directly from [NIST FIPS 197-upd1](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.197-upd1.pdf)
without reference to existing cryptographic libraries.

## What this achieves

Most AES implementations hardcode the S-box as a 256-byte table. This one derives it:
the multiplicative inverse in GF(2^8) followed by the affine transform, because the
point of the exercise is the field arithmetic, not the table.

The derived S-box matches FIPS 197 Figure 7.

## Status

- [x] GF(2^8) multiplication and multiplicative inverse
- [x] S-box derivation and inverse S-box
- [ ] Key expansion
- [ ] SubBytes / ShiftRows / MixColumns / AddRoundKey
- [ ] Inverse cipher
- [ ] Verification against FIPS 197 Appendix C test vectors
- [ ] Constant-time verification

## How the S-box is derived

`xtime` doubles a byte in GF(2^8): shift left one bit, and if the high bit was set,
reduce modulo the AES polynomial x^8 + x^4 + x^3 + x + 1 (0x11b) by XORing 0x1b.

`gmul` generalises this. Writing b in binary as the sum of b_i * x^i, the product
a * b is the XOR of the terms a * x^i for each i where b_i is set, and each a * x^i
is i applications of `xtime`.

`build_inv` finds, for every nonzero byte a, the unique b with `gmul(a, b) == 1`.
Zero maps to zero by convention. The field guarantees every other byte has exactly
one inverse.

`affine_transform` applies the FIPS 197 affine map to the inverse:

    s = x ^ rotl(x,1) ^ rotl(x,2) ^ rotl(x,3) ^ rotl(x,4) ^ 0x63

`build_sbox` composes the two and fills the inverse S-box by reversing the mapping.

## Verified so far

- `gmul(0x57, 0x83) == 0xc1` and `gmul(0x57, 0x13) == 0xfe` (FIPS 197 section 4.2)
- multiplicative identity and zero across all 256 values
- `inv[0x53] == 0xca`
- S-box row 0 matches Figure 7: `63 7c 77 7b f2 6b 6f c5 30 01 67 2b fe d7 ab 76`
- `sbox[0x53] == 0xed`, `inv_sbox[0xed] == 0x53`, `sbox[0x00] == 0x63`
- `inv_sbox[sbox[x]] == x` for all 256 values
- the S-box is a bijection

## Build

`src/aes.c` compiles clean with warnings as errors:

```
gcc -std=c17 -O1 -Wall -Wextra -Wpedantic -Werror -c src/aes.c
```

`src/aes.h` declares the public interface: `xtime`, `gmul`, `build_sbox`, and the
SubBytes accessors `sub_byte` / `inv_sub_byte`. The lookup tables and the helpers
that build them are `static` and not reachable from outside the module.

A test harness and an entry point come next.
