/* Test harness: checks the GF(2^8) and S-box code against FIPS 197 values,
   and the constant-time versions against the reference (branching/table) versions.
   Build and run: make test */
#include <stdio.h>
#include <stdint.h>
#include "aes.h"

static int failures = 0;
#define CHECK(cond, ...) do { if (!(cond)) { failures++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)

int main(void) {
    build_sbox();

    /* FIPS 197 section 4.2 multiplication examples */
    CHECK(gmul(0x57, 0x83) == 0xc1, "gmul(57,83) = %02x, want c1", gmul(0x57, 0x83));
    CHECK(gmul(0x57, 0x13) == 0xfe, "gmul(57,13) = %02x, want fe", gmul(0x57, 0x13));
    /* FIPS 197 section 4.2.1 xtime examples */
    CHECK(xtime(0x57) == 0xae, "xtime(57) = %02x, want ae", xtime(0x57));
    CHECK(xtime(0xae) == 0x47, "xtime(ae) = %02x, want 47", xtime(0xae));
    CHECK(xtime(0x47) == 0x8e, "xtime(47) = %02x, want 8e", xtime(0x47));
    CHECK(xtime(0x8e) == 0x07, "xtime(8e) = %02x, want 07", xtime(0x8e));

    /* identity, zero, and commutativity over every byte pair */
    for (int a = 0; a < 256; a++) {
        CHECK(gmul((uint8_t)a, 1) == a, "gmul(%02x,1)", a);
        CHECK(gmul((uint8_t)a, 0) == 0, "gmul(%02x,0)", a);
        for (int b = 0; b < 256; b++)
            CHECK(gmul((uint8_t)a, (uint8_t)b) == gmul((uint8_t)b, (uint8_t)a), "gmul(%02x,%02x) not commutative", a, b);
    }

    /* S-box row 0 from FIPS 197 Figure 7 */
    static const uint8_t row0[16] = {0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76};
    for (int i = 0; i < 16; i++)
        CHECK(sub_byte((uint8_t)i) == row0[i], "sbox[%02x] = %02x, want %02x", i, sub_byte((uint8_t)i), row0[i]);
    CHECK(sub_byte(0x53) == 0xed, "sbox[53] = %02x, want ed", sub_byte(0x53));
    CHECK(inv_sub_byte(0xed) == 0x53, "inv_sbox[ed] = %02x, want 53", inv_sub_byte(0xed));

    /* bijection and round trip over all 256 inputs */
    int seen[256] = {0};
    for (int x = 0; x < 256; x++) {
        seen[sub_byte((uint8_t)x)]++;
        CHECK(inv_sub_byte(sub_byte((uint8_t)x)) == x, "inv_sbox[sbox[%02x]] != %02x", x, x);
    }
    for (int y = 0; y < 256; y++) CHECK(seen[y] == 1, "sbox output %02x appears %d times", y, seen[y]);

    /* Constant-time versions must agree with the references on every input */
    for (int a = 0; a < 256; a++) {
        CHECK(xtime((uint8_t)a) == refxtime((uint8_t)a), "xtime(%02x) != refxtime", a);
        CHECK(sub_byte((uint8_t)a) == refsub_byte((uint8_t)a), "sub_byte(%02x) = %02x, table says %02x", a, sub_byte((uint8_t)a), refsub_byte((uint8_t)a));
        CHECK(inv_sub_byte((uint8_t)a) == refinv_sub_byte((uint8_t)a), "inv_sub_byte(%02x) = %02x, table says %02x", a, inv_sub_byte((uint8_t)a), refinv_sub_byte((uint8_t)a));
        for (int b = 0; b < 256; b++)
            CHECK(gmul((uint8_t)a, (uint8_t)b) == refgmul((uint8_t)a, (uint8_t)b), "gmul(%02x,%02x) != refgmul", a, b);
    }

    if (failures == 0) printf("All checks passed.\n");
    else printf("%d check(s) failed.\n", failures);
    return failures != 0;
}
