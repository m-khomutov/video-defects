/* 
 * File:   socket.h
 * Author: mkh
 *
 * Created on 25 января 2023 г., 9:24
 */

#ifndef RTSP_SOCKET_H
#define RTSP_SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdexcept>

namespace rtsp {

    class SocketError: public std::runtime_error
    {
    public:
        SocketError( const std::string & what );
    };

    class Socket {
    public:
        explicit Socket( uint16_t b_port );
        Socket(const Socket& orig) = delete;
        Socket &operator =(const Socket& orig) = delete;
        ~Socket();

    operator int() const {
        return m_fd;
    }

private:
    int m_fd;

};


}  //namespace rtsp

#endif /* RTSP_SOCKET_H */
