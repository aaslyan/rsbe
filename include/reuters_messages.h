#pragma once

#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace reuters_protocol {

// SOFH (Simple Open Framing Header) Structure
struct SOFHeader {
    uint32_t message_length; // Total message length including SOFH
    uint16_t encoding_type; // 23520 for big-endian
    uint32_t decryption_id; // 0 for unencrypted (but only 4 bytes used)

    SOFHeader()
        : message_length(0)
        , encoding_type(23520)
        , decryption_id(0)
    {
    }

    void pack(uint8_t* buffer) const
    {
        uint32_t net_length = htonl(message_length);
        memcpy(buffer, &net_length, 4);

        uint16_t net_encoding = htons(encoding_type);
        memcpy(buffer + 4, &net_encoding, 2);

        uint32_t net_decrypt = htonl(decryption_id);
        memcpy(buffer + 6, &net_decrypt, 4);
    }

    void unpack(const uint8_t* buffer)
    {
        message_length = ntohl(*reinterpret_cast<const uint32_t*>(buffer));
        encoding_type = ntohs(*reinterpret_cast<const uint16_t*>(buffer + 4));
        decryption_id = ntohl(*reinterpret_cast<const uint32_t*>(buffer + 6));
    }

    static constexpr size_t size() { return 10; }
};

// SBE Message Header Structure
struct SBEMessageHeader {
    uint16_t root_block_length; // Size of fixed fields
    uint16_t template_id; // Message type identifier
    uint16_t schema_id; // 101 for LSEG FX
    uint16_t schema_version; // Schema version
    uint16_t num_groups; // Number of repeating groups
    uint16_t num_var_data_fields; // Number of variable data fields

    SBEMessageHeader()
        : root_block_length(0)
        , template_id(0)
        , schema_id(101)
        , schema_version(52)
        , num_groups(0)
        , num_var_data_fields(0)
    {
    }

    void pack(uint8_t* buffer) const
    {
        uint16_t net_block_len = htons(root_block_length);
        memcpy(buffer, &net_block_len, 2);

        uint16_t net_template = htons(template_id);
        memcpy(buffer + 2, &net_template, 2);

        uint16_t net_schema_id = htons(schema_id);
        memcpy(buffer + 4, &net_schema_id, 2);

        uint16_t net_schema_ver = htons(schema_version);
        memcpy(buffer + 6, &net_schema_ver, 2);

        uint16_t net_groups = htons(num_groups);
        memcpy(buffer + 8, &net_groups, 2);

        uint16_t net_var_data = htons(num_var_data_fields);
        memcpy(buffer + 10, &net_var_data, 2);
    }

    void unpack(const uint8_t* buffer)
    {
        root_block_length = ntohs(*reinterpret_cast<const uint16_t*>(buffer));
        template_id = ntohs(*reinterpret_cast<const uint16_t*>(buffer + 2));
        schema_id = ntohs(*reinterpret_cast<const uint16_t*>(buffer + 4));
        schema_version = ntohs(*reinterpret_cast<const uint16_t*>(buffer + 6));
        num_groups = ntohs(*reinterpret_cast<const uint16_t*>(buffer + 8));
        num_var_data_fields = ntohs(*reinterpret_cast<const uint16_t*>(buffer + 10));
    }

    static constexpr size_t size() { return 12; }
};

// Complete Reuters Message Structure
struct ReutersMessage {
    SOFHeader sofh;
    SBEMessageHeader sbe_header;
    std::vector<uint8_t> payload;

    size_t total_size() const
    {
        return SOFHeader::size() + SBEMessageHeader::size() + payload.size();
    }

    void pack(std::vector<uint8_t>& buffer) const
    {
        buffer.resize(total_size());

        sofh.pack(buffer.data());
        sbe_header.pack(buffer.data() + SOFHeader::size());

        if (!payload.empty()) {
            memcpy(buffer.data() + SOFHeader::size() + SBEMessageHeader::size(),
                payload.data(), payload.size());
        }
    }
};

// Message Type Constants (Template IDs from LSEG Schema)
namespace MessageTypes {
    constexpr uint16_t NEGOTIATE = 500;
    constexpr uint16_t NEGOTIATE_RESPONSE = 500; // Same template, different flow
    constexpr uint16_t NEGOTIATE_REJECT = 500; // Same template, different flow
    constexpr uint16_t ESTABLISH = 501;
    constexpr uint16_t ESTABLISH_ACK = 501; // Same template, different flow
    constexpr uint16_t ESTABLISH_REJECT = 501; // Same template, different flow
    constexpr uint16_t SESSION_REJECT = 502;
    constexpr uint16_t TERMINATE = 503;
    constexpr uint16_t HEARTBEAT = 504;
    constexpr uint16_t TOPIC = 510;
    constexpr uint16_t CONTEXT = 511;
    constexpr uint16_t FINISHED_SENDING = 512;
    constexpr uint16_t SECURITY_DEFINITION_REQUEST = 2;
    constexpr uint16_t MARKET_DATA_REQUEST = 3;
    constexpr uint16_t MARKET_DATA_REQUEST_REJECTION = 6;
    constexpr uint16_t SECURITY_DEFINITION = 7;
    constexpr uint16_t MARKET_DATA_SNAPSHOT_FULL_REFRESH = 8;
    constexpr uint16_t MARKET_DATA_INCREMENTAL_REFRESH = 9;
    constexpr uint16_t SECURITY_STATUS = 10;
    constexpr uint16_t END_OF_CONFLATION = 11;
    constexpr uint16_t ENCRYPTED_WRAP = 12;
}

// Message Type Enums
enum class MessageTypeEnum : char {
    NEGOTIATE = 'N',
    NEGOTIATE_RESPONSE = 'O',
    NEGOTIATE_REJECT = 'P',
    ESTABLISH = 'E',
    ESTABLISH_ACK = 'F',
    ESTABLISH_REJECT = 'G',
    TERMINATE = 'T',
    HEARTBEAT = 'H',
    TOPIC = 'A',
    CONTEXT = 'C',
    FINISHED_SENDING = 'B'
};

enum class FlowType : char {
    RECOVERABLE = 'R',
    UNSEQUENCED = 'U',
    IDEMPOTENT = 'I',
    NONE = 'N'
};

enum class SessionRejectCode : char {
    SESSION_BLOCKED = 'B',
    CREDENTIALS = 'C',
    DUPLICATE_ID = 'D',
    ALREADY_ESTABLISHED = 'E',
    FLOW_TYPE_NOT_SUPPORTED = 'F',
    KEEPALIVE_INTERVAL = 'K',
    UNNEGOTIATED = 'N',
    UNSPECIFIED = 'U'
};

enum class TerminationCode : char {
    FINISHED = 'F',
    UNSPECIFIED_ERROR = 'U',
    RE_REQUEST_OUT_OF_BOUNDS = 'O',
    RE_REQUEST_IN_PROGRESS = 'R'
};

} // namespace reuters_protocol