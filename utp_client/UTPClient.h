#pragma once

#include "UTPMessages.h"
#include <functional>
#include <string>

class UTPClient {
private:
    int m_socket = -1;
    std::string m_multicast_group;
    int m_port;

    // Callback functions for different message types
    std::function<void(const AdminHeartbeat&)> m_heartbeat_callback;
    std::function<void(const SecurityDefinition&)> m_security_def_callback;
    std::function<void(const MDFullRefresh&)> m_full_refresh_callback;
    std::function<void(const MDIncrementalRefresh&)> m_incremental_callback;

public:
    UTPClient(const std::string& multicast_group, int port);
    ~UTPClient();

    // Connection management
    bool connect();
    void disconnect();

    // Message callbacks
    void set_heartbeat_callback(std::function<void(const AdminHeartbeat&)> callback);
    void set_security_definition_callback(std::function<void(const SecurityDefinition&)> callback);
    void set_full_refresh_callback(std::function<void(const MDFullRefresh&)> callback);
    void set_incremental_callback(std::function<void(const MDIncrementalRefresh&)> callback);

    // Message processing
    void process_messages();
    void process_single_message();

private:
    // Message parsing helpers
    void parse_message(const uint8_t* buffer, size_t size);
    void parse_admin_heartbeat(const uint8_t* buffer);
    void parse_security_definition(const uint8_t* buffer);
    void parse_md_full_refresh(const uint8_t* buffer);
    void parse_md_incremental_refresh(const uint8_t* buffer);

    // Network helpers
    bool setup_multicast_socket();
    bool join_multicast_group();
    ssize_t receive_data(uint8_t* buffer, size_t max_size);

    // Debug helpers
    void print_message_header(const UTPMessageHeader& header);
    void hex_dump(const uint8_t* data, size_t size);
};