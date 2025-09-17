/* Generated SBE (Simple Binary Encoding) message codec */
#ifndef _UTP_SBE_MDENTRYTYPE_CXX_H_
#define _UTP_SBE_MDENTRYTYPE_CXX_H_

#if !defined(__STDC_LIMIT_MACROS)
#  define __STDC_LIMIT_MACROS 1
#endif

#include <cstdint>
#include <iomanip>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <sstream>
#include <string>

#define SBE_NULLVALUE_INT8 (std::numeric_limits<std::int8_t>::min)()
#define SBE_NULLVALUE_INT16 (std::numeric_limits<std::int16_t>::min)()
#define SBE_NULLVALUE_INT32 (std::numeric_limits<std::int32_t>::min)()
#define SBE_NULLVALUE_INT64 (std::numeric_limits<std::int64_t>::min)()
#define SBE_NULLVALUE_UINT8 (std::numeric_limits<std::uint8_t>::max)()
#define SBE_NULLVALUE_UINT16 (std::numeric_limits<std::uint16_t>::max)()
#define SBE_NULLVALUE_UINT32 (std::numeric_limits<std::uint32_t>::max)()
#define SBE_NULLVALUE_UINT64 (std::numeric_limits<std::uint64_t>::max)()

namespace utp_sbe {

class MDEntryType
{
public:
    enum Value
    {
        BID = static_cast<char>(48),
        OFFER = static_cast<char>(49),
        TRADE = static_cast<char>(50),
        OPENING_PRICE = static_cast<char>(52),
        SETTLEMENT_PRICE = static_cast<char>(54),
        TRADING_SESSION_HIGH_PRICE = static_cast<char>(55),
        TRADING_SESSION_LOW_PRICE = static_cast<char>(56),
        TRADE_VOLUME = static_cast<char>(66),
        OPEN_INTEREST = static_cast<char>(67),
        IMPLIED_BID = static_cast<char>(69),
        IMPLIED_OFFER = static_cast<char>(70),
        EMPTY_BOOK = static_cast<char>(74),
        SESSION_HIGH_BID = static_cast<char>(78),
        SESSION_LOW_OFFER = static_cast<char>(79),
        FIXING_PRICE = static_cast<char>(87),
        ELECTRONIC_VOLUME = static_cast<char>(101),
        THRESHOLD_LIMITS_AND_PRICE_BAND_VARIATION = static_cast<char>(103),
        NULL_VALUE = static_cast<char>(0)
    };

    static MDEntryType::Value get(const char value)
    {
        switch (value)
        {
            case static_cast<char>(48): return BID;
            case static_cast<char>(49): return OFFER;
            case static_cast<char>(50): return TRADE;
            case static_cast<char>(52): return OPENING_PRICE;
            case static_cast<char>(54): return SETTLEMENT_PRICE;
            case static_cast<char>(55): return TRADING_SESSION_HIGH_PRICE;
            case static_cast<char>(56): return TRADING_SESSION_LOW_PRICE;
            case static_cast<char>(66): return TRADE_VOLUME;
            case static_cast<char>(67): return OPEN_INTEREST;
            case static_cast<char>(69): return IMPLIED_BID;
            case static_cast<char>(70): return IMPLIED_OFFER;
            case static_cast<char>(74): return EMPTY_BOOK;
            case static_cast<char>(78): return SESSION_HIGH_BID;
            case static_cast<char>(79): return SESSION_LOW_OFFER;
            case static_cast<char>(87): return FIXING_PRICE;
            case static_cast<char>(101): return ELECTRONIC_VOLUME;
            case static_cast<char>(103): return THRESHOLD_LIMITS_AND_PRICE_BAND_VARIATION;
            case static_cast<char>(0): return NULL_VALUE;
        }

        throw std::runtime_error("unknown value for enum MDEntryType [E103]");
    }

    static const char *c_str(const MDEntryType::Value value)
    {
        switch (value)
        {
            case BID: return "BID";
            case OFFER: return "OFFER";
            case TRADE: return "TRADE";
            case OPENING_PRICE: return "OPENING_PRICE";
            case SETTLEMENT_PRICE: return "SETTLEMENT_PRICE";
            case TRADING_SESSION_HIGH_PRICE: return "TRADING_SESSION_HIGH_PRICE";
            case TRADING_SESSION_LOW_PRICE: return "TRADING_SESSION_LOW_PRICE";
            case TRADE_VOLUME: return "TRADE_VOLUME";
            case OPEN_INTEREST: return "OPEN_INTEREST";
            case IMPLIED_BID: return "IMPLIED_BID";
            case IMPLIED_OFFER: return "IMPLIED_OFFER";
            case EMPTY_BOOK: return "EMPTY_BOOK";
            case SESSION_HIGH_BID: return "SESSION_HIGH_BID";
            case SESSION_LOW_OFFER: return "SESSION_LOW_OFFER";
            case FIXING_PRICE: return "FIXING_PRICE";
            case ELECTRONIC_VOLUME: return "ELECTRONIC_VOLUME";
            case THRESHOLD_LIMITS_AND_PRICE_BAND_VARIATION: return "THRESHOLD_LIMITS_AND_PRICE_BAND_VARIATION";
            case NULL_VALUE: return "NULL_VALUE";
        }

        throw std::runtime_error("unknown value for enum MDEntryType [E103]:");
    }

    template<typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits> & operator << (
        std::basic_ostream<CharT, Traits> &os, MDEntryType::Value m)
    {
        return os << MDEntryType::c_str(m);
    }
};

}

#endif
