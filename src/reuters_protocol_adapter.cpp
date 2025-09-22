#include "../include/reuters_protocol_adapter.h"
#include "../include/reuters_encoder.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>

namespace reuters_protocol {

ReutersProtocolAdapter::ReutersProtocolAdapter(uint16_t port, const ReutersMulticastConfig& multicast_config)
    : port_(port)
    , running_(false)
    , multicast_config_(multicast_config)
{
    // Initialize start time (atomics initialize themselves to 0)
    stats_.start_time = std::chrono::steady_clock::now();
    
    std::cout << "[DEBUG] ReutersProtocolAdapter initialized with events counter: " << stats_.market_events_processed.load() << std::endl;
}

ReutersProtocolAdapter::~ReutersProtocolAdapter()
{
    shutdown();
}

bool ReutersProtocolAdapter::initialize_with_multicast()
{
    // Initialize multicast publisher only - no TCP needed for UTP
    multicast_publisher_ = std::make_unique<ReutersMulticastPublisher>(multicast_config_);
    if (!multicast_publisher_->initialize()) {
        std::cerr << "Failed to initialize UTP multicast publisher" << std::endl;
        return false;
    }

    std::cout << "UTP multicast publisher initialized successfully on "
              << multicast_config_.incremental_feed_a.multicast_ip << ":" << multicast_config_.incremental_feed_a.port << std::endl;

    running_ = true;
    return true;
}

void ReutersProtocolAdapter::run_once()
{
    if (!running_)
        return;

    // Send multicast heartbeats if needed
    static auto last_heartbeat = std::chrono::steady_clock::now();
    static auto last_security_definitions = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();

    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_heartbeat).count() >= 30) {
        if (multicast_publisher_) {
            multicast_publisher_->send_heartbeat();
            last_heartbeat = now;
        }
    }

    // Send security definitions every 10 seconds for late-joining clients
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_security_definitions).count() >= 10) {
        if (multicast_publisher_ && instruments_) {
            std::cout << "Sending periodic security definitions..." << std::endl;
            send_security_definitions(*instruments_);
            last_security_definitions = now;
        }
    }
}

void ReutersProtocolAdapter::shutdown()
{
    running_ = false;

    // Shutdown multicast publisher
    if (multicast_publisher_) {
        multicast_publisher_->shutdown();
        multicast_publisher_.reset();
    }
}

void ReutersProtocolAdapter::on_market_event(
    const std::shared_ptr<market_core::MarketEvent>& event)
{
    if (!running_ || !multicast_publisher_)
        return;

    stats_.market_events_processed++;
    
    // Debug: every 1000 events, print the counter
    if (stats_.market_events_processed % 1000 == 0) {
        std::cout << "[DEBUG] Market events processed: " << stats_.market_events_processed << std::endl;
    }

    // Publish to UTP multicast channels only
    switch (event->type) {
    case market_core::MarketEvent::QUOTE_UPDATE: {
        auto quote = std::static_pointer_cast<market_core::QuoteEvent>(event);
        multicast_publisher_->publish_incremental(*quote);
        break;
    }
    case market_core::MarketEvent::TRADE: {
        auto trade = std::static_pointer_cast<market_core::TradeEvent>(event);
        multicast_publisher_->publish_incremental(*trade);
        break;
    }
    case market_core::MarketEvent::SNAPSHOT: {
        auto snapshot = std::static_pointer_cast<market_core::SnapshotEvent>(event);
        multicast_publisher_->publish_snapshot(*snapshot);
        break;
    }
    case market_core::MarketEvent::STATISTICS: {
        auto stats = std::static_pointer_cast<market_core::StatisticsEvent>(event);
        multicast_publisher_->publish_statistics(*stats);
        break;
    }
    default:
        break;
    }
}

void ReutersProtocolAdapter::send_security_definitions(
    const std::vector<market_core::Instrument>& instruments)
{
    if (!running_ || !multicast_publisher_)
        return;

    // Store instruments for periodic sending
    if (!instruments_) {
        instruments_ = std::make_unique<std::vector<market_core::Instrument>>(instruments);
        std::cout << "Storing " << instruments.size() << " instruments for periodic security definition updates" << std::endl;
    }

    std::cout << "Sending security definitions for " << instruments.size() << " instruments via UTP multicast" << std::endl;

    for (const auto& instrument : instruments) {
        multicast_publisher_->publish_security_definition(instrument);
        std::cout << "Sent SecurityDefinition for " << instrument.primary_symbol << " (ID: " << instrument.instrument_id << ")" << std::endl;
    }
}

} // namespace reuters_protocol