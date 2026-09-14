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

#include "Bit.h"

bool initSockets ()
{
#ifdef _WIN32
        WSADATA wsa_data;
        return WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0;
#else
        return true;
#endif
}

void cleanupSockets ()
{
#ifdef _WIN32
        WSACleanup();
#endif
}

bool noBlocking (socket_t sock)
{
#ifdef _WIN32
        u_long mode = 1;
        return ioctlsocket(sock, FIONBIO, &mode) == 0;
#else
        int flags = fcntl(sock, F_GETFL, 0);
        if (flags < 0)
                return false;
        return fcntl(sock, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
}

void corruptPayload (uint8_t* data, size_t length, float bitflip_rate, std::mt19937& rng)
{
        if (length == 0 || bitflip_rate <= 0.0f)
                return;

        std::uniform_real_distribution<float> prob_dist(0.0f, 1.0f);
        std::uniform_int_distribution<int> bit_dist(0, 7);

        for (size_t i = 0; i < length; ++i)
        {
                if (prob_dist(rng) < bitflip_rate)
                {
                        uint8_t bit_mask = 1 << bit_dist(rng);
                        data[i] ^= bit_mask;
                }
        }
}

bool Drop (float drop_rate, std::mt19937& rng)
{
        if (drop_rate <= 0.0f)
                return false;
        std::uniform_real_distribution<float> prob(0.0f, 1.0f);
        return prob(rng) < drop_rate;
}