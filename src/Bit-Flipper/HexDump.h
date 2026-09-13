#ifndef HEXDUMP_H
#define HEXDUMP_H

#include <cstddef>
#include <cstdint>

void printHex (const uint8_t* orig, const uint8_t* mutated, size_t size, size_t bytes_per_line = 8);

#endif