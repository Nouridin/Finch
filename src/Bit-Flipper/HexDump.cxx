/*
 * Finch - UDP Fault Injection Proxy
 * Copyright (C) 2026 Nouridin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "HexDump.h"

#include <cctype>
#include <iomanip>
#include <iostream>

void printHex (const uint8_t* orig, const uint8_t* mutated, size_t size, size_t bytes_per_line)
{
        std::cout << "\n--- PACKET MUTATION INSPECTOR (" << size << " bytes) ---\n";
        std::cout << "  OFFSET  | ORIGINAL HEX | MUTATED HEX  | ORIG  MUT\n";
        std::cout << "----------+--------------+--------------+-----------\n";

        for (size_t line = 0; line < size; line += bytes_per_line)
        {
                std::cout << "  0x" << std::setw(4) << std::setfill('0') << std::hex
                          << std::uppercase << line << "  | ";

                for (size_t i = 0; i < bytes_per_line; ++i)
                {
                        if (line + i < size)
                        {
                                std::cout << std::setw(2) << std::setfill('0')
                                          << static_cast<int>(orig[line + i]) << " ";
                        }
                        else
                        {
                                std::cout << "   ";
                        }
                }
                std::cout << "| ";

                for (size_t i = 0; i < bytes_per_line; ++i)
                {
                        if (line + i < size)
                        {
                                std::cout << std::setw(2) << std::setfill('0')
                                          << static_cast<int>(mutated[line + i]) << " ";
                        }
                        else
                        {
                                std::cout << "   ";
                        }
                }
                std::cout << "| ";

                for (size_t i = 0; i < bytes_per_line; ++i)
                {
                        if (line + i < size)
                        {
                                char c = static_cast<char>(orig[line + i]);
                                std::cout
                                    << (std::isprint(static_cast<unsigned char>(c)) ? c : '.');
                        }
                        else
                        {
                                std::cout << " ";
                        }
                }
                std::cout << " ";

                for (size_t i = 0; i < bytes_per_line; ++i)
                {
                        if (line + i < size)
                        {
                                char c = static_cast<char>(mutated[line + i]);
                                std::cout
                                    << (std::isprint(static_cast<unsigned char>(c)) ? c : '.');
                        }
                        else
                        {
                                std::cout << " ";
                        }
                }

                std::cout << std::dec << "\n";
        }
        std::cout << "-----------------------------------------------------\n\n";
}