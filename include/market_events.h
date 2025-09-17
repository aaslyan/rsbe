#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace market_core {

// Forward declarations
class Instrument;
class OrderBook;

// Common types across all protocols
enum class Side {
    BID,
    ASK,
    NONE
};

enum class UpdateAction {
    ADD,
    CHANGE,
    DELETE,
    OVERLAY, // Full replacement
    CLEAR
};

enum class InstrumentType {
    FUTURE,
    OPTION,
    FX_SPOT,
    FX_FORWARD,
    EQUITY,
    SPREAD,
    UNKNOWN
};

// Base market event
class MarketEvent {
public:
    enum EventType {
        QUOTE_UPDATE,
        TRADE,
        BOOK_CLEAR,
        STATISTICS,
        STATUS_CHANGE,
        SNAPSHOT,
        IMBALANCE
    };

    EventType type;
    uint32_t instrument_id;
    uint64_t timestamp_ns;
    uint32_t sequence_number;

    MarketEvent(EventType t, uint32_t id)
        : type(t)
        , instrument_id(id)
        , timestamp_ns(0)
        , sequence_number(0)
    {
    }

    virtual ~MarketEvent() = default;
};

// Quote/Level update event
class QuoteEvent : public MarketEvent {
public:
    Side side;
    double price;
    uint64_t quantity;
    UpdateAction action;
    uint32_t order_count;

    // Optional fields for different protocols
    std::optional<uint8_t> price_level; // For CME levels
    std::optional<uint32_t> rpt_seq; // For CME sequence
    std::optional<std::string> market_maker_id; // For Reuters contributors
    std::optional<uint64_t> implied_quantity; // For CME implied

    QuoteEvent(uint32_t instrument_id)
        : MarketEvent(EventType::QUOTE_UPDATE, instrument_id)
        , side(Side::BID)
        , price(0.0)
        , quantity(0)
        , action(UpdateAction::ADD)
        , order_count(0)
    {
    }
};

// Trade event
class TradeEvent : public MarketEvent {
public:
    double price;
    uint64_t quantity;
    std::optional<Side> aggressor_side;
    std::optional<std::string> trade_id;
    std::optional<uint32_t> rpt_seq;

    TradeEvent(uint32_t instrument_id)
        : MarketEvent(EventType::TRADE, instrument_id)
        , price(0.0)
        , quantity(0)
    {
    }
};

// Statistics event (OHLC, settlement, etc.)
class StatisticsEvent : public MarketEvent {
public:
    enum StatType {
        OPEN,
        HIGH,
        LOW,
        CLOSE,
        SETTLEMENT,
        VWAP,
        TRADE_VOLUME
    };

    StatType stat_type;
    double value;
    std::optional<uint64_t> volume;

    StatisticsEvent(uint32_t instrument_id)
        : MarketEvent(EventType::STATISTICS, instrument_id)
        , stat_type(StatType::CLOSE)
        , value(0.0)
    {
    }
};

// Snapshot event - full book state
class SnapshotEvent : public MarketEvent {
public:
    std::vector<QuoteEvent> bid_levels;
    std::vector<QuoteEvent> ask_levels;
    std::optional<double> last_trade_price;
    std::optional<uint64_t> total_volume;
    std::optional<uint32_t> rpt_seq;

    SnapshotEvent(uint32_t instrument_id)
        : MarketEvent(EventType::SNAPSHOT, instrument_id)
    {
    }
};

// Status change event
class StatusEvent : public MarketEvent {
public:
    enum Status {
        PRE_OPEN,
        OPENING_AUCTION,
        CONTINUOUS_TRADING,
        CLOSING_AUCTION,
        POST_CLOSE,
        HALTED,
        PAUSED
    };

    Status status;
    std::optional<std::string> halt_reason;

    StatusEvent(uint32_t instrument_id)
        : MarketEvent(EventType::STATUS_CHANGE, instrument_id)
        , status(Status::CONTINUOUS_TRADING)
    {
    }
};

} // namespace market_core