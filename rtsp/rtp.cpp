//
// Created by mkh on 12.02.2024.
//

#include "rtp.h"

size_t rtp::RTP::expected_size( size_t size )
{
    size_t rc = 0;
    if( size < FU_SIZE )
    {
        rc = Interleaved::SIZE + Header::SIZE + size;
    }
    else
    {
        --size; // первый пакет - naluheader перезаписывается fu-indicator-ом

        size_t off = 0;
        while( off < size )
        {
            size_t sz = size >= off + FU_SIZE ? FU_SIZE : size - off;

            rc += Interleaved::SIZE + Header::SIZE + FU::SIZE + sz;
            off += sz;
        }
    }

    return rc;
}

void rtp::RTP::serialize( const uint8_t *data, size_t size, uint8_t *frame, int delay )
{
    if( size < FU_SIZE )
    {
        m_interleaved.serialize( frame, Header::SIZE + size );
        m_header.serialize( frame + Interleaved::SIZE, delay, true );
        memcpy( frame + Interleaved::SIZE + Header::SIZE, data, size );
    }
    else
    {
        FU fu( *data );
        // первый пакет - naluheader перезаписывается fu-indicator-ом
        ++data;
        --size;

        size_t i = 0;
        size_t off = 0;
        while( off < size )
        {
            size_t sz = size >= off + FU_SIZE ? FU_SIZE : size - off;

            m_interleaved.serialize( frame, Header::SIZE + FU::SIZE + sz );
            frame += Interleaved::SIZE;
            m_header.serialize( frame, delay, off + sz == size );
            frame += Header::SIZE;
            fu.serialize( frame, off + sz == size );
            frame += FU::SIZE;
            memcpy( frame, data, sz );
            off += sz;
            data += sz;
            frame += sz;
        }
    }
}