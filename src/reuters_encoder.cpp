#include "../include/reuters_encoder.h"
#include "../include/utp_sbe/utp_sbe/AdminHeartbeat.h"
#include "../include/utp_sbe/utp_sbe/MDFullRefresh.h"
#include "../include/utp_sbe/utp_sbe/MDIncrementalRefresh.h"
#include "../include/utp_sbe/utp_sbe/MDIncrementalRefreshTrades.h"
#include "../include/utp_sbe/utp_sbe/MessageHeader.h"
#include "../include/utp_sbe/utp_sbe/SecurityDefinition.h"
#include <algorithm>
#include <arpa/inet.h>
#include <chrono>
#include <cstring>

namespace reuters_protocol {

std::vector<uint8_t> ReutersEncoder::encode_heartbeat()
{
    std::vector<uint8_t> buffer(1024);

    // Initialize SBE message header first
    utp_sbe::MessageHeader header;
    header.wrap(reinterpret_cast<char*>(buffer.data()), 0, 0, buffer.size())
        .blockLength(utp_sbe::AdminHeartbeat::SBE_BLOCK_LENGTH)
        .templateId(utp_sbe::AdminHeartbeat::SBE_TEMPLATE_ID)
        .schemaId(utp_sbe::AdminHeartbeat::SBE_SCHEMA_ID)
        .version(utp_sbe::AdminHeartbeat::SBE_SCHEMA_VERSION);

    // UTP Admin Heartbeat is very simple - just header
    utp_sbe::AdminHeartbeat heartbeat;
    heartbeat.wrapForEncode(reinterpret_cast<char*>(buffer.data()), header.encodedLength(), buffer.size());

    size_t encoded_length = header.encodedLength() + heartbeat.encodedLength();
    buffer.resize(encoded_length);
    return buffer;
}

std::vector<uint8_t> ReutersEncoder::encode_security_definition(
    const market_core::Instrument& instrument)
{
    std::vector<uint8_t> buffer(1024);

    // Initialize SBE message header first
    utp_sbe::MessageHeader header;
    header.wrap(reinterpret_cast<char*>(buffer.data()), 0, 0, buffer.size())
        .blockLength(utp_sbe::SecurityDefinition::SBE_BLOCK_LENGTH)
        .templateId(utp_sbe::SecurityDefinition::SBE_TEMPLATE_ID)
        .schemaId(utp_sbe::SecurityDefinition::SBE_SCHEMA_ID)
        .version(utp_sbe::SecurityDefinition::SBE_SCHEMA_VERSION);

    // Initialize SecurityDefinition message after header
    utp_sbe::SecurityDefinition secDef;
    secDef.wrapForEncode(reinterpret_cast<char*>(buffer.data()), header.encodedLength(), buffer.size());

    // Set basic instrument fields using the correct UTP SBE methods
    secDef.securityID(instrument.instrument_id);
    secDef.putSymbol(instrument.primary_symbol);
    secDef.putCurrency1("USD");
    secDef.putCurrency2("EUR");
    secDef.lastUpdateTime(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch())
                              .count());

    size_t encoded_length = header.encodedLength() + secDef.encodedLength();
    buffer.resize(encoded_length);
    return buffer;
}

std::vector<uint8_t> ReutersEncoder::encode_market_data_snapshot(
    const market_core::SnapshotEvent& snapshot)
{
    std::vector<uint8_t> buffer(4096);

    // Initialize SBE message header first
    utp_sbe::MessageHeader header;
    header.wrap(reinterpret_cast<char*>(buffer.data()), 0, 0, buffer.size())
        .blockLength(utp_sbe::MDFullRefresh::SBE_BLOCK_LENGTH)
        .templateId(utp_sbe::MDFullRefresh::SBE_TEMPLATE_ID)
        .schemaId(utp_sbe::MDFullRefresh::SBE_SCHEMA_ID)
        .version(utp_sbe::MDFullRefresh::SBE_SCHEMA_VERSION);

    // Initialize MDFullRefresh message after header
    utp_sbe::MDFullRefresh mdSnapshot;
    mdSnapshot.wrapForEncode(reinterpret_cast<char*>(buffer.data()), header.encodedLength(), buffer.size());

    mdSnapshot.securityID(snapshot.instrument_id)
        .transactTime(snapshot.timestamp_ns)
        .rptSeq(snapshot.sequence_number);

    // Add bid/ask levels using repeating group
    auto& entries = mdSnapshot.noMDEntriesCount(static_cast<uint16_t>(snapshot.bid_levels.size() + snapshot.ask_levels.size()));

    // Add bid levels
    for (const auto& level : snapshot.bid_levels) {
        auto& entry = entries.next();
        entry.mDEntryType(utp_sbe::MDEntryType::BID);
        entry.mDEntryPx().mantissa(static_cast<int64_t>(level.price * 1e9)); // Convert to fixed point
        entry.mDEntrySize(level.quantity);
        // numberOfOrders not available in UTP MDFullRefresh
    }

    // Add ask levels
    for (const auto& level : snapshot.ask_levels) {
        auto& entry = entries.next();
        entry.mDEntryType(utp_sbe::MDEntryType::OFFER);
        entry.mDEntryPx().mantissa(static_cast<int64_t>(level.price * 1e9)); // Convert to fixed point
        entry.mDEntrySize(level.quantity);
        // numberOfOrders not available in UTP MDFullRefresh
    }

    size_t encoded_length = header.encodedLength() + mdSnapshot.encodedLength();
    buffer.resize(encoded_length);
    return buffer;
}

std::vector<uint8_t> ReutersEncoder::encode_market_data_incremental(
    const market_core::QuoteEvent& quote)
{
    std::vector<uint8_t> buffer(1024);

    // Initialize SBE message header first
    utp_sbe::MessageHeader header;
    header.wrap(reinterpret_cast<char*>(buffer.data()), 0, 0, buffer.size())
        .blockLength(utp_sbe::MDIncrementalRefresh::SBE_BLOCK_LENGTH)
        .templateId(utp_sbe::MDIncrementalRefresh::SBE_TEMPLATE_ID)
        .schemaId(utp_sbe::MDIncrementalRefresh::SBE_SCHEMA_ID)
        .version(utp_sbe::MDIncrementalRefresh::SBE_SCHEMA_VERSION);

    // Initialize MDIncrementalRefresh message after header
    utp_sbe::MDIncrementalRefresh mdIncremental;
    mdIncremental.wrapForEncode(reinterpret_cast<char*>(buffer.data()), header.encodedLength(), buffer.size());

    // Set message-level fields
    mdIncremental.securityID(quote.instrument_id)
        .rptSeq(quote.sequence_number)
        .transactTime(quote.timestamp_ns);

    // Add entry in the repeating group
    auto& entries = mdIncremental.noMDEntriesCount(1);
    auto& entry = entries.next();

    entry.mDUpdateAction(static_cast<utp_sbe::MDUpdateAction::Value>(quote.action))
        .mDEntryType(quote.side == market_core::Side::BID ? utp_sbe::MDEntryType::BID : utp_sbe::MDEntryType::OFFER);
    entry.mDEntryPx().mantissa(static_cast<int64_t>(quote.price * 1e9)); // Convert to fixed point
    entry.mDEntrySize(quote.quantity);

    size_t encoded_length = header.encodedLength() + mdIncremental.encodedLength();
    buffer.resize(encoded_length);
    return buffer;
}

std::vector<uint8_t> ReutersEncoder::encode_market_data_incremental(
    const market_core::TradeEvent& trade)
{
    std::vector<uint8_t> buffer(1024);

    // Initialize SBE message header first
    utp_sbe::MessageHeader header;
    header.wrap(reinterpret_cast<char*>(buffer.data()), 0, 0, buffer.size())
        .blockLength(utp_sbe::MDIncrementalRefreshTrades::SBE_BLOCK_LENGTH)
        .templateId(utp_sbe::MDIncrementalRefreshTrades::SBE_TEMPLATE_ID)
        .schemaId(utp_sbe::MDIncrementalRefreshTrades::SBE_SCHEMA_ID)
        .version(utp_sbe::MDIncrementalRefreshTrades::SBE_SCHEMA_VERSION);

    // Initialize MDIncrementalRefreshTrades message after header
    utp_sbe::MDIncrementalRefreshTrades mdTrade;
    mdTrade.wrapForEncode(reinterpret_cast<char*>(buffer.data()), header.encodedLength(), buffer.size());

    // Set message-level fields
    mdTrade.securityID(trade.instrument_id);

    // Add entry in the repeating group
    auto& entries = mdTrade.noMDEntriesCount(1);
    auto& entry = entries.next();

    entry.transactTime(trade.timestamp_ns);
    entry.mDEntryPx().mantissa(static_cast<int64_t>(trade.price * 1e9)); // Convert to fixed point
    entry.mDEntrySize(trade.quantity);

    // Set aggressor side if available
    if (trade.aggressor_side.has_value()) {
        entry.aggressorSide(trade.aggressor_side.value() == market_core::Side::BID ? utp_sbe::AggressorSide::BUYSIDE : utp_sbe::AggressorSide::SELLSIDE);
    } else {
        entry.aggressorSide(utp_sbe::AggressorSide::NONE);
    }

    size_t encoded_length = header.encodedLength() + mdTrade.encodedLength();
    buffer.resize(encoded_length);
    return buffer;
}

// UTP is pure market data multicast - no session management functions needed

} // namespace reuters_protocol