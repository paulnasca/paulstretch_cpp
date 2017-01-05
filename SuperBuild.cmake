# requires CMake 2.8
cmake_minimum_required(VERSION 2.8)
project(PaulStretch)
#
# Written by Kent Williams chaircrusher@gmail.com
#
# No rights reserved. No warranty for anything.

include(ExternalProject)
# This new build recipe was prompted by the recent addition to CMake
# of the ExternalProject module. This addition makes for extremely concise
# recipes for downloading and installing libraries as needed.

if(APPLE)
# need to do a 32-bit build because of FLTK needing Carbon
set(platform_flags -DCMAKE_OSX_ARCHITECTURES:STRING=i386
-DCMAKE_C_FLAGS:STRING=-m32;-O2
-DCMAKE_CXX_FLAGS:STRING=-m32;-O2)
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set(platform_flags -DBUILD_SHARED_LIBS:BOOL=OFF)
set(extra_config_flags --disable-shared)
endif()

ExternalProject_add(audiofile
  GIT_REPOSITORY "https://github.com/mpruett/audiofile.git"
  GIT_TAG "audiofile-0.2.7"
  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND ./autogen.sh
  ${extra_config_flags}
  --prefix=${CMAKE_CURRENT_BINARY_DIR}/Prereqs
)

ExternalProject_add(fftw
  URL "http://www.fftw.org/fftw-3.3.5.tar.gz"
  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND ./configure
  --prefix=${CMAKE_CURRENT_BINARY_DIR}/Prereqs
  --enable-threads
  --enable-float
)

ExternalProject_add(fltk
URL "http://fltk.org/pub/fltk/1.3.4/fltk-1.3.4-1-source.tar.gz"
URL_MD5 d7fcd27ab928648e1a1366dd2e273970
UPDATE_COMMAND ""
BUILD_IN_SOURCE 1
CONFIGURE_COMMAND ./configure
--prefix=${CMAKE_CURRENT_BINARY_DIR}/Prereqs
)

ExternalProject_add(libmad
URL "http://downloads.sourceforge.net/mad/libmad-0.15.1b.tar.gz"
URL_MD5 1be543bc30c56fb6bea1d7bf6a64e66c
UPDATE_COMMAND ""
BUILD_IN_SOURCE 1
CONFIGURE_COMMAND ./configure
${extra_config_flags}
--prefix=${CMAKE_CURRENT_BINARY_DIR}/Prereqs
)

# simple script to handle find/replace in makefile
set(Script ${CMAKE_CURRENT_LIST_DIR}/RemoveFlags.cmake)
set(libmad_makefile
${CMAKE_CURRENT_BINARY_DIR}/libmad-prefix/src/libmad/Makefile)

ExternalProject_Add_Step(libmad fix_config_idiocy
COMMENT "Get rid of CFLAGS that are stupid"
DEPENDEES configure
DEPENDERS build
ALWAYS 1
COMMAND ${CMAKE_COMMAND} -Dfixfile=${libmad_makefile} -P ${Script}
)

ExternalProject_add(libogg
URL "http://downloads.xiph.org/releases/ogg/libogg-1.3.2.tar.gz"
URL_MD5 0
UPDATE_COMMAND ""
BUILD_IN_SOURCE 1
CONFIGURE_COMMAND ./configure
${extra_config_flags}
--prefix=${CMAKE_CURRENT_BINARY_DIR}/Prereqs
)

ExternalProject_add(libvorbis
URL "http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.5.tar.gz"
URL_MD5 0
UPDATE_COMMAND ""
BUILD_IN_SOURCE 1
CONFIGURE_COMMAND ./configure
--prefix=${CMAKE_CURRENT_BINARY_DIR}/Prereqs
${extra_config_flags}
DEPENDS libogg
)

ExternalProject_add(portaudio
URL "http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz"
URL_MD5 4df8224e047529ca9ad42f0521bf81a8
UPDATE_COMMAND ""
BUILD_IN_SOURCE 1
CONFIGURE_COMMAND ./configure
${extra_config_flags}
--prefix=${CMAKE_CURRENT_BINARY_DIR}/Prereqs
)

set(portaudio_makefile
${CMAKE_CURRENT_BINARY_DIR}/portaudio-prefix/src/portaudio/Makefile)

ExternalProject_Add_Step(portaudio fix_config_idiocy
COMMENT "Get rid of CFLAGS that are stupid"
DEPENDEES configure
DEPENDERS build
ALWAYS 1
COMMAND ${CMAKE_COMMAND} -Dfixfile=${portaudio_makefile} -P ${Script}
)

set(mxml_cmakelist ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt.mxml)
set(mxml_target
${CMAKE_CURRENT_BINARY_DIR}/mxml-prefix/src/mxml/CMakeLists.txt)

set(mxml_config_src ${CMAKE_CURRENT_LIST_DIR}/config.h.mxml)
set(mxml_config_target
${CMAKE_CURRENT_BINARY_DIR}/mxml-prefix/src/mxml/config.h)

ExternalProject_add(mxml
  URL "http://www.msweet.org/files/project3/mxml-2.10.tar.gz"
  PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${mxml_cmakelist} ${mxml_target}
  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  CMAKE_ARGS
  -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/Prereqs
  )

ExternalProject_Add_Step(mxml make_config_h
COMMENT "Copy in config.h"
DEPENDEES configure
DEPENDERS build
ALWAYS 1
COMMAND ${CMAKE_COMMAND} -E copy ${mxml_config_src} ${mxml_config_target}
)
set(ps_cmakelist ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt.PaulStretch)
set(ps_target
${CMAKE_CURRENT_BINARY_DIR}/paulstretch-prefix/src/paulstretch/CMakeLists.txt)

# I had to make my own source tar and put it on my site, since
# Sourceforge seems to refuse to do a proper download.
ExternalProject_add(paulstretch
UPDATE_COMMAND ""
DOWNLOAD_COMMAND ""
INSTALL_COMMAND ""
SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
BINARY_DIR PaulStretch-build
CMAKE_ARGS
-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
-DEXTERNAL_LIB_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/Prereqs
-DCMAKE_BUILD_TYPE:STRING=Release
-DPaulStretch_SUPERBUILD:BOOLEAN=OFF
${platform_flags}
DEPENDS audiofile fftw fltk libmad libogg libvorbis portaudio mxml
)
