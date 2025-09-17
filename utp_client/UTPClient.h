#pragma once

#include "UTPMessages.h"
#include <chrono>
#include <functional>
#include <string>

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
    void parse_multicast_header(const uint8_t* buffer);
    void parse_tr_packet_header(const uint8_t* buffer);
    void parse_sbe_message(const uint8_t* buffer, size_t size);
    void parse_sbe_message_at_offset(const uint8_t* buffer, size_t size, size_t offset);
    void decode_raw_message_content(const uint8_t* buffer, size_t size);
    void parse_message_at_offset(const uint8_t* buffer, size_t size, size_t offset);
    void parse_admin_heartbeat(const uint8_t* buffer, size_t size);
    void parse_security_definition(const uint8_t* buffer, size_t size);
    void parse_security_definition_tr(const uint8_t* buffer, size_t size, uint16_t block_length);
    void parse_md_full_refresh(const uint8_t* buffer, size_t size);
    void parse_md_full_refresh_tr(const uint8_t* buffer, size_t size, uint16_t block_length);
    void parse_md_incremental_refresh(const uint8_t* buffer, size_t size);
    void parse_md_incremental_refresh_tr(const uint8_t* buffer, size_t size, uint16_t block_length);
    void parse_md_incremental_refresh_trades(const uint8_t* buffer, size_t size);

    // Network helpers
    ssize_t receive_data(uint8_t* buffer, size_t max_size);

    // Debug helpers
    void print_message_header(const UTPMessageHeader& header);
    void hex_dump(const uint8_t* data, size_t size);
    void manual_decode_message(const uint8_t* buffer, size_t size);
    void decode_security_definition_manual(const uint8_t* buffer, size_t size);
    void decode_market_data_manual(const uint8_t* buffer, size_t size);
};