##---------------------------------------------------------------------------
# ## File:        CMakeLists.txt
##
## Description: 
##
##---------------------------------------------------------------------------
## Author:      Oliver Eichler
##---------------------------------------------------------------------------

# Set additional include directory
# --------------------------------

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})


# Set source files
# ----------------
set(trackcomp_SRCS
    main.cpp
    CMainWidget.cpp
    CGpx.cpp
    CTrack.cpp
)

# Set header files
# ----------------
set(trackcomp_HDRS
    CMainWidget.h
    CGpx.h
    CTrack.h
)

# Set UIS files
# ----------------
set(trackcomp_UIS
    IMainWidget.ui
)

# Set RC files
# ----------------
set(trackcomp_RCS
    resources.qrc
)

# QT setup
# ----------------
qt4_wrap_cpp(trackcomp_MOC_SRCS ${trackcomp_HDRS})
qt4_wrap_ui(trackcomp_UI_HDRS ${trackcomp_UIS})
qt4_add_resources(trackcomp_RC_SRCS ${trackcomp_RCS} )


include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_INCLUDE_DIR}
    ${QT_QTXML_INCLUDE_DIR}
)


# The target to build
# -------------------
add_executable( ${APPNAME} WIN32 ${trackcomp_SRCS} ${trackcomp_HDRS} ${trackcomp_UI_HDRS} ${trackcomp_RC_SRCS} ${trackcomp_MOC_SRCS})

# General options
# ---------------
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif(WIN32)

if(UNIX)
  add_definitions(-Wall)
endif(UNIX)

add_definitions(${APPDEFINE})


# Additional libraries to link with
# ---------------------------------
if(WIN32)
target_link_libraries(${APPNAME} ${QT_LIBRARIES} ${QT_QTXML_LIBRARIES} ${QT_LIBRARY_DIR}/qtmain.lib)
else(WIN32)
target_link_libraries(${APPNAME} ${QT_LIBRARIES} ${QT_QTXML_LIBRARIES} )
endif(WIN32)


