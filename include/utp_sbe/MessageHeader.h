#ifndef _UTP_SBE_MESSAGEHEADER_H_
#define _UTP_SBE_MESSAGEHEADER_H_

//#define SBE_BOUNDS_CHECK_EXPECT_TRUE(exp) true

#include "sbe/sbe.h"

using namespace sbe;

namespace utp_sbe {

class MessageHeader
{
private:
    char *m_buffer = nullptr;
    std::uint64_t m_bufferLength = 0;
    std::uint64_t m_offset = 0;
    std::uint64_t m_position = 0;
    std::uint64_t m_actingBlockLength = 0;
    std::uint64_t m_actingVersion = 0;

public:
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

    MessageHeader() = default;

    MessageHeader(
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

    MessageHeader(char *buffer, const std::uint64_t bufferLength) :
        MessageHeader(buffer, 0, bufferLength, sbeBlockLength(), sbeSchemaVersion())
    {
    }

    MessageHeader(
        char *buffer,
        const std::uint64_t bufferLength,
        const std::uint64_t actingBlockLength,
        const std::uint64_t actingVersion) :
        MessageHeader(buffer, 0, bufferLength, actingBlockLength, actingVersion)
    {
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeBlockLength() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(8);
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeTemplateId() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(1);
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeSchemaId() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(101);
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeSchemaVersion() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(1);
    }

    SBE_NODISCARD static SBE_CONSTEXPR const char *sbeSemanticType() SBE_NOEXCEPT
    {
        return "";
    }

    SBE_NODISCARD std::uint64_t offset() const SBE_NOEXCEPT
    {
        return m_offset;
    }

    MessageHeader &wrapForEncode(char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)
    {
        return *this = MessageHeader(buffer, offset, bufferLength, sbeBlockLength(), sbeSchemaVersion());
    }

    MessageHeader &wrapForDecode(
        char *buffer,
        const std::uint64_t offset,
        const std::uint64_t actingBlockLength,
        const std::uint64_t actingVersion,
        const std::uint64_t bufferLength)
    {
        return *this = MessageHeader(buffer, offset, bufferLength, actingBlockLength, actingVersion);
    }

    SBE_NODISCARD std::uint64_t encodedLength() const SBE_NOEXCEPT
    {
        return sbePosition() - m_offset;
    }

    SBE_NODISCARD std::uint64_t decodeLength() const
    {
        MessageHeader skipper(m_buffer, m_offset, m_bufferLength, sbeBlockLength(), sbeSchemaVersion());
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

    // Field accessors
    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t blockLengthId() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD std::uint16_t blockLength() const SBE_NOEXCEPT
    {
        std::uint16_t val;
        std::memcpy(&val, m_buffer + m_offset + 0, sizeof(std::uint16_t));
        return SBE_LITTLE_ENDIAN_ENCODE_16(val);
    }

    MessageHeader &blockLength(const std::uint16_t value) SBE_NOEXCEPT
    {
        std::uint16_t val = SBE_LITTLE_ENDIAN_ENCODE_16(value);
        std::memcpy(m_buffer + m_offset + 0, &val, sizeof(std::uint16_t));
        return *this;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t templateIdId() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint16_t templateId() const SBE_NOEXCEPT
    {
        std::uint16_t val;
        std::memcpy(&val, m_buffer + m_offset + 2, sizeof(std::uint16_t));
        return SBE_LITTLE_ENDIAN_ENCODE_16(val);
    }

    MessageHeader &templateId(const std::uint16_t value) SBE_NOEXCEPT
    {
        std::uint16_t val = SBE_LITTLE_ENDIAN_ENCODE_16(value);
        std::memcpy(m_buffer + m_offset + 2, &val, sizeof(std::uint16_t));
        return *this;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t schemaIdId() SBE_NOEXCEPT
    {
        return 2;
    }

    SBE_NODISCARD std::uint16_t schemaId() const SBE_NOEXCEPT
    {
        std::uint16_t val;
        std::memcpy(&val, m_buffer + m_offset + 4, sizeof(std::uint16_t));
        return SBE_LITTLE_ENDIAN_ENCODE_16(val);
    }

    MessageHeader &schemaId(const std::uint16_t value) SBE_NOEXCEPT
    {
        std::uint16_t val = SBE_LITTLE_ENDIAN_ENCODE_16(value);
        std::memcpy(m_buffer + m_offset + 4, &val, sizeof(std::uint16_t));
        return *this;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t versionId() SBE_NOEXCEPT
    {
        return 3;
    }

    SBE_NODISCARD std::uint16_t version() const SBE_NOEXCEPT
    {
        std::uint16_t val;
        std::memcpy(&val, m_buffer + m_offset + 6, sizeof(std::uint16_t));
        return SBE_LITTLE_ENDIAN_ENCODE_16(val);
    }

    MessageHeader &version(const std::uint16_t value) SBE_NOEXCEPT
    {
        std::uint16_t val = SBE_LITTLE_ENDIAN_ENCODE_16(value);
        std::memcpy(m_buffer + m_offset + 6, &val, sizeof(std::uint16_t));
        return *this;
    }

private:
    void onActingBlockLengthChange(const std::uint16_t) SBE_NOEXCEPT {}

    void onActingVersionChange(const std::uint16_t) SBE_NOEXCEPT {}

    SBE_NODISCARD std::uint64_t sbePosition() const SBE_NOEXCEPT
    {
        return m_position;
    }

    void sbePosition(const std::uint64_t position)
    {
        m_position = sbeCheckPosition(position);
    }

    std::uint64_t sbeCheckPosition(const std::uint64_t position)
    {
        if (SBE_BOUNDS_CHECK_EXPECT_TRUE(position <= m_bufferLength))
        {
            return position;
        }
        throw std::runtime_error("buffer too short [E100]");
    }

    void sbeRewind()
    {
        return sbePosition(m_offset + m_actingBlockLength);
    }

    MessageHeader &skip()
    {
        sbeRewind();
        return *this;
    }
};

} // namespace utp_sbe

#endif