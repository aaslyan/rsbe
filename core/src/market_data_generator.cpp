#include "../include/market_data_generator.h"
#include <chrono>
#include <random>

namespace market_core {

MarketDataGenerator::MarketDataGenerator(std::shared_ptr<OrderBookManager> book_manager)
    : book_manager_(book_manager)
    , config_()
    , rng_(std::chrono::steady_clock::now().time_since_epoch().count())
{
    stats_.start_time = std::chrono::steady_clock::now();
}

void MarketDataGenerator::set_market_mode(MarketMode mode)
{
    config_.mode = mode;

    // Set mode-specific parameters
    switch (mode) {
    case MarketMode::NORMAL:
        config_.volatility = 0.0001; // 0.01%
        config_.updates_per_second = 10;
        config_.trade_probability = 0.3;
        break;
    case MarketMode::FAST:
        config_.volatility = 0.0002;
        config_.updates_per_second = 50;
        config_.trade_probability = 0.5;
        break;
    case MarketMode::VOLATILE:
        config_.volatility = 0.001; // 0.1%
        config_.updates_per_second = 20;
        config_.trade_probability = 0.4;
        break;
    case MarketMode::THIN:
        config_.volatility = 0.00005;
        config_.updates_per_second = 3;
        config_.trade_probability = 0.1;
        config_.book_depth_target = 2;
        break;
    case MarketMode::TRENDING:
        config_.volatility = 0.0001;
        config_.trend_bias = 0.3; // Upward bias
        config_.updates_per_second = 15;
        break;
    case MarketMode::STRESSED:
        config_.volatility = 0.002; // 0.2%
        config_.updates_per_second = 100;
        config_.trade_probability = 0.7;
        config_.spread_factor = 3.0;
        break;
    default:
        break;
    }
}

void MarketDataGenerator::generate_update(uint32_t instrument_id)
{
    auto [instrument, book] = book_manager_->get_instrument_and_book(instrument_id);
    if (!instrument || !book) {
        return;
    }

    // Decide what type of update to generate
    if (should_generate_trade()) {
        auto trade_event = generate_trade(instrument_id);
        if (trade_event) {
            notify_listeners(trade_event);
            stats_.trades_generated++;
        }
    } else {
        auto quote_event = generate_quote(instrument_id);
        if (quote_event) {
            notify_listeners(quote_event);
            stats_.quotes_generated++;
        }
    }

    stats_.updates_generated++;
}

void MarketDataGenerator::generate_batch(int count)
{
    auto instrument_ids = book_manager_->get_all_instrument_ids();
    if (instrument_ids.empty()) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        // Pick random instrument
        std::uniform_int_distribution<> inst_dist(0, instrument_ids.size() - 1);
        uint32_t instrument_id = instrument_ids[inst_dist(rng_)];
        generate_update(instrument_id);
    }
}

void MarketDataGenerator::generate_all_instruments()
{
    auto instrument_ids = book_manager_->get_all_instrument_ids();
    for (uint32_t instrument_id : instrument_ids) {
        generate_update(instrument_id);
    }
}

std::shared_ptr<QuoteEvent> MarketDataGenerator::generate_quote(uint32_t instrument_id)
{
    auto [instrument, book] = book_manager_->get_instrument_and_book(instrument_id);
    if (!instrument || !book) {
        return nullptr;
    }

    auto quote = std::make_shared<QuoteEvent>(instrument_id);
    quote->timestamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch())
                              .count();
    quote->sequence_number = get_next_sequence(instrument_id);

    // Choose side
    quote->side = (uniform_dist_(rng_) < 0.5) ? Side::BID : Side::ASK;

    // Choose action
    quote->action = choose_update_action();

    // Get current best prices
    auto best_bid = book->get_best_bid();
    auto best_ask = book->get_best_ask();

    // Generate price based on current market
    double reference_price = 0.0;
    if (best_bid && best_ask) {
        reference_price = (*best_bid + *best_ask) / 2.0;
    } else {
        // Use instrument's initial price if no market exists
        auto initial_price = instrument->get_property<double>("initial_price");
        reference_price = initial_price.value_or(100.0);
    }

    // Apply price movement
    double price_move = calculate_price_movement(reference_price, *instrument);
    double new_price = reference_price + price_move;

    // Round to tick size
    quote->price = apply_tick_rounding(new_price, instrument->tick_size);

    // Adjust price based on side and action
    if (quote->side == Side::BID && best_bid) {
        if (quote->action == UpdateAction::ADD) {
            quote->price = std::min(quote->price, *best_bid - instrument->tick_size);
        }
    } else if (quote->side == Side::ASK && best_ask) {
        if (quote->action == UpdateAction::ADD) {
            quote->price = std::max(quote->price, *best_ask + instrument->tick_size);
        }
    }

    quote->quantity = calculate_quantity(*instrument);
    quote->order_count = std::max(1U, static_cast<uint32_t>(quote->quantity / 1000));

    // Set price level (1-based)
    if (quote->side == Side::BID) {
        auto bids = book->get_bids();
        quote->price_level = static_cast<uint8_t>(bids.size() + 1);
    } else {
        auto asks = book->get_asks();
        quote->price_level = static_cast<uint8_t>(asks.size() + 1);
    }

    return quote;
}

std::shared_ptr<TradeEvent> MarketDataGenerator::generate_trade(uint32_t instrument_id)
{
    auto [instrument, book] = book_manager_->get_instrument_and_book(instrument_id);
    if (!instrument || !book) {
        return nullptr;
    }

    auto best_bid = book->get_best_bid();
    auto best_ask = book->get_best_ask();

    if (!best_bid || !best_ask) {
        return nullptr; // No market to trade against
    }

    auto trade = std::make_shared<TradeEvent>(instrument_id);
    trade->timestamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch())
                              .count();
    trade->sequence_number = get_next_sequence(instrument_id);

    // Choose aggressor side
    Side aggressor = choose_aggressor_side();
    trade->aggressor_side = aggressor;

    // Trade at best price of opposite side
    if (aggressor == Side::BID) {
        trade->price = *best_ask; // Buy at offer
    } else {
        trade->price = *best_bid; // Sell at bid
    }

    trade->quantity = calculate_quantity(*instrument) / 2; // Trades typically smaller

    return trade;
}

std::shared_ptr<StatisticsEvent> MarketDataGenerator::generate_statistics(uint32_t instrument_id)
{
    auto [instrument, book] = book_manager_->get_instrument_and_book(instrument_id);
    if (!instrument || !book) {
        return nullptr;
    }

    auto stats_event = std::make_shared<StatisticsEvent>(instrument_id);
    stats_event->timestamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch())
                                    .count();
    stats_event->sequence_number = get_next_sequence(instrument_id);

    const auto& book_stats = book->get_stats();

    // Generate different types of statistics
    std::uniform_int_distribution<> stat_dist(0, 6);
    int stat_choice = stat_dist(rng_);

    switch (stat_choice) {
    case 0:
        stats_event->stat_type = StatisticsEvent::OPEN;
        stats_event->value = book_stats.open_price;
        break;
    case 1:
        stats_event->stat_type = StatisticsEvent::HIGH;
        stats_event->value = book_stats.high_price;
        break;
    case 2:
        stats_event->stat_type = StatisticsEvent::LOW;
        stats_event->value = book_stats.low_price;
        break;
    case 3:
        stats_event->stat_type = StatisticsEvent::CLOSE;
        stats_event->value = book_stats.last_price;
        break;
    case 4:
        stats_event->stat_type = StatisticsEvent::SETTLEMENT;
        stats_event->value = book_stats.settlement_price;
        break;
    case 5:
        stats_event->stat_type = StatisticsEvent::VWAP;
        stats_event->value = book_stats.vwap;
        break;
    case 6:
        stats_event->stat_type = StatisticsEvent::TRADE_VOLUME;
        stats_event->value = static_cast<double>(book_stats.total_volume);
        stats_event->volume = book_stats.total_volume;
        break;
    }

    return stats_event;
}

std::shared_ptr<SnapshotEvent> MarketDataGenerator::generate_snapshot(uint32_t instrument_id)
{
    auto snapshot = book_manager_->create_snapshot(instrument_id, config_.book_depth_target);
    if (snapshot) {
        snapshot->sequence_number = get_next_sequence(instrument_id);
        stats_.snapshots_generated++;
    }
    return snapshot;
}

void MarketDataGenerator::add_listener(std::weak_ptr<IMarketEventListener> listener)
{
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    listeners_.push_back(listener);
}

void MarketDataGenerator::remove_listener(std::shared_ptr<IMarketEventListener> listener)
{
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    // Remove by comparing the locked weak_ptr
    listeners_.erase(
        std::remove_if(listeners_.begin(), listeners_.end(),
            [&listener](const std::weak_ptr<IMarketEventListener>& wp) {
                auto sp = wp.lock();
                return !sp || sp == listener;
            }),
        listeners_.end());
}

void MarketDataGenerator::clear_listeners()
{
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    listeners_.clear();
}

void MarketDataGenerator::reset_statistics()
{
    stats_ = Statistics {};
    stats_.start_time = std::chrono::steady_clock::now();
    instrument_sequences_.clear();
}

void MarketDataGenerator::notify_listeners(const std::shared_ptr<MarketEvent>& event)
{
    // Apply to local books first
    book_manager_->apply_event(event);

    // Notify protocol adapters
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    for (auto& listener : listeners_) {
        if (auto shared_listener = listener.lock()) {
            shared_listener->on_market_event(event);
        }
    }

    // Remove expired weak pointers
    listeners_.erase(
        std::remove_if(listeners_.begin(), listeners_.end(),
            [](const std::weak_ptr<IMarketEventListener>& wp) {
                return wp.expired();
            }),
        listeners_.end());
}

double MarketDataGenerator::calculate_price_movement(double current_price, const Instrument& instrument)
{
    // Base volatility
    double vol = config_.volatility;

    // Apply trend bias
    double trend = config_.trend_bias * vol * current_price;

    // Random component
    double random_move = normal_dist_(rng_) * vol * current_price;

    return trend + random_move;
}

uint64_t MarketDataGenerator::calculate_quantity(const Instrument& instrument)
{
    // Use Poisson distribution for realistic quantity distribution
    std::poisson_distribution<> qty_dist(static_cast<double>(poisson_dist_.param().mean()));
    uint64_t base_qty = qty_dist(rng_) * 100; // Multiples of 100

    // Adjust based on instrument type
    if (instrument.get_type() == InstrumentType::FX_SPOT) {
        base_qty *= 10000; // FX trades in larger sizes
    }

    return std::max(uint64_t(100), base_qty);
}

bool MarketDataGenerator::should_generate_trade()
{
    return uniform_dist_(rng_) < config_.trade_probability;
}

Side MarketDataGenerator::choose_aggressor_side()
{
    return (uniform_dist_(rng_) < 0.5) ? Side::BID : Side::ASK;
}

UpdateAction MarketDataGenerator::choose_update_action()
{
    double rand = uniform_dist_(rng_);
    if (rand < 0.6) {
        return UpdateAction::ADD;
    } else if (rand < 0.8) {
        return UpdateAction::CHANGE;
    } else {
        return UpdateAction::DELETE;
    }
}

double MarketDataGenerator::apply_tick_rounding(double price, double tick_size) const
{
    return std::round(price / tick_size) * tick_size;
}

uint32_t MarketDataGenerator::get_next_sequence(uint32_t instrument_id)
{
    return ++instrument_sequences_[instrument_id];
}

} // namespace market_core