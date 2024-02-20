//
// Created by mkh on 20.02.2024.
//

#include "window.h"
#include "rtsp/service.h"

#include <signal.h>
#include <sys/time.h>

namespace {
    bool running = true;
    void signal_handler( int s )
    {
        running = false;
    }

    uint64_t now()
    {
        struct timeval tv;
        gettimeofday( &tv, nullptr );
        return (tv.tv_sec * 1000000 + tv.tv_usec) / 1000;
    }
}  // namespace

Window::Window( char const *name )
: m_name( name )
{
    signal( SIGHUP,  signal_handler );
    signal( SIGTERM, signal_handler );
    signal( SIGSEGV, signal_handler);
    signal( SIGINT,  signal_handler);

    cv::namedWindow( name, cv::WINDOW_AUTOSIZE );
}

Window::~Window()
{
    cv::destroyAllWindows();
}

void Window::run( Reader &r )
{
    std::unique_ptr< rtsp::Service > srv( new rtsp::Service( 5555, r.fps() ) );

    cv::Mat frame;

    int delta = 0;
    while( running ) {
        r.read( frame, &delta );
        if( frame.empty() ) {
            r.reopen();
            continue;
        }
        uint64_t ts = now();

        srv->store( m_defects.convert( frame ).clone(), delta );

        cv::imshow( m_name.c_str(), m_defects.testList( m_defects.histogram( frame ) ) );

        int passed = 0;
        do {
            passed = now() - ts;
            if( delta > passed ) {
                int code;
                if( (code = cv::waitKey( delta - passed )) != -1 )
                {
                    f_manage_keycode( code );
                }
            }
        }
        while( delta > passed );
    }
}

void Window::f_manage_keycode( int code )
{
    m_defects.highlight( false );;
    switch( code )
    {
        case 'y':  // Luma histogram
            m_defects.Y();
            break;
        case 'u':  // Chroma U (Cb) histogram
            m_defects.U();
            break;
        case 'v':  // CHroma V (Cr) histogram
            m_defects.V();
            break;
        case 'r':  // Red histogram
            m_defects.R();
            break;
        case 'g':  // Green histogram
            m_defects.G();
            break;
        case 'b':  // Blue histogram
            m_defects.B();
            break;
        case 0x52: // up
            m_defects.Up();
            break;
        case 0x54: // down
            m_defects.Down();
            break;
        case 0x51: // left
            m_defects.Left();
            break;
        case 0x53: // right
            m_defects.Right();
            break;
        case 0x0d: // enter
            m_defects.Enter();
            break;
    }
    m_defects.highlight( true );
}
