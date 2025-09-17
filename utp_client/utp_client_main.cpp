#include "UTPClient.h"
#include <chrono>
#include <iostream>
#include <signal.h>
#include <thread>

// Global flag for graceful shutdown
volatile bool g_running = true;

void signal_handler(int signal)
{
    std::cout << "\nReceived signal " << signal << ", shutting down gracefully...\n";
    g_running = false;
}

void print_usage(const char* program_name)
{
    std::cout << "Usage: " << program_name << " <multicast_group> <port>\n";
    std::cout << "Example: " << program_name << " 224.0.1.100 5000\n";
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }

    std::string multicast_group = argv[1];
    int port = std::stoi(argv[2]);

    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    std::cout << "=== UTP Market Data Client ===\n";
    std::cout << "Multicast Group: " << multicast_group << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "Protocol: UTP_CLIENT_MKTDATA (Schema ID 101, Version 1)\n";
    std::cout << "Byte Order: Little Endian\n\n";

    UTPClient client(multicast_group, port);

    // Set up message callbacks
    client.set_heartbeat_callback([](const AdminHeartbeat& hb) {
        std::cout << "[CALLBACK] AdminHeartbeat processed\n";
    });

    client.set_security_def_callback([](const SecurityDefinition& secDef) {
        std::cout << "[CALLBACK] SecurityDefinition processed\n";
    });

    client.set_full_refresh_callback([](const MDFullRefresh& refresh) {
        std::cout << "[CALLBACK] MDFullRefresh processed - "
                  << refresh.mdEntries.size() << " entries\n";
    });

    client.set_incremental_refresh_callback([](const MDIncrementalRefresh& incremental) {
        std::cout << "[CALLBACK] MDIncrementalRefresh processed - "
                  << incremental.mdEntries.size() << " entries\n";
    });

    // Connect to multicast feed
    if (!client.connect()) {
        std::cerr << "Failed to connect to UTP multicast feed\n";
        return 1;
    }

    std::cout << "Connected successfully. Press Ctrl+C to exit.\n\n";

    // Message processing loop
    try {
        while (g_running) {
            // Process messages with timeout to allow checking g_running flag
            fd_set readfds;
            struct timeval timeout;

            FD_ZERO(&readfds);
            FD_SET(0, &readfds); // stdin for potential future use

            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            // For now, just process single messages in a loop
            client.process_single_message();

            // Small sleep to prevent busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in message processing: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "UTP client shutdown complete.\n";
    return 0;
}