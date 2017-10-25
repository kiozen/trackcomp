##---------------------------------------------------------------------------
## File:        CMakeLists.txt (Master)
##
## Description: CMake file for building TrackComp
##
##---------------------------------------------------------------------------
## Author:      Oliver Eichler
##---------------------------------------------------------------------------


cmake_minimum_required(VERSION 2.8)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)
MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)


# User section
# ------------
project(TrackComp)


# project version
# ---------------
set(VER_MAJOR     0)
set(VER_MINOR     1)
set(VER_STEP      1)

add_definitions(-DVER_MAJOR=${VER_MAJOR} -DVER_MINOR=${VER_MINOR} -DVER_STEP=${VER_STEP})


# Set default target build (None Debug Release RelWithDebInfo MinSizeRel)
# -----------------------------------------------------------------------
IF(NOT CMAKE_BUILD_TYPE)
SET( CMAKE_BUILD_TYPE "Debug" )
ENDIF(NOT CMAKE_BUILD_TYPE)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# Add project parts
# -----------------
set(APPNAME trackcomp)
add_subdirectory(./src)



