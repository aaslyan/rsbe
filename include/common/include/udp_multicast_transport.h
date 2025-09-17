#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <string>
#include <vector>

namespace protocol_common {

class UDPTransport {
public:
    UDPTransport();
    ~UDPTransport();

    // Multicast sender setup
    bool create_multicast_sender(const std::string& multicast_ip,
        uint16_t port,
        const std::string& interface_ip = "0.0.0.0");

    // Multicast receiver setup
    bool create_multicast_receiver(const std::string& multicast_ip,
        uint16_t port,
        const std::string& interface_ip = "0.0.0.0");

    // Send data
    bool send(const std::vector<uint8_t>& data);
    bool send(const uint8_t* data, size_t length);

    // Receive data
    std::vector<uint8_t> receive(size_t max_size = 65536);

    // Socket options
    void set_send_buffer_size(size_t size);
    void set_recv_buffer_size(size_t size);
    void set_ttl(int ttl);
    void set_multicast_loop(bool enable);

    // Status
    bool is_valid() const { return socket_fd_ >= 0; }
    std::string get_last_error() const { return last_error_; }

    // Close socket
    void close();

private:
    int socket_fd_;
    struct sockaddr_in send_addr_;
    std::string multicast_ip_;
    uint16_t port_;
    std::string interface_ip_;
    bool is_sender_;
    std::string last_error_;

    bool join_multicast_group();
    bool set_multicast_interface();
};

} // namespace protocol_common