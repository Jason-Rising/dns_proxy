#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../headers/util.h"

// Constants
#define BIT_32 4

// Concatenate two bytes into 16 bit value. Assumes Big Endianness, "byteA" is most signifcant
short concat_bytes(char byte_A, char byte_B){
    return ((short)byte_A << 8) +  ((short)byte_B & 0xFF); 
}

// Concatenate two 16 bit values into 32 bit value
int concat_shorts(short upper, short lower){
    return ((int) upper << 16) + lower;
}

// Print hex representation of byte
void print_hex(char byte){
    const char hex_rep[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F'
    };

    char upperNibble = hex_rep[(byte >> 4)& 0x0F];
    char lowerNibble = hex_rep[byte & 0x0F];

    fprintf(stderr, "%c%c ", upperNibble, lowerNibble);

}

// Split 32-bit value into 4 bytes, returned as char array
char* split_32_t_bytes(int value){
    short upper = value >> 16;
    short lower = value;

    char* bytes = (char*)malloc(sizeof(bytes) * BIT_32);
    assert(bytes);

    bytes[0] =  upper >> 8; 
    bytes[1] =  upper; 
    bytes[2] =  lower >> 8; 
    bytes[3] =  lower; 

    return bytes;
}
