//
// Created by mkh on 20.02.2024.
//

#ifndef VIDEODEFECTS_WINDOW_H
#define VIDEODEFECTS_WINDOW_H


#include "reader.h"
#include "defects.h"
#include <string>

class Window {
public:
    Window( char const *name );
    ~Window();

    void run( Reader &r );

private:
    std::string m_name;
    Defects m_defects;

private:
    void f_manage_keycode( int code );
};


#endif //VIDEODEFECTS_WINDOW_H
