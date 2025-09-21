#include "../include/reuters_multicast_publisher.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

namespace reuters_protocol {

// MulticastMessageHeader implementation (deprecated - using Thomson Reuters header now)
void MulticastMessageHeader::pack(uint8_t* buffer) const
{
    // Note: This method is deprecated. We now use Thomson Reuters Binary Packet Header
    // Keeping for backward compatibility only
    memcpy(buffer, &sequence_number, 8);
    memcpy(buffer + 8, &channel_id, 4);
    memcpy(buffer + 12, &send_time_ns, 8);
    memcpy(buffer + 20, &message_count, 2);
    buffer[22] = flags;
}

void MulticastMessageHeader::unpack(const uint8_t* buffer)
{
    // Note: This method is deprecated. We now use Thomson Reuters Binary Packet Header
    // Keeping for backward compatibility only
    memcpy(&sequence_number, buffer, 8);
    memcpy(&channel_id, buffer + 8, 4);
    memcpy(&send_time_ns, buffer + 12, 8);
    memcpy(&message_count, buffer + 20, 2);
    flags = buffer[22];
}

// ReutersMulticastPublisher implementation
ReutersMulticastPublisher::ReutersMulticastPublisher(const ReutersMulticastConfig& config)
    : config_(config)
{
    stats_.start_time = std::chrono::steady_clock::now();
    last_heartbeat_ = stats_.start_time;
    last_snapshot_ = stats_.start_time;
}

ReutersMulticastPublisher::~ReutersMulticastPublisher()
{
    shutdown();
}

bool ReutersMulticastPublisher::initialize()
{
    try {
        // Create main incremental feed sockets (A and B)
        incremental_transport_a_ = std::make_unique<protocol_common::UDPTransport>();
        incremental_transport_b_ = std::make_unique<protocol_common::UDPTransport>();

        if (!incremental_transport_a_->create_multicast_sender(
                config_.incremental_feed_a.multicast_ip,
                config_.incremental_feed_a.port,
                config_.incremental_feed_a.interface_ip)) {
            std::cerr << "Failed to create incremental feed A multicast socket" << std::endl;
            return false;
        }

        if (!incremental_transport_b_->create_multicast_sender(
                config_.incremental_feed_b.multicast_ip,
                config_.incremental_feed_b.port,
                config_.incremental_feed_b.interface_ip)) {
            std::cerr << "Failed to create incremental feed B multicast socket" << std::endl;
            return false;
        }

        // Create security definition feed socket
        security_def_transport_ = std::make_unique<protocol_common::UDPTransport>();
        if (!security_def_transport_->create_multicast_sender(
                config_.security_definition_feed.multicast_ip,
                config_.security_definition_feed.port,
                config_.security_definition_feed.interface_ip)) {
            std::cerr << "Failed to create security definition multicast socket" << std::endl;
            return false;
        }

        // Create snapshot feed socket
        snapshot_transport_ = std::make_unique<protocol_common::UDPTransport>();
        if (!snapshot_transport_->create_multicast_sender(
                config_.snapshot_feed.multicast_ip,
                config_.snapshot_feed.port,
                config_.snapshot_feed.interface_ip)) {
            std::cerr << "Failed to create snapshot multicast socket" << std::endl;
            return false;
        }

        // Create channel-specific sockets for both A and B feeds
        for (const auto& channel : config_.channel_feeds_a) {
            auto transport = std::make_unique<protocol_common::UDPTransport>();
            if (!transport->create_multicast_sender(
                    channel.multicast_ip,
                    channel.port,
                    channel.interface_ip)) {
                std::cerr << "Failed to create channel " << channel.channel_id
                          << " feed A multicast socket" << std::endl;
                return false;
            }
            channel_transports_a_[channel.channel_id] = std::move(transport);
            sequence_numbers_[channel.channel_id] = 0;
            channel_enabled_[channel.channel_id] = true;

            // Build instrument to channel mapping
            for (const auto& symbol : channel.instruments) {
                // Note: In real implementation, would need proper instrument ID lookup
                // For now, using simple hash of symbol as ID
                uint32_t instrument_id = std::hash<std::string> {}(symbol) & 0xFFFF;
                instrument_channel_map_[instrument_id] = channel.channel_id;
            }
        }

        for (const auto& channel : config_.channel_feeds_b) {
            auto transport = std::make_unique<protocol_common::UDPTransport>();
            if (!transport->create_multicast_sender(
                    channel.multicast_ip,
                    channel.port,
                    channel.interface_ip)) {
                std::cerr << "Failed to create channel " << channel.channel_id
                          << " feed B multicast socket" << std::endl;
                return false;
            }
            channel_transports_b_[channel.channel_id] = std::move(transport);
        }

        // Initialize global sequence number
        sequence_numbers_[0] = 0;

        std::cout << "Reuters multicast publisher initialized:" << std::endl;
        std::cout << "  Incremental Feed A: " << config_.incremental_feed_a.multicast_ip
                  << ":" << config_.incremental_feed_a.port << std::endl;
        std::cout << "  Incremental Feed B: " << config_.incremental_feed_b.multicast_ip
                  << ":" << config_.incremental_feed_b.port << std::endl;
        std::cout << "  Security Definitions: " << config_.security_definition_feed.multicast_ip
                  << ":" << config_.security_definition_feed.port << std::endl;
        std::cout << "  Snapshots: " << config_.snapshot_feed.multicast_ip
                  << ":" << config_.snapshot_feed.port << std::endl;
        std::cout << "  Channel feeds: " << config_.channel_feeds_a.size() << " channels" << std::endl;

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception during multicast publisher initialization: " << e.what() << std::endl;
        return false;
    }
}

void ReutersMulticastPublisher::shutdown()
{
    // Send end-of-stream messages
    send_end_of_conflation();

    // Close all sockets
    incremental_transport_a_.reset();
    incremental_transport_b_.reset();
    security_def_transport_.reset();
    snapshot_transport_.reset();
    channel_transports_a_.clear();
    channel_transports_b_.clear();
}

void ReutersMulticastPublisher::publish_incremental(const market_core::QuoteEvent& quote)
{
    // Encode the quote update
    auto message = ReutersEncoder::encode_market_data_incremental(quote);

    // Get channel for this instrument
    int channel_id = get_channel_for_instrument(quote.instrument_id);

    if (channel_id > 0 && channel_enabled_[channel_id]) {
        // Send to channel-specific feeds
        send_to_channel_feeds(message, channel_id);
    } else {
        // Send to global feeds
        send_to_both_feeds(message, 0);
    }

    stats_.messages_sent_a++;
    stats_.messages_sent_b++;
    stats_.bytes_sent += message.size() * 2;
}

void ReutersMulticastPublisher::publish_incremental(const market_core::TradeEvent& trade)
{
    // Encode the trade
    auto message = ReutersEncoder::encode_market_data_incremental(trade);

    // Get channel for this instrument
    int channel_id = get_channel_for_instrument(trade.instrument_id);

    if (channel_id > 0 && channel_enabled_[channel_id]) {
        send_to_channel_feeds(message, channel_id);
    } else {
        send_to_both_feeds(message, 0);
    }

    stats_.messages_sent_a++;
    stats_.messages_sent_b++;
    stats_.bytes_sent += message.size() * 2;
}

void ReutersMulticastPublisher::publish_snapshot(const market_core::SnapshotEvent& snapshot)
{
    // Encode the snapshot
    auto message = ReutersEncoder::encode_market_data_snapshot(snapshot);

    // Add multicast header with sequence number
    uint64_t seq = get_next_sequence_number(0);
    auto packet = add_sequence_header(message, seq, 0);

    // Send snapshots only on the snapshot feed
    if (snapshot_transport_) {
        snapshot_transport_->send(packet);
    }

    stats_.snapshots_sent++;
    stats_.bytes_sent += packet.size();
    last_snapshot_ = std::chrono::steady_clock::now();
}

void ReutersMulticastPublisher::publish_security_definition(const market_core::Instrument& instrument)
{
    // Encode the security definition
    auto message = ReutersEncoder::encode_security_definition(instrument);

    // Add multicast header
    uint64_t seq = get_next_sequence_number(0);
    auto packet = add_sequence_header(message, seq, 0);

    // Send on security definition feed
    if (security_def_transport_) {
        security_def_transport_->send(packet);
    }

    stats_.definitions_sent++;
    stats_.bytes_sent += packet.size();
}

void ReutersMulticastPublisher::publish_statistics(const market_core::StatisticsEvent& stats)
{
    // Statistics would be encoded and sent similar to incremental updates
    // For now, this is a placeholder
}

void ReutersMulticastPublisher::send_heartbeat()
{
    auto message = ReutersEncoder::encode_heartbeat();

    // Send heartbeat on all active channels
    send_to_both_feeds(message, 0);

    for (const auto& [channel_id, enabled] : channel_enabled_) {
        if (enabled) {
            send_to_channel_feeds(message, channel_id);
        }
    }

    stats_.heartbeats_sent++;
    last_heartbeat_ = std::chrono::steady_clock::now();
}

void ReutersMulticastPublisher::send_end_of_conflation()
{
    // Send end-of-conflation marker if using conflation
    if (config_.conflation_interval_ms > 0) {
        // Create end-of-conflation message
        MulticastMessageHeader header;
        header.sequence_number = get_next_sequence_number(0);
        header.channel_id = 0;
        header.send_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch())
                                  .count();
        header.message_count = 0;
        header.flags = 0x02; // End-of-stream flag

        std::vector<uint8_t> packet(MulticastMessageHeader::SIZE);
        header.pack(packet.data());

        send_to_both_feeds(packet, 0);
    }
}

uint64_t ReutersMulticastPublisher::get_next_sequence_number(int channel_id)
{
    return ++sequence_numbers_[channel_id];
}

int ReutersMulticastPublisher::get_channel_for_instrument(uint32_t instrument_id) const
{
    auto it = instrument_channel_map_.find(instrument_id);
    if (it != instrument_channel_map_.end()) {
        return it->second;
    }
    return 0; // Global channel
}

void ReutersMulticastPublisher::enable_channel(int channel_id, bool enabled)
{
    channel_enabled_[channel_id] = enabled;
}

bool ReutersMulticastPublisher::is_channel_enabled(int channel_id) const
{
    auto it = channel_enabled_.find(channel_id);
    if (it != channel_enabled_.end()) {
        return it->second;
    }
    return false;
}

void ReutersMulticastPublisher::send_to_both_feeds(const std::vector<uint8_t>& message, int channel_id)
{
    // Add sequence header
    uint64_t seq = get_next_sequence_number(channel_id);
    auto packet = add_sequence_header(message, seq, channel_id);

    // Send to both A and B feeds for redundancy
    if (incremental_transport_a_) {
        incremental_transport_a_->send(packet);
    }

    if (incremental_transport_b_) {
        incremental_transport_b_->send(packet);
    }
}

void ReutersMulticastPublisher::send_to_channel_feeds(const std::vector<uint8_t>& message, int channel_id)
{
    // Add sequence header
    uint64_t seq = get_next_sequence_number(channel_id);
    auto packet = add_sequence_header(message, seq, channel_id);

    // Send to channel-specific A feed
    auto it_a = channel_transports_a_.find(channel_id);
    if (it_a != channel_transports_a_.end() && it_a->second) {
        it_a->second->send(packet);
    }

    // Send to channel-specific B feed
    auto it_b = channel_transports_b_.find(channel_id);
    if (it_b != channel_transports_b_.end() && it_b->second) {
        it_b->second->send(packet);
    }
}

std::vector<uint8_t> ReutersMulticastPublisher::add_sequence_header(
    const std::vector<uint8_t>& message,
    uint64_t sequence,
    int channel_id)
{
    // Thomson Reuters Binary Packet Header (20 bytes) - Chapter 6.1 of spec
    // All fields are little-endian as per Thomson Reuters specification
    constexpr size_t TR_HEADER_SIZE = 20;

    // Create packet with TR header + SBE message
    std::vector<uint8_t> packet(TR_HEADER_SIZE + message.size());

    // Build Thomson Reuters header fields
    uint64_t msg_seq_num = sequence;
    uint64_t sending_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    uint8_t hdr_len = TR_HEADER_SIZE;  // Always 20
    uint8_t hdr_ver = 1;                // Version 1
    uint16_t packet_len = packet.size(); // Total packet length including header

    // Pack header in little-endian format (Thomson Reuters uses little-endian)
    memcpy(packet.data(), &msg_seq_num, 8);      // Offset 0: MsgSeqNum (8 bytes)
    memcpy(packet.data() + 8, &sending_time, 8); // Offset 8: SendingTime (8 bytes)
    packet[16] = hdr_len;                         // Offset 16: HdrLen (1 byte)
    packet[17] = hdr_ver;                         // Offset 17: HdrVer (1 byte)
    memcpy(packet.data() + 18, &packet_len, 2);  // Offset 18: PacketLen (2 bytes)

    // Copy SBE message after header
    memcpy(packet.data() + TR_HEADER_SIZE, message.data(), message.size());

    return packet;
}

} // namespace reuters_protocol