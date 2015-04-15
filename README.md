# Return of Dr. Destructo

Return of Dr. Destructo is a cross-platform, open-source remake of ZX Spectrum game "Island of Dr. Destructo" (1987).

Binary downloads for the game could be found at: http://zxstudio.org/projects/drdestructo/

Source code for this game is licensed under MIT license and art is CC-BY-SA. Please consult docs/LICENSE-CODE.txt and docs/LICENSE-ART.txt for details.

## Building

Building Return of Dr. Destructo is not a very easy process, because it was mostly custom-tailored for my own needs and tastes, and also to finally release the damn thing :)

I use CMake for build system, and the only non-standard dependency which is not provided along with game's sources is Allegro game library (http://allegro.cc).

Unfortunately, you will need a custom patch for Allegro library, which is provided in the root folder. Also, the game expects Allegro to be built in a particular manner. Please enable WANT_MONOLITH for all platforms. For Linux and MacOS X builds, a statically linked build is expected.

After building Allegro with the patch, you should place the custom binaries and headers into project's root directory:

### Windows:

allegro_win32\bin - DLLs

allegro_win32\include - header files

allegro_win32\lib - .lib files

### Linux & MacOS X:

allegro_unix\include - header files

allegro_unix\lib - .a files

## A note on building for MacOS X with XCode:

I only tested Makefile-based build for MacOS X. Though the game must build with XCode too, I do not guarantee it will run without further tweaking of project.