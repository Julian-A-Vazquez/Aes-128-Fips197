#include <stdint.h>   /* uint8_t, uint32_t */
#include <string.h>   /* memcpy, memset */
#include "aes.h"
uint8_t xtime(uint8_t b){
    uint8_t shifted = (uint8_t)(b << 1); 
    uint8_t reduced = (uint8_t)((-(b >> 7 & 1)));
    uint8_t masked = (uint8_t)((reduced & 0x1b));
    return (uint8_t)(shifted ^ masked); 
}

uint8_t gmul(uint8_t a, uint8_t b){
    int i;
    uint8_t result = 0x00;
    for (i = 0; i < 8; i++){ 
        uint8_t mask =(-(b & 1));
             result ^= (a & mask); 
        b >>= 1; 
        a = xtime(a); 
    }
    return result;
}

uint8_t refxtime(uint8_t b){
    uint8_t shifted = (uint8_t)(b << 1);
    uint8_t reduced = (uint8_t)((b & 0x80) ? 0x1b : 0x00);
    return (uint8_t)(shifted ^ reduced);
}
uint8_t refgmul(uint8_t a, uint8_t b){
    int i;
    uint8_t result = 0x00;
    for (i = 0; i < 8; i++){
        if(b & 1){
            result ^= a;
        }
        b >>= 1;
        a = refxtime(a);
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
static uint8_t compb254(uint8_t b){
    int i; 
    const uint8_t b1 = b; // OK: the original b, copied once.
    for(i = 0; i<253; i++){
        b = gmul(b, b1); // OK: exponent goes up by 1 each pass, b^1 -> b^254.

    }
    return b;
}
static uint8_t affine_transform(uint8_t b){ 
    return (uint8_t) (b ^ rotl8(b, 1) ^ rotl8(b, 2)
    ^ rotl8(b, 3) ^ rotl8(b, 4) ^ 0x63);
}
static uint8_t ntablesbox(uint8_t b){
    uint8_t b254 = compb254(b);
    uint8_t affine = affine_transform(b254);
    return affine;
}
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
static uint8_t invaffine(uint8_t x){
    x = rotl8(x,1) ^ rotl8(x,3) ^ rotl8(x,6) ^ 0x05;
    return x; 
} 
static uint8_t invsbox(uint8_t b){
    uint8_t unaff = invaffine(b); 
    uint8_t inverse = compb254(unaff);
    return inverse; 
} 
uint8_t refsub_byte(uint8_t b){ return sbox[b]; }
uint8_t refinv_sub_byte(uint8_t b){ return inv_sbox[b]; }
uint8_t sub_byte(uint8_t b){ return ntablesbox(b);}
uint8_t inv_sub_byte(uint8_t b){ return invsbox(b);}
