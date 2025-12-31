cmake_minimum_required(VERSION 3.16)
project(st2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf QUIET)
if(NOT SDL2_ttf_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2TTF REQUIRED SDL2_ttf)
    add_library(SDL2_ttf::SDL2_ttf INTERFACE IMPORTED)
    target_include_directories(SDL2_ttf::SDL2_ttf INTERFACE ${SDL2TTF_INCLUDE_DIRS})
    target_link_libraries(SDL2_ttf::SDL2_ttf INTERFACE ${SDL2TTF_LIBRARIES})
endif()

add_executable(st2
    main.cpp
    gui/gui.cpp
    core/runtime.cpp
)

target_include_directories(st2 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(st2 PRIVATE
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
)
