#pragma once

#include "instrument.h"
#include "market_events.h"
#include "reuters_messages.h"
#include <chrono>
#include <string>
#include <vector>

namespace reuters_protocol {

class ReutersEncoder {
public:
    static constexpr size_t MAX_MESSAGE_SIZE = 65536; // 64KB - much larger buffer
    static constexpr uint16_t SCHEMA_ID = 101;
    static constexpr uint16_t SCHEMA_VERSION = 52;
    static constexpr uint16_t BIG_ENDIAN_ENCODING = 23520;

    // Session Management Messages
    static std::vector<uint8_t> encode_negotiate_response(
        const std::string& session_id,
        FlowType flow_type,
        const std::string& username = "",
        const std::string& password = "");

    static std::vector<uint8_t> encode_negotiate_reject(
        const std::string& session_id,
        SessionRejectCode reject_code,
        const std::string& reason);

    static std::vector<uint8_t> encode_establish_ack(
        const std::string& session_id,
        uint32_t keepalive_interval);

    static std::vector<uint8_t> encode_establish_reject(
        const std::string& session_id,
        SessionRejectCode reject_code,
        const std::string& reason);

    static std::vector<uint8_t> encode_terminate(
        const std::string& session_id,
        TerminationCode termination_code,
        const std::string& reason);

    static std::vector<uint8_t> encode_heartbeat();

    // Market Data Messages
    static std::vector<uint8_t> encode_security_definition(
        const market_core::Instrument& instrument);

    static std::vector<uint8_t> encode_market_data_snapshot(
        const market_core::SnapshotEvent& snapshot);

    static std::vector<uint8_t> encode_market_data_incremental(
        const market_core::QuoteEvent& quote);

    static std::vector<uint8_t> encode_market_data_incremental(
        const market_core::TradeEvent& trade);

    // Market Data Request Response
    static std::vector<uint8_t> encode_market_data_request_rejection(
        const std::string& md_req_id,
        const std::string& rejection_reason);

private:
    // Helper methods for SBE encoding
    static uint64_t get_current_timestamp_ns();
    static int64_t to_sbe_decimal(double price);
    static int64_t to_sbe_quantity(uint64_t quantity);
};

} // namespace reuters_protocol