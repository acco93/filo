cmake_minimum_required(VERSION 3.16)
project(filo)

set(CMAKE_CXX_STANDARD 17)

set(WARNING_FLAGS "-Wall -Wextra -Wpedantic -Wuninitialized")
set(PROFILING_FLAGS "-fno-omit-frame-pointer ")
set(OPT_FLAGS "-O3 -march=native -ffat-lto-objects -flto -fwhole-program")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${PROFILING_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OPT_FLAGS}")

set(LIBRARIES cobra)

set(SOURCE bpp.hpp routemin.hpp main.cpp RuinAndRecreate.hpp)

option(ENABLE_VERBOSE "Enable verbose output" OFF)
option(ENABLE_GUI "Enable graphical interface" OFF)

message("-- Build options")

if(ENABLE_VERBOSE)
    message("--- Verbose output ENABLED")
    add_definitions(-DVERBOSE)
else()
    message("--- Verbose output DISABLED")
endif()

if(ENABLE_GUI)
    message("--- Graphical interface ENABLED")
    add_definitions(-DGUI)
    set(LIBRARIES ${LIBRARIES} glfw GL)
    set(SOURCE ${SOURCE} Renderer.hpp)
else()
    message("--- Graphical interface DISABLED")
endif()

if(TIMEBASED_TERMINATION)
    message("--- Time based termination ENABLED")
    add_definitions(-DTIMEBASED)
else()
    message("--- Time based termination DISABLED")
endif()

find_package(cobra 1.0.0 REQUIRED)

add_executable(filo ${SOURCE})
target_link_libraries(filo PUBLIC ${LIBRARIES})