add_subdirectory(imgui)
add_library(os_stubs
        SHARED
        os_stubs.c
        controller/M64Reader.cpp
        graphics/pc_gfx.cpp
        graphics/OpenGLRenderer.cpp
        graphics/DisplayList.cpp
        graphics/Window.cpp
        cart/Cartrdige.cpp)

