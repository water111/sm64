cmake_minimum_required(VERSION 3.0)

# Build settings
add_definitions(
        -DNON_MATCHING          # Decomp setting required to build.
        -DAVOID_UB              # Decomp setting to remove undefined behavior
        -DVERSION_US            # Decomp setting for ROM version
        -DNO_GODDARD            # Remove GODDARD (compatible with N64)
        -DDISABLE_SOUND         # Remove audio
        -DPC_PORT               # PC Port
        -DPC_PORT_GFX           # Use OpenGL renderer
        -DPRINT_LEVEL_SCRIPT    # Debug print the current level script.
)

include_directories(
        # Normal Includes
        include/
        src/
        .        # for actors/... includes to work properl

        # Includes to get assets from the decomp
        build/us/
        build/us/include
)

set(CMAKE_C_FLAGS "-O3 -ggdb -Wall -Wextra -Wno-implicit-fallthrough")
set(CMAKE_CXX_FLAGS "-O3 -ggdb -Wall -Wextra")
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(actors)
add_subdirectory(bin)
add_subdirectory(data)
add_subdirectory(levels)
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(pc)

# Assets
add_library(assets
        SHARED
        build/us/assets/demo_data.c
        build/us/assets/mario_anim_data.c)

add_library(pc_assets
        SHARED
        ./pc_assets/bitfs_skybox.c
        ./pc_assets/wdw_skybox.c
        ./pc_assets/clouds_skybox.c
        ./pc_assets/ssl_skybox.c
        ./pc_assets/ccm_skybox.c
        ./pc_assets/gSoundDataADSR.c
        ./pc_assets/bbh_skybox.c
        ./pc_assets/gMusicData.c
        ./pc_assets/water_skybox.c
        ./pc_assets/cloud_floor_skybox.c
        ./pc_assets/bits_skybox.c
        ./pc_assets/gSoundDataRaw.c
        ./pc_assets/gBankSetsData.c
        ./pc_assets/bidw_skybox.c
    )

add_executable(mario main.cpp)

target_link_libraries(mario
        game        # The "src/game" folder.
        actors      # The "actors" folder.
        bin         # The "bin" folder.
        data        # The "data" folder.
        levels      # The "levels" folder.
        real_os     # The "lib" folder.
        audio       # The "src/audio" folder.
        buffers     # The "src/buffers" folder.
        engine      # The "src/engine" folder.
        goddard     # The "src/goddard" folder.
        menu        # The "src/menu" folder.
        os_stubs    # OS Functions in Assembly stubs
        assets      # Stuff in bulid/us/assets/*.c
        pc_assets   # Stuff extracted by build_pc_assets.py
        GL
        glfw
        GLEW
        imgui
        )