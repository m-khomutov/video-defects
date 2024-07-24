#include "reader.h"
#include "window.h"
#include <getopt.h>
#include <iostream>

namespace
{
    void show_api_keys_and_exit( const char *prog, int rc )
    {
        std::cerr << "Клавиши управления " << prog <<  ": y, u, v, r, g, b, left, right, up, down, enter (повторное нажатие - отмена управления)\n\n";
        std::cerr << "Клавиши управления выводом гистограммы:\n";
        std::cerr << "\ty\tYUV схема. Вывод яркости Y\n";
        std::cerr << "\tu\tYUV схема. Вывод цветности U (Cb)\n";
        std::cerr << "\tv\tYUV схема. Вывод цветности V (Cr)\n";
        std::cerr << "\tr\tRGB схема. Вывод \"красной\" части гистограммы\n";
        std::cerr << "\tg\tRGB схема. Вывод \"зеленой\" части гистограммы\n";
        std::cerr << "\tb\tRGB схема. Вывод \"синей\" части гистограммы\n";
        std::cerr << "Тесты (перемещение, запуск остановка):\n";
        std::cerr << "\tup\tвыбор теста выше по списку (выбранный тест выделяется)\n";
        std::cerr << "\tdown\tвыбор теста ниже по списку (выбранный тест выделяется)\n";
        std::cerr << "\tenter\tзапуск/остановка теста (запущенный тест маркируется *)\n";
        std::cerr << "\tleft\tуменьшение значения параметра теста (у каждого теста свой параметр)\n";
        std::cerr << "\tright\tувеличение значения параметра теста (у каждого теста свой параметр)\n";
        ::exit( rc );
    }

    void show_options_and_exit( const char *prog, int rc )
    {
        std::cerr << "Запуск: " << prog <<  "[-s] [-c] [-v] [-h]\n\n";
        std::cerr << "\t-f\tфайл на воспроизведение\n";
        std::cerr << "\t-c\tкамера на воспроизведение (int)\n";
        std::cerr << "\t-v\tвывод клавиш управления\n";
        std::cerr << "\t-h\tвывод параметров запуска\n";
        ::exit( rc );
    }
}

int main( int argc, char *argv[]) {

    const char *src = nullptr;
    int c;
    while ((c = getopt (argc, argv, "f:c:vh")) != -1)
    {
        switch (c)
        {
        case 'f':
            src = optarg;
            break;
        case 'c':
            if( !std::isdigit( optarg[0] ) )
            {
                show_options_and_exit( argv[0], EXIT_SUCCESS );
            }
            src = optarg;
            break;
        case 'v':
            show_api_keys_and_exit( argv[0], EXIT_SUCCESS );
            break;
        case 'h':
        default:
            show_options_and_exit( argv[0], EXIT_SUCCESS );
        }
    }

    if( ! src ) {
        show_options_and_exit( argv[0], EXIT_FAILURE );
    }

    try {
        Reader r;
        if( std::isdigit( src[0] ) ) {
            r.open(  std::stoi( src ) );
        }
        else {
            r.open( src );
        }

        Window( src ).run( r );
    }
    catch( const std::exception & e ) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
