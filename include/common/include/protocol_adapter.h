#pragma once

#include "../../core/include/instrument.h"
#include "../../core/include/market_events.h"
#include "../../core/include/order_book.h"
#include <functional>
#include <memory>
#include <vector>

namespace market_protocols {

// Forward declaration
class IMessageTransport;

// Base interface for protocol adapters
class IProtocolAdapter {
public:
    virtual ~IProtocolAdapter() = default;

    // Protocol identification
    virtual std::string get_protocol_name() const = 0;
    virtual std::string get_protocol_version() const = 0;

    // Event handling - core events to protocol messages
    virtual void process_quote_event(
        const market_core::Instrument& instrument,
        const market_core::QuoteEvent& event)
        = 0;

    virtual void process_trade_event(
        const market_core::Instrument& instrument,
        const market_core::TradeEvent& event)
        = 0;

    virtual void process_snapshot_event(
        const market_core::Instrument& instrument,
        const market_core::SnapshotEvent& event)
        = 0;

    virtual void process_statistics_event(
        const market_core::Instrument& instrument,
        const market_core::StatisticsEvent& event)
        = 0;

    virtual void process_status_event(
        const market_core::Instrument& instrument,
        const market_core::StatusEvent& event)
        = 0;

    // Instrument definition
    virtual void send_instrument_definition(
        const market_core::Instrument& instrument)
        = 0;

    // Protocol capabilities
    virtual bool supports_incremental_updates() const = 0;
    virtual bool supports_market_depth() const = 0;
    virtual bool supports_implied_prices() const = 0;
    virtual bool supports_statistics() const = 0;
    virtual size_t max_depth_levels() const = 0;
    virtual size_t max_message_size() const = 0;

    // Transport configuration
    virtual void set_transport(std::shared_ptr<IMessageTransport> transport) = 0;

    // Sequence management (if needed by protocol)
    virtual uint32_t get_next_sequence() { return 0; }
    virtual void reset_sequence() { }

    // Heartbeat support
    virtual void send_heartbeat() { }
};

// Message transport interface
class IMessageTransport {
public:
    virtual ~IMessageTransport() = default;

    // Send encoded message
    virtual bool send_message(const std::vector<uint8_t>& data) = 0;

    // Send multiple messages as batch (if supported)
    virtual bool send_batch(const std::vector<std::vector<uint8_t>>& messages)
    {
        bool success = true;
        for (const auto& msg : messages) {
            success &= send_message(msg);
        }
        return success;
    }

    // Transport info
    virtual std::string get_transport_type() const = 0; // "UDP", "TCP", etc.
    virtual bool is_connected() const = 0;
};

// Event listener for protocol adapters
class IProtocolEventListener {
public:
    virtual ~IProtocolEventListener() = default;

    // Called when adapter sends a message
    virtual void on_message_sent(
        const std::string& protocol,
        size_t message_size,
        uint32_t sequence)
        = 0;

    // Called on errors
    virtual void on_error(
        const std::string& protocol,
        const std::string& error)
        = 0;
};

// Factory for creating protocol adapters
class ProtocolAdapterFactory {
public:
    using CreateFunc = std::function<std::shared_ptr<IProtocolAdapter>()>;

    static ProtocolAdapterFactory& instance()
    {
        static ProtocolAdapterFactory factory;
        return factory;
    }

    void register_adapter(const std::string& name, CreateFunc creator)
    {
        creators_[name] = creator;
    }

    std::shared_ptr<IProtocolAdapter> create(const std::string& name)
    {
        auto it = creators_.find(name);
        if (it != creators_.end()) {
            return it->second();
        }
        return nullptr;
    }

    std::vector<std::string> get_available_protocols() const
    {
        std::vector<std::string> names;
        for (const auto& [name, _] : creators_) {
            names.push_back(name);
        }
        return names;
    }

private:
    std::map<std::string, CreateFunc> creators_;
};

} // namespace market_protocols