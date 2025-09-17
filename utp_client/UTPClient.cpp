#include "UTPClient.h"
#include <algorithm>
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <endian.h>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

// Include SBE headers for proper decoding
#include "../include/utp_sbe/utp_sbe/AdminHeartbeat.h"
#include "../include/utp_sbe/utp_sbe/MDFullRefresh.h"
#include "../include/utp_sbe/utp_sbe/MDIncrementalRefresh.h"
#include "../include/utp_sbe/utp_sbe/MDIncrementalRefreshTrades.h"
#include "../include/utp_sbe/utp_sbe/MessageHeader.h"
#include "../include/utp_sbe/utp_sbe/SecurityDefinition.h"

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
    if (size < 20) {
        std::cerr << "Message too small for Thomson Reuters packet header: " << size << " bytes\n";
        return;
    }

    std::cout << "\n=== Thomson Reuters Message Received ===\n";
    std::cout << "Message size: " << size << " bytes\n";
    hex_dump(buffer, std::min(size, size_t(48)));

    // Parse Binary Packet Header (20 bytes) according to Thomson Reuters spec
    parse_tr_packet_header(buffer);

    // SBE message starts at offset 20
    if (size > 20) {
        std::cout << "\n=== SBE Message (offset 20) ===\n";
        parse_sbe_message(buffer + 20, size - 20);
    } else {
        std::cout << "No SBE message payload\n";
    }
}

void UTPClient::parse_tr_packet_header(const uint8_t* buffer)
{
    std::cout << "\n--- Thomson Reuters Binary Packet Header ---\n";

    // Parse according to TR specification (Chapter 6.1)
    uint64_t msg_seq_num = *reinterpret_cast<const uint64_t*>(buffer); // 8 bytes
    uint64_t sending_time = *reinterpret_cast<const uint64_t*>(buffer + 8); // 8 bytes
    uint8_t hdr_len = buffer[16]; // 1 byte (should be 20)
    uint8_t hdr_ver = buffer[17]; // 1 byte (should be 1)
    uint16_t packet_len = *reinterpret_cast<const uint16_t*>(buffer + 18); // 2 bytes

    std::cout << "MsgSeqNum: " << msg_seq_num << std::endl;
    std::cout << "SendingTime: " << sending_time << std::endl;
    std::cout << "HdrLen: " << static_cast<int>(hdr_len) << std::endl;
    std::cout << "HdrVer: " << static_cast<int>(hdr_ver) << std::endl;
    std::cout << "PacketLen: " << packet_len << std::endl;
}

void UTPClient::parse_sbe_message(const uint8_t* buffer, size_t size)
{
    if (size < 8) {
        std::cerr << "SBE message too small: " << size << " bytes\n";
        return;
    }

    std::cout << "Parsing SBE Header (first 8 bytes)...\n";
    hex_dump(buffer, std::min(size, size_t(32)));

    // Parse SBE Header according to Thomson Reuters spec (Chapter 6.2)
    uint16_t block_length = *reinterpret_cast<const uint16_t*>(buffer); // offset 0
    uint16_t template_id = *reinterpret_cast<const uint16_t*>(buffer + 2); // offset 2
    uint16_t schema_id = *reinterpret_cast<const uint16_t*>(buffer + 4); // offset 4
    uint16_t version = *reinterpret_cast<const uint16_t*>(buffer + 6); // offset 6

    std::cout << "\n--- SBE Header ---\n";
    std::cout << "Block Length: " << block_length << std::endl;
    std::cout << "Template ID: " << template_id << std::endl;
    std::cout << "Schema ID: " << schema_id << std::endl;
    std::cout << "Version: " << version << std::endl;

    // Now parse the specific message type
    std::cout << "\n--- SBE Message Body ---\n";
    switch (template_id) {
    case 0: // Heartbeat
        std::cout << "Heartbeat message\n";
        break;

    case 1: // Admin Heartbeat
        std::cout << "Admin Heartbeat message\n";
        break;

    case 12: // Security Definition (type 'd')
        std::cout << "Security Definition message\n";
        parse_security_definition_tr(buffer + 8, size - 8, block_length);
        break;

    case 13: // Market Data Snapshot Full Refresh (type 'W')
        std::cout << "Market Data Snapshot Full Refresh message\n";
        parse_md_full_refresh_tr(buffer + 8, size - 8, block_length);
        break;

    case 14: // Market Data Incremental Refresh (type 'X')
        std::cout << "Market Data Incremental Refresh message\n";
        parse_md_incremental_refresh_tr(buffer + 8, size - 8, block_length);
        break;

    default:
        std::cout << "Unknown or unsupported template ID: " << template_id << std::endl;
        decode_raw_message_content(buffer + 8, size - 8);
        break;
    }
}

void UTPClient::parse_sbe_message_at_offset(const uint8_t* buffer, size_t size, size_t offset)
{
    try {
        utp_sbe::MessageHeader header;
        header.wrap(const_cast<char*>(reinterpret_cast<const char*>(buffer)), offset, 0, size);

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
            std::cout << "Unsupported SBE message type: " << header.templateId() << std::endl;
            break;
        }
    } catch (const std::exception& e) {
        std::cout << "Failed to parse SBE message at offset " << offset << ": " << e.what() << std::endl;
    }
}

void UTPClient::decode_raw_message_content(const uint8_t* buffer, size_t size)
{
    std::cout << "\n=== Raw Message Content Analysis ===\n";

    // Look for potential numeric data that could be prices/sizes
    for (size_t i = 0; i <= size - 8; i += 4) {
        uint32_t val32 = *reinterpret_cast<const uint32_t*>(buffer + i);
        uint64_t val64 = *reinterpret_cast<const uint64_t*>(buffer + i);

        // Check for potential security ID (reasonable range)
        if (val32 > 0 && val32 < 100000) {
            std::cout << "Potential Security ID " << val32 << " at offset " << i << std::endl;
        }

        // Check for potential price (as fixed point)
        if (val64 > 1000000 && val64 < 1000000000000ULL) {
            double price = static_cast<double>(val64) / 1e9;
            if (price > 0.001 && price < 10000.0) {
                std::cout << "Potential Price " << price << " at offset " << i << std::endl;
            }
        }
    }

    // Look for ASCII strings (symbols)
    for (size_t i = 0; i < size - 3; i++) {
        if (buffer[i] >= 'A' && buffer[i] <= 'Z') {
            size_t len = 0;
            while (i + len < size && buffer[i + len] >= 'A' && buffer[i + len] <= 'Z' && len < 16) {
                len++;
            }
            if (len >= 3) {
                std::string symbol(reinterpret_cast<const char*>(buffer + i), len);
                std::cout << "Potential Symbol '" << symbol << "' at offset " << i << std::endl;
                i += len - 1; // Skip ahead
            }
        }
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
    char symbol[17] = { 0 };
    secDef.getSymbol(symbol, sizeof(symbol) - 1);
    std::cout << "  Symbol: " << symbol << std::endl;

    // Get currencies
    char currency1[4] = { 0 };
    char currency2[4] = { 0 };
    secDef.getCurrency1(currency1, sizeof(currency1) - 1);
    secDef.getCurrency2(currency2, sizeof(currency2) - 1);
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

void UTPClient::parse_security_definition_tr(const uint8_t* buffer, size_t size, uint16_t block_length)
{
    std::cout << "=== Thomson Reuters SecurityDefinition ===\n";

    if (size < 16) {
        std::cout << "Message too small for manual parsing: " << size << " bytes\n";
        return;
    }

    // Manual parsing based on Thomson Reuters specification
    size_t pos = 0;

    // Parse basic fields from the beginning
    if (pos + 4 <= size) {
        uint32_t securityID = *reinterpret_cast<const uint32_t*>(buffer + pos);
        std::cout << "  Security ID: " << securityID << std::endl;
        pos += 4;
    }

    // Look for printable symbol string
    for (size_t i = pos; i < std::min(size - 8, pos + 32); i++) {
        if (buffer[i] >= 'A' && buffer[i] <= 'Z') {
            std::string symbol;
            size_t j = i;
            while (j < size && ((buffer[j] >= 'A' && buffer[j] <= 'Z') || buffer[j] == '/') && symbol.length() < 16) {
                symbol += static_cast<char>(buffer[j]);
                j++;
            }
            if (symbol.length() >= 3 && symbol.find('/') != std::string::npos) {
                std::cout << "  Symbol: " << symbol << std::endl;
                break;
            }
        }
    }

    std::cout << "  Block Length: " << block_length << std::endl;
    std::cout << "  Message Size: " << size << " bytes" << std::endl;
}

void UTPClient::parse_md_full_refresh_tr(const uint8_t* buffer, size_t size, uint16_t block_length)
{
    std::cout << "=== Thomson Reuters MDFullRefresh ===\n";

    if (size < 12) {
        std::cout << "Message too small for parsing: " << size << " bytes\n";
        return;
    }

    size_t pos = 0;

    // Parse security ID
    if (pos + 4 <= size) {
        uint32_t securityID = *reinterpret_cast<const uint32_t*>(buffer + pos);
        std::cout << "  Security ID: " << securityID << std::endl;
        pos += 4;
    }

    // Parse sequence number
    if (pos + 8 <= size) {
        uint64_t rptSeq = *reinterpret_cast<const uint64_t*>(buffer + pos);
        std::cout << "  Report Sequence: " << rptSeq << std::endl;
        pos += 8;
    }

    // Look for price/size pairs
    std::cout << "  Market Data Entries:" << std::endl;
    size_t entry_count = 0;
    while (pos + 16 <= size && entry_count < 10) {
        uint64_t price_raw = *reinterpret_cast<const uint64_t*>(buffer + pos);
        uint64_t size_val = *reinterpret_cast<const uint64_t*>(buffer + pos + 8);

        if (price_raw > 0 && size_val > 0 && size_val < 1000000000) {
            double price = static_cast<double>(price_raw) / 1e9;
            std::cout << "    Entry " << entry_count << ": Price=" << price
                      << ", Size=" << size_val << std::endl;
        }
        pos += 16;
        entry_count++;
    }

    std::cout << "  Block Length: " << block_length << std::endl;
}

void UTPClient::parse_md_incremental_refresh_tr(const uint8_t* buffer, size_t size, uint16_t block_length)
{
    std::cout << "=== Thomson Reuters MDIncrementalRefresh ===\n";

    if (size < 12) {
        std::cout << "Message too small for parsing: " << size << " bytes\n";
        return;
    }

    size_t pos = 0;

    // Parse security ID
    if (pos + 4 <= size) {
        uint32_t securityID = *reinterpret_cast<const uint32_t*>(buffer + pos);
        std::cout << "  Security ID: " << securityID << std::endl;
        pos += 4;
    }

    // Parse sequence number
    if (pos + 8 <= size) {
        uint64_t rptSeq = *reinterpret_cast<const uint64_t*>(buffer + pos);
        std::cout << "  Report Sequence: " << rptSeq << std::endl;
        pos += 8;
    }

    // Look for incremental entries with action codes
    std::cout << "  Incremental Entries:" << std::endl;
    size_t entry_count = 0;
    while (pos + 17 <= size && entry_count < 5) {
        uint8_t action = buffer[pos];
        uint64_t price_raw = *reinterpret_cast<const uint64_t*>(buffer + pos + 1);
        uint64_t size_val = *reinterpret_cast<const uint64_t*>(buffer + pos + 9);

        if (action <= 2 && price_raw > 0) {
            double price = static_cast<double>(price_raw) / 1e9;
            const char* action_str = (action == 0) ? "New" : (action == 1) ? "Change"
                                                                           : "Delete";
            std::cout << "    Entry " << entry_count << ": Action=" << action_str
                      << ", Price=" << price << ", Size=" << size_val << std::endl;
        }
        pos += 17;
        entry_count++;
    }

    std::cout << "  Block Length: " << block_length << std::endl;
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
            char symbol[17] = { 0 };
            memcpy(symbol, buffer + pos, 16);
            // Clean non-printable chars
            for (int i = 0; i < 16; i++) {
                if (symbol[i] < 32 || symbol[i] > 126)
                    symbol[i] = '.';
            }
            std::cout << "Potential symbol: '" << symbol << "'" << std::endl;
            pos += 16;
        }

        // Look for currencies (3 bytes each)
        if (pos + 6 <= size) {
            char currency1[4] = { 0 };
            char currency2[4] = { 0 };
            memcpy(currency1, buffer + pos, 3);
            memcpy(currency2, buffer + pos + 3, 3);
            // Clean non-printable chars
            for (int i = 0; i < 3; i++) {
                if (currency1[i] < 32 || currency1[i] > 126)
                    currency1[i] = '.';
                if (currency2[i] < 32 || currency2[i] > 126)
                    currency2[i] = '.';
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

            if (pos >= start + 64)
                break; // Don't go too far
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