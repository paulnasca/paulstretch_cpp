PROJECT(PaulStretch)
# this is my CMake build recipe for paulstretch, an audio application
# for extreme time stretching.
# Source is here: https://sourceforge.net/project/showfiles.php?group_id=164941

cmake_minimum_required(VERSION 2.8)

if(LINUX)
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS -DHAVE_JACK)
endif(LINUX)

if(NOT USE_SYSTEM_PREREQS)
#
# paulstretch depends on numerous libraries that need to be built first.
# My strategy is to install them all in a single 'sandbox' directory, and
# reference them via EXTERNAL_LIB_PREFIX

SET(EXTERNAL_LIB_PREFIX "" CACHE PATH
  "Directory root to find lib and include of dependencies"
  )

#
# fltk is installed in the sandbox but trying to use FindFLTK/UseFLTK
# seems to cause problems. Adding just this variable makes Fluid work
set(FLTK_FLUID_EXECUTABLE ${EXTERNAL_LIB_PREFIX}/bin/fluid)
include_directories(${EXTERNAL_LIB_PREFIX}/include)
link_directories(${EXTERNAL_LIB_PREFIX}/lib)
else(NOT USE_SYSTEM_PREREQS)
  find_package(FLTK REQUIRED)
endif(NOT USE_SYSTEM_PREREQS)

#
# GUI files built in fluid.
fltk_wrap_ui(paulstretch FreeEditUI.fl GUI.fl)
#
# some source nested in Input
set(Input_source
  Input/AInputS.cpp
  Input/AInputS.h
  Input/InputS.h
  Input/MP3InputS.cpp
  Input/MP3InputS.h
  Input/VorbisInputS.cpp
  Input/VorbisInputS.h
  )
set(Output_source
  Output/AOutputS.cpp
  Output/VorbisOutputS.cpp
  )
#
# source for paulstretch
set (Source_files
  BinauralBeats.cpp
  Control.cpp
  FreeEdit.cpp
  JAaudiooutput.cpp
  Mutex.cpp
  PAaudiooutput.cpp
  Player.cpp
  ProcessedStretch.cpp
  Stretch.cpp
  Thread.cpp
  XMLwrapper.cpp
  globals.cpp
  ${Input_source}
  ${Output_source}
  ${paulstretch_FLTK_UI_SRCS}
  )

include_directories(${PaulStretch_SOURCE_DIR})

#
# on Apple, need to link to Carbon/Cocoa etc. Don't know the 'CMake Way'
# to get these frameworks linked in so punt.  This is what fltk does for fluid.
IF(APPLE)
  SET( FLTK_PLATFORM_DEPENDENT_LIBS
    "-framework Carbon -framework Cocoa -framework ApplicationServices -lz")
ELSE(APPLE)
  SET( FLTK_PLATFORM_DEPENDENT_LIBS
    -lX11)
ENDIF(APPLE)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(EXTRA_LIBS rt asound pthread -ldl -lXrender -lXext -lXinerama -lXcursor -lXfixes -lXft -lXpm -lX11 -lz -lfontconfig)
endif()

#
# the application.
add_executable(paulstretch MACOSX_BUNDLE
  ${Source_files}
  )

#
# all the dependencies
target_link_libraries(paulstretch audiofile
  fftw3f vorbisenc vorbisfile vorbis ogg portaudio pthread
  mad fltk fltk_forms fltk_gl fltk_images mxml
  ${FLTK_PLATFORM_DEPENDENT_LIBS}
  ${EXTRA_LIBS}
  )

get_target_property(TARGET_EXEC_DIR paulstretch RUNTIME_OUTPUT_DIRECTORY)
set(TARGET_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")

set(DEFAULT_MODULE_SEARCH_PATH
  "${TARGET_EXEC_DIR}/Modules")

#--------------------------------------------------------------------------------
# Now the installation stuff below
#--------------------------------------------------------------------------------
SET(plugin_dest_dir bin)
SET(qtconf_dest_dir bin)
SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/paulstretch")
IF(APPLE)
  SET(plugin_dest_dir paulstretch.app/Contents/MacOS)
  SET(qtconf_dest_dir paulstretch.app/Contents/Resources)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/paulstretch.app")
ENDIF(APPLE)
IF(WIN32)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/paulstretch.exe")
ENDIF(WIN32)

#--------------------------------------------------------------------------------
# Install the paulstretch application, on Apple, the bundle is at the root of the
# install tree, and on other platforms it'll go into the bin directory.
INSTALL(TARGETS paulstretch
  BUNDLE DESTINATION . COMPONENT Runtime
  RUNTIME DESTINATION bin COMPONENT Runtime
  )

#--------------------------------------------------------------------------------
# Use BundleUtilities to get all other dependencies for the application to work.
# It takes a bundle or executable along with possible plugins and inspects it
# for dependencies.  If they are not system dependencies, they are copied.

# directories to look for dependencies
SET(DIRS ${EXTERNAL_LIB_PREFIX}/lib )
# Now the work of copying dependencies into the bundle/package
# The quotes are escaped and variables to use at install time have their $ escaped
# An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
# Note that the image plugins depend on QtSvg and QtXml, and it got those copied
# over.
INSTALL(CODE "
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\" \"${DIRS}\")
    " COMPONENT Runtime)


# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# where CPackConfig.cmake is created by including CPack
# And then there's ways to customize this as well
set(CPACK_BINARY_DRAGNDROP ON)
include(CPack)
