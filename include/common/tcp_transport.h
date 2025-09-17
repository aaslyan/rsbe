#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace protocol_common {

class TCPTransport {
public:
    static int create_server_socket(uint16_t port, int backlog = 10);
    static int accept_connection(int server_socket);
    static void close_socket(int socket_fd);

    static std::vector<uint8_t> receive_data(int socket_fd, size_t max_size = 8192);
    static bool send_data(int socket_fd, const std::vector<uint8_t>& data);

    static bool set_non_blocking(int socket_fd);
    static bool set_socket_options(int socket_fd);

    static std::string get_peer_address(int socket_fd);
    static bool is_socket_connected(int socket_fd);
};

} // namespace protocol_common