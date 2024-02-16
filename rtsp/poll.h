/* 
 * File:   s_poll.h
 * Author: mkh
 *
 * Created on 25 января 2023 г., 10:06
 */

#ifndef RTSP_POLL_H
#define RTSP_POLL_H

#include "socket.h"
#include "../encoder.h"
#include <atomic>
#include <chrono>
#include <memory>
#include <map>
#include <mutex>
#include <vector>

namespace rtsp {

    template< typename T >
    class SafeGuard {
    public:
        class Guard {
        public:
            T& operator *()
            {
                return *m_value;
            }
            T* operator ->()
            {
                return m_value;
            }

            Guard( Guard && rhs )
            : m_mutex( std::move( rhs.m_mutex ) )
            , m_value( rhs.m_value )
            {}

        private:
            Guard( std::mutex *m, T *v )
            : m_mutex( m, []( std::mutex *mutex ){ mutex->unlock(); } )
            , m_value( v )
            {
                m_mutex->lock();
            }

        private:
            std::shared_ptr< std::mutex > m_mutex;
            T *m_value;
            friend class SafeGuard;
        };

    public:
        SafeGuard()
        : m_value( new T )
        {}
        SafeGuard(const SafeGuard& orig) = delete;
        SafeGuard &operator =(const SafeGuard& orig) = delete;

        Guard get()
        {
            return Guard( &m_mutex, m_value.get() );
        }

    private:
        std::mutex m_mutex;
        std::unique_ptr< T > m_value;
    };


    class PollError: public std::runtime_error
    {
    public:
        PollError( const std::string & what );
    };

    class Connection;

    class Poll {
    public:
        Poll( uint16_t port, int fps );
        Poll(const Poll& orig) = delete;
        Poll &operator =(const Poll& orig) = delete;
        ~Poll();

        void run();
        void stop();

        void store( cv::Mat &frame, int delay );

    private:
        enum { maxevents = 32 };

        std::atomic< bool > m_running { true };

        Socket m_socket;
        int m_fd;
        std::map< int, std::shared_ptr< Connection > > m_connections;

        using Frame = std::pair< cv::Mat, int >;
        SafeGuard< Frame > m_frame;
        std::unique_ptr< Encoder > m_encoder;

        int m_fps;
        std::string m_host;
        std::string m_sdp;

    private:
        void f_add( int sock, uint32_t events );
        void f_send_frame();
};

}  // namespace rtsp

#endif /* RTSP_POLL_H */
