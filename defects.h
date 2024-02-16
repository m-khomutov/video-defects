//
// Created by mkh on 14.02.2024.
//

#ifndef VIDEOTESTS_DEFECTS_H
#define VIDEOTESTS_DEFECTS_H


#include <opencv2/core/mat.hpp>

namespace defects
{
    enum { BLUE = 1, GREEN = 2, RED = 4 };

    cv::Mat &blur( cv::Mat &src, cv::Size core_size );
    cv::Mat &atvl( cv::Mat &src, float alpha );
    cv::Mat &moveLuma( cv::Mat &src, float alpha );
    cv::Mat &moveChroma( cv::Mat &src, float alpha );
    cv::Mat &posterize( cv::Mat &src, int div=64 );
    cv::Mat &moveHSV( cv::Mat &src, double alpha = 1.0, int beta = 0 );
    cv::Mat &lumaHistogram( cv::Mat &src );
    cv::Mat &chromaHistogram( cv::Mat &src, uint32_t colors );
    cv::Mat &grayscale( cv::Mat &src );
}


#endif //VIDEOTESTS_DEFECTS_H
