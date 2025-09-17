/* Generated SBE (Simple Binary Encoding) message codec */
#ifndef _UTP_SBE_SECURITYDEFINITION_CXX_H_
#define _UTP_SBE_SECURITYDEFINITION_CXX_H_

#if __cplusplus >= 201103L
#  define SBE_CONSTEXPR constexpr
#  define SBE_NOEXCEPT noexcept
#else
#  define SBE_CONSTEXPR
#  define SBE_NOEXCEPT
#endif

#if __cplusplus >= 201703L
#  include <string_view>
#  define SBE_NODISCARD [[nodiscard]]
#else
#  define SBE_NODISCARD
#endif

#if !defined(__STDC_LIMIT_MACROS)
#  define __STDC_LIMIT_MACROS 1
#endif

#include <cstdint>
#include <limits>
#include <cstring>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>
#include <tuple>

#if defined(WIN32) || defined(_WIN32)
#  define SBE_BIG_ENDIAN_ENCODE_16(v) _byteswap_ushort(v)
#  define SBE_BIG_ENDIAN_ENCODE_32(v) _byteswap_ulong(v)
#  define SBE_BIG_ENDIAN_ENCODE_64(v) _byteswap_uint64(v)
#  define SBE_LITTLE_ENDIAN_ENCODE_16(v) (v)
#  define SBE_LITTLE_ENDIAN_ENCODE_32(v) (v)
#  define SBE_LITTLE_ENDIAN_ENCODE_64(v) (v)
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#  define SBE_BIG_ENDIAN_ENCODE_16(v) __builtin_bswap16(v)
#  define SBE_BIG_ENDIAN_ENCODE_32(v) __builtin_bswap32(v)
#  define SBE_BIG_ENDIAN_ENCODE_64(v) __builtin_bswap64(v)
#  define SBE_LITTLE_ENDIAN_ENCODE_16(v) (v)
#  define SBE_LITTLE_ENDIAN_ENCODE_32(v) (v)
#  define SBE_LITTLE_ENDIAN_ENCODE_64(v) (v)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#  define SBE_LITTLE_ENDIAN_ENCODE_16(v) __builtin_bswap16(v)
#  define SBE_LITTLE_ENDIAN_ENCODE_32(v) __builtin_bswap32(v)
#  define SBE_LITTLE_ENDIAN_ENCODE_64(v) __builtin_bswap64(v)
#  define SBE_BIG_ENDIAN_ENCODE_16(v) (v)
#  define SBE_BIG_ENDIAN_ENCODE_32(v) (v)
#  define SBE_BIG_ENDIAN_ENCODE_64(v) (v)
#else
#  error "Byte Ordering of platform not determined. Set __BYTE_ORDER__ manually before including this file."
#endif

#if !defined(SBE_BOUNDS_CHECK_EXPECT)
#  if defined(SBE_NO_BOUNDS_CHECK)
#    define SBE_BOUNDS_CHECK_EXPECT(exp, c) (false)
#  elif defined(_MSC_VER)
#    define SBE_BOUNDS_CHECK_EXPECT(exp, c) (exp)
#  else 
#    define SBE_BOUNDS_CHECK_EXPECT(exp, c) (__builtin_expect(exp, c))
#  endif

#endif

#define SBE_FLOAT_NAN std::numeric_limits<float>::quiet_NaN()
#define SBE_DOUBLE_NAN std::numeric_limits<double>::quiet_NaN()
#define SBE_NULLVALUE_INT8 (std::numeric_limits<std::int8_t>::min)()
#define SBE_NULLVALUE_INT16 (std::numeric_limits<std::int16_t>::min)()
#define SBE_NULLVALUE_INT32 (std::numeric_limits<std::int32_t>::min)()
#define SBE_NULLVALUE_INT64 (std::numeric_limits<std::int64_t>::min)()
#define SBE_NULLVALUE_UINT8 (std::numeric_limits<std::uint8_t>::max)()
#define SBE_NULLVALUE_UINT16 (std::numeric_limits<std::uint16_t>::max)()
#define SBE_NULLVALUE_UINT32 (std::numeric_limits<std::uint32_t>::max)()
#define SBE_NULLVALUE_UINT64 (std::numeric_limits<std::uint64_t>::max)()


#include "TimeOfDay.h"
#include "MessageHeader.h"
#include "SecurityUpdateAction.h"
#include "MarketDataType.h"
#include "RateTerm.h"
#include "PriceNull.h"
#include "MDUpdateAction.h"
#include "MonthYearDay.h"
#include "GroupSize.h"
#include "MDEntryType.h"
#include "AggressorSide.h"

namespace utp_sbe {

class SecurityDefinition
{
private:
    char *m_buffer = nullptr;
    std::uint64_t m_bufferLength = 0;
    std::uint64_t m_offset = 0;
    std::uint64_t m_position = 0;
    std::uint64_t m_actingBlockLength = 0;
    std::uint64_t m_actingVersion = 0;

    inline std::uint64_t *sbePositionPtr() SBE_NOEXCEPT
    {
        return &m_position;
    }

public:
    static constexpr std::uint16_t SBE_BLOCK_LENGTH = static_cast<std::uint16_t>(106);
    static constexpr std::uint16_t SBE_TEMPLATE_ID = static_cast<std::uint16_t>(18);
    static constexpr std::uint16_t SBE_SCHEMA_ID = static_cast<std::uint16_t>(101);
    static constexpr std::uint16_t SBE_SCHEMA_VERSION = static_cast<std::uint16_t>(1);
    static constexpr const char* SBE_SEMANTIC_VERSION = "FIX5SP2";

    enum MetaAttribute
    {
        EPOCH, TIME_UNIT, SEMANTIC_TYPE, PRESENCE
    };

    union sbe_float_as_uint_u
    {
        float fp_value;
        std::uint32_t uint_value;
    };

    union sbe_double_as_uint_u
    {
        double fp_value;
        std::uint64_t uint_value;
    };

    using messageHeader = MessageHeader;

    SecurityDefinition() = default;

    SecurityDefinition(
        char *buffer,
        const std::uint64_t offset,
        const std::uint64_t bufferLength,
        const std::uint64_t actingBlockLength,
        const std::uint64_t actingVersion) :
        m_buffer(buffer),
        m_bufferLength(bufferLength),
        m_offset(offset),
        m_position(sbeCheckPosition(offset + actingBlockLength)),
        m_actingBlockLength(actingBlockLength),
        m_actingVersion(actingVersion)
    {
    }

    SecurityDefinition(char *buffer, const std::uint64_t bufferLength) :
        SecurityDefinition(buffer, 0, bufferLength, sbeBlockLength(), sbeSchemaVersion())
    {
    }

    SecurityDefinition(
        char *buffer,
        const std::uint64_t bufferLength,
        const std::uint64_t actingBlockLength,
        const std::uint64_t actingVersion) :
        SecurityDefinition(buffer, 0, bufferLength, actingBlockLength, actingVersion)
    {
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeBlockLength() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(106);
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t sbeBlockAndHeaderLength() SBE_NOEXCEPT
    {
        return messageHeader::encodedLength() + sbeBlockLength();
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeTemplateId() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(18);
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeSchemaId() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(101);
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeSchemaVersion() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(1);
    }

    SBE_NODISCARD static const char *sbeSemanticVersion() SBE_NOEXCEPT
    {
        return "FIX5SP2";
    }

    SBE_NODISCARD static SBE_CONSTEXPR const char *sbeSemanticType() SBE_NOEXCEPT
    {
        return "d";
    }

    SBE_NODISCARD std::uint64_t offset() const SBE_NOEXCEPT
    {
        return m_offset;
    }

    SecurityDefinition &wrapForEncode(char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)
    {
        m_buffer = buffer;
        m_bufferLength = bufferLength;
        m_offset = offset;
        m_actingBlockLength = sbeBlockLength();
        m_actingVersion = sbeSchemaVersion();
        m_position = sbeCheckPosition(m_offset + m_actingBlockLength);
        return *this;
    }

    SecurityDefinition &wrapAndApplyHeader(char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)
    {
        messageHeader hdr(buffer, offset, bufferLength, sbeSchemaVersion());

        hdr
            .blockLength(sbeBlockLength())
            .templateId(sbeTemplateId())
            .schemaId(sbeSchemaId())
            .version(sbeSchemaVersion());

        m_buffer = buffer;
        m_bufferLength = bufferLength;
        m_offset = offset + messageHeader::encodedLength();
        m_actingBlockLength = sbeBlockLength();
        m_actingVersion = sbeSchemaVersion();
        m_position = sbeCheckPosition(m_offset + m_actingBlockLength);
        return *this;
    }

    SecurityDefinition &wrapForDecode(
        char *buffer,
        const std::uint64_t offset,
        const std::uint64_t actingBlockLength,
        const std::uint64_t actingVersion,
        const std::uint64_t bufferLength)
    {
        m_buffer = buffer;
        m_bufferLength = bufferLength;
        m_offset = offset;
        m_actingBlockLength = actingBlockLength;
        m_actingVersion = actingVersion;
        m_position = sbeCheckPosition(m_offset + m_actingBlockLength);
        return *this;
    }

    SecurityDefinition &sbeRewind()
    {
        return wrapForDecode(m_buffer, m_offset, m_actingBlockLength, m_actingVersion, m_bufferLength);
    }

    SBE_NODISCARD std::uint64_t sbePosition() const SBE_NOEXCEPT
    {
        return m_position;
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    std::uint64_t sbeCheckPosition(const std::uint64_t position)
    {
        if (SBE_BOUNDS_CHECK_EXPECT((position > m_bufferLength), false))
        {
            throw std::runtime_error("buffer too short [E100]");
        }
        return position;
    }

    void sbePosition(const std::uint64_t position)
    {
        m_position = sbeCheckPosition(position);
    }

    SBE_NODISCARD std::uint64_t encodedLength() const SBE_NOEXCEPT
    {
        return sbePosition() - m_offset;
    }

    SBE_NODISCARD std::uint64_t decodeLength() const
    {
        SecurityDefinition skipper(m_buffer, m_offset, m_bufferLength, sbeBlockLength(), m_actingVersion);
        skipper.skip();
        return skipper.encodedLength();
    }

    SBE_NODISCARD const char *buffer() const SBE_NOEXCEPT
    {
        return m_buffer;
    }

    SBE_NODISCARD char *buffer() SBE_NOEXCEPT
    {
        return m_buffer;
    }

    SBE_NODISCARD std::uint64_t bufferLength() const SBE_NOEXCEPT
    {
        return m_bufferLength;
    }

    SBE_NODISCARD std::uint64_t actingVersion() const SBE_NOEXCEPT
    {
        return m_actingVersion;
    }

    SBE_NODISCARD static const char *SecurityUpdateActionMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "Char";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t securityUpdateActionId() SBE_NOEXCEPT
    {
        return 279;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t securityUpdateActionSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool securityUpdateActionInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t securityUpdateActionEncodingOffset() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t securityUpdateActionEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD char securityUpdateActionRaw() const SBE_NOEXCEPT
    {
        char val;
        std::memcpy(&val, m_buffer + m_offset + 0, sizeof(char));
        return (val);
    }

    SBE_NODISCARD SecurityUpdateAction::Value securityUpdateAction() const
    {
        char val;
        std::memcpy(&val, m_buffer + m_offset + 0, sizeof(char));
        return SecurityUpdateAction::get((val));
    }

    SecurityDefinition &securityUpdateAction(const SecurityUpdateAction::Value value) SBE_NOEXCEPT
    {
        char val = (value);
        std::memcpy(m_buffer + m_offset + 0, &val, sizeof(char));
        return *this;
    }

    SBE_NODISCARD static const char *LastUpdateTimeMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "UInt64";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t lastUpdateTimeId() SBE_NOEXCEPT
    {
        return 779;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t lastUpdateTimeSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool lastUpdateTimeInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t lastUpdateTimeEncodingOffset() SBE_NOEXCEPT
    {
        return 1;
    }

    static SBE_CONSTEXPR std::uint64_t lastUpdateTimeNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_UINT64;
    }

    static SBE_CONSTEXPR std::uint64_t lastUpdateTimeMinValue() SBE_NOEXCEPT
    {
        return UINT64_C(0x0);
    }

    static SBE_CONSTEXPR std::uint64_t lastUpdateTimeMaxValue() SBE_NOEXCEPT
    {
        return UINT64_C(0xfffffffffffffffe);
    }

    static SBE_CONSTEXPR std::size_t lastUpdateTimeEncodingLength() SBE_NOEXCEPT
    {
        return 8;
    }

    SBE_NODISCARD std::uint64_t lastUpdateTime() const SBE_NOEXCEPT
    {
        std::uint64_t val;
        std::memcpy(&val, m_buffer + m_offset + 1, sizeof(std::uint64_t));
        return SBE_LITTLE_ENDIAN_ENCODE_64(val);
    }

    SecurityDefinition &lastUpdateTime(const std::uint64_t value) SBE_NOEXCEPT
    {
        std::uint64_t val = SBE_LITTLE_ENDIAN_ENCODE_64(value);
        std::memcpy(m_buffer + m_offset + 1, &val, sizeof(std::uint64_t));
        return *this;
    }

    SBE_NODISCARD static const char *ApplIDMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "Int16";
            case MetaAttribute::PRESENCE: return "constant";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t applIDId() SBE_NOEXCEPT
    {
        return 1180;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t applIDSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool applIDInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t applIDEncodingOffset() SBE_NOEXCEPT
    {
        return 9;
    }

    static SBE_CONSTEXPR std::int16_t applIDNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_INT16;
    }

    static SBE_CONSTEXPR std::int16_t applIDMinValue() SBE_NOEXCEPT
    {
        return static_cast<std::int16_t>(-32767);
    }

    static SBE_CONSTEXPR std::int16_t applIDMaxValue() SBE_NOEXCEPT
    {
        return static_cast<std::int16_t>(32767);
    }

    static SBE_CONSTEXPR std::size_t applIDEncodingLength() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::int16_t applID() SBE_NOEXCEPT
    {
        return static_cast<std::int16_t>(18);
    }

    SBE_NODISCARD static const char *MDEntryOriginatorMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "String";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t mDEntryOriginatorId() SBE_NOEXCEPT
    {
        return 282;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t mDEntryOriginatorSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool mDEntryOriginatorInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t mDEntryOriginatorEncodingOffset() SBE_NOEXCEPT
    {
        return 9;
    }

    static SBE_CONSTEXPR char mDEntryOriginatorNullValue() SBE_NOEXCEPT
    {
        return static_cast<char>(0);
    }

    static SBE_CONSTEXPR char mDEntryOriginatorMinValue() SBE_NOEXCEPT
    {
        return static_cast<char>(32);
    }

    static SBE_CONSTEXPR char mDEntryOriginatorMaxValue() SBE_NOEXCEPT
    {
        return static_cast<char>(126);
    }

    static SBE_CONSTEXPR std::size_t mDEntryOriginatorEncodingLength() SBE_NOEXCEPT
    {
        return 16;
    }

    static SBE_CONSTEXPR std::uint64_t mDEntryOriginatorLength() SBE_NOEXCEPT
    {
        return 16;
    }

    SBE_NODISCARD const char *mDEntryOriginator() const SBE_NOEXCEPT
    {
        return m_buffer + m_offset + 9;
    }

    SBE_NODISCARD char *mDEntryOriginator() SBE_NOEXCEPT
    {
        return m_buffer + m_offset + 9;
    }

    SBE_NODISCARD char mDEntryOriginator(const std::uint64_t index) const
    {
        if (index >= 16)
        {
            throw std::runtime_error("index out of range for mDEntryOriginator [E104]");
        }

        char val;
        std::memcpy(&val, m_buffer + m_offset + 9 + (index * 1), sizeof(char));
        return (val);
    }

    SecurityDefinition &mDEntryOriginator(const std::uint64_t index, const char value)
    {
        if (index >= 16)
        {
            throw std::runtime_error("index out of range for mDEntryOriginator [E105]");
        }

        char val = (value);
        std::memcpy(m_buffer + m_offset + 9 + (index * 1), &val, sizeof(char));
        return *this;
    }

    std::uint64_t getMDEntryOriginator(char *const dst, const std::uint64_t length) const
    {
        if (length > 16)
        {
            throw std::runtime_error("length too large for getMDEntryOriginator [E106]");
        }

        std::memcpy(dst, m_buffer + m_offset + 9, sizeof(char) * static_cast<std::size_t>(length));
        return length;
    }

    SecurityDefinition &putMDEntryOriginator(const char *const src) SBE_NOEXCEPT
    {
        std::memcpy(m_buffer + m_offset + 9, src, sizeof(char) * 16);
        return *this;
    }

    SBE_NODISCARD std::string getMDEntryOriginatorAsString() const
    {
        const char *buffer = m_buffer + m_offset + 9;
        std::size_t length = 0;

        for (; length < 16 && *(buffer + length) != '\0'; ++length);
        std::string result(buffer, length);

        return result;
    }

    std::string getMDEntryOriginatorAsJsonEscapedString()
    {
        std::ostringstream oss;
        std::string s = getMDEntryOriginatorAsString();

        for (const auto c : s)
        {
            switch (c)
            {
                case '"': oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;

                default:
                    if ('\x00' <= c && c <= '\x1f')
                    {
                        oss << "\\u" << std::hex << std::setw(4)
                            << std::setfill('0') << (int)(c);
                    }
                    else
                    {
                        oss << c;
                    }
            }
        }

        return oss.str();
    }

    #if __cplusplus >= 201703L
    SBE_NODISCARD std::string_view getMDEntryOriginatorAsStringView() const SBE_NOEXCEPT
    {
        const char *buffer = m_buffer + m_offset + 9;
        std::size_t length = 0;

        for (; length < 16 && *(buffer + length) != '\0'; ++length);
        std::string_view result(buffer, length);

        return result;
    }
    #endif

    #if __cplusplus >= 201703L
    SecurityDefinition &putMDEntryOriginator(const std::string_view str)
    {
        const std::size_t srcLength = str.length();
        if (srcLength > 16)
        {
            throw std::runtime_error("string too large for putMDEntryOriginator [E106]");
        }

        std::memcpy(m_buffer + m_offset + 9, str.data(), srcLength);
        for (std::size_t start = srcLength; start < 16; ++start)
        {
            m_buffer[m_offset + 9 + start] = 0;
        }

        return *this;
    }
    #else
    SecurityDefinition &putMDEntryOriginator(const std::string &str)
    {
        const std::size_t srcLength = str.length();
        if (srcLength > 16)
        {
            throw std::runtime_error("string too large for putMDEntryOriginator [E106]");
        }

        std::memcpy(m_buffer + m_offset + 9, str.c_str(), srcLength);
        for (std::size_t start = srcLength; start < 16; ++start)
        {
            m_buffer[m_offset + 9 + start] = 0;
        }

        return *this;
    }
    #endif

    SBE_NODISCARD static const char *SymbolMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "String";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t symbolId() SBE_NOEXCEPT
    {
        return 55;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t symbolSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool symbolInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t symbolEncodingOffset() SBE_NOEXCEPT
    {
        return 25;
    }

    static SBE_CONSTEXPR char symbolNullValue() SBE_NOEXCEPT
    {
        return static_cast<char>(0);
    }

    static SBE_CONSTEXPR char symbolMinValue() SBE_NOEXCEPT
    {
        return static_cast<char>(32);
    }

    static SBE_CONSTEXPR char symbolMaxValue() SBE_NOEXCEPT
    {
        return static_cast<char>(126);
    }

    static SBE_CONSTEXPR std::size_t symbolEncodingLength() SBE_NOEXCEPT
    {
        return 16;
    }

    static SBE_CONSTEXPR std::uint64_t symbolLength() SBE_NOEXCEPT
    {
        return 16;
    }

    SBE_NODISCARD const char *symbol() const SBE_NOEXCEPT
    {
        return m_buffer + m_offset + 25;
    }

    SBE_NODISCARD char *symbol() SBE_NOEXCEPT
    {
        return m_buffer + m_offset + 25;
    }

    SBE_NODISCARD char symbol(const std::uint64_t index) const
    {
        if (index >= 16)
        {
            throw std::runtime_error("index out of range for symbol [E104]");
        }

        char val;
        std::memcpy(&val, m_buffer + m_offset + 25 + (index * 1), sizeof(char));
        return (val);
    }

    SecurityDefinition &symbol(const std::uint64_t index, const char value)
    {
        if (index >= 16)
        {
            throw std::runtime_error("index out of range for symbol [E105]");
        }

        char val = (value);
        std::memcpy(m_buffer + m_offset + 25 + (index * 1), &val, sizeof(char));
        return *this;
    }

    std::uint64_t getSymbol(char *const dst, const std::uint64_t length) const
    {
        if (length > 16)
        {
            throw std::runtime_error("length too large for getSymbol [E106]");
        }

        std::memcpy(dst, m_buffer + m_offset + 25, sizeof(char) * static_cast<std::size_t>(length));
        return length;
    }

    SecurityDefinition &putSymbol(const char *const src) SBE_NOEXCEPT
    {
        std::memcpy(m_buffer + m_offset + 25, src, sizeof(char) * 16);
        return *this;
    }

    SBE_NODISCARD std::string getSymbolAsString() const
    {
        const char *buffer = m_buffer + m_offset + 25;
        std::size_t length = 0;

        for (; length < 16 && *(buffer + length) != '\0'; ++length);
        std::string result(buffer, length);

        return result;
    }

    std::string getSymbolAsJsonEscapedString()
    {
        std::ostringstream oss;
        std::string s = getSymbolAsString();

        for (const auto c : s)
        {
            switch (c)
            {
                case '"': oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;

                default:
                    if ('\x00' <= c && c <= '\x1f')
                    {
                        oss << "\\u" << std::hex << std::setw(4)
                            << std::setfill('0') << (int)(c);
                    }
                    else
                    {
                        oss << c;
                    }
            }
        }

        return oss.str();
    }

    #if __cplusplus >= 201703L
    SBE_NODISCARD std::string_view getSymbolAsStringView() const SBE_NOEXCEPT
    {
        const char *buffer = m_buffer + m_offset + 25;
        std::size_t length = 0;

        for (; length < 16 && *(buffer + length) != '\0'; ++length);
        std::string_view result(buffer, length);

        return result;
    }
    #endif

    #if __cplusplus >= 201703L
    SecurityDefinition &putSymbol(const std::string_view str)
    {
        const std::size_t srcLength = str.length();
        if (srcLength > 16)
        {
            throw std::runtime_error("string too large for putSymbol [E106]");
        }

        std::memcpy(m_buffer + m_offset + 25, str.data(), srcLength);
        for (std::size_t start = srcLength; start < 16; ++start)
        {
            m_buffer[m_offset + 25 + start] = 0;
        }

        return *this;
    }
    #else
    SecurityDefinition &putSymbol(const std::string &str)
    {
        const std::size_t srcLength = str.length();
        if (srcLength > 16)
        {
            throw std::runtime_error("string too large for putSymbol [E106]");
        }

        std::memcpy(m_buffer + m_offset + 25, str.c_str(), srcLength);
        for (std::size_t start = srcLength; start < 16; ++start)
        {
            m_buffer[m_offset + 25 + start] = 0;
        }

        return *this;
    }
    #endif

    SBE_NODISCARD static const char *SecurityIDMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "Int32";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t securityIDId() SBE_NOEXCEPT
    {
        return 48;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t securityIDSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool securityIDInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t securityIDEncodingOffset() SBE_NOEXCEPT
    {
        return 41;
    }

    static SBE_CONSTEXPR std::int32_t securityIDNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_INT32;
    }

    static SBE_CONSTEXPR std::int32_t securityIDMinValue() SBE_NOEXCEPT
    {
        return INT32_C(-2147483647);
    }

    static SBE_CONSTEXPR std::int32_t securityIDMaxValue() SBE_NOEXCEPT
    {
        return INT32_C(2147483647);
    }

    static SBE_CONSTEXPR std::size_t securityIDEncodingLength() SBE_NOEXCEPT
    {
        return 4;
    }

    SBE_NODISCARD std::int32_t securityID() const SBE_NOEXCEPT
    {
        std::int32_t val;
        std::memcpy(&val, m_buffer + m_offset + 41, sizeof(std::int32_t));
        return SBE_LITTLE_ENDIAN_ENCODE_32(val);
    }

    SecurityDefinition &securityID(const std::int32_t value) SBE_NOEXCEPT
    {
        std::int32_t val = SBE_LITTLE_ENDIAN_ENCODE_32(value);
        std::memcpy(m_buffer + m_offset + 41, &val, sizeof(std::int32_t));
        return *this;
    }

    SBE_NODISCARD static const char *SecurityIDSourceMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "String";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t securityIDSourceId() SBE_NOEXCEPT
    {
        return 22;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t securityIDSourceSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool securityIDSourceInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t securityIDSourceEncodingOffset() SBE_NOEXCEPT
    {
        return 45;
    }

    static SBE_CONSTEXPR std::uint32_t securityIDSourceNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_UINT32;
    }

    static SBE_CONSTEXPR std::uint32_t securityIDSourceMinValue() SBE_NOEXCEPT
    {
        return UINT32_C(0x0);
    }

    static SBE_CONSTEXPR std::uint32_t securityIDSourceMaxValue() SBE_NOEXCEPT
    {
        return UINT32_C(0xfffffffe);
    }

    static SBE_CONSTEXPR std::size_t securityIDSourceEncodingLength() SBE_NOEXCEPT
    {
        return 4;
    }

    SBE_NODISCARD std::uint32_t securityIDSource() const SBE_NOEXCEPT
    {
        std::uint32_t val;
        std::memcpy(&val, m_buffer + m_offset + 45, sizeof(std::uint32_t));
        return SBE_LITTLE_ENDIAN_ENCODE_32(val);
    }

    SecurityDefinition &securityIDSource(const std::uint32_t value) SBE_NOEXCEPT
    {
        std::uint32_t val = SBE_LITTLE_ENDIAN_ENCODE_32(value);
        std::memcpy(m_buffer + m_offset + 45, &val, sizeof(std::uint32_t));
        return *this;
    }

    SBE_NODISCARD static const char *SecurityTypeMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "Int8";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t securityTypeId() SBE_NOEXCEPT
    {
        return 167;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t securityTypeSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool securityTypeInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t securityTypeEncodingOffset() SBE_NOEXCEPT
    {
        return 49;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t securityTypeEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::int8_t securityTypeRaw() const SBE_NOEXCEPT
    {
        std::int8_t val;
        std::memcpy(&val, m_buffer + m_offset + 49, sizeof(std::int8_t));
        return (val);
    }

    SBE_NODISCARD MarketDataType::Value securityType() const
    {
        std::int8_t val;
        std::memcpy(&val, m_buffer + m_offset + 49, sizeof(std::int8_t));
        return MarketDataType::get((val));
    }

    SecurityDefinition &securityType(const MarketDataType::Value value) SBE_NOEXCEPT
    {
        std::int8_t val = (value);
        std::memcpy(m_buffer + m_offset + 49, &val, sizeof(std::int8_t));
        return *this;
    }

    SBE_NODISCARD static const char *SettlDateMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "MonthYear";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t settlDateId() SBE_NOEXCEPT
    {
        return 64;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t settlDateSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool settlDateInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t settlDateEncodingOffset() SBE_NOEXCEPT
    {
        return 50;
    }

private:
    MonthYearDay m_settlDate;

public:
    SBE_NODISCARD MonthYearDay &settlDate()
    {
        m_settlDate.wrap(m_buffer, m_offset + 50, m_actingVersion, m_bufferLength);
        return m_settlDate;
    }

    SBE_NODISCARD static const char *Currency1MetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "Currency";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t currency1Id() SBE_NOEXCEPT
    {
        return 30375;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t currency1SinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool currency1InActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t currency1EncodingOffset() SBE_NOEXCEPT
    {
        return 54;
    }

    static SBE_CONSTEXPR char currency1NullValue() SBE_NOEXCEPT
    {
        return static_cast<char>(0);
    }

    static SBE_CONSTEXPR char currency1MinValue() SBE_NOEXCEPT
    {
        return static_cast<char>(32);
    }

    static SBE_CONSTEXPR char currency1MaxValue() SBE_NOEXCEPT
    {
        return static_cast<char>(126);
    }

    static SBE_CONSTEXPR std::size_t currency1EncodingLength() SBE_NOEXCEPT
    {
        return 3;
    }

    static SBE_CONSTEXPR std::uint64_t currency1Length() SBE_NOEXCEPT
    {
        return 3;
    }

    SBE_NODISCARD const char *currency1() const SBE_NOEXCEPT
    {
        return m_buffer + m_offset + 54;
    }

    SBE_NODISCARD char *currency1() SBE_NOEXCEPT
    {
        return m_buffer + m_offset + 54;
    }

    SBE_NODISCARD char currency1(const std::uint64_t index) const
    {
        if (index >= 3)
        {
            throw std::runtime_error("index out of range for currency1 [E104]");
        }

        char val;
        std::memcpy(&val, m_buffer + m_offset + 54 + (index * 1), sizeof(char));
        return (val);
    }

    SecurityDefinition &currency1(const std::uint64_t index, const char value)
    {
        if (index >= 3)
        {
            throw std::runtime_error("index out of range for currency1 [E105]");
        }

        char val = (value);
        std::memcpy(m_buffer + m_offset + 54 + (index * 1), &val, sizeof(char));
        return *this;
    }

    std::uint64_t getCurrency1(char *const dst, const std::uint64_t length) const
    {
        if (length > 3)
        {
            throw std::runtime_error("length too large for getCurrency1 [E106]");
        }

        std::memcpy(dst, m_buffer + m_offset + 54, sizeof(char) * static_cast<std::size_t>(length));
        return length;
    }

    SecurityDefinition &putCurrency1(const char *const src) SBE_NOEXCEPT
    {
        std::memcpy(m_buffer + m_offset + 54, src, sizeof(char) * 3);
        return *this;
    }

    SecurityDefinition &putCurrency1(
        const char value0,
        const char value1,
        const char value2) SBE_NOEXCEPT
    {
        char val0 = (value0);
        std::memcpy(m_buffer + m_offset + 54, &val0, sizeof(char));
        char val1 = (value1);
        std::memcpy(m_buffer + m_offset + 55, &val1, sizeof(char));
        char val2 = (value2);
        std::memcpy(m_buffer + m_offset + 56, &val2, sizeof(char));

        return *this;
    }

    SBE_NODISCARD std::string getCurrency1AsString() const
    {
        const char *buffer = m_buffer + m_offset + 54;
        std::size_t length = 0;

        for (; length < 3 && *(buffer + length) != '\0'; ++length);
        std::string result(buffer, length);

        return result;
    }

    std::string getCurrency1AsJsonEscapedString()
    {
        std::ostringstream oss;
        std::string s = getCurrency1AsString();

        for (const auto c : s)
        {
            switch (c)
            {
                case '"': oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;

                default:
                    if ('\x00' <= c && c <= '\x1f')
                    {
                        oss << "\\u" << std::hex << std::setw(4)
                            << std::setfill('0') << (int)(c);
                    }
                    else
                    {
                        oss << c;
                    }
            }
        }

        return oss.str();
    }

    #if __cplusplus >= 201703L
    SBE_NODISCARD std::string_view getCurrency1AsStringView() const SBE_NOEXCEPT
    {
        const char *buffer = m_buffer + m_offset + 54;
        std::size_t length = 0;

        for (; length < 3 && *(buffer + length) != '\0'; ++length);
        std::string_view result(buffer, length);

        return result;
    }
    #endif

    #if __cplusplus >= 201703L
    SecurityDefinition &putCurrency1(const std::string_view str)
    {
        const std::size_t srcLength = str.length();
        if (srcLength > 3)
        {
            throw std::runtime_error("string too large for putCurrency1 [E106]");
        }

        std::memcpy(m_buffer + m_offset + 54, str.data(), srcLength);
        for (std::size_t start = srcLength; start < 3; ++start)
        {
            m_buffer[m_offset + 54 + start] = 0;
        }

        return *this;
    }
    #else
    SecurityDefinition &putCurrency1(const std::string &str)
    {
        const std::size_t srcLength = str.length();
        if (srcLength > 3)
        {
            throw std::runtime_error("string too large for putCurrency1 [E106]");
        }

        std::memcpy(m_buffer + m_offset + 54, str.c_str(), srcLength);
        for (std::size_t start = srcLength; start < 3; ++start)
        {
            m_buffer[m_offset + 54 + start] = 0;
        }

        return *this;
    }
    #endif

    SBE_NODISCARD static const char *Currency2MetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "Currency";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t currency2Id() SBE_NOEXCEPT
    {
        return 30376;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t currency2SinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool currency2InActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t currency2EncodingOffset() SBE_NOEXCEPT
    {
        return 57;
    }

    static SBE_CONSTEXPR char currency2NullValue() SBE_NOEXCEPT
    {
        return static_cast<char>(0);
    }

    static SBE_CONSTEXPR char currency2MinValue() SBE_NOEXCEPT
    {
        return static_cast<char>(32);
    }

    static SBE_CONSTEXPR char currency2MaxValue() SBE_NOEXCEPT
    {
        return static_cast<char>(126);
    }

    static SBE_CONSTEXPR std::size_t currency2EncodingLength() SBE_NOEXCEPT
    {
        return 3;
    }

    static SBE_CONSTEXPR std::uint64_t currency2Length() SBE_NOEXCEPT
    {
        return 3;
    }

    SBE_NODISCARD const char *currency2() const SBE_NOEXCEPT
    {
        return m_buffer + m_offset + 57;
    }

    SBE_NODISCARD char *currency2() SBE_NOEXCEPT
    {
        return m_buffer + m_offset + 57;
    }

    SBE_NODISCARD char currency2(const std::uint64_t index) const
    {
        if (index >= 3)
        {
            throw std::runtime_error("index out of range for currency2 [E104]");
        }

        char val;
        std::memcpy(&val, m_buffer + m_offset + 57 + (index * 1), sizeof(char));
        return (val);
    }

    SecurityDefinition &currency2(const std::uint64_t index, const char value)
    {
        if (index >= 3)
        {
            throw std::runtime_error("index out of range for currency2 [E105]");
        }

        char val = (value);
        std::memcpy(m_buffer + m_offset + 57 + (index * 1), &val, sizeof(char));
        return *this;
    }

    std::uint64_t getCurrency2(char *const dst, const std::uint64_t length) const
    {
        if (length > 3)
        {
            throw std::runtime_error("length too large for getCurrency2 [E106]");
        }

        std::memcpy(dst, m_buffer + m_offset + 57, sizeof(char) * static_cast<std::size_t>(length));
        return length;
    }

    SecurityDefinition &putCurrency2(const char *const src) SBE_NOEXCEPT
    {
        std::memcpy(m_buffer + m_offset + 57, src, sizeof(char) * 3);
        return *this;
    }

    SecurityDefinition &putCurrency2(
        const char value0,
        const char value1,
        const char value2) SBE_NOEXCEPT
    {
        char val0 = (value0);
        std::memcpy(m_buffer + m_offset + 57, &val0, sizeof(char));
        char val1 = (value1);
        std::memcpy(m_buffer + m_offset + 58, &val1, sizeof(char));
        char val2 = (value2);
        std::memcpy(m_buffer + m_offset + 59, &val2, sizeof(char));

        return *this;
    }

    SBE_NODISCARD std::string getCurrency2AsString() const
    {
        const char *buffer = m_buffer + m_offset + 57;
        std::size_t length = 0;

        for (; length < 3 && *(buffer + length) != '\0'; ++length);
        std::string result(buffer, length);

        return result;
    }

    std::string getCurrency2AsJsonEscapedString()
    {
        std::ostringstream oss;
        std::string s = getCurrency2AsString();

        for (const auto c : s)
        {
            switch (c)
            {
                case '"': oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;

                default:
                    if ('\x00' <= c && c <= '\x1f')
                    {
                        oss << "\\u" << std::hex << std::setw(4)
                            << std::setfill('0') << (int)(c);
                    }
                    else
                    {
                        oss << c;
                    }
            }
        }

        return oss.str();
    }

    #if __cplusplus >= 201703L
    SBE_NODISCARD std::string_view getCurrency2AsStringView() const SBE_NOEXCEPT
    {
        const char *buffer = m_buffer + m_offset + 57;
        std::size_t length = 0;

        for (; length < 3 && *(buffer + length) != '\0'; ++length);
        std::string_view result(buffer, length);

        return result;
    }
    #endif

    #if __cplusplus >= 201703L
    SecurityDefinition &putCurrency2(const std::string_view str)
    {
        const std::size_t srcLength = str.length();
        if (srcLength > 3)
        {
            throw std::runtime_error("string too large for putCurrency2 [E106]");
        }

        std::memcpy(m_buffer + m_offset + 57, str.data(), srcLength);
        for (std::size_t start = srcLength; start < 3; ++start)
        {
            m_buffer[m_offset + 57 + start] = 0;
        }

        return *this;
    }
    #else
    SecurityDefinition &putCurrency2(const std::string &str)
    {
        const std::size_t srcLength = str.length();
        if (srcLength > 3)
        {
            throw std::runtime_error("string too large for putCurrency2 [E106]");
        }

        std::memcpy(m_buffer + m_offset + 57, str.c_str(), srcLength);
        for (std::size_t start = srcLength; start < 3; ++start)
        {
            m_buffer[m_offset + 57 + start] = 0;
        }

        return *this;
    }
    #endif

    SBE_NODISCARD static const char *BasisPointMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "UInt8";
            case MetaAttribute::PRESENCE: return "optional";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t basisPointId() SBE_NOEXCEPT
    {
        return 30378;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t basisPointSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool basisPointInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t basisPointEncodingOffset() SBE_NOEXCEPT
    {
        return 60;
    }

    static SBE_CONSTEXPR std::uint8_t basisPointNullValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(255);
    }

    static SBE_CONSTEXPR std::uint8_t basisPointMinValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(0);
    }

    static SBE_CONSTEXPR std::uint8_t basisPointMaxValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(254);
    }

    static SBE_CONSTEXPR std::size_t basisPointEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint8_t basisPoint() const SBE_NOEXCEPT
    {
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 60, sizeof(std::uint8_t));
        return (val);
    }

    SecurityDefinition &basisPoint(const std::uint8_t value) SBE_NOEXCEPT
    {
        std::uint8_t val = (value);
        std::memcpy(m_buffer + m_offset + 60, &val, sizeof(std::uint8_t));
        return *this;
    }

    SBE_NODISCARD static const char *RatePrecisionMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "UInt8";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t ratePrecisionId() SBE_NOEXCEPT
    {
        return 30379;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t ratePrecisionSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool ratePrecisionInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t ratePrecisionEncodingOffset() SBE_NOEXCEPT
    {
        return 61;
    }

    static SBE_CONSTEXPR std::uint8_t ratePrecisionNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_UINT8;
    }

    static SBE_CONSTEXPR std::uint8_t ratePrecisionMinValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(0);
    }

    static SBE_CONSTEXPR std::uint8_t ratePrecisionMaxValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(254);
    }

    static SBE_CONSTEXPR std::size_t ratePrecisionEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint8_t ratePrecision() const SBE_NOEXCEPT
    {
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 61, sizeof(std::uint8_t));
        return (val);
    }

    SecurityDefinition &ratePrecision(const std::uint8_t value) SBE_NOEXCEPT
    {
        std::uint8_t val = (value);
        std::memcpy(m_buffer + m_offset + 61, &val, sizeof(std::uint8_t));
        return *this;
    }

    SBE_NODISCARD static const char *RateTermMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "UInt8";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t rateTermId() SBE_NOEXCEPT
    {
        return 30380;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t rateTermSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool rateTermInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t rateTermEncodingOffset() SBE_NOEXCEPT
    {
        return 62;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t rateTermEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint8_t rateTermRaw() const SBE_NOEXCEPT
    {
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 62, sizeof(std::uint8_t));
        return (val);
    }

    SBE_NODISCARD RateTerm::Value rateTerm() const
    {
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 62, sizeof(std::uint8_t));
        return RateTerm::get((val));
    }

    SecurityDefinition &rateTerm(const RateTerm::Value value) SBE_NOEXCEPT
    {
        std::uint8_t val = (value);
        std::memcpy(m_buffer + m_offset + 62, &val, sizeof(std::uint8_t));
        return *this;
    }

    SBE_NODISCARD static const char *Currency1AmtDecimalsMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "UInt8";
            case MetaAttribute::PRESENCE: return "optional";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t currency1AmtDecimalsId() SBE_NOEXCEPT
    {
        return 30381;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t currency1AmtDecimalsSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool currency1AmtDecimalsInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t currency1AmtDecimalsEncodingOffset() SBE_NOEXCEPT
    {
        return 63;
    }

    static SBE_CONSTEXPR std::uint8_t currency1AmtDecimalsNullValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(255);
    }

    static SBE_CONSTEXPR std::uint8_t currency1AmtDecimalsMinValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(0);
    }

    static SBE_CONSTEXPR std::uint8_t currency1AmtDecimalsMaxValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(254);
    }

    static SBE_CONSTEXPR std::size_t currency1AmtDecimalsEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint8_t currency1AmtDecimals() const SBE_NOEXCEPT
    {
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 63, sizeof(std::uint8_t));
        return (val);
    }

    SecurityDefinition &currency1AmtDecimals(const std::uint8_t value) SBE_NOEXCEPT
    {
        std::uint8_t val = (value);
        std::memcpy(m_buffer + m_offset + 63, &val, sizeof(std::uint8_t));
        return *this;
    }

    SBE_NODISCARD static const char *Currency2AmtDecimalsMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "UInt8";
            case MetaAttribute::PRESENCE: return "optional";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t currency2AmtDecimalsId() SBE_NOEXCEPT
    {
        return 30382;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t currency2AmtDecimalsSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool currency2AmtDecimalsInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t currency2AmtDecimalsEncodingOffset() SBE_NOEXCEPT
    {
        return 64;
    }

    static SBE_CONSTEXPR std::uint8_t currency2AmtDecimalsNullValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(255);
    }

    static SBE_CONSTEXPR std::uint8_t currency2AmtDecimalsMinValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(0);
    }

    static SBE_CONSTEXPR std::uint8_t currency2AmtDecimalsMaxValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(254);
    }

    static SBE_CONSTEXPR std::size_t currency2AmtDecimalsEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint8_t currency2AmtDecimals() const SBE_NOEXCEPT
    {
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 64, sizeof(std::uint8_t));
        return (val);
    }

    SecurityDefinition &currency2AmtDecimals(const std::uint8_t value) SBE_NOEXCEPT
    {
        std::uint8_t val = (value);
        std::memcpy(m_buffer + m_offset + 64, &val, sizeof(std::uint8_t));
        return *this;
    }

    SBE_NODISCARD static const char *RGTSMDPSMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "UInt8";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t rGTSMDPSId() SBE_NOEXCEPT
    {
        return 30433;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t rGTSMDPSSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool rGTSMDPSInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t rGTSMDPSEncodingOffset() SBE_NOEXCEPT
    {
        return 65;
    }

    static SBE_CONSTEXPR std::uint8_t rGTSMDPSNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_UINT8;
    }

    static SBE_CONSTEXPR std::uint8_t rGTSMDPSMinValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(0);
    }

    static SBE_CONSTEXPR std::uint8_t rGTSMDPSMaxValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(254);
    }

    static SBE_CONSTEXPR std::size_t rGTSMDPSEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint8_t rGTSMDPS() const SBE_NOEXCEPT
    {
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 65, sizeof(std::uint8_t));
        return (val);
    }

    SecurityDefinition &rGTSMDPS(const std::uint8_t value) SBE_NOEXCEPT
    {
        std::uint8_t val = (value);
        std::memcpy(m_buffer + m_offset + 65, &val, sizeof(std::uint8_t));
        return *this;
    }

    SBE_NODISCARD static const char *LEFT_DPSMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "UInt8";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t lEFT_DPSId() SBE_NOEXCEPT
    {
        return 30434;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t lEFT_DPSSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool lEFT_DPSInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t lEFT_DPSEncodingOffset() SBE_NOEXCEPT
    {
        return 66;
    }

    static SBE_CONSTEXPR std::uint8_t lEFT_DPSNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_UINT8;
    }

    static SBE_CONSTEXPR std::uint8_t lEFT_DPSMinValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(0);
    }

    static SBE_CONSTEXPR std::uint8_t lEFT_DPSMaxValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(254);
    }

    static SBE_CONSTEXPR std::size_t lEFT_DPSEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint8_t lEFT_DPS() const SBE_NOEXCEPT
    {
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 66, sizeof(std::uint8_t));
        return (val);
    }

    SecurityDefinition &lEFT_DPS(const std::uint8_t value) SBE_NOEXCEPT
    {
        std::uint8_t val = (value);
        std::memcpy(m_buffer + m_offset + 66, &val, sizeof(std::uint8_t));
        return *this;
    }

    SBE_NODISCARD static const char *RIGHT_DPSMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "UInt8";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t rIGHT_DPSId() SBE_NOEXCEPT
    {
        return 30435;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t rIGHT_DPSSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool rIGHT_DPSInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t rIGHT_DPSEncodingOffset() SBE_NOEXCEPT
    {
        return 67;
    }

    static SBE_CONSTEXPR std::uint8_t rIGHT_DPSNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_UINT8;
    }

    static SBE_CONSTEXPR std::uint8_t rIGHT_DPSMinValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(0);
    }

    static SBE_CONSTEXPR std::uint8_t rIGHT_DPSMaxValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(254);
    }

    static SBE_CONSTEXPR std::size_t rIGHT_DPSEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint8_t rIGHT_DPS() const SBE_NOEXCEPT
    {
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 67, sizeof(std::uint8_t));
        return (val);
    }

    SecurityDefinition &rIGHT_DPS(const std::uint8_t value) SBE_NOEXCEPT
    {
        std::uint8_t val = (value);
        std::memcpy(m_buffer + m_offset + 67, &val, sizeof(std::uint8_t));
        return *this;
    }

    SBE_NODISCARD static const char *CLSMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "UInt8";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t cLSId() SBE_NOEXCEPT
    {
        return 30436;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t cLSSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool cLSInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t cLSEncodingOffset() SBE_NOEXCEPT
    {
        return 68;
    }

    static SBE_CONSTEXPR std::uint8_t cLSNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_UINT8;
    }

    static SBE_CONSTEXPR std::uint8_t cLSMinValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(0);
    }

    static SBE_CONSTEXPR std::uint8_t cLSMaxValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(254);
    }

    static SBE_CONSTEXPR std::size_t cLSEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint8_t cLS() const SBE_NOEXCEPT
    {
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 68, sizeof(std::uint8_t));
        return (val);
    }

    SecurityDefinition &cLS(const std::uint8_t value) SBE_NOEXCEPT
    {
        std::uint8_t val = (value);
        std::memcpy(m_buffer + m_offset + 68, &val, sizeof(std::uint8_t));
        return *this;
    }

    SBE_NODISCARD static const char *MaxPriceVariationMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "UInt64";
            case MetaAttribute::PRESENCE: return "optional";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t maxPriceVariationId() SBE_NOEXCEPT
    {
        return 1143;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t maxPriceVariationSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool maxPriceVariationInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t maxPriceVariationEncodingOffset() SBE_NOEXCEPT
    {
        return 69;
    }

    static SBE_CONSTEXPR std::uint64_t maxPriceVariationNullValue() SBE_NOEXCEPT
    {
        return UINT64_C(0xffffffffffffffff);
    }

    static SBE_CONSTEXPR std::uint64_t maxPriceVariationMinValue() SBE_NOEXCEPT
    {
        return UINT64_C(0x0);
    }

    static SBE_CONSTEXPR std::uint64_t maxPriceVariationMaxValue() SBE_NOEXCEPT
    {
        return UINT64_C(0xfffffffffffffffe);
    }

    static SBE_CONSTEXPR std::size_t maxPriceVariationEncodingLength() SBE_NOEXCEPT
    {
        return 8;
    }

    SBE_NODISCARD std::uint64_t maxPriceVariation() const SBE_NOEXCEPT
    {
        std::uint64_t val;
        std::memcpy(&val, m_buffer + m_offset + 69, sizeof(std::uint64_t));
        return SBE_LITTLE_ENDIAN_ENCODE_64(val);
    }

    SecurityDefinition &maxPriceVariation(const std::uint64_t value) SBE_NOEXCEPT
    {
        std::uint64_t val = SBE_LITTLE_ENDIAN_ENCODE_64(value);
        std::memcpy(m_buffer + m_offset + 69, &val, sizeof(std::uint64_t));
        return *this;
    }

    SBE_NODISCARD static const char *SnapshotConflationIntervalMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "Time";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t snapshotConflationIntervalId() SBE_NOEXCEPT
    {
        return 30437;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t snapshotConflationIntervalSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool snapshotConflationIntervalInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t snapshotConflationIntervalEncodingOffset() SBE_NOEXCEPT
    {
        return 77;
    }

private:
    TimeOfDay m_snapshotConflationInterval;

public:
    SBE_NODISCARD TimeOfDay &snapshotConflationInterval()
    {
        m_snapshotConflationInterval.wrap(m_buffer, m_offset + 77, m_actingVersion, m_bufferLength);
        return m_snapshotConflationInterval;
    }

    SBE_NODISCARD static const char *IncRefreshConflationIntervalMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "Time";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t incRefreshConflationIntervalId() SBE_NOEXCEPT
    {
        return 30438;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t incRefreshConflationIntervalSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool incRefreshConflationIntervalInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t incRefreshConflationIntervalEncodingOffset() SBE_NOEXCEPT
    {
        return 82;
    }

private:
    TimeOfDay m_incRefreshConflationInterval;

public:
    SBE_NODISCARD TimeOfDay &incRefreshConflationInterval()
    {
        m_incRefreshConflationInterval.wrap(m_buffer, m_offset + 82, m_actingVersion, m_bufferLength);
        return m_incRefreshConflationInterval;
    }

    SBE_NODISCARD static const char *TradesFeedConflationIntervalMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "Time";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t tradesFeedConflationIntervalId() SBE_NOEXCEPT
    {
        return 30430;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t tradesFeedConflationIntervalSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool tradesFeedConflationIntervalInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t tradesFeedConflationIntervalEncodingOffset() SBE_NOEXCEPT
    {
        return 87;
    }

private:
    TimeOfDay m_tradesFeedConflationInterval;

public:
    SBE_NODISCARD TimeOfDay &tradesFeedConflationInterval()
    {
        m_tradesFeedConflationInterval.wrap(m_buffer, m_offset + 87, m_actingVersion, m_bufferLength);
        return m_tradesFeedConflationInterval;
    }

    SBE_NODISCARD static const char *SecurityDefinitionConflationIntervalMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "Time";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t securityDefinitionConflationIntervalId() SBE_NOEXCEPT
    {
        return 30431;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t securityDefinitionConflationIntervalSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool securityDefinitionConflationIntervalInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t securityDefinitionConflationIntervalEncodingOffset() SBE_NOEXCEPT
    {
        return 92;
    }

private:
    TimeOfDay m_securityDefinitionConflationInterval;

public:
    SBE_NODISCARD TimeOfDay &securityDefinitionConflationInterval()
    {
        m_securityDefinitionConflationInterval.wrap(m_buffer, m_offset + 92, m_actingVersion, m_bufferLength);
        return m_securityDefinitionConflationInterval;
    }

    SBE_NODISCARD static const char *DepthOfBookMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "UInt8";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t depthOfBookId() SBE_NOEXCEPT
    {
        return 30439;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t depthOfBookSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool depthOfBookInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t depthOfBookEncodingOffset() SBE_NOEXCEPT
    {
        return 97;
    }

    static SBE_CONSTEXPR std::uint8_t depthOfBookNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_UINT8;
    }

    static SBE_CONSTEXPR std::uint8_t depthOfBookMinValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(0);
    }

    static SBE_CONSTEXPR std::uint8_t depthOfBookMaxValue() SBE_NOEXCEPT
    {
        return static_cast<std::uint8_t>(254);
    }

    static SBE_CONSTEXPR std::size_t depthOfBookEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint8_t depthOfBook() const SBE_NOEXCEPT
    {
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 97, sizeof(std::uint8_t));
        return (val);
    }

    SecurityDefinition &depthOfBook(const std::uint8_t value) SBE_NOEXCEPT
    {
        std::uint8_t val = (value);
        std::memcpy(m_buffer + m_offset + 97, &val, sizeof(std::uint8_t));
        return *this;
    }

    SBE_NODISCARD static const char *MinTradeVolMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::SEMANTIC_TYPE: return "Int64";
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t minTradeVolId() SBE_NOEXCEPT
    {
        return 562;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t minTradeVolSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool minTradeVolInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t minTradeVolEncodingOffset() SBE_NOEXCEPT
    {
        return 98;
    }

    static SBE_CONSTEXPR std::int64_t minTradeVolNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_INT64;
    }

    static SBE_CONSTEXPR std::int64_t minTradeVolMinValue() SBE_NOEXCEPT
    {
        return INT64_C(-9223372036854775807);
    }

    static SBE_CONSTEXPR std::int64_t minTradeVolMaxValue() SBE_NOEXCEPT
    {
        return INT64_C(9223372036854775807);
    }

    static SBE_CONSTEXPR std::size_t minTradeVolEncodingLength() SBE_NOEXCEPT
    {
        return 8;
    }

    SBE_NODISCARD std::int64_t minTradeVol() const SBE_NOEXCEPT
    {
        std::int64_t val;
        std::memcpy(&val, m_buffer + m_offset + 98, sizeof(std::int64_t));
        return SBE_LITTLE_ENDIAN_ENCODE_64(val);
    }

    SecurityDefinition &minTradeVol(const std::int64_t value) SBE_NOEXCEPT
    {
        std::int64_t val = SBE_LITTLE_ENDIAN_ENCODE_64(value);
        std::memcpy(m_buffer + m_offset + 98, &val, sizeof(std::int64_t));
        return *this;
    }

template<typename CharT, typename Traits>
friend std::basic_ostream<CharT, Traits> & operator << (
    std::basic_ostream<CharT, Traits> &builder, const SecurityDefinition &_writer)
{
    SecurityDefinition writer(
        _writer.m_buffer,
        _writer.m_offset,
        _writer.m_bufferLength,
        _writer.m_actingBlockLength,
        _writer.m_actingVersion);

    builder << '{';
    builder << R"("Name": "SecurityDefinition", )";
    builder << R"("sbeTemplateId": )";
    builder << writer.sbeTemplateId();
    builder << ", ";

    builder << R"("SecurityUpdateAction": )";
    builder << '"' << writer.securityUpdateAction() << '"';

    builder << ", ";
    builder << R"("LastUpdateTime": )";
    builder << +writer.lastUpdateTime();

    builder << ", ";
    builder << R"("MDEntryOriginator": )";
    builder << '"' <<
        writer.getMDEntryOriginatorAsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("Symbol": )";
    builder << '"' <<
        writer.getSymbolAsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("SecurityID": )";
    builder << +writer.securityID();

    builder << ", ";
    builder << R"("SecurityIDSource": )";
    builder << +writer.securityIDSource();

    builder << ", ";
    builder << R"("SecurityType": )";
    builder << '"' << writer.securityType() << '"';

    builder << ", ";
    builder << R"("SettlDate": )";
    builder << writer.settlDate();

    builder << ", ";
    builder << R"("Currency1": )";
    builder << '"' <<
        writer.getCurrency1AsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("Currency2": )";
    builder << '"' <<
        writer.getCurrency2AsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("BasisPoint": )";
    builder << +writer.basisPoint();

    builder << ", ";
    builder << R"("RatePrecision": )";
    builder << +writer.ratePrecision();

    builder << ", ";
    builder << R"("RateTerm": )";
    builder << '"' << writer.rateTerm() << '"';

    builder << ", ";
    builder << R"("Currency1AmtDecimals": )";
    builder << +writer.currency1AmtDecimals();

    builder << ", ";
    builder << R"("Currency2AmtDecimals": )";
    builder << +writer.currency2AmtDecimals();

    builder << ", ";
    builder << R"("RGTSMDPS": )";
    builder << +writer.rGTSMDPS();

    builder << ", ";
    builder << R"("LEFT_DPS": )";
    builder << +writer.lEFT_DPS();

    builder << ", ";
    builder << R"("RIGHT_DPS": )";
    builder << +writer.rIGHT_DPS();

    builder << ", ";
    builder << R"("CLS": )";
    builder << +writer.cLS();

    builder << ", ";
    builder << R"("MaxPriceVariation": )";
    builder << +writer.maxPriceVariation();

    builder << ", ";
    builder << R"("SnapshotConflationInterval": )";
    builder << writer.snapshotConflationInterval();

    builder << ", ";
    builder << R"("IncRefreshConflationInterval": )";
    builder << writer.incRefreshConflationInterval();

    builder << ", ";
    builder << R"("TradesFeedConflationInterval": )";
    builder << writer.tradesFeedConflationInterval();

    builder << ", ";
    builder << R"("SecurityDefinitionConflationInterval": )";
    builder << writer.securityDefinitionConflationInterval();

    builder << ", ";
    builder << R"("DepthOfBook": )";
    builder << +writer.depthOfBook();

    builder << ", ";
    builder << R"("MinTradeVol": )";
    builder << +writer.minTradeVol();

    builder << '}';

    return builder;
}

void skip()
{
}

SBE_NODISCARD static SBE_CONSTEXPR bool isConstLength() SBE_NOEXCEPT
{
    return true;
}

SBE_NODISCARD static std::size_t computeLength()
{
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
    std::size_t length = sbeBlockLength();

    return length;
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
}
};
}
#endif
