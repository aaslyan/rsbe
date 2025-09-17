#include "../include/order_book_manager.h"
#include <algorithm>

namespace market_core {

bool OrderBookManager::add_instrument(std::shared_ptr<Instrument> instrument)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!instrument) {
        return false;
    }

    uint32_t id = instrument->instrument_id;
    if (instruments_.find(id) != instruments_.end()) {
        return false; // Already exists
    }

    instruments_[id] = instrument;
    return true;
}

std::shared_ptr<Instrument> OrderBookManager::get_instrument(uint32_t instrument_id) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = instruments_.find(instrument_id);
    if (it != instruments_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Instrument>> OrderBookManager::get_all_instruments() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::shared_ptr<Instrument>> result;
    result.reserve(instruments_.size());

    for (const auto& [id, instrument] : instruments_) {
        result.push_back(instrument);
    }

    return result;
}

std::vector<uint32_t> OrderBookManager::get_all_instrument_ids() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<uint32_t> result;
    result.reserve(instruments_.size());

    for (const auto& [id, instrument] : instruments_) {
        result.push_back(id);
    }

    return result;
}

bool OrderBookManager::create_order_book(
    uint32_t instrument_id,
    const OrderBook::Config& config)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!validate_instrument(instrument_id)) {
        return false; // Instrument doesn't exist
    }

    if (order_books_.find(instrument_id) != order_books_.end()) {
        return false; // Order book already exists
    }

    auto instrument = instruments_[instrument_id];
    auto book = std::make_shared<OrderBook>(instrument_id, instrument->primary_symbol);
    book->set_config(config);

    order_books_[instrument_id] = book;
    return true;
}

std::shared_ptr<OrderBook> OrderBookManager::get_order_book(uint32_t instrument_id) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = order_books_.find(instrument_id);
    if (it != order_books_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<OrderBook>> OrderBookManager::get_all_order_books() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::shared_ptr<OrderBook>> result;
    result.reserve(order_books_.size());

    for (const auto& [id, book] : order_books_) {
        result.push_back(book);
    }

    return result;
}

std::pair<std::shared_ptr<Instrument>, std::shared_ptr<OrderBook>>
OrderBookManager::get_instrument_and_book(uint32_t instrument_id) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto instrument_it = instruments_.find(instrument_id);
    auto book_it = order_books_.find(instrument_id);

    std::shared_ptr<Instrument> instrument = (instrument_it != instruments_.end()) ? instrument_it->second : nullptr;

    std::shared_ptr<OrderBook> book = (book_it != order_books_.end()) ? book_it->second : nullptr;

    return { instrument, book };
}

void OrderBookManager::clear_all_books()
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& [id, book] : order_books_) {
        book->clear();
    }
}

void OrderBookManager::reset_all_books()
{
    std::lock_guard<std::mutex> lock(mutex_);
    order_books_.clear();
}

void OrderBookManager::apply_event(const std::shared_ptr<MarketEvent>& event)
{
    if (!event) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    auto book_it = order_books_.find(event->instrument_id);
    if (book_it != order_books_.end()) {
        book_it->second->apply_event(event);
    }
}

std::shared_ptr<SnapshotEvent> OrderBookManager::create_snapshot(
    uint32_t instrument_id,
    size_t max_levels) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto book_it = order_books_.find(instrument_id);
    if (book_it != order_books_.end()) {
        return book_it->second->create_snapshot_event(max_levels);
    }
    return nullptr;
}

bool OrderBookManager::validate_instrument(uint32_t instrument_id) const
{
    return instruments_.find(instrument_id) != instruments_.end();
}

} // namespace market_core