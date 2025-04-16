/*********************
 * IKM (c) 2024
 ***/

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <optional>
#include <set>
#include <array>

#include "sndMan.h"

class HelloApplication
{
private:
    const char*     mAPPNAME                = "Hello Triangle";
    const int       MAX_FRAMES_IN_FLIGHT    = 2;

public:
    HelloApplication()
    {
    }

public:
    void run()
    {
        mainLoop();
    }

private:

    void initWindow()
    {
    }

    void mainLoop()
    {
        while( 1 )
        {
        }
    }

private:

    CSoundManager               mSndMan;
    
};

int main()
{
    HelloApplication app;

    try
    {
        app.run();
    }
    catch( const std::exception& e )
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

