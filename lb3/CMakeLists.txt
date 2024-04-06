cmake_minimum_required(VERSION 3.27.8)
project(lb3)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_LIBRARY_PATH lib/)
#
#find_package(GLEW REQUIRED PATHS )
#include_directories(${GLEW_INCLUDE_DIRS})
#link_libraries(${GLEW_LIBRARIES})
find_package(OpenGL REQUIRED)
#
## Specify the path to GLFW include directory
#include_directories(lib/glfw-3.4.bin.WIN64/include)
#
## Specify the path to GLFW library directory
#link_directories(lib/glfw-3.4.bin.WIN64/lib-mingw-w64)
#
## Add your executable
add_executable(lb3 main.cpp
#        text_renderer.cpp
#        text_renderer.h
#        Button.cpp
        Button.h
        #        "backup 6.cpp"
        #[["draw rect.cpp"]]
        #[[main_cb.cpp]])

# Link against GLFW
#target_link_libraries(lb2 glfw3)
include_directories( ${OPENGL_INCLUDE_DIRS} )
target_link_libraries(lb3 ${OPENGL_LIBRARIES} winmm)