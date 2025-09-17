#include "include/common/udp_multicast_transport.h"
#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace protocol_common {

UDPTransport::UDPTransport()
    : socket_fd_(-1)
    , port_(0)
    , is_sender_(false)
{
    memset(&send_addr_, 0, sizeof(send_addr_));
}

UDPTransport::~UDPTransport()
{
    close();
}

bool UDPTransport::create_multicast_sender(const std::string& multicast_ip,
    uint16_t port,
    const std::string& interface_ip)
{
    multicast_ip_ = multicast_ip;
    port_ = port;
    interface_ip_ = interface_ip;
    is_sender_ = true;

    // Create UDP socket
    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd_ < 0) {
        last_error_ = "Failed to create socket: " + std::string(strerror(errno));
        return false;
    }

    // Set up destination address
    memset(&send_addr_, 0, sizeof(send_addr_));
    send_addr_.sin_family = AF_INET;
    send_addr_.sin_port = htons(port);
    if (inet_pton(AF_INET, multicast_ip.c_str(), &send_addr_.sin_addr) <= 0) {
        last_error_ = "Invalid multicast address: " + multicast_ip;
        close();
        return false;
    }

    // Set multicast interface if specified
    if (!interface_ip.empty() && interface_ip != "0.0.0.0") {
        if (!set_multicast_interface()) {
            close();
            return false;
        }
    }

    // Set default TTL for multicast
    set_ttl(1); // Local network only by default

    return true;
}

bool UDPTransport::create_multicast_receiver(const std::string& multicast_ip,
    uint16_t port,
    const std::string& interface_ip)
{
    multicast_ip_ = multicast_ip;
    port_ = port;
    interface_ip_ = interface_ip;
    is_sender_ = false;

    // Create UDP socket
    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd_ < 0) {
        last_error_ = "Failed to create socket: " + std::string(strerror(errno));
        return false;
    }

    // Allow multiple sockets to use the same port
    int reuse = 1;
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        last_error_ = "Failed to set SO_REUSEADDR: " + std::string(strerror(errno));
        close();
        return false;
    }

    // Bind to the multicast port
    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(port);
    bind_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd_, (struct sockaddr*)&bind_addr, sizeof(bind_addr)) < 0) {
        last_error_ = "Failed to bind to port " + std::to_string(port) + ": " + std::string(strerror(errno));
        close();
        return false;
    }

    // Join multicast group
    if (!join_multicast_group()) {
        close();
        return false;
    }

    return true;
}

bool UDPTransport::send(const std::vector<uint8_t>& data)
{
    return send(data.data(), data.size());
}

bool UDPTransport::send(const uint8_t* data, size_t length)
{
    if (socket_fd_ < 0 || !is_sender_) {
        last_error_ = "Socket not configured for sending";
        return false;
    }

    ssize_t sent = sendto(socket_fd_, data, length, 0,
        (struct sockaddr*)&send_addr_, sizeof(send_addr_));

    if (sent < 0) {
        last_error_ = "Send failed: " + std::string(strerror(errno));
        return false;
    }

    if (static_cast<size_t>(sent) != length) {
        last_error_ = "Partial send: " + std::to_string(sent) + " of " + std::to_string(length);
        return false;
    }

    return true;
}

std::vector<uint8_t> UDPTransport::receive(size_t max_size)
{
    if (socket_fd_ < 0 || is_sender_) {
        return {};
    }

    std::vector<uint8_t> buffer(max_size);
    struct sockaddr_in src_addr;
    socklen_t src_len = sizeof(src_addr);

    ssize_t received = recvfrom(socket_fd_, buffer.data(), max_size, MSG_DONTWAIT,
        (struct sockaddr*)&src_addr, &src_len);

    if (received < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            last_error_ = "Receive failed: " + std::string(strerror(errno));
        }
        return {};
    }

    buffer.resize(received);
    return buffer;
}

void UDPTransport::set_send_buffer_size(size_t size)
{
    if (socket_fd_ >= 0) {
        int buffer_size = static_cast<int>(size);
        setsockopt(socket_fd_, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));
    }
}

void UDPTransport::set_recv_buffer_size(size_t size)
{
    if (socket_fd_ >= 0) {
        int buffer_size = static_cast<int>(size);
        setsockopt(socket_fd_, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));
    }
}

void UDPTransport::set_ttl(int ttl)
{
    if (socket_fd_ >= 0 && is_sender_) {
        unsigned char ttl_val = static_cast<unsigned char>(ttl);
        setsockopt(socket_fd_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl_val, sizeof(ttl_val));
    }
}

void UDPTransport::set_multicast_loop(bool enable)
{
    if (socket_fd_ >= 0) {
        unsigned char loop = enable ? 1 : 0;
        setsockopt(socket_fd_, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    }
}

void UDPTransport::close()
{
    if (socket_fd_ >= 0) {
        // Leave multicast group if receiver
        if (!is_sender_ && !multicast_ip_.empty()) {
            struct ip_mreq mreq;
            mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip_.c_str());
            mreq.imr_interface.s_addr = INADDR_ANY;
            setsockopt(socket_fd_, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
        }

        ::close(socket_fd_);
        socket_fd_ = -1;
    }
}

bool UDPTransport::join_multicast_group()
{
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip_.c_str());

    if (interface_ip_.empty() || interface_ip_ == "0.0.0.0") {
        mreq.imr_interface.s_addr = INADDR_ANY;
    } else {
        mreq.imr_interface.s_addr = inet_addr(interface_ip_.c_str());
    }

    if (setsockopt(socket_fd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        last_error_ = "Failed to join multicast group: " + std::string(strerror(errno));
        return false;
    }

    return true;
}

bool UDPTransport::set_multicast_interface()
{
    struct in_addr iface_addr;
    if (inet_pton(AF_INET, interface_ip_.c_str(), &iface_addr) <= 0) {
        last_error_ = "Invalid interface IP: " + interface_ip_;
        return false;
    }

    if (setsockopt(socket_fd_, IPPROTO_IP, IP_MULTICAST_IF, &iface_addr, sizeof(iface_addr)) < 0) {
        last_error_ = "Failed to set multicast interface: " + std::string(strerror(errno));
        return false;
    }

    return true;
}

} // namespace protocol_common