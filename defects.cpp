//
// Created by mkh on 14.02.2024.
//

#include "defects.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>

cv::Mat &defects::blur( cv::Mat &src, cv::Size core_size )
{
    std::vector< cv::Mat > planes;

    cv::split( src, planes );
    for( size_t i(0); i < 3; ++i ) {
        cv::blur( planes[i], planes[i], core_size );
    }
    cv::merge( planes, src );

    return src;
}

cv::Mat &defects::posterize( cv::Mat &src, int div )
{
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

    return src;
}

cv::Mat &defects::moveHSV( cv::Mat &src, double alpha, int beta )
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

cv::Mat &defects::moveLuma( cv::Mat &src, float alpha )
{
    cv::Mat yuv;
    cv::cvtColor( src, yuv, CV_RGB2YUV_I420 );

    for( int y(0); y < src.rows; y++ ) {
        for( int x(0); x < src.cols; x++ ) {
            float luma = yuv.at< uchar >(y, x) * alpha;
            if( luma > 255. ) {
                luma = 255.;
            }
            yuv.at< uchar >(y, x) = cv::saturate_cast< uchar >(luma);
        }
    }
    cv::cvtColor( yuv, src, CV_YUV2RGB_I420 );

    return src;
}

cv::Mat &defects::moveChroma( cv::Mat &src, float alpha )
{
    cv::Mat yuv;
    cv::cvtColor( src, yuv, CV_RGB2YUV_I420 );

    int chroma_height = src.rows >> 2;
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

    return src;
}

cv::Mat &defects::atvl( cv::Mat &src, float alpha )
{
    cv::Mat yuv;
    cv::cvtColor( src, yuv, CV_RGB2YUV_I420 );

    for( int y(0); y < src.rows; y++ ) {
        for( int x(0); x < src.cols; x++ ) {
            float luma = yuv.at< uchar >(y, x) * alpha;
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

    return src;
}

cv::Mat &defects::lumaHistogram( cv::Mat &src )
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

    return src;
}
cv::Mat &defects::chromaHistogram( cv::Mat &src, uint32_t colors )
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
        if( colors & BLUE ) {
            line( bg,
                  cv::Point( (i - 1), bg.rows - cvRound(hist[0].at< float >(i - 1)) ),
                  cv::Point( i, bg.rows - cvRound(hist[0].at< float >(i)) ),
                  cv::Scalar( 255, 0, 0) );
        }
        if( colors & GREEN ) {
            line( bg,
                  cv::Point( (i - 1), bg.rows - cvRound(hist[1].at< float >(i - 1)) ),
                  cv::Point( i, bg.rows - cvRound(hist[1].at< float >(i)) ),
                  cv::Scalar( 0, 255, 0) );
        }
        if( colors & RED ) {
            line( bg,
                  cv::Point( (i - 1), bg.rows - cvRound(hist[2].at< float >(i - 1)) ),
                  cv::Point( i, bg.rows - cvRound(hist[2].at< float >(i)) ),
                  cv::Scalar( 0, 0, 255) );
        }
    }
    cv::Mat roi1( src, cv::Rect( 0, src.rows - bg_size.height, bg_size.width, bg_size.height ) );
    cv::addWeighted( roi1, 0.35, bg, 0.65, 0.0, roi1 );

    return src;
}

cv::Mat &defects::grayscale( cv::Mat &src )
{
    cv::Mat gray;
    cv::cvtColor( src, gray, CV_RGB2GRAY );

    std::vector< cv::Mat > planes( 3, gray );
    cv::merge( planes, src );

    return src;
}

