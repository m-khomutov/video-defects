//
// Created by mkh on 09.02.2024.
//

#include "reader.h"

Reader::Reader()
{}

void Reader::open( const char *filename )
{
    m_filename = cv::String(filename);
    m_capture.open( m_filename );
    if( !m_capture.isOpened() ) {
        throw std::logic_error( std::string("error opening file: ") + filename );
    }
    m_delay = 0;

    m_fps = std::ceil( m_capture.get( cv::CAP_PROP_FPS) );
    m_width = m_capture.get(  cv::CAP_PROP_FRAME_WIDTH );
    m_height = m_capture.get(  cv::CAP_PROP_FRAME_HEIGHT );
}

Reader::~Reader()
{
    m_capture.release();
}

void Reader::open( int device )
{
    m_device = device;

    m_capture.open( m_device );
    if( !m_capture.isOpened() ) {
        throw std::logic_error( std::string("error opening device") + std::to_string(device) );
    }
    m_width = m_capture.get(  cv::CAP_PROP_FRAME_WIDTH );
    m_height = m_capture.get(  cv::CAP_PROP_FRAME_HEIGHT );
}

void Reader::reopen()
{
    m_capture.set( cv::CAP_PROP_POS_FRAMES, 0 );
}

void Reader::read( cv::Mat &frame, int *delta )
{
    m_capture >> frame;
    if( frame.empty() ) {
        m_capture.set( cv::CAP_PROP_POS_FRAMES, 0 );
        m_timestamp = -1.;
    }
    ++m_count;

    double pos = m_capture.get( cv::CAP_PROP_POS_MSEC );
    if( m_timestamp < 0. || pos < m_timestamp ) {
        m_timestamp = pos;
    }

    *delta = m_delay > 0 ? m_delay : pos - m_timestamp;
    if( *delta == 0 )
        ++(*delta);

    if( m_delay == 0 )
        m_timestamp = pos;
}
