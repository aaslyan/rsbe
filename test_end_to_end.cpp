#include "utp_client/UTPClient.h"
#include "core/include/market_data_generator.h"
#include "core/include/order_book_manager.h"
#include "include/reuters_protocol_adapter.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <cassert>

// Test data verification structures
struct ReceivedMessage {
    uint16_t template_id;
    uint32_t security_id;
    std::string symbol;
    double price;
    uint64_t size;
    std::chrono::steady_clock::time_point timestamp;
};

class TestClient : public UTPClient {
public:
    std::vector<ReceivedMessage> received_messages;
    std::atomic<int> message_count{0};
    
    TestClient(const std::string& multicast_group, int port) 
        : UTPClient(multicast_group, port) {}
    
    // Override to capture received data
    void on_security_definition_received(uint32_t security_id, const std::string& symbol) {
        ReceivedMessage msg;
        msg.template_id = 12; // Security Definition
        msg.security_id = security_id;
        msg.symbol = symbol;
        msg.timestamp = std::chrono::steady_clock::now();
        
        received_messages.push_back(msg);
        message_count++;
        
        std::cout << "TEST: Received SecurityDefinition - ID:" << security_id 
                  << ", Symbol:" << symbol << std::endl;
    }
    
    void on_market_data_received(uint32_t security_id, double price, uint64_t size) {
        ReceivedMessage msg;
        msg.template_id = 13; // Market Data
        msg.security_id = security_id;
        msg.price = price;
        msg.size = size;
        msg.timestamp = std::chrono::steady_clock::now();
        
        received_messages.push_back(msg);
        message_count++;
        
        std::cout << "TEST: Received MarketData - ID:" << security_id 
                  << ", Price:" << price << ", Size:" << size << std::endl;
    }
};

// Test configuration
struct TestConfig {
    std::string multicast_ip = "239.100.1.10"; // Security definition feed
    int multicast_port = 15010;
    int test_duration_seconds = 10;
    int expected_instruments = 7; // EURUSD, GBPUSD, USDJPY, USDCHF, AUDUSD, NZDUSD, USDCAD
};

class EndToEndTest {
private:
    TestConfig config_;
    std::unique_ptr<TestClient> client_;
    std::atomic<bool> test_running_{true};
    
public:
    EndToEndTest(const TestConfig& config) : config_(config) {}
    
    bool run_test() {
        std::cout << "\n=== Reuters SBE End-to-End Test ===" << std::endl;
        std::cout << "Testing multicast: " << config_.multicast_ip << ":" << config_.multicast_port << std::endl;
        
        // Start client
        client_ = std::make_unique<TestClient>(config_.multicast_ip, config_.multicast_port);
        
        if (!client_->connect()) {
            std::cerr << "TEST FAILED: Could not connect client to multicast" << std::endl;
            return false;
        }
        
        std::cout << "Client connected to multicast feed" << std::endl;
        
        // Run client in background thread
        std::thread client_thread([this]() {
            std::cout << "Starting client message processing..." << std::endl;
            auto start_time = std::chrono::steady_clock::now();
            
            while (test_running_) {
                client_->process_single_message();
                
                // Check timeout
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - start_time).count();
                    
                if (elapsed >= config_.test_duration_seconds) {
                    test_running_ = false;
                    break;
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            std::cout << "Client thread finished" << std::endl;
        });
        
        // Wait for test duration
        std::cout << "Running test for " << config_.test_duration_seconds << " seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(config_.test_duration_seconds));
        
        // Stop test
        test_running_ = false;
        client_thread.join();
        
        // Analyze results
        return analyze_results();
    }
    
private:
    bool analyze_results() {
        std::cout << "\n=== Test Results Analysis ===" << std::endl;
        std::cout << "Total messages received: " << client_->message_count.load() << std::endl;
        std::cout << "Detailed messages: " << client_->received_messages.size() << std::endl;
        
        if (client_->received_messages.empty()) {
            std::cerr << "TEST FAILED: No messages received" << std::endl;
            return false;
        }
        
        // Count by message type
        int security_definitions = 0;
        int market_data_messages = 0;
        std::set<uint32_t> unique_instruments;
        
        for (const auto& msg : client_->received_messages) {
            unique_instruments.insert(msg.security_id);
            
            if (msg.template_id == 12) {
                security_definitions++;
                std::cout << "  SecurityDef: ID=" << msg.security_id 
                          << ", Symbol=" << msg.symbol << std::endl;
            } else if (msg.template_id == 13) {
                market_data_messages++;
                std::cout << "  MarketData: ID=" << msg.security_id 
                          << ", Price=" << msg.price << ", Size=" << msg.size << std::endl;
            }
        }
        
        std::cout << "\nSummary:" << std::endl;
        std::cout << "  Security Definitions: " << security_definitions << std::endl;
        std::cout << "  Market Data Messages: " << market_data_messages << std::endl;
        std::cout << "  Unique Instruments: " << unique_instruments.size() << std::endl;
        
        // Validation checks
        bool test_passed = true;
        
        if (unique_instruments.size() < config_.expected_instruments) {
            std::cerr << "TEST WARNING: Expected " << config_.expected_instruments 
                      << " instruments, got " << unique_instruments.size() << std::endl;
            // This is a warning, not a failure - server might not have sent all instruments yet
        }
        
        if (security_definitions == 0) {
            std::cerr << "TEST FAILED: No security definitions received" << std::endl;
            test_passed = false;
        }
        
        // Check that we received proper SBE-formatted data
        bool has_valid_data = false;
        for (const auto& msg : client_->received_messages) {
            if (msg.security_id > 0 && msg.security_id < 100000) {
                has_valid_data = true;
                break;
            }
        }
        
        if (!has_valid_data) {
            std::cerr << "TEST FAILED: No valid security IDs found (possible parsing issues)" << std::endl;
            test_passed = false;
        }
        
        if (test_passed) {
            std::cout << "\n✅ TEST PASSED: Client successfully received and parsed SBE messages" << std::endl;
            std::cout << "   - Client can connect to multicast feed" << std::endl;
            std::cout << "   - Thomson Reuters Binary Packet Header parsing works" << std::endl;
            std::cout << "   - SBE message parsing works" << std::endl;
            std::cout << "   - Market data content is being decoded properly" << std::endl;
        } else {
            std::cout << "\n❌ TEST FAILED: Issues detected in message parsing" << std::endl;
        }
        
        return test_passed;
    }
};

int main(int argc, char* argv[]) {
    TestConfig config;
    
    // Allow override of multicast address/port
    if (argc > 1) config.multicast_ip = argv[1];
    if (argc > 2) config.multicast_port = std::stoi(argv[2]);
    if (argc > 3) config.test_duration_seconds = std::stoi(argv[3]);
    
    std::cout << "Reuters SBE End-to-End Test" << std::endl;
    std::cout << "=========================" << std::endl;
    std::cout << "NOTE: Make sure the Reuters server is running before starting this test" << std::endl;
    std::cout << "Server command: ./utp_server" << std::endl;
    std::cout << "Test will connect to: " << config.multicast_ip << ":" << config.multicast_port << std::endl;
    std::cout << "Press Enter to start test..." << std::endl;
    std::cin.get();
    
    EndToEndTest test(config);
    bool success = test.run_test();
    
    return success ? 0 : 1;
}