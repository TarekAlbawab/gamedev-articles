cmake_minimum_required(VERSION 3.0)

project(gamedev-articles)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

set(GIT_ROOT ${PROJECT_SOURCE_DIR})
set(SRC_PATH ${GIT_ROOT}/src)
set(EXT_LIB_DIR ${GIT_ROOT}/lib)
set(ROOT_DATA_DIR ${GIT_ROOT}/data)

set(EXECUTABLE_OUTPUT_PATH ${GIT_ROOT}/bin)
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
#set(LIBRARY_OUTPUT_PATH ${GIT_ROOT}/bin)

include_directories("${SRC_PATH}")

set(CMAKE_CXX_STANDARD 11)

# Specify the list of directories to search for cmake modules.
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (APPLE)
  set(CMAKE_MACOSX_RPATH 1)
  add_definitions(-DAPPLE)
endif()

if (WIN32)
  ADD_DEFINITIONS( -DWIN32_LEAN_AND_MEAN )
endif()

#option(BUILD_IOS "Build iOS projects" OFF)

set(USE_GLEW ON)

option(USE_GLFW "Use GLFW" ON)
option(USE_DEVIL "Use DevIL" OFF)
option(USE_GLUT "Use GLUT" OFF)
option(USE_SOIL "Use SOIL" OFF)
option(USE_ASSIMP "Use assimp" OFF)
option(USE_OPENEXR "Use OpenEXR" OFF)
option(USE_EASYLOGGINGPP "Use easyloggingpp" ON)
option(USE_FREETYPE_GL "Use freetype gl" OFF)
option(USE_PHYSFS "Use PhysFS" OFF)
option(USE_AUDIO_ALURE "Use OpenAL alure" OFF)
option(USE_EIGEN "Use eigen" OFF)

# use only if needed in subprojects
set(USE_ZLIB OFF)

# load projects - this will adapt USE_* variables for whichever dependency is needed
include(Projects.cmake)

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    MESSAGE( "DevIL, 64 bit compiler detected" )
    SET( EX_PLATFORM 64 )
    SET( EX_PLATFORM_NAME "x64" )
else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    MESSAGE( "DevIL, 32 bit compiler detected" )
    SET( EX_PLATFORM 32 )
    SET( EX_PLATFORM_NAME "x86" )
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

# default to release
if (NOT CMAKE_BUILD_TYPE)
  message("Setting build type to release by default")
  set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)
endif()

# static libs as much as possible
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# zlib if not present include
if (USE_ZLIB)
#  add_subdirectory(${EXT_LIB_DIR}/zlib)
#  set(ZLIB_FOUND TRUE CACHE BOOL "" FORCE)
#  set(ZLIB_ROOT ${EXT_LIB_DIR}/zlib CACHE BOOL "" FORCE)
#  set(ZLIB_INCLUDE_DIR ${EXT_LIB_DIR}/zlib CACHE BOOL "" FORCE)
#  if (NOT WIN32)
#    set(EXT_LIBS ${EXT_LIBS} zlibstatic)
#  else()
#    set(EXT_LIBS ${EXT_LIBS} zlib1)
#  endif()
#  link_directories(${CMAKE_CURRENT_BINARY_DIR}/lib/zlib)
endif()

# GLFW
if (USE_GLFW)
  set(GLFW_USE_HYBRID_HPG ON CACHE BOOL "" FORCE)
  find_package(OpenGL REQUIRED)
  if (OPENGL_FOUND)
    include_directories(${OPENGL_INCLUDE_DIR})
  endif (OPENGL_FOUND)
  set(EXT_LIBS ${EXT_LIBS} ${OPENGL_gl_LIBRARY})
endif ()

if (USE_GLFW)
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  add_subdirectory(${EXT_LIB_DIR}/glfw)
  set(EXT_LIBS ${EXT_LIBS} glfw)
  # Uncomment this to provide other libraries with GLFW
#  set(GLFW3_INCLUDE_DIR ${EXT_LIB_DIR}/glfw/include)
#  set(GLFW3_LIBRARY glew3)
#  file(COPY
#    ${CMAKE_BINARY_DIR}/lib/glfw/CMakeFiles/Export/lib/cmake/glfw3/glfw3Targets.cmake
#    DESTINATION
#    ${CMAKE_BINARY_DIR}/lib/glfw/src)
#  set(glfw3_DIR ${CMAKE_BINARY_DIR}/lib/glfw/src)

endif (USE_GLFW)

if (APPLE AND (NOT USE_GLEW))
  set(EXT_LIBS ${EXT_LIBS} "-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo")
endif ()

if (USE_GLEW)
  if (NOT EXISTS ${GIT_ROOT}/lib_bin/glew-2.1.0.zip)
    execute_process(
        COMMAND wget --no-check-certificate -P ${GIT_ROOT}/lib_bin https://netcologne.dl.sourceforge.net/project/glew/glew/2.1.0/glew-2.1.0.zip
    )
  endif ()
  if (NOT EXISTS ${GIT_ROOT}/lib/glew-2.1.0)
    execute_process(
      COMMAND unzip ${GIT_ROOT}/lib_bin/glew-2.1.0.zip -d ${GIT_ROOT}/lib
    )
  endif()

  add_definitions(-DGLEW_STATIC)
  set(GLEW_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
  set(BUILD_UTILS OFF CACHE BOOL "" FORCE)
  add_subdirectory(${EXT_LIB_DIR}/glew-2.1.0/build/cmake)
  include_directories(${EXT_LIB_DIR}/glew-2.1.0/include)

  set(GLEW_INCLUDE_PATH ${EXT_LIB_DIR}/glew-2.1.0/include)
  
  if (WIN32)
    set (GLEW_LIB_NAME libglew32)
	link_directories(${CMAKE_CURRENT_BINARY_DIR}/lib/Release)
  else ()
    set (GLEW_LIB_NAME glew)
  endif()
  
  set(GLEW_LIBRARY ${GLEW_LIB_NAME})
  set(EXT_LIBS ${EXT_LIBS} ${GLEW_LIB_NAME})
endif (USE_GLEW)

# GLM
include_directories(${EXT_LIB_DIR}/glm)

# Use glut on win32 / some libraries want glut
if (WIN32 AND USE_GLUT)
  set(FREEGLUT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(FREEGLUT_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
  set(FREEGLUT_REPLACE_GLUT ON CACHE BOOL "" FORCE)
  set(FREEGLUT_BUILD_DEMOS  OFF CACHE BOOL "" FORCE)
  add_subdirectory(${EXT_LIB_DIR}/freeglut)
  include_directories(${GIT_ROOT}/freeglut/include)
  set(GLUT_glut_LIBRARY freeglut_static)
  set(GLUT_INCLUDE_DIR ${GIT_ROOT}/freeglut/include)
endif()

# DEVIL
if (NOT WIN32 AND USE_DEVIL)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(IL_TESTS OFF CACHE BOOL "" FORCE)
  add_subdirectory(${EXT_LIB_DIR}/DevIL/DevIL)
#  set_target_properties(IL PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${GIT_ROOT}/bin")
#  set_target_properties(IL PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${GIT_ROOT}/bin")
#  set_target_properties(IL PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${GIT_ROOT}/bin")
  set_target_properties(ILU PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${GIT_ROOT}/bin")
  set_target_properties(ILU PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${GIT_ROOT}/bin")
#  set_target_properties(ILU PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${GIT_ROOT}/bin")
  set_target_properties(ILUT PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${GIT_ROOT}/bin")
  set_target_properties(ILUT PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${GIT_ROOT}/bin")
#  set_target_properties(ILUT PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${GIT_ROOT}/bin")
  set(EXT_LIBS ${EXT_LIBS} IL ILU ILUT)
  include_directories(${GIT_ROOT}/lib/DevIL/DevIL/include)
endif()

if (USE_DEVIL)
  # set libraries to find DevIL
  set(IL_FOUND ON CACHE BOOL "" FORCE)
  set(IL_INCLUDE_DIR ${GIT_ROOT}/lib/DevIL/DevIL/include CACHE BOOL "" FORCE)
#  if (MSVC)
#    get_target_property(DEVIL_OUTPUT_NAME IL OUTPUT_NAME)
#  endif()

  set(IL_LIBRARIES   ${CMAKE_CURRENT_BINARY_DIR}/lib/DevIL/DevIL/lib/${EX_PLATFORM_NAME}/libIL.a CACHE BOOL "" FORCE)
#  message("IL_LIBRARIES ${IL_LIBRARIES}")
  set(ILU_LIBRARIES  ${GIT_ROOT}/bin CACHE BOOL "" FORCE)
  set(ILUT_LIBRARIES ${GIT_ROOT}/bin CACHE BOOL "" FORCE)
endif()

# DevIL has many dependencies, like libpng libjpeg, etc... not available on win by default
# so download precompiled binaries
if (WIN32 AND USE_DEVIL)
  if (NOT EXISTS ${GIT_ROOT}/lib_bin/DevIL-Windows-SDK-1.8.0.zip)
    execute_process(
        COMMAND wget --no-check-certificate -P ${GIT_ROOT}/lib_bin https://vorboss.dl.sourceforge.net/project/openil/DevIL%20Windows%20SDK/1.8.0/DevIL-Windows-SDK-1.8.0.zip
    )
  endif()
  if (NOT EXISTS ${GIT_ROOT}/lib/DevIL\ Windows\ SDK)
    execute_process(
      COMMAND unzip ${GIT_ROOT}/lib_bin/DevIL-Windows-SDK-1.8.0.zip -d ${GIT_ROOT}/lib
    )
  endif()
  set(DEVIL_SDK_DIR ${GIT_ROOT}/lib/DevIL\ Windows\ SDK)
  set(DEVIL_SDK_LIB_DIR ${GIT_ROOT}/lib/DevIL\ Windows\ SDK/lib/x86/Release)
  include_directories(${DEVIL_SDK_DIR}/include)
  link_directories(${DEVIL_SDK_LIB_DIR})
  set(EXT_LIBS ${EXT_LIBS} DevIL ILU ILUT)
  file(GLOB DevilDlls ${DEVIL_SDK_LIB_DIR}/*.dll)
  file(COPY ${DevilDlls} DESTINATION ${EXECUTABLE_OUTPUT_PATH})
endif()

if (USE_SOIL)
  add_subdirectory(${EXT_LIB_DIR}/soil)
  set(EXT_LIBS ${EXT_LIBS} SOIL)
  include_directories(${GIT_ROOT}/lib/soil/src)
endif()

# assimp
if (USE_ASSIMP)
  list(APPEND CMAKE_PREFIX_PATH /Users/edin-m/Qt/5.8/clang_64/lib/cmake)

  set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(ASSIMP_NO_EXPORT OFF CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_ZLIB OFF CACHE BOOL "" FORCE)
  if (WIN32)
    set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
  endif()
  add_subdirectory(${EXT_LIB_DIR}/assimp)
  set(EXT_LIBS ${EXT_LIBS} assimp)
  message("CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}")
  include_directories(${EXT_LIB_DIR}/assimp/include)
  include_directories(${CMAKE_CURRENT_BINARY_DIR}/lib/assimp/include)
endif()

## OpenEXR - unused
#if (USE_OPENEXR)
#  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
##  add_subdirectory(${EXT_LIB_DIR}/openexr/IlmBase)
#  set(ILMBASE_PACKAGE_PREFIX ${GIT_ROOT}/del/openexr/IlmBase/build/install)
#  add_subdirectory(${EXT_LIB_DIR}/openexr/OpenEXR)
#endif()

# logging
if (USE_EASYLOGGINGPP)
  # EASY_LOGGER is switched on by defualt
  set(build_static_lib ON CACHE BOOL "" FORCE)
  set(test OFF CACHE BOOL "" FORCE)
  add_subdirectory(${EXT_LIB_DIR}/easyloggingpp)
  include_directories(${EXT_LIB_DIR}/easyloggingpp/src)
  set(EXT_LIBS ${EXT_LIBS} easyloggingpp)
  add_definitions(-DLIB_EASYLOGGING)
endif()

# freetype-gl
if (USE_FREETYPE_GL)
  set(freetype-gl_WITH_GLEW ON CACHE BOOL "" FORCE)
  set(freetype-gl_USE_VAO ON CACHE BOOL "" FORCE)
  set(freetype-gl_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
  set(freetype-gl_BUILD_APIDOC OFF CACHE BOOL "" FORCE)
  # harfbuzz is experimental => turn it off
  set(freetype-gl_BUILD_HARFBUZZ OFF CACHE BOOL "" FORCE)
  set(freetype-gl_BUILD_MAKEFONT ON CACHE BOOL "" FORCE)
  set(freetype-gl_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  add_subdirectory(${EXT_LIB_DIR}/freetype-gl)
  include_directories(${EXT_LIB_DIR}/freetype-gl)
  set(EXT_LIBS ${EXT_LIBS} freetype freetype-gl)
  if (WIN32)
    link_directories(${EXT_LIB_DIR}/freetype-gl/windows/freetype)
    include_directories(${EXT_LIB_DIR}/freetype-gl/windows/freetype)
    if (MSVC_VERSION GREATER_EQUAL 1900)
      set(EXT_LIBS ${EXT_LIBS} legacy_stdio_definitions)
    endif()
  endif()
endif()

# argument parser
include_directories(${EXT_LIB_DIR}/argh)

# PhysFS
if (USE_PHYSFS)
  # download - PhysFS is on Mercurial SCM
  if (NOT EXISTS ${GIT_ROOT}/lib_bin/physfs-3.0.1.tar.bz2)
    execute_process(
        COMMAND wget --no-check-certificate -P ${GIT_ROOT}/lib_bin https://icculus.org/physfs/downloads/physfs-3.0.1.tar.bz2
    )
  endif()
  # unpack
  if (NOT EXISTS ${GIT_ROOT}/lib/physfs-3.0.1)
    execute_process(
      COMMAND bash "-c" "bzip2 -d -c ${GIT_ROOT}/lib_bin/physfs-3.0.1.tar.bz2 | tar -xf - -C ${GIT_ROOT}/lib"
    )
  endif()
  # rename uninstall target because of a name collision
  execute_process(
    COMMAND bash "-c" "sed -i -e 's/        uninstall$/        uninstall-physfs/g' ${EXT_LIB_DIR}/physfs-3.0.1/CMakeLists.txt"
  )
  set(PHYSFS_BUILD_STATIC TRUE CACHE BOOL "" FORCE)
  set(PHYSFS_BUILD_TEST FALSE CACHE BOOL "" FORCE)
  set(PHYSFS_BUILD_SHARED FALSE CACHE BOOL "" FORCE)
  add_subdirectory(${EXT_LIB_DIR}/physfs-3.0.1)
  include_directories(${EXT_LIB_DIR}/physfs-3.0.1/src)
  get_target_property(PHYSFS_OUTPUT_NAME physfs-static OUTPUT_NAME)
  set(EXT_LIBS ${EXT_LIBS} physfs-static)
endif()

# OpenAL
if (WIN32)
  # set the location of openal lib
  set(OPENAL_LIBRARY "C:\\Program Files (x86)\\OpenAL 1.1 SDK\\libs\\Win32\\OpenAL32.lib")
  set(OPENAL_INCLUDE_DIR "C:\\Program Files (x86)\\OpenAL 1.1 SDK\\include")
endif()

# alure OpenAL
if (USE_AUDIO_ALURE)
  set(OGG_ROOT ${EXT_LIB_DIR}/ogg)
  add_subdirectory(${OGG_ROOT})

  set(OGG_INCLUDE_DIRS ${OGG_ROOT}/include)
  include_directories(${OGG_INCLUDE_DIRS})
  link_directories(${CMAKE_CURRENT_BINARY_DIR}/lib/ogg)
  set(OGG_LIBRARIES ogg)
  add_subdirectory(${EXT_LIB_DIR}/vorbis)

  set(OGG_INCLUDE_DIR ${OGG_INCLUDE_DIRS})
  set(OGG_LIBRARY ${OGG_LIBRARIES})
  set(VORBIS_INCLUDE_DIR ${EXT_LIB_DIR}/vorbis/include)
  link_directories(${CMAKE_CURRENT_BINARY_DIR}/lib/vorbis)
  set(VORBIS_LIBRARY vorbis)
  set(VORBISFILE_LIBRARY vorbisfile)
  # don't build examples
  set(ALURE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(ALURE_BUILD_SHARED OFF CACHE BOOL "" FORCE)
  set(ALURE_BUILD_STATIC ON CACHE BOOL "" FORCE)
  set(ALURE_INSTALL_TARGETS OFF CACHE BOOL "" FORCE)
  add_subdirectory(${EXT_LIB_DIR}/alure)

  set(EXT_LIBS ${EXT_LIBS} ogg vorbis alure2_s)
endif()

if (USE_EIGEN)
  # rename uninstall target because of a name collision
  execute_process(
    COMMAND bash "-c" "sed -i -e 's/ uninstall$/ uninstall-eigen/g' ${EXT_LIB_DIR}/eigen/CMakeLists.txt"
  )
  set(EIGEN_BUILD_BTL OFF CACHE BOOL "" FORCE)
  set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
  add_subdirectory(${EXT_LIB_DIR}/eigen)
  include_directories(${EXT_LIB_DIR}/eigen)

  set(EXT_LIBS ${EXT_LIBS} eigen)
endif()

## GLFM - ios/andriod glfw
#if (BUILD_IOS OR BUILD_ANDROID)

#endif()

message(${EXT_LIBS})
add_subdirectory(src)

if (MSVC)
# output projects to bin dir for MSVS
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${EXECUTABLE_OUTPUT_PATH})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${EXECUTABLE_OUTPUT_PATH})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${EXECUTABLE_OUTPUT_PATH})
endif()

add_subdirectory(projects)


