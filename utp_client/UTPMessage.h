#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

// UTP Message Header based on UTP_CLIENT_Multicast_MD.xml
struct UTPMessageHeader {
    uint16_t blockLength; // Size of fixed fields
    uint16_t templateId; // Message type identifier
    uint16_t schemaId = 101; // UTP_CLIENT_MKTDATA schema ID
    uint16_t version = 1; // Schema version from XML

    UTPMessageHeader()
        : blockLength(0)
        , templateId(0)
    {
    }

    void pack_little_endian(uint8_t* buffer) const
    {
        memcpy(buffer, &blockLength, 2);
        memcpy(buffer + 2, &templateId, 2);
        memcpy(buffer + 4, &schemaId, 2);
        memcpy(buffer + 6, &version, 2);
    }

    void unpack_little_endian(const uint8_t* buffer)
    {
        memcpy(&blockLength, buffer, 2);
        memcpy(&templateId, buffer + 2, 2);
        memcpy(&schemaId, buffer + 4, 2);
        memcpy(&version, buffer + 6, 2);
    }

    static constexpr size_t size() { return 8; }
};

// Group Size structure for repeating groups
struct GroupSize {
    uint16_t blockLength;
    uint16_t numInGroup;

    void pack_little_endian(uint8_t* buffer) const
    {
        memcpy(buffer, &blockLength, 2);
        memcpy(buffer + 2, &numInGroup, 2);
    }

    void unpack_little_endian(const uint8_t* buffer)
    {
        memcpy(&blockLength, buffer, 2);
        memcpy(&numInGroup, buffer + 2, 2);
    }

    static constexpr size_t size() { return 4; }
};

// Price with mantissa and exponent (-9)
struct PriceNull {
    int64_t mantissa = 9223372036854775807LL; // NULL value
    static constexpr int8_t exponent = -9;

    PriceNull() = default;
    PriceNull(double price)
        : mantissa(static_cast<int64_t>(price * 1000000000.0))
    {
    }

    double to_double() const
    {
        if (mantissa == 9223372036854775807LL)
            return 0.0; // NULL handling
        return static_cast<double>(mantissa) / 1000000000.0;
    }
};

// Enums from XML schema
enum class MDEntryType : char {
    BID = '0',
    OFFER = '1',
    TRADE = '2',
    OPENING_PRICE = '4',
    SETTLEMENT_PRICE = '6',
    TRADING_SESSION_HIGH_PRICE = '7',
    TRADING_SESSION_LOW_PRICE = '8',
    TRADE_VOLUME = 'B',
    OPEN_INTEREST = 'C',
    IMPLIED_BID = 'E',
    IMPLIED_OFFER = 'F',
    EMPTY_BOOK = 'J',
    SESSION_HIGH_BID = 'N',
    SESSION_LOW_OFFER = 'O',
    FIXING_PRICE = 'W',
    ELECTRONIC_VOLUME = 'e',
    THRESHOLD_LIMITS_AND_PRICE_BAND_VARIATION = 'g'
};

enum class SecurityUpdateAction : char {
    ADD = 'A',
    DELETE = 'D',
    MODIFY = 'M',
    NOCHANGE = 'N'
};

enum class MDUpdateAction : int8_t {
    NEW = 0,
    CHANGE = 1,
    DELETE = 2
};

enum class MarketDataType : int8_t {
    FXSPOT = 0,
    FXNDF = 1,
    FXFWD = 2,
    FXSWAP = 3,
    FXNDS = 4,
    INVALID = 127
};

enum class AggressorSide : uint8_t {
    NONE = 0,
    BUYSIDE = 1,
    SELLSIDE = 2
};

// Message Type Constants
namespace MessageTypes {
constexpr uint16_t ADMIN_HEARTBEAT = 10;
constexpr uint16_t SECURITY_DEFINITION = 18;
constexpr uint16_t MD_FULL_REFRESH = 20;
constexpr uint16_t MD_INCREMENTAL_REFRESH = 21;
constexpr uint16_t MD_INCREMENTAL_REFRESH_TRADES = 111;
}
