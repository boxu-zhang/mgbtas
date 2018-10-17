cmake_minimum_required(VERSION 2.8)

PROJECT(mgbtas)

# The version number.
set (MGBTAS_VERSION_MAJOR 1)
set (MGBTAS_VERSION_MINOR 0)
 
# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/mgbtas_config.h.in"
  "${PROJECT_BINARY_DIR}/mgbtas_config.h"
  )
 
# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")

SET(CMAKE_INSTALL_PREFIX "/usr/local")

SET(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/mgbtas" CACHE PATH "Installation directory for libraries")
SET(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include/mgbtas" CACHE PATH "Installation directory for headers")
SET(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin/mgbtas" CACHE PATH "Installation directory for binaries")
SET(INSTALL_MAN_DIR "${CMAKE_INSTALL_PREFIX}/share/man/mgbtas" CACHE PATH "Installation directory for manual pages")
SET(INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files")

CONFIGURE_FILE(
   ${PROJECT_SOURCE_DIR}/mgbtas.pc.cmakein
   ${PROJECT_BINARY_DIR}/mgbtas.pc @ONLY
   )

SET( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-g -fPIC -std=c++11 -lpthread" )

SET( ${PROJECT_NAME}_HDRS
   ${PROJECT_BINARY_DIR}/mgbtas_config.h
   mgbtas_time.h
   mgbtas_ui.h
   mgbtas_core.h
   mgbtas.h
   )

SET( ${PROJECT_NAME}_SRC
   mgbtas_time.cpp
   mgbtas_core.cpp
   mgbtas_ui.cpp
   mgbtas.cpp
   )

# Library
ADD_LIBRARY( ${PROJECT_NAME} ${${PROJECT_NAME}_SRC} )

# Demo
ADD_EXECUTABLE( ${PROJECT_NAME}_demo mgbtas_demo.cpp )

TARGET_LINK_LIBRARIES( ${PROJECT_NAME}_demo ${PROJECT_NAME} )

# Test
ADD_EXECUTABLE( ${PROJECT_NAME}_test mgbtas_test.cpp )

TARGET_LINK_LIBRARIES( ${PROJECT_NAME}_test ${PROJECT_NAME} )

# Install
if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL )
   install(FILES ${${PROJECT_NAME}_HDRS} DESTINATION "${INSTALL_INC_DIR}")
   install(TARGETS ${PROJECT_NAME} DESTINATION "${INSTALL_LIB_DIR}")
   install(TARGETS ${${PROJECT_NAME}_demo} DESTINATION "${INSTALL_BIN_DIR}")
   install(TARGETS ${${PROJECT_NAME}_test} DESTINATION "${INSTALL_BIN_DIR}")
   install(FILES ${PROJECT_BINARY_DIR}/mgbtas.pc DESTINATION "${INSTALL_PKGCONFIG_DIR}")
endif()
