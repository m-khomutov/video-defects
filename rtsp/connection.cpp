/* 
 * File:   connection.cpp
 * Author: mkh
 * 
 * Created on 2 марта 2023 г., 13:29
 */

#include "connection.h"
#include <unistd.h>
#include <fcntl.h>
#include <uuid/uuid.h>
#include <cstring>
#include <iostream>

namespace
{

    std::string saddr2str( const sockaddr_in & addr )
    {
        char buf[64];
        inet_ntop( AF_INET, (char *)&(addr.sin_addr), buf, sizeof(sockaddr_in) );
        return std::string( buf );
    }

    std::string uuid()
    {
        uint8_t uu[ 16 ];
        uuid_generate( uu );

        std::string rc( 36, 0 );
        uuid_unparse( uu, rc.data() );
        return rc;
    }

    const char *ok = "RTSP/1.0 200 OK\r\n";
    const char *options = "Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE\r\n\r\n";

}  // namespace

rtsp::Connection::Connection( int b_sock, const std::string &sdp )
: m_fd( accept( b_sock, (struct sockaddr *)&m_address, &m_socklen ) )
, m_sdp( sdp )
, m_session( uuid() )
{
    fcntl( m_fd, F_SETFD, fcntl( m_fd, F_GETFD, 0) | O_NONBLOCK );
    std::cerr << "[+] connected with " << saddr2str(m_address) << ":" << ntohs(m_address.sin_port) << "\n";
}

rtsp::Connection::~Connection()
{
    close( m_fd );
    std::cerr << "[-] connection " << saddr2str(m_address) << ":" << ntohs(m_address.sin_port) << " closed\n";
}

void rtsp::Connection::on_data( const uint8_t * data, int size )
{
    m_rtsp_sent = 0;
    m_request += std::string((const char*)data, size);

    if( m_request.find( "\r\n\r\n" ) != std::string::npos )
    {
        std::cerr << m_request << std::endl;

        if( m_request.find( "OPTIONS " ) != std::string::npos )
        {
            f_reply( options );
        }
        if( m_request.find( "DESCRIBE " ) != std::string::npos )
        {
            f_reply_describe();
        }
        if( m_request.find( "SETUP " ) != std::string::npos )
        {
            f_reply_setup();
        }
        if( m_request.find( "PLAY " ) != std::string::npos )
        {
            f_reply_play();
        }

        m_request.clear();
    }
}

void rtsp::Connection::on_ready_to_write()
{
    if( m_rtsp_sent < m_reply.size() )
    {
        int rc = ::write( m_fd, m_reply.data() + m_rtsp_sent, m_reply.size() - m_rtsp_sent );
        if( rc > 0 )
        {
            m_rtsp_sent += rc;
        }
    }

    if( m_rtp_sent < m_frame.size() )
    {
        int rc = ::write( m_fd, m_frame.data() + m_rtp_sent, m_frame.size() - m_rtp_sent );
        if( rc > 0 )
        {
            m_rtp_sent += rc;
        }
    }
}

void rtsp::Connection::send_frame( const uint8_t *data, size_t size, size_t full_size, int delay )
{
    if( m_playing )
    {
        if( !f_can_be_sent( ((*data) & 0x1f) ) )
        {
            return;
        }

        size_t left = m_frame.size() - m_rtp_sent;
        std::vector< uint8_t > fr( left + full_size );
        if( left )
        {
            memcpy( fr.data(), m_frame.data() + m_rtp_sent, left );
        }
        m_rtp.serialize( data, size, fr.data() + left, delay );

        std::swap( m_frame, fr );

        m_rtp_sent = 0;
        int rc = ::write( m_fd, m_frame.data(), m_frame.size() );
        if( rc > 0 )
        {
            m_rtp_sent = rc;
        }
    }
}

void rtsp::Connection::f_reply(const char *reply_line)
{
    size_t p1 = m_request.find( "CSeq:" );
    if( p1 != std::string::npos )
    {
        size_t p2 = m_request.find( "\r\n", p1 );
        if( p2 != std::string::npos )
        {
            m_reply = ok + m_request.substr( p1, p2 - p1 + 2 ) + reply_line;

            std::cerr << m_reply << std::endl;

            int rc = ::write( m_fd, m_reply.data(), m_reply.size() );
            if( rc > 0 )
            {
                m_rtsp_sent += rc;
            }
        }
    }
}

void rtsp::Connection::f_reply_describe()
{
    size_t p1 = m_request.find( " RTSP/1." );
    if( p1 != std::string::npos )
    {
        std::string reply = std::string("Date: ") + f_ctime() +
                            std::string("Content-Base: ") + m_request.substr( 9, p1 - 9 ) + "\r\n" +
                            std::string("Content-Type: application/sdp\r\n") +
                            std::string("Content-Length: ") + std::to_string( m_sdp.size() ) + "\r\n\r\n" +
                            m_sdp;
        f_reply( reply.c_str() );
        std::cerr << "\n";
    }
}

void rtsp::Connection::f_reply_setup()
{
    std::string reply = std::string("Session: ") + m_session + "\r\n" +
                        std::string( "Transport: RTP/AVP/TCP;interleaved=0-1\r\n\r\n" );
    f_reply( reply.c_str() );
}

void rtsp::Connection::f_reply_play()
{
    std::string reply = std::string("Date: ") + f_ctime() +
                        std::string("Session: ") + m_session + "\r\n\r\n";
    f_reply( reply.c_str() );

    m_rtsp_sent = 0;
    m_playing = true;
}

std::string rtsp::Connection::f_ctime()
{
    static const char *week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    static const char *month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    char buf[64];
    time_t t = time( nullptr );

    struct tm * tp = gmtime( &t );
    //Sun, 11 Feb 2024 08:54:05 GMT
    sprintf( buf, "%s, %02u %s %u %02u:%02u:%02u GMT\r\n",
             week[tp->tm_wday], tp->tm_mday, month[tp->tm_mon], tp->tm_yday + 1982, tp->tm_hour, tp->tm_min, tp->tm_sec );

    return buf;
}

bool rtsp::Connection::f_can_be_sent( uint8_t nutype )
{
    if( nutype == nal_unit_type_e::NAL_SPS )
    {
        m_sent_flag |= SENT_SPS;
        return true;
    }
    if( nutype == nal_unit_type_e::NAL_PPS )
    {
        m_sent_flag |= SENT_PPS;
        return true;
    }
    if( nutype == nal_unit_type_e::NAL_SLICE_IDR && (m_sent_flag & SENT_SPS) && (m_sent_flag & SENT_PPS) )
    {
        m_sent_flag |= SENT_IDR;
        return true;
    }
    return (m_sent_flag & SENT_IDR);
}