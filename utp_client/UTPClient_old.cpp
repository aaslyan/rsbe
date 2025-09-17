#include "UTPClient.h"
#include <arpa/inet.h>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

// Include SBE headers for proper decoding
#include "../include/utp_sbe/utp_sbe/MessageHeader.h"
#include "../include/utp_sbe/utp_sbe/AdminHeartbeat.h"
#include "../include/utp_sbe/utp_sbe/SecurityDefinition.h"
#include "../include/utp_sbe/utp_sbe/MDFullRefresh.h"
#include "../include/utp_sbe/utp_sbe/MDIncrementalRefresh.h"
#include "../include/utp_sbe/utp_sbe/MDIncrementalRefreshTrades.h"

UTPClient::UTPClient(const std::string& multicast_group, int port)
    : m_multicast_group(multicast_group)
    , m_port(port)
{
}

UTPClient::~UTPClient()
{
    disconnect();
}

bool UTPClient::connect()
{
    if (!setup_multicast_socket()) {
        std::cerr << "Failed to setup multicast socket\n";
        return false;
    }

    if (!join_multicast_group()) {
        std::cerr << "Failed to join multicast group\n";
        return false;
    }

    std::cout << "Successfully connected to UTP multicast feed "
              << m_multicast_group << ":" << m_port << std::endl;
    return true;
}

void UTPClient::disconnect()
{
    if (m_socket != -1) {
        close(m_socket);
        m_socket = -1;
    }
}

bool UTPClient::setup_multicast_socket()
{
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket == -1) {
        perror("socket creation failed");
        return false;
    }

    // Allow socket reuse
    int reuse = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt SO_REUSEADDR failed");
        return false;
    }

    // Bind to the multicast port
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(m_port);

    if (bind(m_socket, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind failed");
        return false;
    }

    return true;
}

bool UTPClient::join_multicast_group()
{
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(m_multicast_group.c_str());
    mreq.imr_interface.s_addr = INADDR_ANY;

    if (setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt IP_ADD_MEMBERSHIP failed");
        return false;
    }

    return true;
}

void UTPClient::set_heartbeat_callback(std::function<void(const AdminHeartbeat&)> callback)
{
    m_heartbeat_callback = callback;
}

void UTPClient::set_security_definition_callback(std::function<void(const SecurityDefinition&)> callback)
{
    m_security_def_callback = callback;
}

void UTPClient::set_full_refresh_callback(std::function<void(const MDFullRefresh&)> callback)
{
    m_full_refresh_callback = callback;
}

void UTPClient::set_incremental_callback(std::function<void(const MDIncrementalRefresh&)> callback)
{
    m_incremental_callback = callback;
}

ssize_t UTPClient::receive_data(uint8_t* buffer, size_t max_size)
{
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    ssize_t bytes_received = recvfrom(m_socket, buffer, max_size, 0,
        (struct sockaddr*)&sender_addr, &sender_len);

    if (bytes_received < 0) {
        perror("recvfrom failed");
        return -1;
    }

    return bytes_received;
}

void UTPClient::process_messages()
{
    std::cout << "Starting UTP message processing loop...\n";

    uint8_t buffer[65536]; // 64KB buffer for UDP packets

    while (true) {
        ssize_t bytes_received = receive_data(buffer, sizeof(buffer));
        if (bytes_received > 0) {
            parse_message(buffer, static_cast<size_t>(bytes_received));
        }
    }
}

void UTPClient::process_single_message()
{
    uint8_t buffer[65536];
    ssize_t bytes_received = receive_data(buffer, sizeof(buffer));

    if (bytes_received > 0) {
        parse_message(buffer, static_cast<size_t>(bytes_received));
    }
}

void UTPClient::parse_message(const uint8_t* buffer, size_t size)
{
    if (size < UTPMessageHeader::size()) {
        std::cerr << "Message too small for header: " << size << " bytes\n";
        return;
    }

    UTPMessageHeader header;
    header.unpack_little_endian(buffer);

    std::cout << "\n=== UTP Message Received ===\n";
    print_message_header(header);

    switch (header.templateId) {
    case MessageTypes::ADMIN_HEARTBEAT:
        parse_admin_heartbeat(buffer);
        break;

    case MessageTypes::SECURITY_DEFINITION:
        parse_security_definition(buffer);
        break;

    case MessageTypes::MD_FULL_REFRESH:
        parse_md_full_refresh(buffer);
        break;

    case MessageTypes::MD_INCREMENTAL_REFRESH:
        parse_md_incremental_refresh(buffer);
        break;

    case MessageTypes::MD_INCREMENTAL_REFRESH_TRADES:
        std::cout << "Trade message received (not fully implemented)\n";
        break;

    default:
        std::cout << "Unknown message type: " << header.templateId << std::endl;
        hex_dump(buffer, std::min(size, size_t(64)));
        break;
    }
}

void UTPClient::parse_admin_heartbeat(const uint8_t* buffer)
{
    AdminHeartbeat heartbeat;
    heartbeat.unpack(buffer);

    std::cout << "AdminHeartbeat received\n";

    if (m_heartbeat_callback) {
        m_heartbeat_callback(heartbeat);
    }
}

void UTPClient::parse_security_definition(const uint8_t* buffer)
{
    SecurityDefinition secDef;
    // Basic parsing - full implementation would unpack all fields
    secDef.header.unpack_little_endian(buffer);

    std::cout << "SecurityDefinition received\n";
    std::cout << "Block Length: " << secDef.header.blockLength << std::endl;

    if (m_security_def_callback) {
        m_security_def_callback(secDef);
    }
}

void UTPClient::parse_md_full_refresh(const uint8_t* buffer)
{
    MDFullRefresh refresh;
    refresh.header.unpack_little_endian(buffer);

    size_t pos = UTPMessageHeader::size();

    // Unpack fixed fields
    memcpy(&refresh.lastMsgSeqNumProcessed, buffer + pos, 8);
    pos += 8;
    memcpy(&refresh.securityID, buffer + pos, 4);
    pos += 4;
    memcpy(&refresh.rptSeq, buffer + pos, 8);
    pos += 8;
    memcpy(&refresh.transactTime, buffer + pos, 8);
    pos += 8;
    memcpy(refresh.mdEntryOriginator, buffer + pos, 16);
    pos += 16;
    refresh.marketDepth = buffer[pos];
    pos += 1;
    refresh.securityType = static_cast<MarketDataType>(buffer[pos]);
    pos += 1;

    // Parse group header
    GroupSize group;
    group.unpack_little_endian(buffer + pos);
    pos += GroupSize::size();

    std::cout << "MDFullRefresh received\n";
    std::cout << "SecurityID: " << refresh.securityID << std::endl;
    std::cout << "RptSeq: " << refresh.rptSeq << std::endl;
    std::cout << "Market Depth: " << static_cast<int>(refresh.marketDepth) << std::endl;
    std::cout << "Number of MD Entries: " << group.numInGroup << std::endl;

    // Parse entries
    for (uint16_t i = 0; i < group.numInGroup; ++i) {
        MDEntry entry;
        entry.mdEntryType = static_cast<MDEntryType>(buffer[pos]);
        pos += 1;
        memcpy(&entry.mdEntryPx.mantissa, buffer + pos, 8);
        pos += 8;
        memcpy(&entry.mdEntrySize, buffer + pos, 8);
        pos += 8;

        std::cout << "  Entry " << i << ": Type=" << static_cast<char>(entry.mdEntryType)
                  << ", Price=" << entry.mdEntryPx.to_double()
                  << ", Size=" << entry.mdEntrySize << std::endl;

        refresh.mdEntries.push_back(entry);
    }

    if (m_full_refresh_callback) {
        m_full_refresh_callback(refresh);
    }
}

void UTPClient::parse_md_incremental_refresh(const uint8_t* buffer)
{
    MDIncrementalRefresh incremental;
    incremental.header.unpack_little_endian(buffer);

    size_t pos = UTPMessageHeader::size();

    // Unpack fixed fields
    memcpy(&incremental.securityID, buffer + pos, 4);
    pos += 4;
    memcpy(&incremental.rptSeq, buffer + pos, 8);
    pos += 8;
    memcpy(&incremental.transactTime, buffer + pos, 8);
    pos += 8;
    memcpy(incremental.mdEntryOriginator, buffer + pos, 16);
    pos += 16;

    // Parse group header
    GroupSize group;
    group.unpack_little_endian(buffer + pos);
    pos += GroupSize::size();

    std::cout << "MDIncrementalRefresh received\n";
    std::cout << "SecurityID: " << incremental.securityID << std::endl;
    std::cout << "RptSeq: " << incremental.rptSeq << std::endl;
    std::cout << "Number of MD Entries: " << group.numInGroup << std::endl;

    // Parse entries
    for (uint16_t i = 0; i < group.numInGroup; ++i) {
        MDIncrementalEntry entry;
        entry.mdUpdateAction = static_cast<MDUpdateAction>(buffer[pos]);
        pos += 1;
        entry.mdEntryType = static_cast<MDEntryType>(buffer[pos]);
        pos += 1;
        memcpy(&entry.mdEntryPx.mantissa, buffer + pos, 8);
        pos += 8;
        memcpy(&entry.mdEntrySize, buffer + pos, 8);
        pos += 8;

        std::cout << "  Entry " << i << ": Action=" << static_cast<int>(entry.mdUpdateAction)
                  << ", Type=" << static_cast<char>(entry.mdEntryType)
                  << ", Price=" << entry.mdEntryPx.to_double()
                  << ", Size=" << entry.mdEntrySize << std::endl;

        incremental.mdEntries.push_back(entry);
    }

    if (m_incremental_callback) {
        m_incremental_callback(incremental);
    }
}

void UTPClient::print_message_header(const UTPMessageHeader& header)
{
    std::cout << "Template ID: " << header.templateId << std::endl;
    std::cout << "Block Length: " << header.blockLength << std::endl;
    std::cout << "Schema ID: " << header.schemaId << std::endl;
    std::cout << "Version: " << header.version << std::endl;
}

void UTPClient::hex_dump(const uint8_t* data, size_t size)
{
    std::cout << "Hex dump (" << size << " bytes):\n";
    for (size_t i = 0; i < size; ++i) {
        if (i % 16 == 0)
            std::cout << std::hex << std::setw(4) << std::setfill('0') << i << ": ";
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
        if ((i + 1) % 16 == 0)
            std::cout << std::endl;
    }
    if (size % 16 != 0)
        std::cout << std::endl;
    std::cout << std::dec;
}
