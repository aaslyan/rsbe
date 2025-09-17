#pragma once

#include "common/udp_multicast_transport.h"
#include "market_events.h"
#include "reuters_encoder.h"
#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace reuters_protocol {

// Configuration for a multicast channel
struct MulticastChannelConfig {
    std::string multicast_ip;
    uint16_t port;
    std::string interface_ip;
    int channel_id;
    std::string description;
    std::vector<std::string> instruments; // Instruments on this channel
};

// Configuration for Reuters multicast feeds
struct ReutersMulticastConfig {
    // Incremental feeds (A and B for redundancy)
    MulticastChannelConfig incremental_feed_a;
    MulticastChannelConfig incremental_feed_b;

    // Security definition feed
    MulticastChannelConfig security_definition_feed;

    // Snapshot feed
    MulticastChannelConfig snapshot_feed;

    // Channel-specific incremental feeds
    std::vector<MulticastChannelConfig> channel_feeds_a;
    std::vector<MulticastChannelConfig> channel_feeds_b;

    // Timing parameters
    uint32_t incremental_interval_ms = 10;
    uint32_t snapshot_interval_seconds = 60;
    uint32_t heartbeat_interval_seconds = 30;
    uint32_t conflation_interval_ms = 0; // 0 = no conflation

    // Book parameters
    uint32_t book_depth = 10;
    bool send_statistics = true;
};

class ReutersMulticastPublisher {
public:
    explicit ReutersMulticastPublisher(const ReutersMulticastConfig& config);
    ~ReutersMulticastPublisher();

    // Initialize all multicast channels
    bool initialize();
    void shutdown();

    // Market data distribution
    void publish_incremental(const market_core::QuoteEvent& quote);
    void publish_incremental(const market_core::TradeEvent& trade);
    void publish_snapshot(const market_core::SnapshotEvent& snapshot);
    void publish_security_definition(const market_core::Instrument& instrument);
    void publish_statistics(const market_core::StatisticsEvent& stats);

    // Heartbeat and sequence management
    void send_heartbeat();
    void send_end_of_conflation();
    uint64_t get_next_sequence_number(int channel_id);

    // Channel management
    int get_channel_for_instrument(uint32_t instrument_id) const;
    void enable_channel(int channel_id, bool enabled);
    bool is_channel_enabled(int channel_id) const;

    // Statistics
    struct PublisherStats {
        uint64_t messages_sent_a = 0;
        uint64_t messages_sent_b = 0;
        uint64_t snapshots_sent = 0;
        uint64_t definitions_sent = 0;
        uint64_t heartbeats_sent = 0;
        uint64_t bytes_sent = 0;
        std::chrono::steady_clock::time_point start_time;
    };

    const PublisherStats& get_statistics() const { return stats_; }

private:
    ReutersMulticastConfig config_;
    PublisherStats stats_;

    // UDP transport for each feed
    std::unique_ptr<protocol_common::UDPTransport> incremental_transport_a_;
    std::unique_ptr<protocol_common::UDPTransport> incremental_transport_b_;
    std::unique_ptr<protocol_common::UDPTransport> security_def_transport_;
    std::unique_ptr<protocol_common::UDPTransport> snapshot_transport_;

    // Channel-specific transports
    std::unordered_map<int, std::unique_ptr<protocol_common::UDPTransport>> channel_transports_a_;
    std::unordered_map<int, std::unique_ptr<protocol_common::UDPTransport>> channel_transports_b_;

    // Sequence numbers per channel
    std::unordered_map<int, std::atomic<uint64_t>> sequence_numbers_;

    // Instrument to channel mapping
    std::unordered_map<uint32_t, int> instrument_channel_map_;

    // Channel enable/disable state
    std::unordered_map<int, std::atomic<bool>> channel_enabled_;

    // Timing
    std::chrono::steady_clock::time_point last_heartbeat_;
    std::chrono::steady_clock::time_point last_snapshot_;

    // Internal methods
    bool create_multicast_socket(const MulticastChannelConfig& config,
        std::unique_ptr<protocol_common::UDPTransport>& transport);
    void send_to_both_feeds(const std::vector<uint8_t>& message, int channel_id);
    void send_to_channel_feeds(const std::vector<uint8_t>& message, int channel_id);
    std::vector<uint8_t> add_sequence_header(const std::vector<uint8_t>& message,
        uint64_t sequence, int channel_id);
};

// Multicast-specific message header for sequencing
struct MulticastMessageHeader {
    uint64_t sequence_number;
    uint32_t channel_id;
    uint64_t send_time_ns;
    uint16_t message_count; // Number of messages in packet
    uint8_t flags; // Bit flags: 0x01=Retransmission, 0x02=EndOfStream

    static constexpr size_t SIZE = 23;

    void pack(uint8_t* buffer) const;
    void unpack(const uint8_t* buffer);
};

} // namespace reuters_protocol