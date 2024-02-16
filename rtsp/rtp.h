//
// Created by mkh on 12.02.2024.
//

#ifndef VIDEOTESTS_RTP_H
#define VIDEOTESTS_RTP_H

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>

namespace rtp {

    class Interleaved {
    public:
        static const size_t SIZE = 4;

        Interleaved( uint8_t ch = 0 ): m_channel( ch )
        {}

        void serialize( uint8_t *data, uint16_t size )
        {
            *data ++ = 0x24;
            *data ++ = m_channel;
            uint16_t sz = htobe16( size );
            ::memcpy( data, &sz, sizeof(sz) );
        }

    private:
        uint8_t m_channel;
    };

    class FU {
    public:
        static const size_t SIZE = 2;

        FU( uint8_t octet )
        : m_indicator( (octet & 0xe0) | 0x1c )
        , m_type( 0x80 | (octet & 0x1f) )
        {}

        void serialize( uint8_t *data, bool end = false )
        {
            *data ++ = m_indicator;
            if( end )
            {
                m_type |= 0x40;
            }
            *data = m_type;

            m_type &= ~(0x80);
        }

    private:
        uint8_t m_indicator;
        uint8_t m_type;
    };

    class Header {
    public:
        static const size_t SIZE = 12;

        Header( uint8_t pt = 96 ) : m_pt( pt )
        {}

        void serialize( uint8_t *data, uint32_t delay, bool frame_boundary = false )
        {
            *data ++ = 0x80;
            if( frame_boundary )
            {
                *data ++ = (0x80 | m_pt);
            }
            else
            {
                *data ++ = m_pt;
            }
            if( m_seqnum == std::numeric_limits< uint16_t >::max() )
            {
                m_seqnum = 0;
            }
            uint16_t sn = htobe16( m_seqnum );
            ::memcpy( data, &sn, sizeof(sn) );
            data += sizeof(sn);
            m_seqnum ++;

            uint32_t ts = htobe32( m_timestamp );
            ::memcpy( data, &ts, sizeof(ts) );
            data += sizeof(ts);
            m_timestamp += delay;

            ::memcpy( data, &m_ssrc, sizeof(m_ssrc) );
        }
    private:
        uint8_t m_pt;
        uint16_t m_seqnum {0};
        uint32_t m_timestamp = rand();
        uint32_t m_ssrc = rand();
    };

    class RTP {
    public:
        static const size_t FU_SIZE = 1460;

        static size_t expected_size( size_t size );

        void serialize( const uint8_t *data, size_t size, uint8_t *frame, int delay );

    private:
        Interleaved m_interleaved;
        Header m_header;
    };
}  // namespace rtp


#endif //VIDEOTESTS_RTP_H
