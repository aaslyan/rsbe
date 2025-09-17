#pragma once

#include "protocol_adapter.h"
#include <netinet/in.h>
#include <string>

namespace market_protocols {

// UDP transport implementation
class UDPTransport : public IMessageTransport {
public:
    UDPTransport(const std::string& host, uint16_t port);
    ~UDPTransport() override;

    // IMessageTransport interface
    bool send_message(const std::vector<uint8_t>& data) override;
    std::string get_transport_type() const override { return "UDP"; }
    bool is_connected() const override { return socket_fd_ > 0; }

    // UDP-specific methods
    bool initialize();
    void close();

    // Multicast support
    bool join_multicast_group(const std::string& group_ip);
    bool leave_multicast_group(const std::string& group_ip);

    // Configuration
    void set_send_buffer_size(size_t size);
    void set_ttl(int ttl);

private:
    std::string host_;
    uint16_t port_;
    int socket_fd_;
    struct sockaddr_in dest_addr_;
    bool is_multicast_;

    bool setup_socket();
    bool is_multicast_address(const std::string& ip);
};

} // namespace market_protocols