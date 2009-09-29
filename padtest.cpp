#include "synaptics.h"
#include <iostream>

int main(int argc, char *argv[] )
{
    std::cout << "PAD TEST" << std::endl;
    std::cout << "using libsynaptics version " << Synaptics::Pad::libraryStr() << std::endl;

    Synaptics::Pad *s = Synaptics::Pad::getInstance();

    s->setParam( TOUCHPADOFF, 1 );

    std::cout << "detected driver version " << s->driverStr() << " which is ";
    if ( !s->hasDriver() ) std::cout << "not "; 
    std::cout << "supported!" << std::endl;

    return 0;
}
