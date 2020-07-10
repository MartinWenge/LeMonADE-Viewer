/*--------------------------------------------------------------------------------
 _______________
|   /       \   | L   attice-based  | LeMonADE-Viewer:
|  /   ooo   \  | e   xtensible     | ----------------
| /  o\.|./o  \ | Mon te-Carlo      | An Open Source Visualization Tool for
|/  o\.\|/./o  \| A   lgorithm and  | for BFM-files generated by LeMonADE-software.
|  oo---0---oo  | D   evelopment    | See: https://github.com/LeMonADE-project/LeMonADE
|\  o/./|\.\o  /| E   nvironment    |
| \  o/.|.\o  / | -                 |
|  \   ooo   /  | Viewer            | Copyright (C) 2013-2015 by
|___\_______/___|                   | LeMonADE-Viewer Principal Developers (see AUTHORS)
----------------------------------------------------------------------------------

This file is part of LeMonADE-Viewer.

LeMonADE-Viewer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

LeMonADE-Viewer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with LeMonADE-Viewer.  If not, see <http://www.gnu.org/licenses/>.

--------------------------------------------------------------------------------*/

#include <cmath>                        // isnan
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include <LeMonADE/core/ConfigureSystem.h>
#include <LeMonADE/core/Ingredients.h>
#include <LeMonADE/feature/FeatureMoleculesIO.h>
#include <LeMonADE/feature/FeatureAttributes.h>
#include <LeMonADE/utility/RandomNumberGenerators.h>

#include "LeMonADE-Viewer/LeMonADEViewer.h"
#include "LeMonADE-Viewer/FeatureVisualize.h"
#include "LeMonADE-Viewer/generatePovRayScript.h"



void printHelp( void )
{
    std::cout
    << "Usage: ./LeMonADE-Viewer [--alpha] [--camera <phi/yaw> [<theta/pitch> [<distance> [<xshift> [<yshift>] [<roll>]]]]] ] [--help] [--script [<semicolon or newline separated commands>]] [--size <width> <height>] [--povray] ./LeMonADE-Viewer filename\n"
    << "\n"
    << "  --alpha Makes background transparent instead of black\n"
    << "  --camera specify the camera position. All angles given are in degree. Phi/yaw is the horizontal and theta/pitch the vertical rotation. The last argument is the distance to the box center with the default being 2*<box size>.\n"
    << "  --help   print this help message\n"
    << "  --script Automatically execute the specified commands. See the GUI !help command for a more up-to-date list of commands\n"
    << "      !setColor:idx1-idx2=(r,g,b)            \n"
    << "      !setColor:all=(r,g,b)                  \n"
    << "      !setColor:BG=(r,g,b)                   \n"
    << "      !setColorAttributes:att=(r,g,b)        \n"
    << "      !setColorLinks:numLinks=(r,g,b)        \n"
    << "      !setColorVisibility:vis=(r,g,b)        \n"
    << "      !setColorGroups:idxGroup=(r,g,b)       \n"
    << "      !setColorGroupsRandom                  \n"
    << "      !setColorTopology:(rS,gS,bS)=(rE,gE,bE)\n"
    << "      !setVisible:idx1-idx2=vis              \n"
    << "      !setVisible:all=vis                    \n"
    << "      !setVisibleAttributes:att=vis          \n"
    << "      !setVisibleLinks:numLinks=vis          \n"
    << "      !setVisibleGroups:idxG1-idxG2=vis      \n"
    << "      !setRadius:idx1-idx2=radius            \n"
    << "      !setRadius:all=radius                  \n"
    << "      !setRadiusAttributes:att=radius        \n"
    << "      !setRadiusLinks:numLinks=radius        \n"
    << "      !setRadiusGroups:idxGroup=radius       \n"
    << "  --size Window and rendered image size in pixels. Default is 800x600.\n"
    << "  --povray Don't start the GUI, just create and execute the povray script. Useful for use in scripts.\n"
    << std::endl;
}


int parseNextArg
(
    int argc,
    char * argv[],
    int * const iArg,
    std::string const & name,
    double * const pResult
)
{
    char * parseEnd;
    if ( *iArg+1 < argc )
    {
        double tmp = strtod( argv[ *iArg+1 ], &parseEnd );
        if ( argv[ *iArg+1 ] != parseEnd )
        {
            ++(*iArg);
            *pResult = tmp;
            std::cerr << "Read " << name << " = " << *pResult << "\n";
        }
        else
        {
            std::cerr << "Failed to parse '" << argv[ *iArg+1 ] << "'\n";
            return 1;
        }
    }
    return 0;
};

int main( int argc, char * argv[] )
{
	try
    {
        /* Command line argument parser */
        std::string filename;
        double cameraPhi      = 0;
        double cameraTheta    = 0;
        double cameraRoll     = 0;
        double cameraDistance = std::numeric_limits< float >::quiet_NaN();
        double cameraXShift   = 0;
        double cameraYShift   = 0;
        bool onlyPovray   = false;
        bool customCamera = false;
        bool customSize   = false;
        bool alphaBackground = false;
        std::string script;
        unsigned int width = 800, height = 800;
        for ( int iArg = 1 /* ignore program name */; iArg < argc; ++iArg )
        {
            if ( argv[ iArg ][0] == '-' )
            {
                std::string sarg( argv[iArg] );
                if ( sarg == "--alpha" )
                    alphaBackground = true;
                else if ( sarg == "--camera" )
                {
                    customCamera = true;
                    if ( parseNextArg( argc, argv, &iArg, "cameraPhi"     , &cameraPhi      ) ) continue;
                    if ( parseNextArg( argc, argv, &iArg, "cameraTheta"   , &cameraTheta    ) ) continue;
                    if ( parseNextArg( argc, argv, &iArg, "cameraDistance", &cameraDistance ) ) continue;
                    if ( parseNextArg( argc, argv, &iArg, "cameraXShift"  , &cameraXShift   ) ) continue;
                    if ( parseNextArg( argc, argv, &iArg, "cameraYShift"  , &cameraYShift   ) ) continue;
                    if ( parseNextArg( argc, argv, &iArg, "cameraRoll"    , &cameraRoll     ) ) continue;
                }
                else if ( sarg == "--povray" )
                    onlyPovray = true;
                else if ( sarg == "--script" )
                    script = argv[ ++iArg ];
                else if ( sarg == "--size" )
                {
                    customSize = true;
                    char * parseEnd;
                    if ( iArg+1 < argc && argv[ iArg+1 ][0] != '-' )
                    {
                        ++iArg;
                        width = strtod( argv[ iArg ], &parseEnd );
                        if ( argv[ iArg ] == parseEnd )
                            std::cerr << "Couldn't parse width '" << argv[iArg] << "' to --size option!\n";
                    }
                    if ( iArg+1 < argc && argv[ iArg+1 ][0] != '-' )
                    {
                        ++iArg;
                        height = strtod( argv[ iArg ], &parseEnd );
                        if ( argv[ iArg ] == parseEnd )
                            std::cerr << "Couldn't parse height '" << argv[iArg] << "' to --size option!\n";
                    }
                }
                else if ( sarg == "-h" || sarg == "--help" )
                    printHelp();
            }
            else if ( filename.empty() )
            {
                std::cerr << "filename = " << filename << "\n";
                filename = argv[ iArg ];
            }
            else
            {
                printHelp();
                std::cerr << "Invalid number of arguments specified! Can't interpret argument " << iArg << " (" << argv[ iArg ] << "). Filename is already '" << filename << "'\n";
                return 1;
            }
        }
        if ( filename.empty() )
            filename = "testfile.bfm";

        /* start main loop or create povray script */
        //just as in the previous examples, we quickly define the features and basic
        //monomer type and create an instance of the thus defined system.

        typedef LOKI_TYPELIST_3(FeatureMoleculesIO,FeatureVisualize,FeatureAttributes< >) Features;

        // in most cases the number of connections of a monomer dosn't exceed 8 bonds
        const uint max_bonds=20;

        typedef ConfigureSystem<VectorInt3,Features,max_bonds> Config;
        typedef Ingredients<Config> IngredientsSystem;

        //seed the globally available random number generators
        RandomNumberGenerators random;
        random.seedAll();
		IngredientsSystem MyLeMonADESystem;


		LeMonADEViewer< IngredientsSystem > ShowLeMonADEViewer( MyLeMonADESystem, filename, onlyPovray );

		ShowLeMonADEViewer.initialize(); // initializes BFM file reader, FLTK window, OpenGL window, command parser
        if ( ! script.empty() )
        {
            /* split script at ; and \n and execute each command */
            int i1 = 0; // last char already inspected in string
            while ( true )
            {
                /* advance until no delimiter, meaning skip over empty lines */
                while ( i1 < script.size() && !( script[i1] != '\n' && script[i1] != ';' ) )
                    ++i1;
                if ( ! ( i1 < script.size() ) )
                    break;
                int i0 = i1;
                /* one increment is guaranteed because of the above loop */
                while ( i1 < script.size() && script[i1] != '\n' && script[i1] != ';' )
                    ++i1;
                std::string const command = script.substr( i0, i1-i0-1 );

                /* execute command */
                ShowLeMonADEViewer.executeCommand( command );
            }
        }
        /* set up camera */
        if ( customCamera )
        {
            Camera & cam = ShowLeMonADEViewer.modifyWinOpenGL()->modifyCamera();
            cam.setCamAngleYaw  ( cameraPhi   );
            cam.setCamAnglePitch( cameraTheta );
            cam.setCamAngleRoll ( cameraRoll  );
            if ( std::isnan( cameraDistance ) )
            {
                cameraDistance = -2 * ShowLeMonADEViewer.getIngredients().getBoxY();
                std::cerr << "No camera distance was given, setting it to default: " << cameraDistance << "\n";
            }
            std::cerr << "Setting camera y pos to " << cameraDistance << "\n";
			cam.setCamYPos( cameraDistance );
			cam.setCamXPos( cameraXShift );
			cam.setCamZPos( cameraYShift );
        }
        if ( customSize )
            ShowLeMonADEViewer.modifyWinOpenGL()->size( width, height );
        if ( onlyPovray )
        {
            std::cout << "generate POV-Ray-script" << std::endl;
            std::string fnameWoExt = filename;
            {
                fnameWoExt.erase( fnameWoExt.length()-4, fnameWoExt.length());
                unsigned found = fnameWoExt.find_last_of( "/\\" );
                fnameWoExt = fnameWoExt.substr( found+1 );
            }
            generatePovRayScript( ShowLeMonADEViewer.getIngredients(), fnameWoExt, width, height, ShowLeMonADEViewer.getWinOpenGL()->getCamera() );

            std::stringstream povrayCommand;
            povrayCommand << "povray -d +I" << fnameWoExt << ".pov +O" << fnameWoExt << ".png +W" << width << " +H" << height << ( alphaBackground ? " +UA" : "" );
            std::cout << "execute command: " << povrayCommand.str() << std::endl;
            system( povrayCommand.str().c_str() );
        }
        else
            ShowLeMonADEViewer.execute(); // start main window loop
		ShowLeMonADEViewer.cleanup();

	}
	catch(std::runtime_error& err){std::cerr<<err.what();}

	return 0;
}
