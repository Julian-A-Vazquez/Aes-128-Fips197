#ifndef AES_H
#define AES_H
#include <stdint.h>

/* Constant-time GF(2^8) arithmetic (no secret-dependent branches) */
uint8_t xtime(uint8_t b);
uint8_t gmul(uint8_t a, uint8_t b);

/* Constant-time S-box: x^254 then affine, no table lookups */
uint8_t sub_byte(uint8_t b);
uint8_t inv_sub_byte(uint8_t b);

/* Reference versions (branching xtime/gmul, table S-box), kept for testing.
   build_sbox() must run before refsub_byte / refinv_sub_byte. */
uint8_t refxtime(uint8_t b);
uint8_t refgmul(uint8_t a, uint8_t b);
void build_sbox(void);
uint8_t refsub_byte(uint8_t b);
uint8_t refinv_sub_byte(uint8_t b);

#endif
