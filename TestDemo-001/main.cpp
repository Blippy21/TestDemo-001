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
        constexpr unsigned inBuffSize = 8;
        char inBuff[inBuffSize] = "";
        bool isRunning = true;

        mSndMan.PlaySound( 0 );

        while( isRunning )
        {
            std::cout << "OPTIONS: " << std::endl;
            std::cout << "q: quit " << std::endl;
            std::cout << "0-9: adjust filter " << std::endl;

            if( fgets( inBuff, inBuffSize, stdin) != NULL )
            {
                char cmd = ' ';
                // Parse the input using sscanf
                if( sscanf( inBuff, "%c", &cmd) == 1 )
                {
                    switch( cmd )
                    {
                        case 'q':   isRunning = false; break;
                        default:
                        {
                            if( cmd >= '0' && cmd <= '9' )
                            {
                                int step = (cmd - '0');
                                int range = '9'-'0';
                                float fParam = (float)step/range;

                                std::cout << "Filter set to %" << std::fixed << std::setprecision(1) << fParam << std::endl;
                                mSndMan.SetFilterParam( fParam );
                            }
                            else
                            {
                                std::cout << "Unrecognized command: " << cmd << std::endl;
                            }
                            break;
                        }
                    }
                }
            }
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

