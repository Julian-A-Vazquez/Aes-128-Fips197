#ifndef AES_H
#define AES_H
#include <stdint.h>

uint8_t xtime(uint8_t b);
uint8_t gmul(uint8_t a, uint8_t b);

void build_sbox(void);

uint8_t sub_byte(uint8_t b);
uint8_t inv_sub_byte(uint8_t b);

#endif