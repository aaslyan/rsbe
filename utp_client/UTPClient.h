#pragma once

#include "UTPMessages.h"
#include <functional>
#include <string>
#include <chrono>

class UTPClient {
private:
    int m_socket = -1;
    std::string m_multicast_group;
    int m_port;
    bool m_is_connected = false;
    bool m_is_running = false;
    std::chrono::steady_clock::time_point m_last_received_time;

    // Callback functions for different message types
    std::function<void(const AdminHeartbeat&)> m_heartbeat_callback;
    std::function<void(const SecurityDefinition&)> m_security_def_callback;
    std::function<void(const MDFullRefresh&)> m_full_refresh_callback;
    std::function<void(const MDIncrementalRefresh&)> m_incremental_refresh_callback;

public:
    UTPClient(const std::string& multicast_group, int port);
    ~UTPClient();

    // Connection management
    bool connect();
    void disconnect();

    // Message processing
    void run();
    void stop();
    void process_single_message();

    // Message callbacks
    void set_heartbeat_callback(std::function<void(const AdminHeartbeat&)> callback);
    void set_security_def_callback(std::function<void(const SecurityDefinition&)> callback);
    void set_full_refresh_callback(std::function<void(const MDFullRefresh&)> callback);
    void set_incremental_refresh_callback(std::function<void(const MDIncrementalRefresh&)> callback);

private:
    // Message parsing helpers
    void parse_message(const uint8_t* buffer, size_t size);
    void parse_admin_heartbeat(const uint8_t* buffer, size_t size);
    void parse_security_definition(const uint8_t* buffer, size_t size);
    void parse_md_full_refresh(const uint8_t* buffer, size_t size);
    void parse_md_incremental_refresh(const uint8_t* buffer, size_t size);
    void parse_md_incremental_refresh_trades(const uint8_t* buffer, size_t size);

    // Network helpers
    ssize_t receive_data(uint8_t* buffer, size_t max_size);

    // Debug helpers
    void print_message_header(const UTPMessageHeader& header);
    void hex_dump(const uint8_t* data, size_t size);
};