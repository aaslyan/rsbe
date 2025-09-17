#pragma once

#include "market_events.h"
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace market_core {

// Price level in the order book
struct PriceLevel {
    double price;
    uint64_t quantity;
    uint32_t order_count;
    uint64_t last_update_time;

    // Optional protocol-specific fields
    std::optional<uint64_t> implied_quantity; // For CME implied prices
    std::optional<std::string> market_maker_id; // For Reuters contributors
    std::optional<uint8_t> level_number; // Explicit level number for some protocols
};

// Trade information
struct Trade {
    double price;
    uint64_t quantity;
    uint64_t timestamp_ns;
    std::optional<Side> aggressor_side;
    std::optional<std::string> trade_id;
};

// Market statistics
struct MarketStats {
    double open_price = 0.0;
    double high_price = 0.0;
    double low_price = 0.0;
    double last_price = 0.0;
    double settlement_price = 0.0;
    uint64_t total_volume = 0;
    uint32_t trade_count = 0;
    double vwap = 0.0;

    // Optional fields for different protocols
    std::optional<double> previous_settlement;
    std::optional<double> open_interest;
    std::optional<double> cleared_volume;
};

// Protocol-agnostic order book
class OrderBook {
public:
    OrderBook(uint32_t instrument_id, const std::string& symbol);
    ~OrderBook() = default;

    // Core order book operations
    void add_level(Side side, const PriceLevel& level);
    void update_level(Side side, const PriceLevel& level);
    void remove_level(Side side, double price);
    void clear_side(Side side);
    void clear();

    // Trade operations
    void add_trade(const Trade& trade);

    // Getters with configurable depth
    std::vector<PriceLevel> get_bids(size_t max_levels = SIZE_MAX) const;
    std::vector<PriceLevel> get_asks(size_t max_levels = SIZE_MAX) const;
    std::vector<Trade> get_recent_trades(size_t count = 10) const;

    // Best prices
    std::optional<double> get_best_bid() const;
    std::optional<double> get_best_ask() const;
    std::optional<double> get_mid_price() const;
    std::optional<double> get_spread() const;

    // Statistics
    const MarketStats& get_stats() const { return stats_; }
    void update_stats(const MarketStats& stats) { stats_ = stats; }

    // Identifiers
    uint32_t get_instrument_id() const { return instrument_id_; }
    const std::string& get_symbol() const { return symbol_; }

    // State queries
    bool is_crossed() const;
    size_t bid_depth() const { return bids_.size(); }
    size_t ask_depth() const { return asks_.size(); }
    bool is_empty() const { return bids_.empty() && asks_.empty(); }

    // Configuration for different protocols
    struct Config {
        size_t max_visible_levels = 10; // How many levels to maintain
        bool maintain_implied_prices = false; // For CME
        bool track_market_makers = false; // For Reuters
        bool aggregate_by_price = true; // Aggregate orders at same price
    };

    void set_config(const Config& config) { config_ = config; }
    const Config& get_config() const { return config_; }

    // Generate snapshot event from current state
    std::shared_ptr<SnapshotEvent> create_snapshot_event(size_t max_levels = SIZE_MAX) const;

    // Apply an event to the book
    void apply_event(const std::shared_ptr<MarketEvent>& event);

private:
    uint32_t instrument_id_;
    std::string symbol_;
    Config config_;

    // Price-ordered maps
    std::map<double, PriceLevel, std::greater<double>> bids_; // Descending
    std::map<double, PriceLevel> asks_; // Ascending

    std::vector<Trade> recent_trades_;
    MarketStats stats_;

    // Helper methods
    void update_stats_on_trade(const Trade& trade);
    void apply_quote_event(const QuoteEvent& quote);
    void apply_trade_event(const TradeEvent& trade);
};

} // namespace market_core