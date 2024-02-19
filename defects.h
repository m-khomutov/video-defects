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
    enum Tests { Monochrome, Overexposed, Shadowed, LowChroma, ATVL, Posterize, Number };

    Defects( char const *name );

    void run( Reader &r );

private:
    enum HistogramFlags { Y_Histogram = 64,
                          R_Histogram = 128,
                          G_Histogram = 256,
                          B_Histogram = 512 };

    cv::Mat &f_blur( cv::Mat &src, cv::Size core_size );
    cv::Mat &f_atvl( cv::Mat &src );
    cv::Mat &f_moveLuma( cv::Mat &src );
    cv::Mat &f_moveChroma( cv::Mat &src );
    cv::Mat &f_posterize( cv::Mat &src );
    cv::Mat &f_moveHSV( cv::Mat &src, double alpha = 1.0, int beta = 0 );
    cv::Mat &f_lumaHistogram( cv::Mat &src );
    cv::Mat &f_chromaHistogram( cv::Mat &src );
    cv::Mat &f_grayscale( cv::Mat &src );

    void f_manage_keycode( int code );
    void f_on_y();
    void f_on_r();
    void f_on_g();
    void f_on_b();
    void f_on_up();
    void f_on_down();
    void f_on_left();
    void f_on_right();
    void f_on_enter();

private:
    struct TestInfo
    {
        std::string name;
        uint32_t flag;
        bool highlighted {false};
        float alpha = 1.0f;

        TestInfo() = default;
        TestInfo( const char *n, uint32_t f ): name( n ), flag( f )
        {}
    };
    std::string m_name;

    TestInfo m_test_info[Tests::Number];
    int m_current_test {-1};
    int m_highlighted {0};
    uint32_t m_test_flags {0u};
};


#endif //VIDEOTESTS_DEFECTS_H
