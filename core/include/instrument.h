#pragma once

#include "market_events.h"
#include <map>
#include <memory>
#include <string>
#include <variant>

namespace market_core {

// Property value type that can hold different data types
using PropertyValue = std::variant<int64_t, double, std::string, bool>;

// Base instrument class
class Instrument {
public:
    // Core identifiers
    uint32_t instrument_id; // Internal unique ID
    std::string primary_symbol; // Primary exchange symbol
    std::string description;

    // Core trading parameters
    double tick_size;
    double multiplier;
    double min_price_increment;
    std::optional<double> max_price_variation;

    // Protocol-specific identifiers
    // e.g., {"CME_SECURITY_ID": "123456", "REUTERS_RIC": "EUR=", "BLOOMBERG": "EURUSD Curncy"}
    std::map<std::string, std::string> external_ids;

    // Extensible properties for protocol-specific fields
    // e.g., {"maturity_date": "2024-12-20", "settlement_type": "PHYSICAL"}
    std::map<std::string, PropertyValue> properties;

    // Constructor
    Instrument(uint32_t id, const std::string& symbol, InstrumentType type)
        : instrument_id(id)
        , primary_symbol(symbol)
        , tick_size(0.01)
        , multiplier(1.0)
        , min_price_increment(0.01)
        , type_(type)
    {
    }

    virtual ~Instrument() = default;

    // Get instrument type
    InstrumentType get_type() const { return type_; }

    // Helper methods for properties
    template <typename T>
    std::optional<T> get_property(const std::string& key) const
    {
        auto it = properties.find(key);
        if (it != properties.end()) {
            if (auto* val = std::get_if<T>(&it->second)) {
                return *val;
            }
        }
        return std::nullopt;
    }

    void set_property(const std::string& key, const PropertyValue& value)
    {
        properties[key] = value;
    }

    // Get external ID for specific protocol
    std::optional<std::string> get_external_id(const std::string& protocol) const
    {
        auto it = external_ids.find(protocol);
        if (it != external_ids.end()) {
            return it->second;
        }
        return std::nullopt;
    }

protected:
    InstrumentType type_;
};

// Futures-specific instrument
class FuturesInstrument : public Instrument {
public:
    std::string underlying;
    std::string maturity_date; // YYYY-MM-DD
    std::string contract_month; // e.g., "Z4" for Dec 2024
    double contract_size;
    double initial_margin;
    double maintenance_margin;

    FuturesInstrument(uint32_t id, const std::string& symbol)
        : Instrument(id, symbol, InstrumentType::FUTURE)
        , contract_size(1.0)
        , initial_margin(0.0)
        , maintenance_margin(0.0)
    {
    }
};

// FX Spot instrument
class FXSpotInstrument : public Instrument {
public:
    std::string base_currency;
    std::string quote_currency;
    std::string settlement_convention; // T+1, T+2
    double standard_lot_size;
    std::optional<std::string> primary_venue; // EBS, Reuters, etc.

    FXSpotInstrument(uint32_t id, const std::string& symbol)
        : Instrument(id, symbol, InstrumentType::FX_SPOT)
        , standard_lot_size(1000000.0) // 1M base currency default
    {
    }
};

// Option instrument
class OptionInstrument : public Instrument {
public:
    enum OptionType {
        CALL,
        PUT
    };

    std::string underlying;
    double strike_price;
    std::string expiry_date;
    OptionType option_type;
    std::string exercise_style; // American, European

    OptionInstrument(uint32_t id, const std::string& symbol)
        : Instrument(id, symbol, InstrumentType::OPTION)
        , strike_price(0.0)
        , option_type(CALL)
        , exercise_style("American")
    {
    }
};

// Spread instrument (for CME calendar spreads, etc.)
class SpreadInstrument : public Instrument {
public:
    std::vector<uint32_t> leg_instrument_ids;
    std::vector<int> leg_ratios; // e.g., [1, -1] for calendar spread

    SpreadInstrument(uint32_t id, const std::string& symbol)
        : Instrument(id, symbol, InstrumentType::SPREAD)
    {
    }
};

// Factory for creating instruments
class InstrumentFactory {
public:
    static std::shared_ptr<Instrument> create(
        InstrumentType type,
        uint32_t id,
        const std::string& symbol)
    {
        switch (type) {
        case InstrumentType::FUTURE:
            return std::make_shared<FuturesInstrument>(id, symbol);
        case InstrumentType::FX_SPOT:
            return std::make_shared<FXSpotInstrument>(id, symbol);
        case InstrumentType::OPTION:
            return std::make_shared<OptionInstrument>(id, symbol);
        case InstrumentType::SPREAD:
            return std::make_shared<SpreadInstrument>(id, symbol);
        default:
            return std::make_shared<Instrument>(id, symbol, type);
        }
    }
};

} // namespace market_core