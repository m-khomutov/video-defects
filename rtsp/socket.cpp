/* 
 * File:   socket.cpp
 * Author: mkh
 * 
 * Created on 25 января 2023 г., 9:24
 */

#include "socket.h"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>

rtsp::SocketError::SocketError( const std::string &what )
: std::runtime_error( what + std::string(" failed: ") + std::string(strerror( errno )) )
{}


rtsp::Socket::Socket( uint16_t b_port )
{
    sockaddr_in b_addr;
    memset( &b_addr, 0, sizeof(struct sockaddr_in) );
    b_addr.sin_family = AF_INET;
    b_addr.sin_addr.s_addr = INADDR_ANY;
    b_addr.sin_port = htons(b_port);
    if( (m_fd = socket( b_addr.sin_family, SOCK_STREAM, 0 )) == -1)
    {
        throw SocketError( "socket" );
    }
    
    long yes { 0 };
    setsockopt( m_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int) );

    if( bind( m_fd, (struct sockaddr *)&b_addr, sizeof(b_addr)) == -1 )
    {
        close( m_fd );
        throw SocketError( "bind" );
    }
    fcntl( m_fd, F_SETFL, fcntl(m_fd, F_GETFL, 0) | O_NONBLOCK );
    if( listen( m_fd, SOMAXCONN ) == -1)
    {
        close( m_fd );
        throw SocketError( "bind" );
    }
    std::cerr << "[*] listening on port " << ntohs(b_addr.sin_port) << "\n";
}

rtsp::Socket::~Socket()
{
    close( m_fd );
    std::cerr << "[*] stop listening\n";
}
