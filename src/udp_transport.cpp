#include "include/common/udp_transport.h"
#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

namespace market_protocols {

UDPTransport::UDPTransport(const std::string& host, uint16_t port)
    : host_(host)
    , port_(port)
    , socket_fd_(-1)
    , is_multicast_(false)
{
    memset(&dest_addr_, 0, sizeof(dest_addr_));
    is_multicast_ = is_multicast_address(host);
}

UDPTransport::~UDPTransport()
{
    close();
}

bool UDPTransport::initialize()
{
    if (socket_fd_ > 0) {
        return true; // Already initialized
    }

    return setup_socket();
}

bool UDPTransport::setup_socket()
{
    // Create UDP socket
    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd_ < 0) {
        return false;
    }

    // Set up destination address
    dest_addr_.sin_family = AF_INET;
    dest_addr_.sin_port = htons(port_);

    if (inet_pton(AF_INET, host_.c_str(), &dest_addr_.sin_addr) <= 0) {
        ::close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    // Configure socket for multicast if needed
    if (is_multicast_) {
        // Set TTL for multicast
        int ttl = 1;
        if (setsockopt(socket_fd_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
            ::close(socket_fd_);
            socket_fd_ = -1;
            return false;
        }

        // Disable loopback (optional)
        int loop = 0;
        setsockopt(socket_fd_, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    }

    return true;
}

bool UDPTransport::send_message(const std::vector<uint8_t>& data)
{
    if (socket_fd_ < 0) {
        if (!initialize()) {
            return false;
        }
    }

    ssize_t sent = sendto(socket_fd_, data.data(), data.size(), 0,
        (struct sockaddr*)&dest_addr_, sizeof(dest_addr_));

    return sent == static_cast<ssize_t>(data.size());
}

void UDPTransport::close()
{
    if (socket_fd_ > 0) {
        ::close(socket_fd_);
        socket_fd_ = -1;
    }
}

bool UDPTransport::join_multicast_group(const std::string& group_ip)
{
    if (socket_fd_ < 0) {
        return false;
    }

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(group_ip.c_str());
    mreq.imr_interface.s_addr = INADDR_ANY;

    return setsockopt(socket_fd_, IPPROTO_IP, IP_ADD_MEMBERSHIP,
               &mreq, sizeof(mreq))
        == 0;
}

bool UDPTransport::leave_multicast_group(const std::string& group_ip)
{
    if (socket_fd_ < 0) {
        return false;
    }

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(group_ip.c_str());
    mreq.imr_interface.s_addr = INADDR_ANY;

    return setsockopt(socket_fd_, IPPROTO_IP, IP_DROP_MEMBERSHIP,
               &mreq, sizeof(mreq))
        == 0;
}

void UDPTransport::set_send_buffer_size(size_t size)
{
    if (socket_fd_ > 0) {
        int buf_size = static_cast<int>(size);
        setsockopt(socket_fd_, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size));
    }
}

void UDPTransport::set_ttl(int ttl)
{
    if (socket_fd_ > 0 && is_multicast_) {
        setsockopt(socket_fd_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    }
}

bool UDPTransport::is_multicast_address(const std::string& ip)
{
    struct sockaddr_in addr;
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        return false;
    }

    uint32_t ip_addr = ntohl(addr.sin_addr.s_addr);
    // Multicast range: 224.0.0.0 to 239.255.255.255
    return (ip_addr >= 0xE0000000) && (ip_addr <= 0xEFFFFFFF);
}

} // namespace market_protocols