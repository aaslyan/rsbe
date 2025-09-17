#include "UTPClient.h"
#include <arpa/inet.h>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <algorithm>
#include <sys/select.h>

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
    // Create socket
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return false;
    }

    // Enable SO_REUSEADDR to allow multiple clients
    int reuse = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "Failed to set SO_REUSEADDR: " << strerror(errno) << std::endl;
        close(m_socket);
        m_socket = -1;
        return false;
    }

    // Bind to the multicast port
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(m_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        close(m_socket);
        m_socket = -1;
        return false;
    }

    // Join multicast group
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(m_multicast_group.c_str());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        std::cerr << "Failed to join multicast group: " << strerror(errno) << std::endl;
        close(m_socket);
        m_socket = -1;
        return false;
    }

    m_is_connected = true;
    std::cout << "Connected to multicast group " << m_multicast_group << ":" << m_port << std::endl;
    return true;
}

void UTPClient::disconnect()
{
    if (m_socket >= 0) {
        // Leave multicast group
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(m_multicast_group.c_str());
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        setsockopt(m_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));

        close(m_socket);
        m_socket = -1;
    }
    m_is_connected = false;
}

ssize_t UTPClient::receive_data(uint8_t* buffer, size_t buffer_size)
{
    if (!m_is_connected) {
        return -1;
    }

    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    // Use select with timeout to avoid blocking indefinitely
    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    FD_SET(m_socket, &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = 100000; // 100ms timeout

    int result = select(m_socket + 1, &readfds, nullptr, nullptr, &tv);
    if (result <= 0) {
        return result; // timeout or error
    }

    ssize_t bytes = recvfrom(m_socket, buffer, buffer_size, 0,
                             (struct sockaddr*)&sender_addr, &sender_len);

    if (bytes > 0) {
        m_last_received_time = std::chrono::steady_clock::now();
    }

    return bytes;
}

void UTPClient::run()
{
    m_is_running = true;
    while (m_is_running && m_is_connected) {
        process_single_message();
        // Small delay to prevent busy-waiting
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void UTPClient::stop()
{
    m_is_running = false;
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
    if (size < 8) {
        std::cerr << "Message too small for any header: " << size << " bytes\n";
        return;
    }

    std::cout << "\n=== UTP Message Received (Raw) ===\n";
    std::cout << "Message size: " << size << " bytes\n";
    hex_dump(buffer, std::min(size, size_t(32)));

    // Try different offsets to find the SBE header
    for (size_t offset = 0; offset <= 8 && offset < size - 8; offset += 4) {
        try {
            utp_sbe::MessageHeader header;
            header.wrap(const_cast<char*>(reinterpret_cast<const char*>(buffer)), offset, 0, size);

            std::cout << "\n--- Trying offset " << offset << " ---\n";
            std::cout << "Template ID: " << header.templateId() << std::endl;
            std::cout << "Schema ID: " << header.schemaId() << std::endl;
            std::cout << "Version: " << header.version() << std::endl;
            std::cout << "Block Length: " << header.blockLength() << std::endl;

            // Check if this looks like a valid template ID (should be 1, 18, 38, 39, 41)
            uint16_t templateId = header.templateId();
            if (templateId == 1 || templateId == 18 || templateId == 38 || templateId == 39 || templateId == 41) {
                std::cout << "*** Found valid template ID at offset " << offset << " ***\n";
                parse_message_at_offset(buffer, size, offset);
                return;
            }
        } catch (const std::exception& e) {
            std::cout << "Failed at offset " << offset << ": " << e.what() << std::endl;
        }
    }

    std::cout << "Could not find valid SBE header in message\n";
}

void UTPClient::parse_message_at_offset(const uint8_t* buffer, size_t size, size_t offset)
{
    utp_sbe::MessageHeader header;
    header.wrap(const_cast<char*>(reinterpret_cast<const char*>(buffer)), offset, 0, size);

    std::cout << "\n=== Parsing SBE Message at offset " << offset << " ===\n";

    switch (header.templateId()) {
    case 1: // ADMIN_HEARTBEAT
        parse_admin_heartbeat(buffer + offset, size - offset);
        break;

    case 18: // SECURITY_DEFINITION
        parse_security_definition(buffer + offset, size - offset);
        break;

    case 39: // MD_FULL_REFRESH
        parse_md_full_refresh(buffer + offset, size - offset);
        break;

    case 38: // MD_INCREMENTAL_REFRESH
        parse_md_incremental_refresh(buffer + offset, size - offset);
        break;

    case 41: // MD_INCREMENTAL_REFRESH_TRADES
        parse_md_incremental_refresh_trades(buffer + offset, size - offset);
        break;

    default:
        std::cout << "Unknown message type: " << header.templateId() << std::endl;
        hex_dump(buffer + offset, std::min(size - offset, size_t(64)));
        break;
    }
}

void UTPClient::parse_admin_heartbeat(const uint8_t* buffer, size_t size)
{
    utp_sbe::AdminHeartbeat heartbeat;
    heartbeat.wrapForDecode(const_cast<char*>(reinterpret_cast<const char*>(buffer)), 
                            0, sizeof(utp_sbe::MessageHeader), 
                            sizeof(utp_sbe::MessageHeader), size);
    
    std::cout << "AdminHeartbeat received\n";
}

void UTPClient::parse_security_definition(const uint8_t* buffer, size_t size)
{
    utp_sbe::SecurityDefinition secDef;
    secDef.wrapForDecode(const_cast<char*>(reinterpret_cast<const char*>(buffer)), 
                         0, sizeof(utp_sbe::MessageHeader), 
                         sizeof(utp_sbe::MessageHeader), size);
    
    std::cout << "=== SecurityDefinition ===\n";
    std::cout << "  Security ID: " << secDef.securityID() << std::endl;
    
    // Get symbol
    char symbol[17] = {0};
    secDef.getSymbol(symbol, sizeof(symbol)-1);
    std::cout << "  Symbol: " << symbol << std::endl;
    
    // Get currencies
    char currency1[4] = {0};
    char currency2[4] = {0};
    secDef.getCurrency1(currency1, sizeof(currency1)-1);
    secDef.getCurrency2(currency2, sizeof(currency2)-1);
    std::cout << "  Currency1: " << currency1 << std::endl;
    std::cout << "  Currency2: " << currency2 << std::endl;
    
    std::cout << "  Last Update Time: " << secDef.lastUpdateTime() << std::endl;
    std::cout << "  Security Type: " << static_cast<int>(secDef.securityType()) << std::endl;
    std::cout << "  Depth of Book: " << static_cast<int>(secDef.depthOfBook()) << std::endl;
    std::cout << "  Min Trade Volume: " << secDef.minTradeVol() << std::endl;
}

void UTPClient::parse_md_full_refresh(const uint8_t* buffer, size_t size)
{
    utp_sbe::MDFullRefresh refresh;
    refresh.wrapForDecode(const_cast<char*>(reinterpret_cast<const char*>(buffer)), 
                          0, sizeof(utp_sbe::MessageHeader), 
                          sizeof(utp_sbe::MessageHeader), size);
    
    std::cout << "=== MDFullRefresh ===\n";
    std::cout << "  Security ID: " << refresh.securityID() << std::endl;
    std::cout << "  RptSeq: " << refresh.rptSeq() << std::endl;
    std::cout << "  TransactTime: " << refresh.transactTime() << std::endl;
    std::cout << "  Market Depth: " << static_cast<int>(refresh.marketDepth()) << std::endl;
    
    // Parse MD entries
    auto& entries = refresh.noMDEntries();
    std::cout << "  Number of Entries: " << entries.count() << std::endl;
    
    while (entries.hasNext()) {
        auto& entry = entries.next();
        double price = static_cast<double>(entry.mDEntryPx().mantissa()) / 1e9;
        
        std::cout << "    Entry: Type=" << static_cast<int>(entry.mDEntryType())
                  << " (0=Bid, 1=Offer), Price=" << price
                  << ", Size=" << entry.mDEntrySize() << std::endl;
    }
}

void UTPClient::parse_md_incremental_refresh(const uint8_t* buffer, size_t size)
{
    utp_sbe::MDIncrementalRefresh incremental;
    incremental.wrapForDecode(const_cast<char*>(reinterpret_cast<const char*>(buffer)), 
                              0, sizeof(utp_sbe::MessageHeader), 
                              sizeof(utp_sbe::MessageHeader), size);
    
    std::cout << "=== MDIncrementalRefresh ===\n";
    std::cout << "  Security ID: " << incremental.securityID() << std::endl;
    std::cout << "  RptSeq: " << incremental.rptSeq() << std::endl;
    std::cout << "  TransactTime: " << incremental.transactTime() << std::endl;
    
    // Parse MD entries
    auto& entries = incremental.noMDEntries();
    std::cout << "  Number of Entries: " << entries.count() << std::endl;
    
    while (entries.hasNext()) {
        auto& entry = entries.next();
        double price = static_cast<double>(entry.mDEntryPx().mantissa()) / 1e9;
        
        std::cout << "    Entry: Action=" << static_cast<int>(entry.mDUpdateAction())
                  << " (0=New, 1=Change, 2=Delete), Type=" << static_cast<int>(entry.mDEntryType())
                  << " (0=Bid, 1=Offer), Price=" << price
                  << ", Size=" << entry.mDEntrySize() << std::endl;
    }
}

void UTPClient::parse_md_incremental_refresh_trades(const uint8_t* buffer, size_t size)
{
    utp_sbe::MDIncrementalRefreshTrades trades;
    trades.wrapForDecode(const_cast<char*>(reinterpret_cast<const char*>(buffer)), 
                         0, sizeof(utp_sbe::MessageHeader), 
                         sizeof(utp_sbe::MessageHeader), size);
    
    std::cout << "=== MDIncrementalRefreshTrades ===\n";
    std::cout << "  Security ID: " << trades.securityID() << std::endl;
    
    // Parse trade entries
    auto& entries = trades.noMDEntries();
    std::cout << "  Number of Trades: " << entries.count() << std::endl;
    
    while (entries.hasNext()) {
        auto& entry = entries.next();
        double price = static_cast<double>(entry.mDEntryPx().mantissa()) / 1e9;
        
        std::cout << "    Trade: Price=" << price
                  << ", Size=" << entry.mDEntrySize()
                  << ", TransactTime=" << entry.transactTime()
                  << ", Aggressor=" << static_cast<int>(entry.aggressorSide()) 
                  << " (0=None, 1=Buy, 2=Sell)" << std::endl;
    }
}

void UTPClient::hex_dump(const uint8_t* buffer, size_t size)
{
    std::cout << "Hex dump:\n";
    for (size_t i = 0; i < size; i += 16) {
        std::cout << std::setfill('0') << std::setw(4) << std::hex << i << ": ";
        for (size_t j = 0; j < 16 && i + j < size; ++j) {
            std::cout << std::setw(2) << static_cast<int>(buffer[i + j]) << " ";
        }
        std::cout << std::dec << std::endl;
    }
}

void UTPClient::print_message_header(const UTPMessageHeader& header)
{
    std::cout << "Template ID: " << header.templateId << std::endl;
    std::cout << "Block Length: " << header.blockLength << std::endl;
    std::cout << "Schema ID: " << header.schemaId << std::endl;
    std::cout << "Version: " << static_cast<int>(header.version) << std::endl;
}

// Callbacks - empty implementations
void UTPClient::set_heartbeat_callback(std::function<void(const AdminHeartbeat&)> callback)
{
    m_heartbeat_callback = callback;
}

void UTPClient::set_security_def_callback(std::function<void(const SecurityDefinition&)> callback)
{
    m_security_def_callback = callback;
}

void UTPClient::set_full_refresh_callback(std::function<void(const MDFullRefresh&)> callback)
{
    m_full_refresh_callback = callback;
}

void UTPClient::set_incremental_refresh_callback(std::function<void(const MDIncrementalRefresh&)> callback)
{
    m_incremental_refresh_callback = callback;
}