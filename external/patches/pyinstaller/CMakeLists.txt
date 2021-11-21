cmake_minimum_required(VERSION 3.13)

set(project_name "pyinstaller-bootloader-native")
if(TARGET ${project_name}-lib)
  return()
endif()

project(${project_name} C CXX)

file(GLOB_RECURSE src ${CMAKE_CURRENT_LIST_DIR}/bootloader/src/pyi_*.c)

file(GLOB_RECURSE exe_src ${CMAKE_CURRENT_LIST_DIR}/bootloader/src/main.c)

file(GLOB_RECURSE public_header
     ${CMAKE_CURRENT_LIST_DIR}/bootloader/src/pyi_*.h)

# ================================================

option(PY_INSTALLER_WINDOWED "Build with window mode." OFF)

set(PREDEFINED_DEFINES "")

if(PY_INSTALLER_WINDOWED)
  if(APPLE)
    find_package(Carbon REQUIRED)
  endif()
  list(APPEND PREDEFINED_DEFINES "WINDOWED")
endif()

if(WIN32)
  list(APPEND PREDEFINED_DEFINES "WIN32" "UNICODE" "_UNICODE")
else()
  if(UNIX)
    if(APPLE)
      list(APPEND PREDEFINED_DEFINES "_DARWIN_C_SOURCE")
    else()
      list(APPEND PREDEFINED_DEFINES "_DEFAULT_SOURCE")
    endif()
  endif()
endif()

# ================================================

add_library(${project_name}-lib STATIC ${src})

target_include_directories(
  ${project_name}-lib
  PUBLIC "${CMAKE_CURRENT_LIST_DIR}/bootloader/src/"
  PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(
  ${project_name}-lib
  PUBLIC ${CMAKE_THREAD_LIBS_INIT}
  PUBLIC ${DL_LIBRARY}
  PUBLIC ZLIB::ZLIB)

if(APPLE)
  if(PY_INSTALLER_WINDOWED)
    target_link_libraries(${project_name}-lib PUBLIC ${Carbon_LIBRARIES})
  endif()
endif()

target_compile_definitions(${project_name}-lib PUBLIC ${PREDEFINED_DEFINES})

# ================================================

add_executable(${project_name}-exe ${src} ${exe_src})

if(PY_INSTALLER_WINDOWED)
  if(WIN32)
    set_target_properties(${project_name}-exe PROPERTIES WIN32_EXECUTABLE TRUE)
  else()
    if(APPLE)
      set_target_properties(${project_name}-exe PROPERTIES MACOSX_BUNDLE TRUE)
    endif()
  endif()
endif()

target_link_libraries(
  ${project_name}-exe
  PUBLIC ${CMAKE_THREAD_LIBS_INIT}
  PUBLIC ${DL_LIBRARY}
  PUBLIC ${project_name}-lib)
