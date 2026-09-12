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

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "Bit-Flipper/Bit.h"
#include "Bit-Flipper/HexDump.h"
#include "Bit-Flipper/Config.h"

#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <random>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

struct DelayedPacket 
{
    std::vector<uint8_t> payload;
    std::chrono::steady_clock::time_point release_time;
    size_t id;
    sockaddr_in dest_addr;

    bool operator>(const DelayedPacket& other) const 
    {
        return release_time > other.release_time;
    }
};

int main(int argc, char* argv[]) 
{
    FinchConfig config = FinchConfig::parse(argc, argv);
    if (config.show_help) 
    {
        FinchConfig::print_usage(argv[0]);
        return 1;
    }

    if (!initSockets()) 
    {
        std::cerr << "[ERROR] Failed to init sockets.\n";
        return 1;
    }

    std::mt19937 rng(config.seed);

    socket_t proxy_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (IS_INVALID_SOCKET(proxy_sock)) {
        std::cerr << "[ERROR] Could not create UDP socket.\n";
        cleanupSockets();
        return 1;
    }

    sockaddr_in listen_addr{};
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(config.listen_port);
    inet_pton(AF_INET, config.listen_ip.c_str(), &listen_addr.sin_addr);

    if (bind(proxy_sock, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) < 0) {
        std::cerr << "[ERROR] Failed to bind socket to " 
                  << config.listen_ip << ":" << config.listen_port << "\n";
        CLOSE_SOCKET(proxy_sock);
        cleanupSockets();
        return 1;
    }

    sockaddr_in target_addr{};
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(config.forward_port);
    inet_pton(AF_INET, config.forward_ip.c_str(), &target_addr.sin_addr);

    if (!noBlocking(proxy_sock)) {
        std::cerr << "[WARN] Failed to set socket non-blocking mode.\n";
    }

    std::cout << "========================================================\n";
    std::cout << " Finch v0.1.0\n";
    std::cout << " Listening on : " << config.listen_ip << ":" << config.listen_port << "\n";
    std::cout << " Forwarding to: " << config.forward_ip << ":" << config.forward_port << "\n";
    std::cout << " Bitflip Rate : " << (config.bitflip_rate * 100.0) << "%\n";
    std::cout << " Drop Rate    : " << (config.drop_rate * 100.0) << "%\n";
    std::cout << " Latency      : " << config.delay_ms << "ms (+/-" << config.jitter_ms << "ms jitter)\n";
    std::cout << " PRNG Seed    : " << config.seed << "\n";
    std::cout << "========================================================\n";

    uint8_t buffer[65535];
    sockaddr_in last_known_client_addr{};
    bool client_connected = false;

    size_t total_packets = 0;
    size_t dropped_packets = 0;

    std::priority_queue<DelayedPacket, std::vector<DelayedPacket>, std::greater<DelayedPacket>> packet_queue;

    while (true) {
        sockaddr_in sender_addr{};
        socklen_t sender_len = sizeof(sender_addr);

        int bytes_received = recvfrom(proxy_sock, 
                                      (char*)buffer, 
                                      sizeof(buffer), 
                                      0, 
                                      (struct sockaddr*)&sender_addr, 
                                      &sender_len);

        if (bytes_received > 0) {
            total_packets++;

            bool is_from_target = (sender_addr.sin_addr.s_addr == target_addr.sin_addr.s_addr &&
                                   sender_addr.sin_port == target_addr.sin_port);

            sockaddr_in destination_addr{};

            if (is_from_target) {
                if (!client_connected) {
                    std::cout << "[WARN] Packet received from target, but client endpoint unknown.\n";
                    continue;
                }
                destination_addr = last_known_client_addr;
                std::cout << "[REPLY] Target -> Client (" << bytes_received << " bytes)\n";
            } else {
                last_known_client_addr = sender_addr;
                client_connected = true;
                destination_addr = target_addr;
                std::cout << "[FWD] Client -> Target (" << bytes_received << " bytes)\n";
            }

            if (Drop((float)config.drop_rate, rng)) {
                dropped_packets++;
                std::cout << "[DROP] Packet #" << total_packets << " (" << bytes_received << " bytes)\n";
            } else {
                std::vector<uint8_t> original_buffer(buffer, buffer + bytes_received);
                corruptPayload(buffer, (size_t)bytes_received, (float)config.bitflip_rate, rng);

                printHex(original_buffer.data(), buffer, (size_t)bytes_received);

                uint32_t applied_delay = config.delay_ms;
                if (config.jitter_ms > 0) {
                    std::uniform_int_distribution<int32_t> jitter_dist(-(int32_t)config.jitter_ms, (int32_t)config.jitter_ms);
                    int32_t jitter_val = jitter_dist(rng);
                    int32_t total_delay = (int32_t)config.delay_ms + jitter_val;
                    applied_delay = (uint32_t)std::max(0, total_delay);
                }

                auto release_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(applied_delay);
                packet_queue.push({ 
                    std::vector<uint8_t>(buffer, buffer + bytes_received), 
                    release_time, 
                    total_packets, 
                    destination_addr 
                });
            }
        }

        // Process packets scheduled for release
        auto now = std::chrono::steady_clock::now();
        while (!packet_queue.empty() && packet_queue.top().release_time <= now) {
            auto pkt = packet_queue.top();
            packet_queue.pop();

            sendto(proxy_sock, 
                   (const char*)pkt.payload.data(), 
                   (int)pkt.payload.size(), 
                   0, 
                   (struct sockaddr*)&pkt.dest_addr, 
                   sizeof(pkt.dest_addr));
        }

        #ifdef _WIN32
        Sleep(1);
        #else
        usleep(1000);
        #endif
    }

    CLOSE_SOCKET(proxy_sock);
    cleanupSockets();
    return 0;
}