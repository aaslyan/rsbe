#pragma once

#include "UTPMessage.h"
#include <chrono>

// AdminHeartbeat Message (Template ID 10)
struct AdminHeartbeat {
    UTPMessageHeader header;

    AdminHeartbeat()
    {
        header.templateId = MessageTypes::ADMIN_HEARTBEAT;
        header.blockLength = 0; // No fields in heartbeat
    }

    size_t pack(std::vector<uint8_t>& buffer) const
    {
        buffer.resize(UTPMessageHeader::size());
        header.pack_little_endian(buffer.data());
        return buffer.size();
    }

    void unpack(const uint8_t* buffer)
    {
        header.unpack_little_endian(buffer);
    }
};

// Month/Year/Day structure
struct MonthYearDay {
    uint16_t year = 65535; // NULL value
    uint8_t month = 0; // 0 for NULL
    uint8_t day = 0; // 0 for NULL

    MonthYearDay() = default;
    MonthYearDay(uint16_t y, uint8_t m, uint8_t d)
        : year(y)
        , month(m)
        , day(d)
    {
    }

    static constexpr size_t size() { return 4; }
};

// Time of Day structure
struct TimeOfDay {
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
    uint16_t millisecond = 0;

    TimeOfDay() = default;
    TimeOfDay(uint8_t h, uint8_t m, uint8_t s, uint16_t ms)
        : hour(h)
        , minute(m)
        , second(s)
        , millisecond(ms)
    {
    }

    static constexpr size_t size() { return 5; }
};

// SecurityDefinition Message (Template ID 18)
struct SecurityDefinition {
    UTPMessageHeader header;

    // Fixed fields (blockLength = 106)
    SecurityUpdateAction securityUpdateAction;
    uint64_t lastUpdateTime;
    int16_t applID = 18; // Constant
    char mdEntryOriginator[16];
    char symbol[16];
    int32_t securityID;
    uint32_t securityIDSource;
    MarketDataType securityType;
    MonthYearDay settlDate;
    char currency1[3];
    char currency2[3];
    uint8_t basisPoint = 255; // NULL
    uint8_t ratePrecision;
    uint8_t rateTerm;
    uint8_t currency1AmtDecimals = 255; // NULL
    uint8_t currency2AmtDecimals = 255; // NULL
    uint8_t rgtsmdps;
    uint8_t leftDps;
    uint8_t rightDps;
    uint8_t cls;
    uint64_t maxPriceVariation = 18446744073709551615ULL; // NULL
    TimeOfDay snapshotConflationInterval;
    TimeOfDay incRefreshConflationInterval;
    TimeOfDay tradesFeedConflationInterval;
    TimeOfDay securityDefinitionConflationInterval;
    uint8_t depthOfBook;
    int64_t minTradeVol;

    SecurityDefinition()
    {
        header.templateId = MessageTypes::SECURITY_DEFINITION;
        header.blockLength = 106;
        memset(mdEntryOriginator, 0, sizeof(mdEntryOriginator));
        memset(symbol, 0, sizeof(symbol));
        memset(currency1, 0, sizeof(currency1));
        memset(currency2, 0, sizeof(currency2));
    }

    size_t pack(std::vector<uint8_t>& buffer) const
    {
        size_t total_size = UTPMessageHeader::size() + header.blockLength;
        buffer.resize(total_size);

        header.pack_little_endian(buffer.data());

        size_t pos = UTPMessageHeader::size();
        buffer[pos] = static_cast<char>(securityUpdateAction);
        pos += 1;
        memcpy(buffer.data() + pos, &lastUpdateTime, 8);
        pos += 8;
        // Skip applID - it's constant
        memcpy(buffer.data() + pos, mdEntryOriginator, 16);
        pos += 16;
        memcpy(buffer.data() + pos, symbol, 16);
        pos += 16;
        memcpy(buffer.data() + pos, &securityID, 4);
        pos += 4;
        memcpy(buffer.data() + pos, &securityIDSource, 4);
        pos += 4;
        buffer[pos] = static_cast<int8_t>(securityType);
        pos += 1;
        // Pack MonthYearDay, TimeOfDay structures, etc...

        return total_size;
    }
};

// MD Entry for Full Refresh
struct MDEntry {
    MDEntryType mdEntryType;
    PriceNull mdEntryPx;
    int64_t mdEntrySize;

    static constexpr size_t size() { return 17; } // 1 + 8 + 8
};

// MDFullRefresh Message (Template ID 20)
struct MDFullRefresh {
    UTPMessageHeader header;

    // Fixed fields (blockLength = 46)
    int64_t lastMsgSeqNumProcessed;
    int32_t securityID;
    int64_t rptSeq;
    uint64_t transactTime;
    char mdEntryOriginator[16];
    uint8_t marketDepth;
    MarketDataType securityType;

    // Repeating group
    std::vector<MDEntry> mdEntries;

    MDFullRefresh()
    {
        header.templateId = MessageTypes::MD_FULL_REFRESH;
        header.blockLength = 46;
        memset(mdEntryOriginator, 0, sizeof(mdEntryOriginator));
        transactTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch())
                           .count();
    }

    size_t pack(std::vector<uint8_t>& buffer) const
    {
        size_t group_size = GroupSize::size() + mdEntries.size() * MDEntry::size();
        size_t total_size = UTPMessageHeader::size() + header.blockLength + group_size;

        buffer.resize(total_size);
        header.pack_little_endian(buffer.data());

        size_t pos = UTPMessageHeader::size();

        // Pack fixed fields
        memcpy(buffer.data() + pos, &lastMsgSeqNumProcessed, 8);
        pos += 8;
        memcpy(buffer.data() + pos, &securityID, 4);
        pos += 4;
        memcpy(buffer.data() + pos, &rptSeq, 8);
        pos += 8;
        memcpy(buffer.data() + pos, &transactTime, 8);
        pos += 8;
        memcpy(buffer.data() + pos, mdEntryOriginator, 16);
        pos += 16;
        buffer[pos] = marketDepth;
        pos += 1;
        buffer[pos] = static_cast<int8_t>(securityType);
        pos += 1;

        // Pack group header
        GroupSize group;
        group.blockLength = 17;
        group.numInGroup = static_cast<uint16_t>(mdEntries.size());
        group.pack_little_endian(buffer.data() + pos);
        pos += GroupSize::size();

        // Pack entries
        for (const auto& entry : mdEntries) {
            buffer[pos] = static_cast<char>(entry.mdEntryType);
            pos += 1;
            memcpy(buffer.data() + pos, &entry.mdEntryPx.mantissa, 8);
            pos += 8;
            memcpy(buffer.data() + pos, &entry.mdEntrySize, 8);
            pos += 8;
        }

        return total_size;
    }
};

// MD Incremental Entry
struct MDIncrementalEntry {
    MDUpdateAction mdUpdateAction;
    MDEntryType mdEntryType;
    PriceNull mdEntryPx;
    int64_t mdEntrySize;

    static constexpr size_t size() { return 18; } // 1 + 1 + 8 + 8
};

// MDIncrementalRefresh Message (Template ID 21)
struct MDIncrementalRefresh {
    UTPMessageHeader header;

    // Fixed fields (blockLength = 36)
    int32_t securityID;
    int64_t rptSeq;
    uint64_t transactTime;
    char mdEntryOriginator[16];

    // Repeating group
    std::vector<MDIncrementalEntry> mdEntries;

    MDIncrementalRefresh()
    {
        header.templateId = MessageTypes::MD_INCREMENTAL_REFRESH;
        header.blockLength = 36;
        memset(mdEntryOriginator, 0, sizeof(mdEntryOriginator));
        transactTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch())
                           .count();
    }

    size_t pack(std::vector<uint8_t>& buffer) const
    {
        size_t group_size = GroupSize::size() + mdEntries.size() * MDIncrementalEntry::size();
        size_t total_size = UTPMessageHeader::size() + header.blockLength + group_size;

        buffer.resize(total_size);
        header.pack_little_endian(buffer.data());

        size_t pos = UTPMessageHeader::size();

        // Pack fixed fields
        memcpy(buffer.data() + pos, &securityID, 4);
        pos += 4;
        memcpy(buffer.data() + pos, &rptSeq, 8);
        pos += 8;
        memcpy(buffer.data() + pos, &transactTime, 8);
        pos += 8;
        memcpy(buffer.data() + pos, mdEntryOriginator, 16);
        pos += 16;

        // Pack group header
        GroupSize group;
        group.blockLength = 18;
        group.numInGroup = static_cast<uint16_t>(mdEntries.size());
        group.pack_little_endian(buffer.data() + pos);
        pos += GroupSize::size();

        // Pack entries
        for (const auto& entry : mdEntries) {
            buffer[pos] = static_cast<int8_t>(entry.mdUpdateAction);
            pos += 1;
            buffer[pos] = static_cast<char>(entry.mdEntryType);
            pos += 1;
            memcpy(buffer.data() + pos, &entry.mdEntryPx.mantissa, 8);
            pos += 8;
            memcpy(buffer.data() + pos, &entry.mdEntrySize, 8);
            pos += 8;
        }

        return total_size;
    }
};
