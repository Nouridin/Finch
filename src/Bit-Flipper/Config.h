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

#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

struct FinchConfig
{
        std::string listen_ip = "";
        uint16_t listen_port = 0;
        std::string forward_ip = "";
        uint16_t forward_port = 0;

        double bitflip_rate = 0.0;
        double drop_rate = 0.0;
        uint32_t seed = 1337;

        uint32_t delay_ms = 0;
        uint32_t jitter_ms = 0;

        bool show_help = false;

        static void
        print_usage (const char* exec_name)
        {
                std::cout
                    << "Usage: " << exec_name
                    << " --listen-ip <ip> --listen-port <port> --forward-ip <ip> --forward-port "
                       "<port> [options]\n\n"
                    << "Required:\n"
                    << "  --listen-ip <ip>      IP address to listen on (e.g. 0.0.0.0)\n"
                    << "  --listen-port <port>  Port to listen on\n"
                    << "  --forward-ip <ip>     Destination target IP address\n"
                    << "  --forward-port <port> Destination target port\n\n"
                    << "Fault Options:\n"
                    << "  --flip-rate <rate>    Bitflip probability 0.0 - 1.0 (default: 0.0)\n"
                    << "  --drop-rate <rate>    Packet drop probability 0.0 - 1.0 (default: 0.0)\n"
                    << "  --delay <ms>          Base latency delay in ms (default: 0)\n"
                    << "  --jitter <ms>         Max random jitter variance in ms (default: 0)\n"
                    << "  --seed <uint32>       PRNG seed for testing (default: 1337)\n"
                    << "  -h, --help            Show this help message\n";
        }

        static FinchConfig
        parse (int argc, char* argv[])
        {
                FinchConfig cfg;

                if (argc <= 1)
                {
                        cfg.show_help = true;
                        return cfg;
                }

                for (int i = 1; i < argc; ++i)
                {
                        std::string arg = argv[i];

                        if (arg == "-h" || arg == "--help")
                        {
                                cfg.show_help = true;
                                return cfg;
                        }

                        if (i + 1 >= argc)
                        {
                                std::cerr << "[!] Error: Flag '" << arg << "' needs a value.\n";
                                cfg.show_help = true;
                                return cfg;
                        }

                        if (arg == "--listen-ip")
                        {
                                cfg.listen_ip = argv[++i];
                        }
                        else if (arg == "--listen-port")
                        {
                                cfg.listen_port = (uint16_t)std::atoi(argv[++i]);
                        }
                        else if (arg == "--forward-ip")
                        {
                                cfg.forward_ip = argv[++i];
                        }
                        else if (arg == "--forward-port")
                        {
                                cfg.forward_port = (uint16_t)std::atoi(argv[++i]);
                        }
                        else if (arg == "--flip-rate")
                        {
                                cfg.bitflip_rate = std::atof(argv[++i]);
                        }
                        else if (arg == "--drop-rate")
                        {
                                cfg.drop_rate = std::atof(argv[++i]);
                        }
                        else if (arg == "--delay")
                        {
                                cfg.delay_ms = (uint32_t)std::strtoul(argv[++i], NULL, 10);
                        }
                        else if (arg == "--jitter")
                        {
                                cfg.jitter_ms = (uint32_t)std::strtoul(argv[++i], NULL, 10);
                        }
                        else if (arg == "--seed")
                        {
                                cfg.seed = (uint32_t)std::strtoul(argv[++i], NULL, 10);
                        }
                        else
                        {
                                std::cerr << "[!] Unknown option: " << arg << "\n";
                                cfg.show_help = true;
                                return cfg;
                        }
                }

                // Validate mandatory parameters
                if (cfg.listen_ip.empty() || cfg.listen_port == 0 || cfg.forward_ip.empty() ||
                    cfg.forward_port == 0)
                {
                        std::cerr << "[!] Error: Missing required network parameters (--listen-ip, "
                                     "--listen-port, --forward-ip, --forward-port).\n\n";
                        cfg.show_help = true;
                }

                return cfg;
        }
};

#endif