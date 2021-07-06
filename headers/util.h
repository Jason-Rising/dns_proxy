#ifndef UTIL_H
#define UTIL_H

// Print hex representation of byte
void print_hex(char byte);

// Concatenate two bytes into 16 bit value. Assumes Big Endianness, "byte_A" is most signifcant
short concat_bytes(char byte_A, char byte_B);

// Concatenate two 16 bit values into 32 bit value
int concat_shorts(short upper, short lower);

// Split 32-bit value into 4 bytes, returned as char array
char* split_32_t_bytes(int value);

#endif