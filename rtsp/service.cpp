/* 
 * File:   base.cpp
 * Author: mkh
 * 
 * Created on 5 февраля 2023 г., 16:14
 */

#include "service.h"

rtsp::Service::Service( uint16_t port, int fps )
: m_poll( port, fps )
, m_poll_thread( &m_poll )
{}

rtsp::Service::~Service()
{}
