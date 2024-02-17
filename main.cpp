#include "reader.h"
#include "defects.h"
#include <iostream>

int main( int argc, char *argv[]) {

    if( argc != 2 ) {
        std::cerr << "run #" << argv[0] << " videofile\n";
        return EXIT_FAILURE;
    }

    try {
        Reader r;
        if( std::isdigit( argv[1][0] ) ) {
            r.open(  std::stoi( argv[1]) );
        }
        else {
            r.open( argv[1] );
        }

        Defects( argv[1]).run( r );
    }
    catch( const std::exception & e ) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
