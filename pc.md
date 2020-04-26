Requirements
-------------
- Only tested with Linux (64-bit)
- CMake
- C and C++ compiler
- OpenGL
- GLEW
- glfw
- python3

You can probably get most things installed with
`sudo apt install cmake make build-essential libglu1-mesa-dev mesa-common-dev libglfw3-dev libglew-dev python3`


Building
-----------

- Run decomp on the US version (you should still get OK)
- Extract PC Port Assets: `python3 ./build_pc_assets.py` (this must be run from the `sm64` folder)
- Create build folder: `mkdir build_pc`
- Go into build folder: `cd build_pc`
- Run cmake: `cmake ..`
- Build `make -j`
- Acquire M64 file for inputs
(for example, the all 120 stars "Mupen64 movie (.m64)" here http://tasvideos.org/Game/n64-super-mario-64.html)
- Copy the M64 file into the build directory, and rename to `tas.m64`
- Run `./mario`


Bugs
-------
- The graphics are bad (consider using https://github.com/Emill/n64-fast3d-engine instead)
- The renderer is in progress and may crash or trigger an assert
- There is no audio (and the data extracted for audio is bad)
- There is no saving
- There are no inputs
- There is no Creepy Mario Head
- Coins and other items do not reload correctly when entering a level after the first time
