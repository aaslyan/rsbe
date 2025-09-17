/* Generated SBE (Simple Binary Encoding) message codec */
#ifndef _UTP_SBE_MARKETDATATYPE_CXX_H_
#define _UTP_SBE_MARKETDATATYPE_CXX_H_

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

class MarketDataType
{
public:
    enum Value
    {
        FXSPOT = static_cast<std::int8_t>(0),
        FXNDF = static_cast<std::int8_t>(1),
        FXFWD = static_cast<std::int8_t>(2),
        FXSWAP = static_cast<std::int8_t>(3),
        FXNDS = static_cast<std::int8_t>(4),
        INVALID = static_cast<std::int8_t>(127),
        NULL_VALUE = static_cast<std::int8_t>(-128)
    };

    static MarketDataType::Value get(const std::int8_t value)
    {
        switch (value)
        {
            case static_cast<std::int8_t>(0): return FXSPOT;
            case static_cast<std::int8_t>(1): return FXNDF;
            case static_cast<std::int8_t>(2): return FXFWD;
            case static_cast<std::int8_t>(3): return FXSWAP;
            case static_cast<std::int8_t>(4): return FXNDS;
            case static_cast<std::int8_t>(127): return INVALID;
            case static_cast<std::int8_t>(-128): return NULL_VALUE;
        }

        throw std::runtime_error("unknown value for enum MarketDataType [E103]");
    }

    static const char *c_str(const MarketDataType::Value value)
    {
        switch (value)
        {
            case FXSPOT: return "FXSPOT";
            case FXNDF: return "FXNDF";
            case FXFWD: return "FXFWD";
            case FXSWAP: return "FXSWAP";
            case FXNDS: return "FXNDS";
            case INVALID: return "INVALID";
            case NULL_VALUE: return "NULL_VALUE";
        }

        throw std::runtime_error("unknown value for enum MarketDataType [E103]:");
    }

    template<typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits> & operator << (
        std::basic_ostream<CharT, Traits> &os, MarketDataType::Value m)
    {
        return os << MarketDataType::c_str(m);
    }
};

}

#endif
