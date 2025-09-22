#pragma once

#include "../core/include/market_data_generator.h"
#include "../core/include/market_events.h"
#include "reuters_encoder.h"
#include "reuters_multicast_publisher.h"
#include "utp_sbe/utp_sbe/MessageHeader.h"
#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace reuters_protocol {

class ReutersProtocolAdapter : public market_core::IMarketEventListener {
public:
    ReutersProtocolAdapter(uint16_t port, const ReutersMulticastConfig& multicast_config);
    ~ReutersProtocolAdapter();

    // IMarketEventListener implementation
    void on_market_event(const std::shared_ptr<market_core::MarketEvent>& event) override;

    // UTP multicast server operations
    bool initialize_with_multicast(); // Initialize UTP multicast publisher
    void run_once(); // Process one iteration of server loop
    void shutdown();

    // Send security definitions via multicast
    void send_security_definitions(const std::vector<market_core::Instrument>& instruments);

    // Statistics - put non-atomic members first to avoid alignment issues
    struct Statistics {
        std::chrono::steady_clock::time_point start_time;  // Move to front
        std::atomic<size_t> sessions_created{0};
        std::atomic<size_t> messages_sent{0};
        std::atomic<size_t> messages_received{0};
        std::atomic<size_t> market_events_processed{0};
        
        Statistics() {
            start_time = std::chrono::steady_clock::now();
        }
    };
    
    // Return non-atomic copy for safe reading
    struct StatisticsSnapshot {
        size_t sessions_created;
        size_t messages_sent;
        size_t messages_received;
        size_t market_events_processed;
        std::chrono::steady_clock::time_point start_time;
    };

    StatisticsSnapshot get_statistics() const { 
        StatisticsSnapshot snapshot;
        snapshot.sessions_created = stats_.sessions_created.load();
        snapshot.messages_sent = stats_.messages_sent.load();
        snapshot.messages_received = stats_.messages_received.load();
        snapshot.market_events_processed = stats_.market_events_processed.load();
        snapshot.start_time = stats_.start_time;
        
        // Debug: print values when reading statistics
        static size_t debug_call_count = 0;
        if (++debug_call_count % 10 == 1) { // Only print every 10th call to avoid spam
            std::cout << "[DEBUG] get_statistics() called #" << debug_call_count << ":" << std::endl;
            std::cout << "[DEBUG] - events_processed atomic value: " << stats_.market_events_processed.load() << std::endl;
            std::cout << "[DEBUG] - snapshot events_processed: " << snapshot.market_events_processed << std::endl;
            std::cout << "[DEBUG] - original start_time epoch: " << stats_.start_time.time_since_epoch().count() << " ns" << std::endl;
            std::cout << "[DEBUG] - snapshot start_time epoch: " << snapshot.start_time.time_since_epoch().count() << " ns" << std::endl;
        }
        
        return snapshot;
    }
    
    size_t get_total_messages_sent() const {
        if (multicast_publisher_) {
            const auto& pub_stats = multicast_publisher_->get_statistics();
            return pub_stats.messages_sent_a + pub_stats.messages_sent_b 
                 + pub_stats.snapshots_sent + pub_stats.definitions_sent 
                 + pub_stats.heartbeats_sent;
        }
        return 0;
    }

private:
    uint16_t port_;
    bool running_;
    Statistics stats_;
    ReutersMulticastConfig multicast_config_;
    std::unique_ptr<ReutersMulticastPublisher> multicast_publisher_;
    std::unique_ptr<std::vector<market_core::Instrument>> instruments_;
};

} // namespace reuters_protocol