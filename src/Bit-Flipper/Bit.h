#ifndef BIT_H
#define BIT_H

#include "../Crossplatform/CROSS.h"

#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

/* Socket Initializers */
bool initSockets ();
void cleanupSockets ();
bool noBlocking (socket_t sock);

/* Payload */
void corruptPayload (uint8_t* data, size_t length, float bitflip_rate, std::mt19937& rng);
bool Drop (float drop_rate, std::mt19937& rng);
#endif