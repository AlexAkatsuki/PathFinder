cmake_minimum_required(VERSION 3.16)
project(PathFinder VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g0")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
endif()

find_package(Qt6Widgets REQUIRED)
find_package(Qt6Core REQUIRED)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp

    src/model/gridmodel.cpp
    src/model/pathfinder.cpp

    src/view/mainwindow.cpp
    src/view/gridscene.cpp
)

set(HEADERS
    src/model/gridmodel.h
    src/model/pathfinder.h

    src/view/mainwindow.h
    src/view/gridscene.h
)

qt_add_executable(PathFinder
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(PathFinder
    PRIVATE
        Qt6::Widgets
        Qt6::Core
)

# Enable precompiled headers for faster builds
target_precompile_headers(PathFinder PRIVATE
    src/view/mainwindow.h
    src/model/gridmodel.h
)

# Set include directories
target_include_directories(PathFinder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view
)
