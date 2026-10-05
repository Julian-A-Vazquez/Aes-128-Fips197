#include <stdint.h>   /* uint8_t, uint32_t */
#include <string.h>   /* memcpy, memset */
#include "aes.h"
//Equation that doubles bytes by shifting them left
uint8_t xtime(uint8_t b){
    uint8_t shifted = (uint8_t)(b << 1);
    uint8_t reduced = (uint8_t)((b & 0x80) ? 0x1b : 0x00);
    return (uint8_t)(shifted ^ reduced);
}
uint8_t gmul(uint8_t a, uint8_t b){
    int i;
    uint8_t result = 0x00;
    for (i = 0; i < 8; i++){
        if(b & 1){
            result ^= a;
        }
        b >>= 1;
        a = xtime(a);
    }
    return result;
} 
static uint8_t inv[256];
void build_inv(void){
    inv[0] = 0;
    int i;
    int j;
    for (i = 1; i < 256;i++){
        for (j = 1; j < 256;j++){
         if (gmul(i, j) == 1){
            inv[i] = j;
            break;
            }
        }
}
}
static uint8_t rotl8(uint8_t x, int n) {
    return (uint8_t)((x << n) | (x >> (8 - n)));
}
static uint8_t affine_transform(uint8_t x){
    return (uint8_t) (x ^ rotl8(x, 1) ^ rotl8(x, 2)
    ^ rotl8(x, 3) ^ rotl8(x, 4) ^ 0x63);
}
//
static uint8_t sbox[256];
static uint8_t inv_sbox[256];
void build_sbox(void){
    int i;
    build_inv();
    for (i = 0; i<256;i++){
    sbox[i] = affine_transform(inv[i]);
    inv_sbox[sbox[i]] = i;
    }
}
uint8_t sub_byte(uint8_t b){ return sbox[b]; }
uint8_t inv_sub_byte(uint8_t b){ return inv_sbox[b]; }

