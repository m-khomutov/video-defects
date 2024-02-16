#include "reader.h"
#include "defects.h"
#include "rtsp/service.h"
#include <signal.h>
#include <sys/time.h>
#include <iostream>

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

int main( int argc, char *argv[]) {

    if( argc != 2 ) {
        std::cerr << "run #" << argv[0] << " videofile\n";
        return EXIT_FAILURE;
    }

    signal( SIGHUP,  signal_handler );
    signal( SIGTERM, signal_handler );
    signal( SIGSEGV, signal_handler);
    signal( SIGINT,  signal_handler);

    int switch_value;

    cv::namedWindow( argv[1], cv::WINDOW_AUTOSIZE );
    cv::createTrackbar( "name", argv[1], &switch_value, 10, nullptr );

    try {
        Reader r;
        if( std::isdigit( argv[1][0] ) ) {
            r.open(  std::stoi( argv[1]) );
        }
        else {
            r.open( argv[1] );
        }

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

            cv::Mat fr = frame.clone();//defects::posterize( frame ).clone();
            cv::imshow( argv[1], defects::lumaHistogram( frame ) );
            srv->store( fr, delta );

            int passed = 0;
            do {
                passed = now() - ts;
                if( delta > passed ) {
                    int code;
                    if( (code = cv::waitKey( delta - passed )) != -1 )
                    {}
                }
            }
            while( delta > passed );
        }
    }
    catch( const std::exception & e ) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
