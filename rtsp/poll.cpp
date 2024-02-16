/* 
 * File:   s_poll.cpp
 * Author: mkh
 * 
 * Created on 25 января 2023 г., 10:06
 */

#include "poll.h"
#include "connection.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <ifaddrs.h>

namespace {

    const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                     "abcdefghijklmnopqrstuvwxyz"
                                     "0123456789+/";

    std::string base64_encode( unsigned char const* bytes_to_encode, unsigned int in_len )
    {
        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        while (in_len--)
        {
            char_array_3[i++] = *(bytes_to_encode++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for(i = 0; (i <4) ; i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i)
        {
            for(j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; (j < i + 1); j++)
                ret += base64_chars[char_array_4[j]];

            while((i++ < 3))
                ret += '=';
        }

        return ret;
    }

    std::string IP()
    {
        std::string rc;
        ifaddrs * if_addr = nullptr;
        if( getifaddrs( &if_addr ) == 0 && if_addr != nullptr )
        {
            ifaddrs * ifa = nullptr;
            for( ifa = if_addr; ifa; ifa = ifa->ifa_next )
            {
                if( !ifa->ifa_addr )
                {
                    continue;
                }
                if( ifa->ifa_addr->sa_family == AF_INET ) {
                    in_addr_t s_addr = ((sockaddr_in*)ifa->ifa_addr)->sin_addr.s_addr;
                    if( s_addr == 0x0100007f )
                    {
                        continue;
                    }
                    void *ptr = &((sockaddr_in*)ifa->ifa_addr)->sin_addr;
                    char buf[ INET_ADDRSTRLEN ];
                    inet_ntop( AF_INET, ptr, buf, INET_ADDRSTRLEN );
                    rc.assign( buf );
                    break;
                }
            }
            freeifaddrs( if_addr );
        }
        return std::move( rc );
    }

}  // namespace


rtsp::PollError::PollError( const std::string &what )
: std::runtime_error( what + std::string(" failed: ") + std::string(strerror( errno )) )
{}


rtsp::Poll::Poll( uint16_t port, int fps )
: m_socket( port )
, m_fd( epoll_create( 1 ) )
, m_fps( fps )
, m_host( IP() )
{
    try
    {
        f_add( m_socket, EPOLLIN | EPOLLOUT | EPOLLET );
    }
    catch( const std::runtime_error & err )
    {
        close( m_fd );
        throw;
    }
}

rtsp::Poll::~Poll()
{
    close( m_fd );
}

void rtsp::Poll::run()
{
    epoll_event events[maxevents];
    uint8_t buffer[0xffff];
    
    while( m_running.load() )
    {
        int fd_count = epoll_wait( m_fd, events, maxevents, 10 );
        for( int i(0); i < fd_count; ++i )
        {
            if( events[i].data.fd == m_socket )
            {
                std::shared_ptr< Connection > conn( new Connection( events[i].data.fd, m_sdp ) );
                try
                {
                    f_add( *conn, EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP | EPOLLHUP );
                }
                catch( const std::runtime_error &err )
                {
                    std::cerr << "error: " <<err.what() << std::endl;
                    continue;
                }
                m_connections[*conn] = conn;
            }
            else if( events[i].events & EPOLLIN )
            {
                int rc = ::read( events[i].data.fd, buffer, sizeof(buffer) );
                if( rc > 0 )
                {
                    auto p = m_connections.find( events[i].data.fd );
                    if( p != m_connections.end() )
                    {
                        p->second->on_data( buffer, rc );
                    }
                }
            }
            else if( events[i].events & EPOLLOUT )
            {
                auto p = m_connections.find( events[i].data.fd );
                if( p != m_connections.end() )
                {
                    p->second->on_ready_to_write();
                }
            }
            else
            {
                std::cerr << "[?] unexpected event\n";
            }
            /* check if the connection is closing */
            if( events[i].events & (EPOLLRDHUP | EPOLLHUP) )
            {
                epoll_ctl( m_fd, EPOLL_CTL_DEL, events[i].data.fd, nullptr );
                m_connections.erase( events[i].data.fd );
            }
        }
        f_send_frame();
    }
}

void rtsp::Poll::stop()
{
    m_running.store( false );
}

void rtsp::Poll::f_add( int sock, uint32_t events )
{
    epoll_event ev;
    ev.events = events;
    ev.data.fd = sock;
    if( epoll_ctl( m_fd, EPOLL_CTL_ADD, sock, &ev ) == -1 )
    {
        throw PollError( "epoll_ctl" );
    }
}

void rtsp::Poll::store( cv::Mat &frame, int delay )
{
    if( !m_encoder )
    {
        m_encoder.reset( new Encoder( frame.cols, frame.rows, m_fps ) );
    }
    auto g = m_frame.get();
    g->first = std::move( frame );
    g->second = delay;
}

void rtsp::Poll::f_send_frame()
{
    if( !m_encoder )
    {
        return;
    }

    cv::Mat fr;
    int delay;
    {
        auto g = m_frame.get();
        if( g->second >= 0 )
        {
            std::swap( fr, g->first );
            delay = g->second;
            g->second = -1;
        }
    }

    if( !fr.empty() )
    {
        Encoder::PS sps, pps;

        m_encoder->encode( fr, delay, &sps, &pps );
        if( !sps.empty() && !pps.empty() )
        {
            char buf[32];
            sprintf( buf, "%02x%02x%02x", sps[1], sps[2], sps[3] );

            m_sdp = std::string("v=0\r\n") +
                    std::string("o=- 0 0 IN IP4 ") + m_host + "\r\n" +
                    std::string("s=No Title\r\n") +
                    std::string("c=IN IP4 0.0.0.0\r\n") +
                    std::string("t=0 0\r\n") +
                    std::string("m=video 0 RTP/AVP 96\r\n") +
                    std::string("a=rtpmap:96 H264/90000\r\n") +
                    std::string("a=control:1\r\n") +
                    std::string("a=fmtp:96 packetization-mode=1;sprop-parameter-sets=") +
                    base64_encode( sps.data(), sps.size() ) + "," + base64_encode( pps.data(), pps.size() ) + ";" +
                    std::string("profile-level-id=") + buf;
        }

        size_t full_size = rtp::RTP::expected_size( m_encoder->nalusize() );

        for( auto p : m_connections )
        {
            if( !sps.empty() )
            {
                p.second->send_frame( sps.data(), sps.size(), rtp::RTP::expected_size( sps.size() ), 0 );
            }
            if( !pps.empty() )
            {
                p.second->send_frame( pps.data(), pps.size(), rtp::RTP::expected_size( pps.size() ), 0 );
            }
            p.second->send_frame( m_encoder->nalunit(), m_encoder->nalusize(), full_size, delay );
        }
    }
}
