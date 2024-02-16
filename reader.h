//
// Created by mkh on 09.02.2024.
//

#ifndef VIDEOTESTS_READER_H
#define VIDEOTESTS_READER_H


#include "encoder.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>

class Reader {
public:
    Reader();
    ~Reader();

    void open( const char *filename );
    void open( int device );
    void reopen();

    void read( cv::Mat &frame, int *delta );

    int width() const
    {
        return m_width;
    }
    int height() const
    {
        return m_height;
    }
    double fps() const
    {
        return m_fps;
    }

private:
    cv::VideoCapture m_capture;
    cv::String m_filename;
    int m_device {-1};

    double m_fps = 25.;
    int m_delay = 1000. / m_fps;
    double m_timestamp = -1.0;

    size_t m_count {0};
    int m_width {0};
    int m_height {0};

};


#endif //VIDEOTESTS_READER_H
