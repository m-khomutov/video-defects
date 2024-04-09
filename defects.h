//
// Created by mkh on 14.02.2024.
//

#ifndef VIDEOTESTS_DEFECTS_H
#define VIDEOTESTS_DEFECTS_H

#include <opencv2/core/mat.hpp>
#include <string>

class Defects {
public:
    enum Tests { Monochrome, Overexposed, Shadowed, LowChroma, ATVL, Posterize, Noise, Equalize, Number };

    Defects();
    ~Defects();

    cv::Mat convert( cv::Mat &frame );
    cv::Mat &testList( cv::Mat &frame );
    cv::Mat &histogram( cv::Mat &frame );
    cv::Mat &result( cv::Mat &frame );

    void Y()
    {
        f_manage_histogram( HistogramFlags::Y_Histogram );
    }
    void U()
    {
        f_manage_histogram( HistogramFlags::U_Histogram );
    }
    void V()
    {
        f_manage_histogram( HistogramFlags::V_Histogram );
    }
    void R()
    {
        f_manage_histogram( HistogramFlags::R_Histogram );
    }
    void G()
    {
        f_manage_histogram( HistogramFlags::G_Histogram );
    }
    void B()
    {
        f_manage_histogram( HistogramFlags::B_Histogram );
    }
    void Up();
    void Down();
    void Left();
    void Right();
    void Enter();

    void highlight( bool on );

private:
    enum HistogramFlags { Y_Histogram = 0x0100,
                          U_Histogram = 0x0200,
                          V_Histogram = 0x0400,
                          R_Histogram = 0x0800,
                          G_Histogram = 0x1000,
                          B_Histogram = 0x2000 };

    void f_manage_histogram( uint32_t flag );

    cv::Mat &f_blur( cv::Mat &src, cv::Size core_size );
    cv::Mat &f_atvl( cv::Mat &src );
    cv::Mat &f_moveLuma( cv::Mat &src );
    cv::Mat &f_moveChroma( cv::Mat &src );
    cv::Mat &f_posterize( cv::Mat &src );
    cv::Mat &f_moveHSV( cv::Mat &src, double alpha = 1.0, int beta = 0 );
    cv::Mat &f_lumaHistogram( cv::Mat &src );
    cv::Mat &f_chromaHistogram( cv::Mat &src );
    cv::Mat &f_grayscale( cv::Mat &src );
    cv::Mat &f_noise( cv::Mat &src );
    cv::Mat &f_equalize( cv::Mat &src );

    double f_peak_sn( cv::Mat &src, cv::Mat &noised );

private:
    struct TestInfo
    {
        std::string name;
        uint32_t flag;
        bool highlighted {false};
        float alpha = 1.0f;
        float result = 0.f;

        TestInfo() = default;
        TestInfo( const char *n, uint32_t f ): name( n ), flag( f )
        {}
    };

    TestInfo m_test_info[Tests::Number];
    int m_current_test {-1};
    int m_highlighted {0};
    uint32_t m_test_flags {0u};
    std::string m_test_result;
};


#endif //VIDEOTESTS_DEFECTS_H
