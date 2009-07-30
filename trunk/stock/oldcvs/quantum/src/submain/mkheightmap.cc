/***************************************************************************
 *            mkheightmap.cc
 *
 *  Copyright  2008  Sebastian Mach
 *  seb@greenhybrid.net
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/// \todo add an [optional] alpha channell to the heightmap, so it becomes usefull for creating 2d clouds
/// \todo include IEEE-floating-point binary-export option
/// \todo include RAW-export option
/// \todo include PNG-export option
/// \todo include more visualisation modes than only greyscale
/// \todo include an option to only display a subset of the map
/// \todo include an option to zoom in/out
/// \todo include ram-less heightmap creation (will be slower when using normalization)
/// \todo include an option to sequentially save the heightmap, one chunk by another
/// \todo include options to set the x/y-center on the height-function
/// \todo This actually a project global todo: get rid of stuff like FLT_MAX, we want our project be portable.


#include <stdio.h>
#include <ctype.h>

#include <iostream>

#include <SDL/SDL.h>

#include <picogen/picogen.h>
#include <picogen/errors.h>


using picogen::real;
using picogen::geometrics::Vector3d;



template <typename T> struct Heightmap {

    public:

        typedef T Hexel; // yes, hexel, as in pixel :P

    private:

    public:

        /// \todo hide member variables away into private section
        const real scaling;
        const real addx, addz;
        const unsigned int width;
        const unsigned int height;

        Hexel *map; // this is going to be a little bit low-level (as compared to operator [] overloading stuff)

        Heightmap (real scaling, unsigned int width, unsigned int height)
        : scaling (scaling), addx (0), addz (0),
          width (width), height (height), map (new Hexel[width*height])
        { }



        ~Heightmap() {
            delete [] map;
        }



        // okay, we provide an operator (), but just for screen dumping (hence i const it)
        const Hexel operator () ( unsigned int x, unsigned int y ) const {
            const Hexel tmp = map[ ((width-1)-y)*width + x ];
            return tmp < 0.0 ? 0.0 : tmp > 1.0 ? 1.0 : tmp;
        }



        void fillZero() {
            for (unsigned int x=0; x<width*height; x++) {
                map[ x ] = static_cast<Hexel>(0.0);
            }
        }



        void fill (const picogen::misc::functional::Function_R2_R1 &f, unsigned int antiAliasFactor) {

            using namespace std;

            fillZero();

            for (unsigned int y=0; y<height*antiAliasFactor; y++) {

                const unsigned int ofs_y = (y/antiAliasFactor)*width;
                const real v = scaling * (addz+(static_cast<real>(y) / static_cast<real>(width*antiAliasFactor)));

                for (unsigned int x=0; x<width*antiAliasFactor; x++) {

                    const real u = scaling * (addx+(static_cast<real>(x) / static_cast<real>(width*antiAliasFactor)));
                    map[ (x/antiAliasFactor) + ofs_y ] += static_cast<Hexel>(f(u,v)) / static_cast<Hexel>(antiAliasFactor*antiAliasFactor);

                }
            }
        }



        void normalize () {
            /// \todo Rewrite to 1d as in fillZero()
            using namespace std;
            Hexel minHeight_(map[0]);
            Hexel maxHeight_(map[0]);

            for (unsigned int y=0; y<height; y++) {
                const unsigned int ofs_y = y*width;
                for (unsigned int x=0; x<width; x++) {
                    const Hexel & m = map[ x + ofs_y ];
                    if( m<minHeight_ )
                        minHeight_ = m;
                    if( m>maxHeight_ )
                        maxHeight_ = m;
                }
            }

            const Hexel minHeight(minHeight_);
            const Hexel totalHeight   (1.0 / (maxHeight_ - minHeight_));
            for (unsigned int y=0; y<height; y++) {
                const unsigned int ofs_y = y*width;
                for (unsigned int x=0; x<width; x++) {
                    Hexel &m = map[ x + ofs_y ];
                    m = ( m - minHeight ) * totalHeight;
                }
            }
        }



        void printInfo () {
            using namespace std;
            Hexel minHeight_(map[0]);
            Hexel maxHeight_(map[0]);

            for (unsigned int y=0; y<height; y++) {
                const unsigned int ofs_y = y*width;
                for (unsigned int x=0; x<width; x++) {
                    const Hexel & m = map[ x + ofs_y ];
                    if( m<minHeight_ )
                        minHeight_ = m;
                    if( m>maxHeight_ )
                        maxHeight_ = m;
                }
            }

            std::cout << "info:min..max:" << minHeight_ << ".."  << maxHeight_ << std::endl;
        }

};




/// \todo strip down function draw
template<class t_surface>
void
draw (
    SDL_Surface *p_target,
    const t_surface &surface,
    float scale,
    float exp_tone,
    float saturation,
    float waterLevel
) {
    if (SDL_MUSTLOCK (p_target) && SDL_LockSurface (p_target) <0)
        return;
    int x,y;
    for (y=0; y<p_target->h; y++) {
        /// \todo FIX we are currently assuming a 32bit SDL buffer
        /// \todo get rid of pointer arithmetic
        Uint32 *bufp   = (Uint32*) p_target->pixels + y* (p_target->pitch>>2);
        for (x=0; x<p_target->w; x++) {

            real accu_r=real (0), accu_g=real (0), accu_b=real (0);
            const real h_ = surface (x,y);
            const real h  = h_<0.0 ? 0.0 : h_>1.0 ? 1.0 : h_;

            if (h<waterLevel) {
                accu_r = 0.3;
                accu_g = 0.3;
                accu_b = 0.8;
            } else {
                accu_r = accu_g = accu_b = h;

                /*
                unsigned int tmp = static_cast<unsigned int> (h * 255.0 * 255.0 * 255.0);
                accu_r = static_cast<real> ( ( tmp >> 16 ) & 0xFF ) / 255.0;
                accu_g = static_cast<real> ( ( tmp >>  8 ) & 0xFF ) / 255.0;
                accu_b = static_cast<real> ( ( tmp >>  0 ) & 0xFF ) / 255.0;
                */
            }


            * (bufp++) =
                SDL_MapRGB (p_target->format,
                            (unsigned char) (255.0*accu_r),
                            (unsigned char) (255.0*accu_g),
                            (unsigned char) (255.0*accu_b)
                           );
        }
    }


    if (SDL_MUSTLOCK (p_target))
        SDL_UnlockSurface (p_target);
    SDL_Flip (p_target);
}



template <typename T> picogen::error_codes::code_t showPreviewWindow (T &heightmap, real waterLevel) {
    using namespace std;
    using namespace picogen::error_codes;

    if (SDL_Init (SDL_INIT_VIDEO) < 0) {
        cerr << "Unable to init SDL for preview: " << SDL_GetError() << endl;
        return mkheightmap_sdl_not_initialised;
    }
    atexit (SDL_Quit);
    SDL_Surface *screen = SDL_SetVideoMode (heightmap.width, heightmap.height, 32, SDL_HWSURFACE);
    if (0 == screen) {
        cerr << "Unable to set video-mode for preview: " << SDL_GetError() << endl;
        return mkheightmap_set_video_mode_failed;
    }

    // dump the heightmap onto the screen
    draw (screen, heightmap, 1.0, 1.0, 1.0, waterLevel);

    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent (&event)) {
            if (event.type == SDL_QUIT) {
                done = true;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    done = true;
                }
            }
        }
    }

    SDL_FreeSurface (screen);
    SDL_Quit();
    return mkheightmap_okay;
}



void drawShader (
    SDL_Surface *p_target, 
    const ::picogen::graphics::material::abstract::IShader & shader, 
    const picogen::misc::functional::Function_R2_R1 & h
) {
    
    using picogen::graphics::color::Color;
    using picogen::geometrics::Vector3d;
    
    if (SDL_MUSTLOCK (p_target) && SDL_LockSurface (p_target) <0)
            return;

    int x,y;
    for (y=0; y<p_target->h; y++) {
        
        /// \todo FIX we are currently assuming a 32bit SDL buffer
        /// \todo get rid of pointer arithmetic

        Uint32 *bufp   = (Uint32*) p_target->pixels + y* (p_target->pitch>>2);
        const real fy = 1.0 - static_cast <real> (y) / static_cast <real> (p_target->h);

        for (x=0; x<p_target->w; x++) {

            const real fx = static_cast <real> (x) / static_cast <real> (p_target->w);

            real r, g, b;            
            Color color;
            Vector3d normal (0.0, 1.0, 0.0); // Not like that!
            Vector3d position (fx, h (fx, fy), fy);
            shader.shade (color, normal, position);
            
            color.saturate (Color(0,0,0), Color(1,1,1)).to_rgb (r,g,b);

            * (bufp++) =
                SDL_MapRGB (p_target->format,
                            (unsigned char) (255.0*r),
                            (unsigned char) (255.0*g),
                            (unsigned char) (255.0*b)
                           );
        }
    }


    if (SDL_MUSTLOCK (p_target))
        SDL_UnlockSurface (p_target);
    SDL_Flip (p_target);
}



picogen::error_codes::code_t showShaderPreviewWindow (
    const ::picogen::graphics::material::abstract::IShader & shader, 
    const picogen::misc::functional::Function_R2_R1 & h, 
    const int width, const int height
) {
    using namespace std;
    using namespace picogen::error_codes;

    if (SDL_Init (SDL_INIT_VIDEO) < 0) {
        cerr << "Unable to init SDL for preview: " << SDL_GetError() << endl;
        return mkheightmap_sdl_not_initialised;
    }
    atexit (SDL_Quit);
    SDL_Surface *screen = SDL_SetVideoMode (width, height, 32, SDL_HWSURFACE);
    if (0 == screen) {
        cerr << "Unable to set video-mode for preview: " << SDL_GetError() << endl;
        return mkheightmap_set_video_mode_failed;
    }

    // dump the heightmap onto the screen
    drawShader (screen, shader, h);

    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent (&event)) {
            if (event.type == SDL_QUIT) {
                done = true;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    done = true;
                }
            }
        }
    }

    SDL_FreeSurface (screen);
    SDL_Quit();
    return mkheightmap_okay;
}



template <typename T> std::string exportText (T &heightmap, const std::string &outputFilename, bool forceOverwrite) {
    using namespace std;

    // Check if file already exists.
    if (!forceOverwrite) {
        FILE *f = fopen (outputFilename.c_str(), "r");
        if (0 != f) {
            fclose (f);
            return "File '" + outputFilename + "' already exists. Remove it or invoke the -f option to force overwrite.";
        }
    }

    // Open file for writing.
    FILE *f = fopen (outputFilename.c_str(), "w");
    if (0 == f) {
        return "Could not open '" + outputFilename + "' for writing.";
    }

    fprintf (f, "width:%u\nheight:%u\nvalues:",
        static_cast<unsigned int> (heightmap.width),
        static_cast<unsigned int> (heightmap.height)
    );

    for (unsigned int y=0; y<heightmap.height; ++y) {
        for (unsigned int x=0; x<heightmap.width; ++x) {
            const float h = static_cast<float> (heightmap(x,y));
            fprintf (f, "%f ", h);
        }
    }

    fclose (f);
    return "";
}



template <typename T> std::string exportWinBMP (T &heightmap, const std::string &outputFilename, bool forceOverwrite) {
    using namespace std;

    // Check if file already exists.
    if (!forceOverwrite) {
        FILE *f = fopen (outputFilename.c_str(), "r");
        if (0 != f) {
            fclose (f);
            return "File '" + outputFilename + "' already exists. Remove it or invoke the -f option to force overwrite.";
        }
    }

    if (SDL_Init (SDL_INIT_VIDEO) < 0) {
        return "Unable to init SDL for bitmap export: " + string (SDL_GetError());
    }

    atexit (SDL_Quit);
    SDL_Surface *surface = SDL_CreateRGBSurface (SDL_SWSURFACE, heightmap.width, heightmap.height, 32,
        0xFF000000, 0x00FF0000, 0x0000FF00, 0x00000000);
    if (0 == surface) {
        return "Unable to create surface for bitmap export: " + string (SDL_GetError());
    }

    draw (surface, heightmap, 1.0, 1.0, 1.0, -10000000);
    SDL_SaveBMP (surface, outputFilename.c_str());
    SDL_FreeSurface (surface);
    SDL_Quit();
    return "";
}



static void printUsage() {
    std::cout
        << "Invocation of mkheightmap: picogen mkheightmap [options]\n"
        << "In the version of picogen you have installed, the following options are possible:\n"
        << "\n"
        << "-Lhs / --Lheight-slang <program : string>:  -define an input program in \n"
        << "                                             height-slang syntax.\n"
        << "                                             (see http://picogen.org)\n"
        << " -En / --ExportName <name : string>:        -define a basis-name for exported \n"
        << "                                             files. \n"
        << "                                             default is 'mkheightmap-out'\n"
        << "-Et / --Etext:                              -export as plaintext. \n"
        << "                                             the format is self-explanatory.\n"
        << "-Ebmp / --Ebitmap:                          -export as standard bmp file.\n"
        << "-p / --preview:                             -show a window with the heightmap.\n"
        << "-l / --preview-water-level <L : floating point>:-set a water level for preview\n"
        << "                                             (only useful together with \n"
        << "                                              --preview)\n"
        << "-f / --force-overwrite:                     -force overwrite of existing files.\n"
        << "-w / --width <positive integer>:            -set target width, in pixels. \n"
        << "                                             standard is 512.\n"
        << "-h / --height <positive integer>:           -set target height, in pixels. \n"
        << "                                             standard is 512.\n"
        << "-a / --anti-aliasing <X : positive integer>:-enable X*X antialiasing.\n"
        << "                                             standard is 1.\n"
        << "-W / --domain-width <S : floating point>:   -scales the input coordinates from \n"
        << "                                             [0..1)x[0..1) to [0..S)x[0..S)\n"
        << "-n / --normalize :                          -enable normalization, that is, \n"
        << "                                             scale the height-values linearly \n"
        << "                                             so that all values are in [0..1)\n"
        << "-i / --info :                               -print some information about the \n"
        << "                                             function. \n"
        << "--help : show this help and exit"
    << std::endl;
}



picogen::error_codes::code_t main_mkheightmap (int argc, char *argv[]) {
    using ::picogen::real;
    using ::picogen::geometrics::Vector3d;
    using ::picogen::geometrics::Ray;
    using ::picogen::geometrics::BoundingBox;
    
    --argc;
    ++argv;

    using namespace std;
    using namespace picogen::misc::functional;
    using namespace picogen::error_codes;

    if (argc<=0) {
        cerr << "error: no arguments given to `picogen mkheightmap`" << endl;
        printUsage();
        return mkheightmap_no_options_given;
    }

    /// \todo MINOR Someone please cleanup the following line.
    typedef enum Lingua_t {
        Lingua_inlisp,
        Lingua_unknown
    } Lingua;
    std::string code(""), shader_code ("");
    Lingua lingua = Lingua_unknown;
    bool showPreview = false;
    unsigned int width=512, height=512;
    float scaling = 1.0;
    float waterLevel = -10000000.0;
    unsigned int antiAliasFactor = 1;
    //float heightmapNormalizationAccuracy = -1.0;
    bool doNormalize = false;
    bool doPrintInfo = false;
    string exportFileNameBase ("mkheightmap-out");
    bool forceOverwriteFiles = false;

    struct ExportFlags {
        bool text : 1;
        bool winBMP : 1;
        ExportFlags ()
        : text(false), winBMP(false)
        {}
    };

    ExportFlags exportFlags;
    try {
        while (argc>0) {
            const std::string option (argv[0]);
            argc--;
            argv++;

            /// \todo add an option to specifiy the output-filename

            // -Lx
            if (option[1] == 'L' && Lingua_unknown != lingua) {
                // We already have read some code, more is not allowed.
                cerr << "error: only one formula or program allowed" << endl;
                printUsage();
                return picogen::error_codes::mkheightmap_bad_option_syntax;
            }
            if (option == "-Lhs" || option == "--Lheight-slang") {
                lingua = Lingua_inlisp;
                if (argc<=0) {
                    cerr << "error: no argument given to option: " << option << endl;
                    printUsage();
                    return picogen::error_codes::mkheightmap_bad_option_syntax;
                }
                code = string (argv[0]);
                argc--;
                argv++;
            } else if (option == "-s" || option == "--shader") {
                if (argc<=0) {
                    cerr << "error: no argument given to option: " << option << endl;
                    printUsage();
                    return picogen::error_codes::mkheightmap_bad_option_syntax;
                }
                shader_code = string (argv[0]);
                argc--;
                argv++;
            } else if (option == "-En" || option == "--ExportName") {
                if (argc<=0) {
                    cerr << "error: no argument given to option: " << option << endl;
                    printUsage();
                    return picogen::error_codes::mkheightmap_bad_option_syntax;
                }
                exportFileNameBase = string (argv[0]);
                argc--;
                argv++;
            } else if (option == "-Et" || option == "--Etext" ) {
                    exportFlags.text = true;
            } else if (option == "-Ebmp" || option == "--Ebitmap" ) {
                    exportFlags.winBMP = true;
            } else if (option == "-p" || option == "--preview") {
                showPreview = true;
            } else if (option == "-f" || option == "--force-overwrite") {
                forceOverwriteFiles = true;
            } else if (option == "-w"  || option == "--width") {

                if (argc<=0) {
                    cerr << "error: no argument given to option: " << option << endl;
                    printUsage();
                    return picogen::error_codes::mkheightmap_bad_option_syntax;
                }
                const string intStr = string (argv[0]);
                argc--;
                argv++;
                // check for unsigned-integer'ness
                for (string::const_iterator it = intStr.begin(); it!=intStr.end(); ++it) {
                    if (!isdigit (*it)) {
                        cerr << "error: only positive integer numbers are allowed for option " << option << endl;
                        printUsage();
                        return picogen::error_codes::mkheightmap_bad_option_syntax;
                    }
                }
                /// \todo get rid of below sscanf
                sscanf (intStr.c_str(), "%u", &width);

            } else if (option == "-h"  || option == "--height") {
                if (argc<=0) {
                    cerr << "error: no argument given to option: " << option << endl;
                    printUsage();
                    return picogen::error_codes::mkheightmap_bad_option_syntax;
                }
                const string intStr = string (argv[0]);
                argc--;
                argv++;
                // check for unsigned-integer'ness
                for (string::const_iterator it = intStr.begin(); it!=intStr.end(); ++it) {
                    if (!isdigit (*it)) {
                        cerr << "error: only positive integer numbers are allowed for option " << option << endl;
                        printUsage();
                        return picogen::error_codes::mkheightmap_bad_option_syntax;
                    }
                }
                /// \todo get rid of below sscanf
                sscanf (intStr.c_str(), "%u", &height);
            } else if (option == "-a"  || option == "--anti-aliasing") {
                if (argc<=0) {
                    cerr << "error: no argument given to option: " << option << endl;
                    printUsage();
                    return picogen::error_codes::mkheightmap_bad_option_syntax;
                }
                const string intStr = string (argv[0]);
                argc--;
                argv++;
                // check for unsigned-integer'ness
                for (string::const_iterator it = intStr.begin(); it!=intStr.end(); ++it) {
                    if (!isdigit (*it)) {
                        cerr << "error: only positive integer numbers >= 1 are allowed for option " << option << endl;
                        printUsage();
                        return picogen::error_codes::mkheightmap_bad_option_syntax;
                    }
                }
                /// \todo get rid of below sscanf
                sscanf (intStr.c_str(), "%u", &antiAliasFactor);
                if (antiAliasFactor<1) {
                    cerr << "error: only positive integer numbers >= 1 are allowed for option " << option << endl;
                    printUsage();
                    return picogen::error_codes::mkheightmap_bad_option_syntax;
                }
            } else if (option == "-W"  || option == "--domain-width") {
                if (argc<=0) {
                    cerr << "error: no argument given to option: " << option << endl;
                    printUsage();
                    return picogen::error_codes::mkheightmap_bad_option_syntax;
                }
                const string intStr = string (argv[0]);
                argc--;
                argv++;

                // Check for correct format.
                bool hasDot = false;
                string::const_iterator it = intStr.begin();
                if (*it == '-' ) {
                    ++it;
                }
                for ( ; it!=intStr.end(); ++it) {
                    if (*it == '.') {
                        if (hasDot) {
                            cerr << "error: wrong format for option " << option << " (too many dots)" << endl;
                            printUsage();
                            return picogen::error_codes::mkheightmap_bad_option_syntax;
                        }
                        hasDot = true;
                    } else if (!isdigit (*it)) {
                        cerr << "error: only floating point numbers are allowed for option " << option << endl;
                        printUsage();
                        return picogen::error_codes::mkheightmap_bad_option_syntax;
                    }
                }
                /// \todo get rid of below sscanf
                sscanf (intStr.c_str(), "%f", &scaling);

            } else if (option == "-l"  || option == "--preview-water-level") {
                if (argc<=0) {
                    cerr << "error: no argument given to option: " << option << endl;
                    printUsage();
                    return picogen::error_codes::mkheightmap_bad_option_syntax;
                }
                const string intStr = string (argv[0]);
                argc--;
                argv++;

                // Check for correct format.
                bool hasDot = false;
                string::const_iterator it = intStr.begin();
                if (*it == '-' ) {
                    ++it;
                }
                for ( ; it!=intStr.end(); ++it) {
                    if (*it == '.') {
                        if (hasDot) {
                            cerr << "error: wrong format for option " << option << " (too many dots)" << endl;
                            printUsage();
                            return picogen::error_codes::mkheightmap_bad_option_syntax;
                        }
                        hasDot = true;
                    } else if (!isdigit (*it)) {
                        cerr << "error: only floating point numbers are allowed for option " << option << endl;
                        printUsage();
                        return picogen::error_codes::mkheightmap_bad_option_syntax;
                    }
                }
                /// \todo get rid of below sscanf
                sscanf (intStr.c_str(), "%f", &waterLevel);
            } else if (option == "-n"  || option == "--normalize") {
                doNormalize = true;
                /*if (argc<=0) {
                    cerr << "error: no argument given to option: " << option << endl;
                    printUsage();
                    return -1;
                }
                const string intStr = string (argv[0]);
                argc--;
                argv++;
                // check for unsigned-integer'ness
                bool hasDot = false;
                for (string::const_iterator it = intStr.begin(); it!=intStr.end(); ++it) {
                    if (*it == '.') {
                        if (hasDot) {
                            cerr << "error: wrong format for option " << option << " (too many dots)" << endl;
                            printUsage();
                            return -1;
                        }
                        hasDot = true;
                    }else if (!isdigit (*it)) {
                        cerr << "error: only floating point numbers are allowed for option " << option << endl;
                        printUsage();
                        return -1;
                    }
                }
                /// \todo get rid of below sscanf
                sscanf (intStr.c_str(), "%f", &heightmapNormalizationAccuracy);
                if (heightmapNormalizationAccuracy < 0.0 || heightmapNormalizationAccuracy > 1.0) {
                    cerr << "error: the argument for option " << option << " must be >= 0.0 and <= 1.0 (where 0.0 being 0%, 1.0 being 100%)" << endl;
                    printUsage();
                    return -1;
                }*/
            } else if (option == "-i" || option == "--info") {
                doPrintInfo = true;
            } else if (option == string ("-?") || option == string ("?") || option == string ("--help")) {
                printUsage ();
                return picogen::error_codes::mkheightmap_okay;
            } else {
                cerr << "unknown option in argument list: " << option << endl;
                printUsage();
                return picogen::error_codes::mkheightmap_unknown_commandline_option;
            }
        }


       
        // TODO: hmm, not oo
        using ::picogen::graphics::material::abstract::IShader;
        extern IShader *parse_shader (const std::string &code);
        
        if (lingua == Lingua_unknown && 0 == shader_code.size()) {
            cerr << "error: unkown language or wrong option used" << endl;
            printUsage();
            return picogen::error_codes::mkheightmap_unknown_language;
        }
        
        if (0 == shader_code.size()) {
            if (code.size() == 0) {
                throw functional_general_exeption ("you gave me no code");
            }

            Heightmap<double> heightmap (scaling, width, height);
            heightmap.fill (Function_R2_R1 (code), antiAliasFactor);
            if (doNormalize)
                heightmap.normalize();

            if (doPrintInfo)
                heightmap.printInfo();

            #define MKHEIGHTMAP_EXPORT(FLAG,EXT,FUN)                                    \
            if (exportFlags.FLAG) {                                                     \
                const string outputFilename = exportFileNameBase + string(EXT);         \
                cerr << "Exporting as text to file '" + outputFilename + "' ...";       \
                std::string s = FUN (heightmap, outputFilename, forceOverwriteFiles);   \
                if (s != string("")) {                                                  \
                    cerr << "\nerror: " << s << endl;                                   \
                } else {                                                                \
                    cerr << "okay." << endl;                                            \
                }                                                                       \
            }

            MKHEIGHTMAP_EXPORT (text, ".txt", exportText );
            MKHEIGHTMAP_EXPORT (winBMP, ".bmp", exportWinBMP );


            if (showPreview) {
                picogen::error_codes::code_t c = showPreviewWindow (heightmap, waterLevel);
                return c;
            }
        
        } else {
                
            if (code.size() == 0) {
                code = "0.0";
            }           
            
            /*
            real accuracy = 10000.0 / static_cast<real>(resolution*resolution);
            QuadtreeHeightField *heightField = QuadtreeHeightField::create(
                width, Function_R2_R1 (code), 
                ::picogen::misc::BoundingBox (
                    ::picogen::misc::Vecto3d, 
                    (center+size*0.5)), 
                    accuracy, 
                    false
            );
            
            IShader *shader = parse_shader (code);

            if (0 != shader && 0 != heightField) {
                heightField->setBRDF (currentBRDF);                
                heightField->setShader (shader);
            
                if (showPreview) {
                    ;//return showShaderPreviewWindow (*shader, width, height);
                }
            }
            
            if (0 != shader) {
                delete shader;
            }
            if (0 != heightField) {
                delete heightField;
            }
            */
            
            IShader *shader = parse_shader (shader_code);
            picogen::error_codes::code_t ret = mkheightmap_okay;
            if (0 != shader) {
                if (showPreview) {
                    ret = showShaderPreviewWindow (*shader, Function_R2_R1 (code), width, height);
                }
            }            
            delete shader;
            return ret;
        }             

    } catch (const functional_general_exeption &e) {
        cerr << "error: " << e.getMessage() << endl;
        return picogen::error_codes::mkheightmap_syntax_error;
    } catch (...) {
        cerr << "unknown exception." << endl;
        return picogen::error_codes::mkheightmap_unknown_error;
    }
    return picogen::error_codes::mkheightmap_okay;
}