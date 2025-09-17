#include "../include/common/tcp_transport.h"
#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

namespace protocol_common {

int TCPTransport::create_server_socket(uint16_t port, int backlog)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        throw std::runtime_error("Failed to create server socket: " + std::string(strerror(errno)));
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to set SO_REUSEADDR: " + std::string(strerror(errno)));
    }

    // Bind to address
    sockaddr_in address {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to bind to port " + std::to_string(port) + ": " + std::string(strerror(errno)));
    }

    // Listen for connections
    if (listen(server_fd, backlog) < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to listen on socket: " + std::string(strerror(errno)));
    }

    return server_fd;
}

int TCPTransport::accept_connection(int server_socket)
{
    sockaddr_in client_addr {};
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(server_socket, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
    if (client_fd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return -1; // No pending connections (non-blocking mode)
        }
        throw std::runtime_error("Failed to accept connection: " + std::string(strerror(errno)));
    }

    return client_fd;
}

void TCPTransport::close_socket(int socket_fd)
{
    if (socket_fd >= 0) {
        close(socket_fd);
    }
}

std::vector<uint8_t> TCPTransport::receive_data(int socket_fd, size_t max_size)
{
    std::vector<uint8_t> buffer(max_size);

    ssize_t bytes_received = recv(socket_fd, buffer.data(), max_size, MSG_DONTWAIT);
    if (bytes_received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return {}; // No data available (non-blocking)
        }
        throw std::runtime_error("Failed to receive data: " + std::string(strerror(errno)));
    }

    if (bytes_received == 0) {
        throw std::runtime_error("Connection closed by peer");
    }

    buffer.resize(bytes_received);
    return buffer;
}

bool TCPTransport::send_data(int socket_fd, const std::vector<uint8_t>& data)
{
    if (data.empty())
        return true;

    size_t total_sent = 0;
    while (total_sent < data.size()) {
        ssize_t sent = send(socket_fd, data.data() + total_sent,
            data.size() - total_sent, MSG_NOSIGNAL);

        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue; // Try again
            }
            return false; // Error occurred
        }

        total_sent += sent;
    }

    return true;
}

bool TCPTransport::set_non_blocking(int socket_fd)
{
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }

    return fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) >= 0;
}

bool TCPTransport::set_socket_options(int socket_fd)
{
    // Disable Nagle's algorithm for low-latency
    int tcp_nodelay = 1;
    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY,
            &tcp_nodelay, sizeof(tcp_nodelay))
        < 0) {
        return false;
    }

    // Set keep-alive
    int keepalive = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE,
            &keepalive, sizeof(keepalive))
        < 0) {
        return false;
    }

    return true;
}

std::string TCPTransport::get_peer_address(int socket_fd)
{
    sockaddr_in peer_addr {};
    socklen_t peer_len = sizeof(peer_addr);

    if (getpeername(socket_fd, reinterpret_cast<sockaddr*>(&peer_addr), &peer_len) < 0) {
        return "unknown";
    }

    char ip_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &peer_addr.sin_addr, ip_str, sizeof(ip_str)) == nullptr) {
        return "unknown";
    }

    return std::string(ip_str) + ":" + std::to_string(ntohs(peer_addr.sin_port));
}

bool TCPTransport::is_socket_connected(int socket_fd)
{
    char buffer[1];
    ssize_t result = recv(socket_fd, buffer, 1, MSG_PEEK | MSG_DONTWAIT);

    if (result == 0) {
        return false; // Connection closed
    }

    if (result < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true; // No data, but connection alive
        }
        return false; // Error occurred
    }

    return true; // Data available, connection alive
}

} // namespace protocol_common