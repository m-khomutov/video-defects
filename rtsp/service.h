/* 
 * File:   base.h
 * Author: mkh
 *
 * Created on 5 февраля 2023 г., 16:14
 */

#ifndef RTSP_SERVICE_H
#define RTSP_SERVICE_H

#include "poll.h"
#include <thread>
#include <iostream>

namespace rtsp {

    template< typename T >
    class ScopedThread
    {
     public:
         explicit ScopedThread( T *obj )
         : m_object( obj )
         , m_thread( [this](){ try { m_object->run(); } catch ( const std::exception &e ) { std::cerr << e.what() <<std::endl; } } )
         {}

         ScopedThread( ScopedThread const &orig ) = delete;
         ScopedThread &operator =( ScopedThread const &rhs ) = delete;
         ~ScopedThread()
         {
             m_object->stop();
             if( m_thread.joinable() )
             {
                 m_thread.join();
             }
         }

    private:
        T *m_object;
        std::thread m_thread;
    };


    class Service {
    public:
        explicit Service( uint16_t port, int fps );

        Service(const Service& orig) = delete;
        Service &operator =(const Service& orig) = delete;
        ~Service();

        void store( cv::Mat frame, int delay )
        {
            m_poll.store( frame, delay );
        }

    private:
        Poll m_poll;
        ScopedThread< Poll > m_poll_thread;
    };

}  // namespace rtsp

#endif /* RTSP_RERVICE_H */

