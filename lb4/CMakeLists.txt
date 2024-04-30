cmake_minimum_required(VERSION 3.27.8)
project(latest_version)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")

find_package(OpenGL REQUIRED)

add_executable(latest_version main.cpp Button.h
        Character.h
        LevelRenderer.h)

include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(latest_version ${OPENGL_LIBRARIES} winmm)

#message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}")

# Copy DLLs to the output directory
add_custom_command(TARGET latest_version POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "O:\\Program Files\\CodeBlocks\\libwinpthread-1.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/libwinpthread-1.dll"
        $<TARGET_FILE_DIR:latest_version>
)


#[[
cmake_minimum_required(VERSION 3.27.8)
project(lb3)

set(CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")

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
        #[=["draw rect.cpp"]=]
        #[=[main_cb.cpp]=])

# Link against GLFW
#target_link_libraries(lb2 glfw3)
include_directories( ${OPENGL_INCLUDE_DIRS} )
target_link_libraries(lb3 ${OPENGL_LIBRARIES} winmm -lpthread)]]
