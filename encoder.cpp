//
// Created by mkh on 09.02.2024.
//

#include "encoder.h"
#include <stdexcept>
#include <cstring>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>

namespace {
    uint32_t get_avcC_size( uint8_t *ptr ) {
        uint32_t rc;
        ::memcpy( &rc, ptr, sizeof(rc) );

        return be32toh( rc );
    }
} // namespace

Encoder::Encoder( uint32_t width, uint32_t height, uint32_t fps  )
{
    x264_param_default( &m_params );
    if( x264_param_default_preset( &m_params, "superfast", "animation" ) ) {
        throw std::logic_error( "[x264_enc] failed to set slow preset" );
    }

    m_params.i_csp = X264_CSP_YV12;
    m_params.i_threads = 6;
    m_params.i_width = width;
    m_params.i_height = height;

    m_params.i_fps_num = fps;
    m_params.i_fps_den = 1;

    // Intra refres:
    m_params.i_keyint_max = m_params.i_fps_num;
    m_params.b_intra_refresh = 0;
    //For streaming:
    m_params.b_repeat_headers = 1;
    m_params.b_annexb = 0;

    //Rate control
    m_params.rc.i_rc_method = X264_RC_CRF;
    m_params.rc.f_rf_constant = 23;
    m_params.rc.f_rf_constant_max = m_params.rc.f_rf_constant;

    if( x264_param_apply_profile( &m_params, "baseline" ) < 0 )
        throw std::logic_error( "[x264_enc] failed to set baseline profile" );

    x264_picture_init( &m_picture );
    m_picture.img.i_csp = X264_CSP_I420;
    m_picture.img.i_plane = 3;
    m_picture.img.i_stride[0] = width;
    m_picture.img.i_stride[1] = width >> 1;
    m_picture.img.i_stride[2] = width >> 1;
    m_picture.i_pts = rand();
    if( !(m_encoder = x264_encoder_open( &m_params ) ) )
        throw std::logic_error( "[x264_enc] failed to open encoder" );
}

Encoder::~Encoder()
{
    x264_encoder_close( m_encoder );
}

void Encoder::encode( const cv::Mat &rgb, int delay, PS *sps, PS *pps ) {
    cv::Mat yuv;
    cv::cvtColor( rgb, yuv, CV_RGB2YUV_I420 );

    m_picture.img.plane[ 0 ] = yuv.data;
    m_picture.img.plane[ 1 ] = yuv.data + rgb.rows * rgb.cols + ((rgb.rows * rgb.cols) >> 2);
    m_picture.img.plane[ 2 ] = yuv.data + rgb.rows * rgb.cols;

    m_picture.i_pts += delay;

    int nals_count{0};
    x264_picture_t picture_out;

    int size = x264_encoder_encode( m_encoder, &m_nalunits, &nals_count, &m_picture, &picture_out );
    if( size && m_nalunits->p_payload ) {
        uint8_t *ptr = m_nalunits->p_payload;
        while( ptr < m_nalunits->p_payload + size ) {
            ptr += f_store_nalunit( ptr, sps, pps ) + sizeof(uint32_t);
        }
    }
}

uint32_t Encoder::f_store_nalunit( uint8_t *ptr, PS *sps, PS *pps )
{
    uint32_t sz = get_avcC_size( ptr );
    ptr += sizeof( sz );

    m_nalutype = (*ptr) & 0x1f;
    if( m_nalutype == nal_unit_type_e::NAL_SPS ) {
        *sps = PS(ptr, ptr + sz);
    }
    else if( m_nalutype == nal_unit_type_e::NAL_PPS ) {
        *pps = PS(ptr, ptr + sz);
    }
    else if( m_nalutype >= nal_unit_type_e::NAL_SLICE && m_nalutype <= nal_unit_type_e::NAL_SLICE_IDR ) {
        m_nalunit = ptr;
        m_nalusz = sz;
    }

    return sz;
}
