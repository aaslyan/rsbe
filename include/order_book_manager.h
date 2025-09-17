#pragma once

#include "instrument.h"
#include "order_book.h"
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace market_core {

// Manages multiple order books and instruments
class OrderBookManager {
public:
    OrderBookManager() = default;
    ~OrderBookManager() = default;

    // Instrument management
    bool add_instrument(std::shared_ptr<Instrument> instrument);
    std::shared_ptr<Instrument> get_instrument(uint32_t instrument_id) const;
    std::vector<std::shared_ptr<Instrument>> get_all_instruments() const;
    std::vector<uint32_t> get_all_instrument_ids() const;

    // Order book management
    bool create_order_book(
        uint32_t instrument_id,
        const OrderBook::Config& config = OrderBook::Config {});
    std::shared_ptr<OrderBook> get_order_book(uint32_t instrument_id) const;
    std::vector<std::shared_ptr<OrderBook>> get_all_order_books() const;

    // Combined getters
    std::pair<std::shared_ptr<Instrument>, std::shared_ptr<OrderBook>>
    get_instrument_and_book(uint32_t instrument_id) const;

    // Bulk operations
    void clear_all_books();
    void reset_all_books();
    size_t instrument_count() const { return instruments_.size(); }
    size_t book_count() const { return order_books_.size(); }

    // Thread-safe event application
    void apply_event(const std::shared_ptr<MarketEvent>& event);

    // Generate events
    std::shared_ptr<SnapshotEvent> create_snapshot(
        uint32_t instrument_id,
        size_t max_levels = SIZE_MAX) const;

private:
    // Thread safety
    mutable std::mutex mutex_;

    // Instrument and order book storage
    std::unordered_map<uint32_t, std::shared_ptr<Instrument>> instruments_;
    std::unordered_map<uint32_t, std::shared_ptr<OrderBook>> order_books_;

    // Helper to ensure instrument exists before creating book
    bool validate_instrument(uint32_t instrument_id) const;
};

} // namespace market_core