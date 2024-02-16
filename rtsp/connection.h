/* 
 * File:   connection.h
 * Author: mkh
 *
 * Created on 2 марта 2023 г., 13:29
 */

#ifndef RTSP_CONNECTION_H
#define RTSP_CONNECTION_H

#include "rtp.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <memory>
#include <string>
#include <vector>
#include <cstring>
#include <x264.h>

namespace rtsp {

    class Connection {
    public:
        explicit Connection( int b_sock, const std::string &sdp );
        Connection(const Connection& orig) = delete;
        Connection &operator =(const Connection& orig) = delete;
        ~Connection();

        operator int() const {
            return m_fd;
        }

        void on_data( const uint8_t * data, int size );
        void on_ready_to_write();
        void send_frame( const uint8_t *data, size_t size, size_t full_size, int delay );

    private:
        enum { SENT_NOTHING = 0, SENT_SPS = 1, SENT_PPS = 2, SENT_IDR = 4 };

        socklen_t m_socklen { sizeof(sockaddr_in) };
        int m_fd;
        sockaddr_in m_address;
        std::string m_request;
        std::string m_reply;
        size_t m_rtsp_sent {0};
        size_t m_rtp_sent {0};

        std::string m_sdp;
        std::string m_session;

        bool m_playing {false};
        std::vector< uint8_t > m_frame;
        rtp::RTP m_rtp;
        uint8_t m_sent_flag = SENT_NOTHING;

    private:
        void f_reply( const char *reply_line );
        void f_reply_describe();
        void f_reply_setup();
        void f_reply_play();
        bool f_can_be_sent( uint8_t nutype );

        std::string f_ctime();
};

}  // namespace rtsp

#endif /* RTSP_CONNECTION_H */