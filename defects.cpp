//
// Created by mkh on 14.02.2024.
//

#include "defects.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>

namespace {
    char const *test_names[Defects::Tests::Number] = {
        "  monochrome",
        "  overexposed",
        "  shadowed",
        "  low chroma",
        "  atvl",
        "  posterize",
        "  noise"
    };
}  // namespace

Defects::Defects()
{
    for( size_t i(0); i < Tests::Number; ++i )
    {
        m_test_info[i] = { test_names[i], (1u << i) };
    }
    m_test_info[0].highlighted = true;
}

Defects::~Defects()
{}

cv::Mat Defects::convert( cv::Mat &frame )
{
    return f_grayscale(
               f_moveLuma(
                   f_moveChroma(
                       f_atvl(
                           f_posterize(
                               f_noise(  frame )
                           )
                       )
                   )
               )
           );
}

cv::Mat &Defects::testList( cv::Mat &frame )
{
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
    return frame;
}

cv::Mat &Defects::histogram( cv::Mat &frame )
{
    return f_lumaHistogram( f_chromaHistogram( frame ) );
}

void Defects::Up()
{
    if( --m_highlighted == -1 )
    {
        m_highlighted = Tests::Number - 1;
    }
}

void Defects::Down()
{
    if( ++m_highlighted == Tests::Number )
    {
        m_highlighted = 0;
    }
}

void Defects::Left()
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
        case Tests::Noise:
            if( m_test_info[Tests::Noise].alpha > .0 ) {
                m_test_info[Tests::Noise].alpha -= 1.0f;
            }
    }
}

void Defects::Right()
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
        case Tests::Noise:
            if( m_test_info[Tests::Noise].alpha < 256.0 ) {
                m_test_info[Tests::Noise].alpha += 1.0f;
            }
            break;
    }
}

void Defects::Enter()
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

void Defects::highlight( bool on )
{
    m_test_info[m_highlighted].highlighted = on;
}

void Defects::f_manage_histogram( uint32_t flag )
{
    if( (m_test_flags & flag) ) {
        m_test_flags &= ~flag;
    }
    else {
        m_test_flags |= flag;
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

cv::Mat &Defects::f_noise( cv::Mat &src )
{
    if( (m_test_flags & (1 << Tests::Noise)) )
    {
        cv::Mat gaussian_noise = cv::Mat(src.size(),CV_8UC3);
        cv::randn( gaussian_noise, 0, m_test_info[Tests::Noise].alpha );
        src += gaussian_noise;
        cv::normalize( src, src, 0, 255, CV_MINMAX, CV_8UC3 );
    }
    return src;
}

double Defects::f_peak_sn( cv::Mat &src )
{
    cv::Mat blurred = src.clone();
    f_blur( blurred, cv::Size( 3, 3) );

    cv::Mat diff;
    cv::absdiff( src, blurred, diff );
    diff.convertTo( diff, CV_32F );
    diff = diff.mul( diff );

    cv::Scalar s = cv::sum( diff );
    double sse = s.val[0] + s.val[1] + s.val[2];
    if( sse <= 1e-10 ) // for small values return zero
    {
        return 0.;
    }

    double mse = sse /(double)(src.channels() * src.total());
    return 10.0 * log10( (255 * 255) / mse );
}

cv::Mat &Defects::f_lumaHistogram( cv::Mat &src )
{
    if( (m_test_flags & HistogramFlags::Y_Histogram) || (m_test_flags & HistogramFlags::U_Histogram) || (m_test_flags & HistogramFlags::V_Histogram) )
    {
        cv::Mat yuv;
        cv::cvtColor( src, yuv, CV_RGB2YUV_I420 );
        cv::Mat u( src.rows >> 2, src.cols, yuv.type(), yuv.data + src.rows * src.cols );
        cv::Mat v( src.rows >> 2, src.cols, yuv.type(), yuv.data + (src.rows + (src.rows >> 2)) * src.cols );

        int h_size = 256;
        float range[] = { 0, float(h_size) } ;
        const float* h_range = { range };

        cv::Mat hist[3];
        calcHist( &yuv, 1, 0, cv::Mat(), hist[0], 1, &h_size, &h_range, true, false );
        calcHist( &u, 1, 0, cv::Mat(), hist[1], 1, &h_size, &h_range, true, false );
        calcHist( &v, 1, 0, cv::Mat(), hist[2], 1, &h_size, &h_range, true, false );

        cv::Size bg_size( 256, 128 );
        cv::Mat bg( bg_size.height, bg_size.width, CV_8UC3, cv::Scalar(0 ,0, 0 ) );

        cv::normalize(hist[0], hist[0], 0, bg.rows, cv::NORM_MINMAX, -1, cv::Mat() );
        cv::normalize(hist[1], hist[1], 0, bg.rows, cv::NORM_MINMAX, -1, cv::Mat() );
        cv::normalize(hist[2], hist[2], 0, bg.rows, cv::NORM_MINMAX, -1, cv::Mat() );

        for( size_t i(1); i < h_size; ++i )
        {
            if( (m_test_flags & HistogramFlags::Y_Histogram) ) {
                cv::line( bg,
                          cv::Point( (i - 1), bg.rows - cvRound( hist[0].at< float>(i - 1) ) ),
                          cv::Point( i, bg.rows - cvRound( hist[0].at< float >(i) ) ),
                          cv::Scalar( 255, 255, 255) );
            }
            if( (m_test_flags & HistogramFlags::U_Histogram) ) {
                cv::line( bg,
                          cv::Point( (i - 1), bg.rows - cvRound( hist[1].at< float>(i - 1) ) ),
                          cv::Point( i, bg.rows - cvRound( hist[1].at< float >(i) ) ),
                          cv::Scalar( 255, 255, 0) );
            }
            if( (m_test_flags & HistogramFlags::V_Histogram) ) {
                cv::line( bg,
                          cv::Point( (i - 1), bg.rows - cvRound( hist[2].at< float>(i - 1) ) ),
                          cv::Point( i, bg.rows - cvRound( hist[2].at< float >(i) ) ),
                          cv::Scalar( 0, 255, 255) );
            }
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
