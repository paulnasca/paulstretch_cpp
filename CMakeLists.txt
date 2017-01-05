# -*-cmake-*-
# Author: Kent Williams kwilliams@leepfrog.com
# top-level CMakeLists.txt This implements the SuperBuild CMake pattern.
#
# When you run came using this directory as the source directory, it
# invokes this file, which then includes SuperBuild.cmake, which
# configures building all the prerequisites using the ExternalProject
# module, then the main application.
#
# During main application configuration CMake includes PageWiz.cmake,
# which configures the Leepfrog CGI build to use all the already-built
# prerequisites.

cmake_minimum_required(VERSION 2.8)

option(USE_SYSTEM_PREREQS "Only use system-installed prerequisite libraries" ON)

# PaulStretch_SUPERBUILD determines whether to build the prerequisites
# (ON) or build the application (OFF)
option(PaulStretch_SUPERBUILD "Build PaulStretch and the project it depends on via SuperBuild.cmake" ON)
mark_as_advanced(PaulStretch_SUPERBUILD)


if(${PaulStretch_SUPERBUILD})
  project(SuperBuild_PaulStretch)
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  return()
else()
  project(PaulStretch)
  include("${CMAKE_CURRENT_SOURCE_DIR}/PaulStretch.cmake")
  return()
endif()
