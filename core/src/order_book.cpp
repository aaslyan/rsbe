#include "../include/order_book.h"
#include <algorithm>
#include <chrono>
#include <cmath>

namespace market_core {

OrderBook::OrderBook(uint32_t instrument_id, const std::string& symbol)
    : instrument_id_(instrument_id)
    , symbol_(symbol)
    , config_()
{
}

void OrderBook::add_level(Side side, const PriceLevel& level)
{
    if (side == Side::BID) {
        bids_[level.price] = level;
    } else if (side == Side::ASK) {
        asks_[level.price] = level;
    }
}

void OrderBook::update_level(Side side, const PriceLevel& level)
{
    if (level.quantity == 0) {
        remove_level(side, level.price);
        return;
    }

    if (side == Side::BID) {
        bids_[level.price] = level;
    } else if (side == Side::ASK) {
        asks_[level.price] = level;
    }
}

void OrderBook::remove_level(Side side, double price)
{
    if (side == Side::BID) {
        bids_.erase(price);
    } else if (side == Side::ASK) {
        asks_.erase(price);
    }
}

void OrderBook::clear_side(Side side)
{
    if (side == Side::BID) {
        bids_.clear();
    } else if (side == Side::ASK) {
        asks_.clear();
    }
}

void OrderBook::clear()
{
    bids_.clear();
    asks_.clear();
    recent_trades_.clear();
    stats_ = MarketStats {};
}

void OrderBook::add_trade(const Trade& trade)
{
    recent_trades_.push_back(trade);

    // Keep only recent trades
    if (recent_trades_.size() > 100) {
        recent_trades_.erase(recent_trades_.begin());
    }

    update_stats_on_trade(trade);
}

std::vector<PriceLevel> OrderBook::get_bids(size_t max_levels) const
{
    std::vector<PriceLevel> result;
    result.reserve(std::min(max_levels, bids_.size()));

    size_t count = 0;
    for (const auto& [price, level] : bids_) {
        if (count >= max_levels)
            break;
        result.push_back(level);
        ++count;
    }

    return result;
}

std::vector<PriceLevel> OrderBook::get_asks(size_t max_levels) const
{
    std::vector<PriceLevel> result;
    result.reserve(std::min(max_levels, asks_.size()));

    size_t count = 0;
    for (const auto& [price, level] : asks_) {
        if (count >= max_levels)
            break;
        result.push_back(level);
        ++count;
    }

    return result;
}

std::vector<Trade> OrderBook::get_recent_trades(size_t count) const
{
    if (count >= recent_trades_.size()) {
        return recent_trades_;
    }

    return std::vector<Trade>(
        recent_trades_.end() - count,
        recent_trades_.end());
}

std::optional<double> OrderBook::get_best_bid() const
{
    if (bids_.empty()) {
        return std::nullopt;
    }
    return bids_.begin()->first;
}

std::optional<double> OrderBook::get_best_ask() const
{
    if (asks_.empty()) {
        return std::nullopt;
    }
    return asks_.begin()->first;
}

std::optional<double> OrderBook::get_mid_price() const
{
    auto best_bid = get_best_bid();
    auto best_ask = get_best_ask();

    if (best_bid && best_ask) {
        return (*best_bid + *best_ask) / 2.0;
    }
    return std::nullopt;
}

std::optional<double> OrderBook::get_spread() const
{
    auto best_bid = get_best_bid();
    auto best_ask = get_best_ask();

    if (best_bid && best_ask) {
        return *best_ask - *best_bid;
    }
    return std::nullopt;
}

bool OrderBook::is_crossed() const
{
    auto best_bid = get_best_bid();
    auto best_ask = get_best_ask();

    if (best_bid && best_ask) {
        return *best_bid >= *best_ask;
    }
    return false;
}

std::shared_ptr<SnapshotEvent> OrderBook::create_snapshot_event(size_t max_levels) const
{
    auto snapshot = std::make_shared<SnapshotEvent>(instrument_id_);

    // Get current timestamp
    auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch())
                   .count();
    snapshot->timestamp_ns = now;

    // Add bid levels
    size_t bid_count = 0;
    for (const auto& [price, level] : bids_) {
        if (bid_count >= max_levels)
            break;

        QuoteEvent bid_quote(instrument_id_);
        bid_quote.side = Side::BID;
        bid_quote.price = level.price;
        bid_quote.quantity = level.quantity;
        bid_quote.order_count = level.order_count;
        if (level.level_number) {
            bid_quote.price_level = *level.level_number;
        } else {
            bid_quote.price_level = static_cast<uint8_t>(bid_count + 1);
        }

        snapshot->bid_levels.push_back(bid_quote);
        ++bid_count;
    }

    // Add ask levels
    size_t ask_count = 0;
    for (const auto& [price, level] : asks_) {
        if (ask_count >= max_levels)
            break;

        QuoteEvent ask_quote(instrument_id_);
        ask_quote.side = Side::ASK;
        ask_quote.price = level.price;
        ask_quote.quantity = level.quantity;
        ask_quote.order_count = level.order_count;
        if (level.level_number) {
            ask_quote.price_level = *level.level_number;
        } else {
            ask_quote.price_level = static_cast<uint8_t>(ask_count + 1);
        }

        snapshot->ask_levels.push_back(ask_quote);
        ++ask_count;
    }

    // Add statistics
    snapshot->last_trade_price = (stats_.last_price > 0) ? std::optional<double>(stats_.last_price) : std::nullopt;
    snapshot->total_volume = stats_.total_volume;

    return snapshot;
}

void OrderBook::apply_event(const std::shared_ptr<MarketEvent>& event)
{
    switch (event->type) {
    case MarketEvent::EventType::QUOTE_UPDATE: {
        auto quote = std::static_pointer_cast<QuoteEvent>(event);
        apply_quote_event(*quote);
        break;
    }
    case MarketEvent::EventType::TRADE: {
        auto trade = std::static_pointer_cast<TradeEvent>(event);
        apply_trade_event(*trade);
        break;
    }
    case MarketEvent::EventType::BOOK_CLEAR: {
        clear();
        break;
    }
    default:
        // Ignore other event types for now
        break;
    }
}

void OrderBook::update_stats_on_trade(const Trade& trade)
{
    stats_.last_price = trade.price;
    stats_.total_volume += trade.quantity;
    stats_.trade_count++;

    // Update OHLC
    if (stats_.open_price == 0.0) {
        stats_.open_price = trade.price;
    }

    if (trade.price > stats_.high_price || stats_.high_price == 0.0) {
        stats_.high_price = trade.price;
    }

    if (trade.price < stats_.low_price || stats_.low_price == 0.0) {
        stats_.low_price = trade.price;
    }

    // Update VWAP
    if (stats_.trade_count > 1) {
        double total_value = stats_.vwap * (stats_.total_volume - trade.quantity) + trade.price * trade.quantity;
        stats_.vwap = total_value / stats_.total_volume;
    } else {
        stats_.vwap = trade.price;
    }
}

void OrderBook::apply_quote_event(const QuoteEvent& quote)
{
    PriceLevel level;
    level.price = quote.price;
    level.quantity = quote.quantity;
    level.order_count = quote.order_count;
    level.last_update_time = quote.timestamp_ns;
    level.level_number = quote.price_level;

    switch (quote.action) {
    case UpdateAction::ADD:
    case UpdateAction::CHANGE:
    case UpdateAction::OVERLAY:
        update_level(quote.side, level);
        break;
    case UpdateAction::DELETE:
        remove_level(quote.side, quote.price);
        break;
    case UpdateAction::CLEAR:
        clear_side(quote.side);
        break;
    }
}

void OrderBook::apply_trade_event(const TradeEvent& trade_event)
{
    Trade trade;
    trade.price = trade_event.price;
    trade.quantity = trade_event.quantity;
    trade.timestamp_ns = trade_event.timestamp_ns;
    trade.aggressor_side = trade_event.aggressor_side;
    trade.trade_id = trade_event.trade_id;

    add_trade(trade);
}

} // namespace market_core