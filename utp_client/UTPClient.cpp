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
#include <endian.h>

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
    if (size < 23) {
        std::cerr << "Message too small for multicast header: " << size << " bytes\n";
        return;
    }

    std::cout << "\n=== UTP Message Received ===\n";
    std::cout << "Message size: " << size << " bytes\n";
    hex_dump(buffer, std::min(size, size_t(48)));

    // Parse MulticastMessageHeader (23 bytes)
    parse_multicast_header(buffer);

    // SBE message starts at offset 23
    if (size > 23) {
        std::cout << "\n=== SBE Message (offset 23) ===\n";
        parse_sbe_message(buffer + 23, size - 23);
    } else {
        std::cout << "No SBE message payload\n";
    }
}

void UTPClient::parse_multicast_header(const uint8_t* buffer)
{
    std::cout << "\n--- Multicast Header ---\n";
    
    // Extract header fields (in network byte order)
    uint64_t sequence_number = be64toh(*reinterpret_cast<const uint64_t*>(buffer));
    uint32_t channel_id = ntohl(*reinterpret_cast<const uint32_t*>(buffer + 8));
    uint64_t send_time_ns = be64toh(*reinterpret_cast<const uint64_t*>(buffer + 12));
    uint16_t message_count = ntohs(*reinterpret_cast<const uint16_t*>(buffer + 20));
    uint8_t flags = buffer[22];
    
    std::cout << "Sequence Number: " << sequence_number << std::endl;
    std::cout << "Channel ID: " << channel_id << std::endl;
    std::cout << "Send Time (ns): " << send_time_ns << std::endl;
    std::cout << "Message Count: " << message_count << std::endl;
    std::cout << "Flags: 0x" << std::hex << static_cast<int>(flags) << std::dec << std::endl;
}

void UTPClient::parse_sbe_message(const uint8_t* buffer, size_t size)
{
    if (size < 8) {
        std::cerr << "SBE message too small: " << size << " bytes\n";
        return;
    }

    try {
        utp_sbe::MessageHeader header;
        header.wrap(const_cast<char*>(reinterpret_cast<const char*>(buffer)), 0, 0, size);

        std::cout << "SBE Template ID: " << header.templateId() << std::endl;
        std::cout << "SBE Schema ID: " << header.schemaId() << std::endl;
        std::cout << "SBE Version: " << header.version() << std::endl;
        std::cout << "SBE Block Length: " << header.blockLength() << std::endl;

        switch (header.templateId()) {
        case 1: // ADMIN_HEARTBEAT
            parse_admin_heartbeat(buffer, size);
            break;

        case 18: // SECURITY_DEFINITION
            parse_security_definition(buffer, size);
            break;

        case 39: // MD_FULL_REFRESH
            parse_md_full_refresh(buffer, size);
            break;

        case 38: // MD_INCREMENTAL_REFRESH
            parse_md_incremental_refresh(buffer, size);
            break;

        case 41: // MD_INCREMENTAL_REFRESH_TRADES
            parse_md_incremental_refresh_trades(buffer, size);
            break;

        default:
            std::cout << "Unknown SBE message type: " << header.templateId() << std::endl;
            hex_dump(buffer, std::min(size, size_t(64)));
            break;
        }
    } catch (const std::exception& e) {
        std::cout << "Failed to parse SBE message: " << e.what() << std::endl;
        hex_dump(buffer, std::min(size, size_t(32)));
    }
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

void UTPClient::manual_decode_message(const uint8_t* buffer, size_t size)
{
    std::cout << "\n=== Manual Message Decode ===\n";
    
    if (size < 16) {
        std::cout << "Message too short for manual decode\n";
        return;
    }
    
    // Try to extract useful information from common offsets
    // Look for potential template IDs in the first 20 bytes
    for (size_t i = 0; i < std::min(size - 2, size_t(20)); i += 2) {
        uint16_t val = *reinterpret_cast<const uint16_t*>(buffer + i);
        if (val == 1 || val == 18 || val == 38 || val == 39 || val == 41) {
            std::cout << "Potential template ID " << val << " at offset " << i << std::endl;
        }
    }
    
    // Look for potential security IDs (4-byte integers)
    for (size_t i = 0; i < std::min(size - 4, size_t(32)); i += 4) {
        uint32_t val = *reinterpret_cast<const uint32_t*>(buffer + i);
        if (val > 0 && val < 1000000) { // reasonable range for security ID
            std::cout << "Potential security ID " << val << " at offset " << i << std::endl;
        }
    }
    
    // Look for potential timestamps (8-byte values)
    for (size_t i = 0; i < std::min(size - 8, size_t(40)); i += 8) {
        uint64_t val = *reinterpret_cast<const uint64_t*>(buffer + i);
        if (val > 1000000000000000ULL && val < 9999999999999999999ULL) { // reasonable timestamp range
            std::cout << "Potential timestamp " << val << " at offset " << i << std::endl;
        }
    }
    
    // Look for printable strings (potential symbols)
    for (size_t i = 0; i < size - 4; i++) {
        bool printable = true;
        size_t len = 0;
        for (size_t j = i; j < std::min(i + 16, size) && buffer[j] != 0; j++) {
            if (buffer[j] < 32 || buffer[j] > 126) {
                printable = false;
                break;
            }
            len++;
        }
        if (printable && len >= 3 && len <= 16) {
            std::string str(reinterpret_cast<const char*>(buffer + i), len);
            std::cout << "Potential symbol '" << str << "' at offset " << i << std::endl;
            i += len; // skip ahead
        }
    }
    
    // Try to decode assuming it's a SecurityDefinition (template 18)
    if (size >= 50) {
        std::cout << "\n--- Assuming SecurityDefinition format ---\n";
        decode_security_definition_manual(buffer, size);
    }
    
    // Try to decode assuming it's market data
    if (size >= 30) {
        std::cout << "\n--- Assuming Market Data format ---\n";
        decode_market_data_manual(buffer, size);
    }
}

void UTPClient::decode_security_definition_manual(const uint8_t* buffer, size_t size)
{
    // Try different starting positions
    for (size_t start = 0; start <= 16 && start < size - 40; start += 4) {
        std::cout << "\n--- SecurityDef attempt at offset " << start << " ---\n";
        
        size_t pos = start;
        
        // Look for security ID (4 bytes)
        if (pos + 4 <= size) {
            uint32_t securityID = *reinterpret_cast<const uint32_t*>(buffer + pos);
            std::cout << "Security ID: " << securityID << std::endl;
            pos += 4;
        }
        
        // Look for timestamp (8 bytes)
        if (pos + 8 <= size) {
            uint64_t timestamp = *reinterpret_cast<const uint64_t*>(buffer + pos);
            std::cout << "Timestamp: " << timestamp << std::endl;
            pos += 8;
        }
        
        // Look for symbol (try 16 bytes)
        if (pos + 16 <= size) {
            char symbol[17] = {0};
            memcpy(symbol, buffer + pos, 16);
            // Clean non-printable chars
            for (int i = 0; i < 16; i++) {
                if (symbol[i] < 32 || symbol[i] > 126) symbol[i] = '.';
            }
            std::cout << "Potential symbol: '" << symbol << "'" << std::endl;
            pos += 16;
        }
        
        // Look for currencies (3 bytes each)
        if (pos + 6 <= size) {
            char currency1[4] = {0};
            char currency2[4] = {0};
            memcpy(currency1, buffer + pos, 3);
            memcpy(currency2, buffer + pos + 3, 3);
            // Clean non-printable chars
            for (int i = 0; i < 3; i++) {
                if (currency1[i] < 32 || currency1[i] > 126) currency1[i] = '.';
                if (currency2[i] < 32 || currency2[i] > 126) currency2[i] = '.';
            }
            std::cout << "Potential currencies: '" << currency1 << "' / '" << currency2 << "'" << std::endl;
        }
    }
}

void UTPClient::decode_market_data_manual(const uint8_t* buffer, size_t size)
{
    for (size_t start = 0; start <= 16 && start < size - 20; start += 4) {
        std::cout << "\n--- Market Data attempt at offset " << start << " ---\n";
        
        size_t pos = start;
        
        // Look for security ID
        if (pos + 4 <= size) {
            uint32_t securityID = *reinterpret_cast<const uint32_t*>(buffer + pos);
            std::cout << "Security ID: " << securityID << std::endl;
            pos += 4;
        }
        
        // Look for sequence number
        if (pos + 8 <= size) {
            uint64_t rptSeq = *reinterpret_cast<const uint64_t*>(buffer + pos);
            std::cout << "Report Sequence: " << rptSeq << std::endl;
            pos += 8;
        }
        
        // Look for timestamp
        if (pos + 8 <= size) {
            uint64_t timestamp = *reinterpret_cast<const uint64_t*>(buffer + pos);
            std::cout << "Timestamp: " << timestamp << std::endl;
            pos += 8;
        }
        
        // Look for potential price/size pairs
        while (pos + 16 <= size) {
            uint64_t price_raw = *reinterpret_cast<const uint64_t*>(buffer + pos);
            uint64_t size_val = *reinterpret_cast<const uint64_t*>(buffer + pos + 8);
            
            if (price_raw > 0 && size_val > 0 && size_val < 1000000000) {
                double price = static_cast<double>(price_raw) / 1e9;
                std::cout << "  Potential Price/Size: " << price << " / " << size_val << std::endl;
            }
            pos += 16;
            
            if (pos >= start + 64) break; // Don't go too far
        }
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