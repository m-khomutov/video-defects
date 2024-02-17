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
    static const size_t kTestNumber = 6;

    Defects( char const *name );

    void run( Reader &r );

private:
    enum Tests { None = 0,
                 Monochrome  = 1,
                 Overexposed = 2,
                 Shadowed    = 4,
                 LowChroma   = 8,
                 ATVL        = 16,
                 Posterize   = 32,
                 Y_Histogram = 64,
                 R_Histogram = 128,
                 G_Histogram = 256,
                 B_Histogram = 512 };

    cv::Mat &f_blur( cv::Mat &src, cv::Size core_size );
    cv::Mat &f_atvl( cv::Mat &src );
    cv::Mat &f_moveLuma( cv::Mat &src );
    cv::Mat &f_moveChroma( cv::Mat &src );
    cv::Mat &f_posterize( cv::Mat &src, int div=64 );
    cv::Mat &f_moveHSV( cv::Mat &src, double alpha = 1.0, int beta = 0 );
    cv::Mat &f_lumaHistogram( cv::Mat &src );
    cv::Mat &f_chromaHistogram( cv::Mat &src );
    cv::Mat &f_grayscale( cv::Mat &src );

    void f_manage_keycode( int code );

private:
    std::string m_name;

    std::pair< std::string, bool > m_tests[kTestNumber];
    int m_current_test {-1};
    int m_highlighted {0};
    uint32_t m_test_flags {Tests::None};
    float m_alpha = 1.0f;
};


#endif //VIDEOTESTS_DEFECTS_H
