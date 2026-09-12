#ifndef BIT_H
#define BIT_H

#include "../Crossplatform/CROSS.h"

#include <iostream>
#include <vector>
#include <cstdint>
#include <random>
#include <cstring>
#include <thread>
#include <chrono>

/* Socket Initializers */
bool init_sockets();
void cleanup_sockets();
bool set_non_blocking(socket_t sock);

/* Payload */
void corruptPayload(uint8_t* data, size_t length, float bitflip_rate, std::mt19937& rng);
bool should_drop(float drop_rate, std::mt19937& rng);
#endif