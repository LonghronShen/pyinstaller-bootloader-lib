find_package(Threads REQUIRED)

if(UNIX)
    find_package(DL REQUIRED)
endif()

include(FetchContent)

# googletest
option(PYINSTALLER_LIB_BUILD_GTEST "Build Gtest for PYINSTALLER_LIB_BUILD_GTEST." OFF)
message(STATUS "PYINSTALLER_LIB_BUILD_GTEST: ${PYINSTALLER_LIB_BUILD_GTEST}")
if(PYINSTALLER_LIB_BUILD_GTEST)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG release-1.10.0)

    FetchContent_GetProperties(googletest)
    if(NOT googletest_POPULATED)
        FetchContent_Populate(googletest)
        add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
        include(GoogleTest)
    endif()
endif()

# zilb
find_package(ZLIB QUIET)
if(ZLIB_FOUND)
    message(STATUS "Using system zlib: ${ZLIB_INCLUDE_DIRS}")
    include_directories(BEFORE SYSTEM ${ZLIB_INCLUDE_DIRS})
else()
    FetchContent_Declare(zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG v1.2.11)

    FetchContent_GetProperties(zlib)
    if(NOT zlib_POPULATED)
        FetchContent_Populate(zlib)
        add_subdirectory(${zlib_SOURCE_DIR} ${zlib_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()

    message(STATUS "Using local zlib: ${zlib_SOURCE_DIR}")
    include_directories(
        "${zlib_SOURCE_DIR}"
        "${zlib_BINARY_DIR}")
    set(ZLIB_LIBRARIES zlibstatic CACHE STRING "ZLIB_LIBRARIES" FORCE)

    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()


# PyInstaller
FetchContent_Declare(pyinstaller
  GIT_REPOSITORY https://github.com/pyinstaller/pyinstaller.git
  GIT_TAG v4.5.1
  GIT_SHALLOW TRUE)

FetchContent_GetProperties(pyinstaller)
if(NOT pyinstaller_POPULATED)
  FetchContent_Populate(pyinstaller)
  file(COPY 
    "${CMAKE_CURRENT_LIST_DIR}/patches/pyinstaller/CMakeLists.txt" 
  DESTINATION 
    "${pyinstaller_SOURCE_DIR}/")
  add_subdirectory(${pyinstaller_SOURCE_DIR} ${pyinstaller_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()


# mio
FetchContent_Declare(mio
  GIT_REPOSITORY https://github.com/mandreyel/mio.git
  GIT_TAG master
  GIT_SHALLOW TRUE)

FetchContent_GetProperties(mio)
if(NOT mio_POPULATED)
  FetchContent_Populate(mio)
  add_subdirectory(${mio_SOURCE_DIR} ${smio_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()