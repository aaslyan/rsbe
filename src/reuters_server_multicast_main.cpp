#include "../core/include/market_data_generator.h"
#include "../core/include/order_book_manager.h"
#include "../include/reuters_protocol_adapter.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <signal.h>
#include <thread>

std::atomic<bool> running(true);

void signal_handler(int signal)
{
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    running = false;
}

// Load configuration from JSON file
reuters_protocol::ReutersMulticastConfig load_multicast_config(const std::string& config_file)
{
    reuters_protocol::ReutersMulticastConfig config;

    try {
        std::ifstream file(config_file);
        if (!file.is_open()) {
            std::cerr << "Could not open config file: " << config_file << std::endl;
            return config;
        }

        // For simplicity, using hardcoded config
        // In production, would parse JSON properly
        config.incremental_feed_a.multicast_ip = "239.100.1.1";
        config.incremental_feed_a.port = 15001;
        config.incremental_feed_a.interface_ip = "0.0.0.0";
        config.incremental_feed_a.channel_id = 0;

        config.incremental_feed_b.multicast_ip = "239.100.1.2";
        config.incremental_feed_b.port = 15002;
        config.incremental_feed_b.interface_ip = "0.0.0.0";
        config.incremental_feed_b.channel_id = 0;

        config.security_definition_feed.multicast_ip = "239.100.1.10";
        config.security_definition_feed.port = 15010;
        config.security_definition_feed.interface_ip = "0.0.0.0";

        config.snapshot_feed.multicast_ip = "239.100.1.20";
        config.snapshot_feed.port = 15020;
        config.snapshot_feed.interface_ip = "0.0.0.0";

        // Channel 1 - Major FX pairs
        reuters_protocol::MulticastChannelConfig channel1_a;
        channel1_a.multicast_ip = "239.100.2.1";
        channel1_a.port = 15101;
        channel1_a.interface_ip = "0.0.0.0";
        channel1_a.channel_id = 1;
        channel1_a.instruments = { "EURUSD", "GBPUSD", "USDJPY", "USDCHF" };
        config.channel_feeds_a.push_back(channel1_a);

        reuters_protocol::MulticastChannelConfig channel1_b;
        channel1_b.multicast_ip = "239.100.2.2";
        channel1_b.port = 15102;
        channel1_b.interface_ip = "0.0.0.0";
        channel1_b.channel_id = 1;
        channel1_b.instruments = { "EURUSD", "GBPUSD", "USDJPY", "USDCHF" };
        config.channel_feeds_b.push_back(channel1_b);

        // Channel 2 - Commodity currencies
        reuters_protocol::MulticastChannelConfig channel2_a;
        channel2_a.multicast_ip = "239.100.3.1";
        channel2_a.port = 15201;
        channel2_a.interface_ip = "0.0.0.0";
        channel2_a.channel_id = 2;
        channel2_a.instruments = { "AUDUSD", "NZDUSD", "USDCAD" };
        config.channel_feeds_a.push_back(channel2_a);

        reuters_protocol::MulticastChannelConfig channel2_b;
        channel2_b.multicast_ip = "239.100.3.2";
        channel2_b.port = 15202;
        channel2_b.interface_ip = "0.0.0.0";
        channel2_b.channel_id = 2;
        channel2_b.instruments = { "AUDUSD", "NZDUSD", "USDCAD" };
        config.channel_feeds_b.push_back(channel2_b);

        config.incremental_interval_ms = 100; // Slower rate: 10 events/sec instead of 100
        config.snapshot_interval_seconds = 60;
        config.heartbeat_interval_seconds = 30;
        config.book_depth = 10;

    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
    }

    return config;
}

int main(int argc, char* argv[])
{
    // Install signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    std::cout << "Reuters FX Market Data Server with Multicast Support" << std::endl;
    std::cout << "====================================================" << std::endl;

    try {
        // Initialize core market data system
        auto book_manager = std::make_shared<market_core::OrderBookManager>();
        auto data_generator = std::make_shared<market_core::MarketDataGenerator>(book_manager);

        // Create FX instruments
        std::cout << "Creating FX instruments..." << std::endl;

        // Major pairs
        auto eurusd = std::make_shared<market_core::Instrument>(1001, "EURUSD", market_core::InstrumentType::FX_SPOT);
        eurusd->tick_size = 0.00001;
        eurusd->set_property("initial_price", 1.0850);
        eurusd->set_property("initial_spread", 0.00002);
        book_manager->add_instrument(eurusd);

        auto gbpusd = std::make_shared<market_core::Instrument>(1002, "GBPUSD", market_core::InstrumentType::FX_SPOT);
        gbpusd->tick_size = 0.00001;
        gbpusd->set_property("initial_price", 1.2650);
        gbpusd->set_property("initial_spread", 0.00003);
        book_manager->add_instrument(gbpusd);

        auto usdjpy = std::make_shared<market_core::Instrument>(1003, "USDJPY", market_core::InstrumentType::FX_SPOT);
        usdjpy->tick_size = 0.001;
        usdjpy->set_property("initial_price", 149.50);
        usdjpy->set_property("initial_spread", 0.002);
        book_manager->add_instrument(usdjpy);

        auto usdchf = std::make_shared<market_core::Instrument>(1004, "USDCHF", market_core::InstrumentType::FX_SPOT);
        usdchf->tick_size = 0.00001;
        usdchf->set_property("initial_price", 0.8950);
        usdchf->set_property("initial_spread", 0.00002);
        book_manager->add_instrument(usdchf);

        // Commodity currencies
        auto audusd = std::make_shared<market_core::Instrument>(1005, "AUDUSD", market_core::InstrumentType::FX_SPOT);
        audusd->tick_size = 0.00001;
        audusd->set_property("initial_price", 0.6680);
        audusd->set_property("initial_spread", 0.00002);
        book_manager->add_instrument(audusd);

        auto nzdusd = std::make_shared<market_core::Instrument>(1006, "NZDUSD", market_core::InstrumentType::FX_SPOT);
        nzdusd->tick_size = 0.00001;
        nzdusd->set_property("initial_price", 0.6020);
        nzdusd->set_property("initial_spread", 0.00003);
        book_manager->add_instrument(nzdusd);

        auto usdcad = std::make_shared<market_core::Instrument>(1007, "USDCAD", market_core::InstrumentType::FX_SPOT);
        usdcad->tick_size = 0.00001;
        usdcad->set_property("initial_price", 1.3620);
        usdcad->set_property("initial_spread", 0.00002);
        book_manager->add_instrument(usdcad);

        std::cout << "Created " << book_manager->get_all_instrument_ids().size() << " FX instruments" << std::endl;

        // Create order books for all instruments
        std::cout << "Creating order books..." << std::endl;
        market_core::OrderBook::Config book_config;
        book_config.max_visible_levels = 10;
        book_config.track_market_makers = true; // Enable for Reuters

        for (const auto& instrument_id : book_manager->get_all_instrument_ids()) {
            if (book_manager->create_order_book(instrument_id, book_config)) {
                std::cout << "Created order book for instrument " << instrument_id << std::endl;
            } else {
                std::cerr << "Failed to create order book for instrument " << instrument_id << std::endl;
            }
        }

        // Initialize market data
        data_generator->set_market_mode(market_core::MarketMode::NORMAL);
        std::cout << "Generating initial market state..." << std::endl;
        data_generator->generate_all_instruments();

        // Load multicast configuration
        std::string config_file = "config/reuters_config.json";
        if (argc > 1) {
            config_file = argv[1];
        }

        auto multicast_config = load_multicast_config(config_file);

        // Initialize Reuters protocol adapter with multicast
        uint16_t tcp_port = 11501;
        if (argc > 2) {
            tcp_port = static_cast<uint16_t>(std::stoi(argv[2]));
        }

        auto reuters_adapter = std::make_unique<reuters_protocol::ReutersProtocolAdapter>(tcp_port, multicast_config);

        // Convert to shared_ptr for listener management
        std::shared_ptr<reuters_protocol::ReutersProtocolAdapter> reuters_shared(reuters_adapter.release());
        data_generator->add_listener(std::weak_ptr<market_core::IMarketEventListener>(reuters_shared));

        // Initialize with multicast support
        if (!reuters_shared->initialize_with_multicast()) {
            std::cerr << "Failed to initialize Reuters server with multicast" << std::endl;
            return 1;
        }

        // Send initial security definitions
        std::vector<market_core::Instrument> instruments;
        for (const auto& instrument_ptr : book_manager->get_all_instruments()) {
            instruments.push_back(*instrument_ptr);
        }
        reuters_shared->send_security_definitions(instruments);

        std::cout << "\n=== Reuters Multicast Configuration ===" << std::endl;
        std::cout << "TCP Session Management: port " << tcp_port << std::endl;
        std::cout << "\nMulticast Feeds:" << std::endl;
        std::cout << "  Incremental A: " << multicast_config.incremental_feed_a.multicast_ip
                  << ":" << multicast_config.incremental_feed_a.port << std::endl;
        std::cout << "  Incremental B: " << multicast_config.incremental_feed_b.multicast_ip
                  << ":" << multicast_config.incremental_feed_b.port << std::endl;
        std::cout << "  Security Def:  " << multicast_config.security_definition_feed.multicast_ip
                  << ":" << multicast_config.security_definition_feed.port << std::endl;
        std::cout << "  Snapshots:     " << multicast_config.snapshot_feed.multicast_ip
                  << ":" << multicast_config.snapshot_feed.port << std::endl;
        std::cout << "\nChannel-specific feeds:" << std::endl;
        std::cout << "  Channel 1 (Major): 239.100.2.1-2:15101-15102" << std::endl;
        std::cout << "  Channel 2 (Commodity): 239.100.3.1-2:15201-15202" << std::endl;
        std::cout << "\nPress Ctrl+C to shutdown" << std::endl;
        std::cout << "======================================\n"
                  << std::endl;

        // Main server loop
        auto last_market_update = std::chrono::steady_clock::now();
        auto last_stats_print = std::chrono::steady_clock::now();
        auto last_snapshot = std::chrono::steady_clock::now();

        while (running) {
            auto now = std::chrono::steady_clock::now();

            // Process Reuters protocol (TCP connections, sessions)
            reuters_shared->run_once();

            // Generate market data updates (reduced frequency)
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_market_update).count() >= multicast_config.incremental_interval_ms) {
                // Generate updates for only 1-2 instruments at a time instead of all 7
                auto instrument_ids = book_manager->get_all_instrument_ids();
                if (!instrument_ids.empty()) {
                    // Pick 2 random instruments per update cycle
                    auto time_seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                    std::default_random_engine generator(time_seed);
                    std::shuffle(instrument_ids.begin(), instrument_ids.end(), generator);

                    for (size_t i = 0; i < std::min(size_t(2), instrument_ids.size()); ++i) {
                        data_generator->generate_update(instrument_ids[i]);
                    }
                }
                last_market_update = now;
            }

            // Generate snapshots periodically
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_snapshot).count() >= multicast_config.snapshot_interval_seconds) {
                // Trigger snapshot generation for all instruments
                for (auto id : book_manager->get_all_instrument_ids()) {
                    auto book = book_manager->get_order_book(id);
                    if (book) {
                        auto snapshot = std::make_shared<market_core::SnapshotEvent>(id);
                        // Fill snapshot from book
                        const auto& bids = book->get_bids(10);
                        const auto& asks = book->get_asks(10);

                        for (const auto& level : bids) {
                            market_core::QuoteEvent quote(id);
                            quote.side = market_core::Side::BID;
                            quote.price = level.price;
                            quote.quantity = level.quantity;
                            quote.order_count = level.order_count;
                            quote.action = market_core::UpdateAction::ADD;
                            snapshot->bid_levels.push_back(quote);
                        }

                        for (const auto& level : asks) {
                            market_core::QuoteEvent quote(id);
                            quote.side = market_core::Side::ASK;
                            quote.price = level.price;
                            quote.quantity = level.quantity;
                            quote.order_count = level.order_count;
                            quote.action = market_core::UpdateAction::ADD;
                            snapshot->ask_levels.push_back(quote);
                        }

                        reuters_shared->on_market_event(snapshot);
                    }
                }
                last_snapshot = now;
            }

            // Print statistics
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_stats_print).count() >= 10) {
                const auto& stats = reuters_shared->get_statistics();
                auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - stats.start_time).count();

                std::cout << "Stats [" << uptime << "s]: "
                          << "Sessions=" << stats.sessions_created
                          << ", Sent=" << stats.messages_sent
                          << ", Recv=" << stats.messages_received
                          << ", Events=" << stats.market_events_processed
                          << std::endl;

                last_stats_print = now;
            }

            // Small sleep to prevent busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        std::cout << "\nShutting down Reuters multicast server..." << std::endl;
        reuters_shared->shutdown();

        // Print final statistics
        const auto& final_stats = reuters_shared->get_statistics();
        auto total_uptime = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - final_stats.start_time)
                                .count();

        std::cout << "\nFinal Statistics:" << std::endl;
        std::cout << "  Uptime: " << total_uptime << " seconds" << std::endl;
        std::cout << "  Sessions created: " << final_stats.sessions_created << std::endl;
        std::cout << "  Messages sent: " << final_stats.messages_sent << std::endl;
        std::cout << "  Messages received: " << final_stats.messages_received << std::endl;
        std::cout << "  Market events processed: " << final_stats.market_events_processed << std::endl;

        std::cout << "Reuters multicast server shutdown complete." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}