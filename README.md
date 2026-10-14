# AES-128 from FIPS 197

This is an implementation of AES-128 encryption and decryption in C, written
directly from [NIST FIPS 197-upd1](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.197-upd1.pdf)
without reference to existing cryptographic libraries.

## What this achieves

Most AES implementations hardcode the S-box as a 256-byte table. This one derives it:
the multiplicative inverse in GF(2^8) followed by the affine transform, because the
point of the exercise is the field arithmetic, not the table.

The derived S-box matches FIPS 197 Table 4 (Figure 7 in the original 2001 edition).

## Status

- [x] GF(2^8) multiplication and multiplicative inverse
- [x] S-box derivation and inverse S-box
- [x] Constant-time `xtime`, `gmul`, S-box and inverse S-box (no secret-dependent branches or table lookups)
- [ ] Key expansion
- [ ] SubBytes / ShiftRows / MixColumns / AddRoundKey
- [ ] Inverse cipher
- [ ] Verification against the AES-128 cipher example in FIPS 197 Appendix B
- [ ] Constant-time verification of the full cipher (assembly check done for the S-box layer; dudect not yet run)

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

## Constant time

A table lookup like `sbox[b]` compiles to a load whose address depends on `b`. When `b` is
derived from the key, which cache lines get touched depends on the key, and an attacker who
can time the cipher can recover it (Bernstein, 2005). Branches on secret bits leak the same
way through timing. So the functions the cipher calls avoid both:

- `xtime` and `gmul` select with masks instead of `if`/`?:`. `(uint8_t)(-(bit))` is `0xFF`
  when the bit is 1 and `0x00` when it is 0 (two's complement), so `value & mask` either keeps
  `value` or zeroes it, with the same instructions either way.
- `sub_byte` computes the inverse as b^254 instead of looking it up. In GF(2^8), b^255 = 1 for
  every nonzero b, so b^254 = b^-1, and 0^254 = 0 matches the convention for zero. The loop runs
  a fixed 253 multiplications for every input, followed by the affine transform.
- `inv_sub_byte` undoes the affine transform first, `rotl(s,1) ^ rotl(s,3) ^ rotl(s,6) ^ 0x05`,
  then takes b^254.

The original branching and table versions are kept as `refxtime`, `refgmul`, `refsub_byte` and
`refinv_sub_byte`. They are only used by the tests, as the known-good answer the constant-time
versions have to match.

Checked in the assembly (`make asm`, gcc -O0 and -O2): the only branches left in these functions
are the fixed loop counters (8 and 253 iterations), and the only indexed loads in the file are in
`build_sbox` and the `ref` lookups. At -O2 gcc compiles the mask in `xtime` to `sarb $7; andl $27`.

This costs speed. The b^254 S-box takes about 2.6 microseconds per byte against a few nanoseconds
for the table, because 253 multiplications is the most direct way to reach b^254, not the fastest.
Square-and-multiply would cut it to around a dozen.

## Verified so far

- `gmul(0x57, 0x13) == 0xfe` (FIPS 197 section 4.2, Eq. 4.7) and `gmul(0x57, 0x83) == 0xc1` (section 4.2 of the 2001 edition)
- multiplicative identity and zero across all 256 values
- `inv[0x53] == 0xca`
- S-box row 0 matches Table 4: `63 7c 77 7b f2 6b 6f c5 30 01 67 2b fe d7 ab 76`
- `sbox[0x53] == 0xed`, `inv_sbox[0xed] == 0x53`, `sbox[0x00] == 0x63`
- `inv_sbox[sbox[x]] == x` for all 256 values
- the S-box is a bijection
- constant-time `xtime` and the S-box / inverse S-box match the reference versions on all 256 inputs
- constant-time `gmul` matches the reference on all 65,536 input pairs
- the suite passes at gcc -O0, -O1, -O2 and -O3 and clang -O2, under AddressSanitizer and UBSan

## Build

```
make test
```

builds `src/aes.c` with the test harness in `tests/` under
`-std=c17 -Wall -Wextra -Wpedantic -Werror` plus AddressSanitizer and UBSan, and runs it.
`make asm` writes the -O2 assembly to `aes-O2.s` for inspection.
