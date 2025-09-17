#pragma once

#include "instrument.h"
#include "market_events.h"
#include "order_book_manager.h"
#include <chrono>
#include <functional>
#include <memory>
#include <random>
#include <vector>

namespace market_core {

// Market behavior modes
enum class MarketMode {
    NORMAL,
    FAST,
    THIN,
    VOLATILE,
    TRENDING,
    AUCTION,
    STRESSED
};

// Market generation configuration
struct MarketConfig {
    MarketMode mode = MarketMode::NORMAL;
    double volatility = 0.0001; // Base volatility (0.01%)
    double spread_factor = 1.0; // Multiplier for spread width
    int updates_per_second = 10; // Rate of update generation
    double trade_probability = 0.3; // Probability of trade per update
    double trend_bias = 0.0; // Directional bias (-1 to 1)
    double book_depth_target = 5; // Target number of levels per side
    bool generate_implied = false; // Generate implied prices (CME)
    bool generate_statistics = true; // Generate OHLC stats
};

// Event listener interface
class IMarketEventListener {
public:
    virtual ~IMarketEventListener() = default;
    virtual void on_market_event(const std::shared_ptr<MarketEvent>& event) = 0;
};

// Market data generator - protocol agnostic
class MarketDataGenerator {
public:
    MarketDataGenerator(std::shared_ptr<OrderBookManager> book_manager);
    ~MarketDataGenerator() = default;

    // Configuration
    void set_config(const MarketConfig& config) { config_ = config; }
    const MarketConfig& get_config() const { return config_; }

    // Set market mode presets
    void set_market_mode(MarketMode mode);

    // Event generation
    void generate_update(uint32_t instrument_id);
    void generate_batch(int count);
    void generate_all_instruments();

    // Specific event generation
    std::shared_ptr<QuoteEvent> generate_quote(uint32_t instrument_id);
    std::shared_ptr<TradeEvent> generate_trade(uint32_t instrument_id);
    std::shared_ptr<StatisticsEvent> generate_statistics(uint32_t instrument_id);
    std::shared_ptr<SnapshotEvent> generate_snapshot(uint32_t instrument_id);

    // Event listeners (protocols register here)
    void add_listener(std::weak_ptr<IMarketEventListener> listener);
    void remove_listener(std::shared_ptr<IMarketEventListener> listener);
    void clear_listeners();

    // Statistics
    struct Statistics {
        uint64_t updates_generated = 0;
        uint64_t trades_generated = 0;
        uint64_t quotes_generated = 0;
        uint64_t snapshots_generated = 0;
        std::chrono::steady_clock::time_point start_time;
    };
    const Statistics& get_statistics() const { return stats_; }
    void reset_statistics();

private:
    std::shared_ptr<OrderBookManager> book_manager_;
    MarketConfig config_;
    Statistics stats_;

    // Event listeners
    std::vector<std::weak_ptr<IMarketEventListener>> listeners_;
    std::mutex listeners_mutex_;

    // Random number generation
    std::mt19937 rng_;
    std::uniform_real_distribution<> uniform_dist_ { 0.0, 1.0 };
    std::normal_distribution<> normal_dist_ { 0.0, 1.0 };
    std::poisson_distribution<> poisson_dist_ { 3 };

    // Sequence tracking
    std::unordered_map<uint32_t, uint32_t> instrument_sequences_;

    // Helper methods
    void notify_listeners(const std::shared_ptr<MarketEvent>& event);
    double calculate_price_movement(double current_price, const Instrument& instrument);
    uint64_t calculate_quantity(const Instrument& instrument);
    bool should_generate_trade();
    Side choose_aggressor_side();
    UpdateAction choose_update_action();
    double apply_tick_rounding(double price, double tick_size) const;

    // Instrument-specific generation
    void generate_futures_update(const FuturesInstrument& futures);
    void generate_fx_update(const FXSpotInstrument& fx);
    void generate_option_update(const OptionInstrument& option);

    // Get next sequence number for instrument
    uint32_t get_next_sequence(uint32_t instrument_id);
};

} // namespace market_core