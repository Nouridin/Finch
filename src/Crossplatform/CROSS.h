#ifndef CROSS_H
#define CROSS_H

/* CROSS PLATFORM SOCKET LAYER HEADER */
#ifdef _WIN32
#include <WS2tcpip.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib") // for MSVC
using socket_t = SOCKET;
#define IS_INVALID_SOCKET(s) ((s) == INVALID_SOCKET)
#define CLOSE_SOCKET(s) closesocket(s)
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using socket_t = int;
#define IS_INVALID_SOCKET(s) ((s) < 0)
#define CLOSE_SOCKET(s) close(s)
#endif

#endif