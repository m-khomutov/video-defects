//
// Created by mkh on 09.02.2024.
//

#ifndef VIDEOTESTS_ENCODER_H
#define VIDEOTESTS_ENCODER_H

#include <stdint.h>
#include <x264.h>
#include <vector>
#include <fstream>
#include <chrono>
#include <opencv2/core/mat.hpp>

class Encoder {
public:
    using PS = std::vector< uint8_t >;

    Encoder( uint32_t width, uint32_t height, uint32_t fps );
    ~Encoder();

    void encode( const cv::Mat &rgb, int delay, PS *sps, PS *pps );
    void store( std::ofstream &f );

    const uint8_t *nalunit() const
    {
        return m_nalunit;
    }
    uint32_t nalusize() const
    {
        return m_nalusz;
    }
    bool keyframe() const
    {
        return m_nalutype == nal_unit_type_e::NAL_SLICE_IDR;
    }

private:
    x264_t *m_encoder;

    x264_param_t m_params;
    x264_picture_t m_picture;
    x264_nal_t *m_nalunits {nullptr};

    uint8_t *m_nalunit {nullptr};
    uint32_t m_nalusz {0};
    uint8_t m_nalutype {nal_unit_type_e::NAL_UNKNOWN};

private:
    uint32_t f_store_nalunit( uint8_t *ptr, PS *sps, PS *pps );
};


#endif //VIDEOTESTS_ENCODER_H
