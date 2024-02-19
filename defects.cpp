//
// Created by mkh on 14.02.2024.
//

#include "defects.h"
#include "rtsp/service.h"
#include <signal.h>
#include <sys/time.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>

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

    char const *test_names[Defects::Tests::Number] = {
        "  monochrome",
        "  overexposed",
        "  shadowed",
        "  low chroma",
        "  atvl",
        "  posterize"
    };
}  // namespace

Defects::Defects( char const *name )
: m_name( name )
{
    signal( SIGHUP,  signal_handler );
    signal( SIGTERM, signal_handler );
    signal( SIGSEGV, signal_handler);
    signal( SIGINT,  signal_handler);

    cv::namedWindow( name, cv::WINDOW_AUTOSIZE );
    //int switch_value;
    //cv::createTrackbar( "name", name, &switch_value, 10, nullptr );

    for( size_t i(0); i < Tests::Number; ++i )
    {
        m_test_info[i] = { test_names[i], (1u << i) };
    }
    m_test_info[0].highlighted = true;
}

void Defects::run( Reader &r )
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

        srv->store( f_grayscale(
                        f_moveLuma(
                            f_moveChroma(
                                f_atvl(
                                    f_posterize( frame )
                                )
                            )
                        )
                    ).clone(),
                    delta
                  );

        for( size_t i(0); i < Tests::Number; ++i )
        {
            int thickness = 1 + m_test_info[i].highlighted;
            cv::putText( frame,
                         m_test_info[i].name.c_str(),
                         cv::Point(10, (i + 1) * 15),
                         cv::FONT_HERSHEY_PLAIN,
                         1,
                         cv::Scalar(0,0,255),
                         thickness,
                         false );
        }

        cv::imshow( m_name.c_str(),
                    f_lumaHistogram(
                        f_chromaHistogram( frame )
                    )
                  );

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

void Defects::f_manage_keycode( int code )
{
    m_test_info[m_highlighted].highlighted = false;
    switch( code )
    {
        case 'y':  // Luma histogram
            f_on_y();
            break;
        case 'r':  // Red histogram
            f_on_r();
            break;
        case 'g':  // Green histogram
            f_on_g();
            break;
        case 'b':  // Blue histogram
            f_on_b();
            break;
        case 0x52: // up
            f_on_up();
            break;
        case 0x54: // down
            f_on_down();
            break;
        case 0x51: // left
            f_on_left();
            break;
        case 0x53: // right
            f_on_right();
            break;
        case 0x0d: // enter
            f_on_enter();
            break;
    }
    m_test_info[m_highlighted].highlighted = true;
}

void Defects::f_on_y()
{
    if( (m_test_flags & HistogramFlags::Y_Histogram) ) {
        m_test_flags &= ~HistogramFlags::Y_Histogram;
    }
    else {
        m_test_flags |= HistogramFlags::Y_Histogram;
    }
}

void Defects::f_on_r()
{
    if( (m_test_flags & HistogramFlags::R_Histogram) ) {
        m_test_flags &= ~HistogramFlags::R_Histogram;
    }
    else {
        m_test_flags |= HistogramFlags::R_Histogram;
    }
}

void Defects::f_on_g()
{
    if( (m_test_flags & HistogramFlags::G_Histogram) ) {
        m_test_flags &= ~HistogramFlags::G_Histogram;
    }
    else {
        m_test_flags |= HistogramFlags::G_Histogram;
    }
}

void Defects::f_on_b()
{
    if( (m_test_flags & HistogramFlags::B_Histogram) ) {
        m_test_flags &= ~HistogramFlags::B_Histogram;
    }
    else {
        m_test_flags |= HistogramFlags::B_Histogram;
    }
}

void Defects::f_on_up()
{
    if( --m_highlighted == -1 )
    {
        m_highlighted = Tests::Number - 1;
    }
}

void Defects::f_on_down()
{
    if( ++m_highlighted == Tests::Number )
    {
        m_highlighted = 0;
    }
}

void Defects::f_on_left()
{
    switch( m_current_test )
    {
        case Tests::Overexposed:
            if( m_test_info[Tests::Overexposed].alpha > 1.0 ) {
                m_test_info[Tests::Overexposed].alpha -= 0.1f;
            }
            break;
        case Tests::Shadowed:
            if( m_test_info[Tests::Shadowed].alpha > 0.0 ) {
                m_test_info[Tests::Shadowed].alpha -= 0.1f;
            }
            break;
        case Tests::Posterize:
            if( m_test_info[Tests::Posterize].alpha > 1.0 ) {
                m_test_info[Tests::Posterize].alpha -= 1.0f;
            }
            break;
    }
}

void Defects::f_on_right()
{
    switch( m_current_test ) {
        case Tests::Overexposed:
            if( m_test_info[Tests::Overexposed].alpha < 4.0 ) {
                m_test_info[Tests::Overexposed].alpha += 0.1f;
            }
            break;
        case Tests::Shadowed:
            if( m_test_info[Tests::Shadowed].alpha < 1.0 ) {
                m_test_info[Tests::Shadowed].alpha += 0.1f;
            }
            break;
        case Tests::Posterize:
            if( m_test_info[Tests::Posterize].alpha < 256.0 ) {
                m_test_info[Tests::Posterize].alpha += 1.0f;
            }
            break;
    }
}

void Defects::f_on_enter()
{
    if( m_current_test != -1 )
    {
        m_test_info[m_current_test].name[0] = ' ';
        if( m_current_test == m_highlighted )
        {
            m_test_flags &= ~m_test_info[m_current_test].flag;
            m_current_test = -1;
        }
        else
        {
            m_test_flags &= ~m_test_info[m_current_test].flag;

            m_current_test = m_highlighted;
            m_test_info[m_current_test].name[0] = '*';
            m_test_flags |= m_test_info[m_current_test].flag;
        }
    }
    else
    {
        m_current_test = m_highlighted;
        m_test_info[m_current_test].name[0] = '*';
        m_test_flags |= m_test_info[m_current_test].flag;
    }
}

cv::Mat &Defects::f_blur( cv::Mat &src, cv::Size core_size )
{
    std::vector< cv::Mat > planes;

    cv::split( src, planes );
    for( size_t i(0); i < 3; ++i ) {
        cv::blur( planes[i], planes[i], core_size );
    }
    cv::merge( planes, src );

    return src;
}

cv::Mat &Defects::f_posterize( cv::Mat &src )
{
    if( (m_test_flags & (1 << Tests::Posterize)) )
    {
        int div = m_test_info[Tests::Posterize].alpha;
        for( size_t y(0); y < src.rows; ++y )
        {
            uchar *b = src.ptr( y );
            uchar *g = b + 1;
            uchar *r = g + 1;
            for( size_t x(0); x < src.cols; ++x )
            {
                // process each pixel
                *b = *b / div * div;// + div / 2;
                b += 3;
                *g = *g / div * div;// + div / 2;
                g += 3;
                *r = *r / div * div;// + div / 2;
                r += 3;
            }
        }
    }
    return src;
}

cv::Mat &Defects::f_moveHSV( cv::Mat &src, double alpha, int beta )
{
    for( int y(0); y < src.rows; y++ ) {
        for( int x(0); x < src.cols; x++ ) {
            for( int c(0); c < src.channels(); ++c ) {
                if( int(src.at< cv::Vec3b >(y, x)[c]) + beta < 0 ) {
                   src.at< cv::Vec3b >(y, x)[c] = 0;
                }
                else if( int(src.at< cv::Vec3b >(y, x)[c]) + beta > 255 ) {
                   src.at< cv::Vec3b >(y, x)[c] = 255;
                }
                else {
                    src.at< cv::Vec3b >(y, x)[c] = cv::saturate_cast< uchar >( alpha * src.at< cv::Vec3b >(y, x)[c] + beta );
                }
            }
        }
    }

    return src;
}

cv::Mat &Defects::f_moveLuma( cv::Mat &src )
{
    bool overexposed = (m_test_flags & (1 << Tests::Overexposed));
    bool shadowed = (m_test_flags & (1 << Tests::Shadowed));

    if( overexposed || shadowed )
    {
        cv::Mat yuv;
        cv::cvtColor( src, yuv, CV_RGB2YUV_I420 );

        for( int y(0); y < src.rows; y++ ) {
            for( int x(0); x < src.cols; x++ ) {
                float alpha = overexposed ? m_test_info[Tests::Overexposed].alpha : m_test_info[Tests::Shadowed].alpha;
                float luma = yuv.at< uchar >(y, x) * alpha;
                if( luma > 255. ) {
                    luma = 255.;
                }
                yuv.at< uchar >(y, x) = cv::saturate_cast< uchar >(luma);
            }
        }
        cv::cvtColor( yuv, src, CV_YUV2RGB_I420 );
    }
    return src;
}

cv::Mat &Defects::f_moveChroma( cv::Mat &src )
{
    if( (m_test_flags & (1 << Tests::LowChroma)) )
    {
        cv::Mat yuv;
        cv::cvtColor( src, yuv, CV_RGB2YUV_I420 );

        int chroma_height = src.rows >> 2;
        float alpha = m_test_info[Tests::LowChroma].alpha;
        for( int y(0); y < chroma_height; y++ ) {
            for( int x(0); x < src.cols; x++ ) {
                float u = yuv.at< uchar >(src.rows + y, x) * alpha;
                if( u > 255. ) {
                    u = 255.;
                }
                yuv.at< uchar >(src.rows + y, x) = cv::saturate_cast< uchar >(u);

                float v = yuv.at< uchar >(src.rows + chroma_height + y, x) * alpha;
                if( v > 255. ) {
                    v = 255.;
                }
                yuv.at< uchar >(src.rows + chroma_height + y, x) = cv::saturate_cast< uchar >(v);

            }
        }
        cv::cvtColor( yuv, src, CV_YUV2RGB_I420 );
    }
    return src;
}

cv::Mat &Defects::f_atvl( cv::Mat &src )
{
    if( (m_test_flags & (1 << Tests::ATVL)) )
    {
        cv::Mat yuv;
        cv::cvtColor( src, yuv, CV_RGB2YUV_I420 );

        float alpha = m_test_info[Tests::ATVL].alpha;
        for( int y(0); y < src.rows; y++ ) {
            for( int x(0); x < src.cols; x++ ) {
                float luma = yuv.at< uchar >(y, x);
                if( luma < 80. ) {
                    luma = luma / alpha;
                }
                if( luma > 80. ) {
                    luma = luma * alpha;
                }
                if( luma > 255. ) {
                    luma = 255.;
                }
                yuv.at< uchar >(y, x) = cv::saturate_cast< uchar >(luma);
            }
        }
        cv::cvtColor( yuv, src, CV_YUV2RGB_I420 );
    }
    return src;
}

cv::Mat &Defects::f_grayscale( cv::Mat &src )
{
    if( (m_test_flags & (1 << Tests::Monochrome)) )
    {
        cv::Mat gray;
        cv::cvtColor( src, gray, CV_RGB2GRAY );

        std::vector< cv::Mat > planes( 3, gray );
        cv::merge( planes, src );
    }
    return src;
}

cv::Mat &Defects::f_lumaHistogram( cv::Mat &src )
{
    if( (m_test_flags & HistogramFlags::Y_Histogram) )
    {
        cv::Mat yuv;
        cv::cvtColor( src, yuv, CV_RGB2YUV_I420 );

        int h_size = 256;
        float range[] = { 0, float(h_size) } ;
        const float* h_range = { range };

        cv::Mat hist;
        calcHist( &yuv, 1, 0, cv::Mat(), hist, 1, &h_size, &h_range, true, false );

        cv::Size bg_size( 256, 128 );
        cv::Mat bg( bg_size.height, bg_size.width, CV_8UC3, cv::Scalar(0 ,0, 0 ) );
        cv::normalize(hist, hist, 0, bg.rows, cv::NORM_MINMAX, -1, cv::Mat() );

        for( size_t i(1); i < hist.rows; ++i )
        {
            cv::line( bg,
                      cv::Point( (i - 1), bg.rows - cvRound( hist.at< float>(i - 1) ) ),
                      cv::Point( i, bg.rows - cvRound( hist.at< float >(i) ) ),
                      cv::Scalar( 255, 255, 255) );
        }
        cv::Mat roi1( src, cv::Rect( 0, src.rows - bg_size.height, bg_size.width, bg_size.height ) );
        cv::addWeighted( roi1, 0.35, bg, 0.65, 0.0, roi1 );
    }
    return src;
}
cv::Mat &Defects::f_chromaHistogram( cv::Mat &src )
{
    if( (m_test_flags & HistogramFlags::R_Histogram) ||
        (m_test_flags & HistogramFlags::G_Histogram) ||
        (m_test_flags & HistogramFlags::B_Histogram))
    {
        std::vector< cv::Mat > planes;
        cv::split( src, planes );

        int h_size = 256;
        float range[] = { 0, float(h_size) } ;
        const float* h_range = { range };

        cv::Mat hist[3];
        calcHist( &planes[0], 1, 0, cv::Mat(), hist[0], 1, &h_size, &h_range, true, false );
        calcHist( &planes[1], 1, 0, cv::Mat(), hist[1], 1, &h_size, &h_range, true, false );
        calcHist( &planes[2], 1, 0, cv::Mat(), hist[2], 1, &h_size, &h_range, true, false );

        cv::Size bg_size( 256, 128 );
        cv::Mat bg( bg_size.height, bg_size.width, CV_8UC3, cv::Scalar(0 ,0, 0 ) );

        cv::normalize(hist[0], hist[0], 0, bg.rows, cv::NORM_MINMAX, -1, cv::Mat() );
        cv::normalize(hist[1], hist[1], 0, bg.rows, cv::NORM_MINMAX, -1, cv::Mat() );
        cv::normalize(hist[2], hist[2], 0, bg.rows, cv::NORM_MINMAX, -1, cv::Mat() );

        for( int i(1); i < h_size; ++i )
        {
            if( (m_test_flags & HistogramFlags::B_Histogram) ) {
                line( bg,
                      cv::Point( (i - 1), bg.rows - cvRound(hist[0].at< float >(i - 1)) ),
                      cv::Point( i, bg.rows - cvRound(hist[0].at< float >(i)) ),
                      cv::Scalar( 255, 0, 0) );
            }
            if( (m_test_flags & HistogramFlags::G_Histogram) ) {
                line( bg,
                      cv::Point( (i - 1), bg.rows - cvRound(hist[1].at< float >(i - 1)) ),
                      cv::Point( i, bg.rows - cvRound(hist[1].at< float >(i)) ),
                      cv::Scalar( 0, 255, 0) );
            }
            if( (m_test_flags & HistogramFlags::R_Histogram) ) {
                line( bg,
                      cv::Point( (i - 1), bg.rows - cvRound(hist[2].at< float >(i - 1)) ),
                      cv::Point( i, bg.rows - cvRound(hist[2].at< float >(i)) ),
                      cv::Scalar( 0, 0, 255) );
            }
        }
        cv::Mat roi1( src, cv::Rect( 0, src.rows - bg_size.height, bg_size.width, bg_size.height ) );
        cv::addWeighted( roi1, 0.35, bg, 0.65, 0.0, roi1 );
    }
    return src;
}
