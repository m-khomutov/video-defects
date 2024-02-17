//
// Created by mkh on 14.02.2024.
//

#ifndef VIDEOTESTS_DEFECTS_H
#define VIDEOTESTS_DEFECTS_H

#include "reader.h"
#include <opencv2/core/mat.hpp>
#include <string>

class Defects {
public:
    static const size_t kTestNumber = 5;

    Defects( char const *name );

    void run( Reader &r );

private:
    enum { BLUE = 1, GREEN = 2, RED = 4 };

    cv::Mat &f_blur( cv::Mat &src, cv::Size core_size );
    cv::Mat &f_atvl( cv::Mat &src, float alpha );
    cv::Mat &f_moveLuma( cv::Mat &src, float alpha );
    cv::Mat &f_moveChroma( cv::Mat &src, float alpha );
    cv::Mat &f_posterize( cv::Mat &src, int div=64 );
    cv::Mat &f_moveHSV( cv::Mat &src, double alpha = 1.0, int beta = 0 );
    cv::Mat &f_lumaHistogram( cv::Mat &src );
    cv::Mat &f_chromaHistogram( cv::Mat &src, uint32_t colors );
    cv::Mat &f_grayscale( cv::Mat &src );

    void f_manage_keycode( int code );

private:
    std::string m_name;

    std::pair< std::string, bool > m_tests[kTestNumber];
    int m_current_test {-1};
    int m_highlighted {0};
};


#endif //VIDEOTESTS_DEFECTS_H
