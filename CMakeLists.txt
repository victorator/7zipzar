cmake_minimum_required(VERSION 3.20)
project(ZArchive7z LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  zstd
  GIT_REPOSITORY https://github.com/facebook/zstd.git
  GIT_TAG v1.5.6
  SOURCE_SUBDIR build/cmake
)

set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(ZSTD_MULTITHREAD_SUPPORT OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(zstd)

add_library(zarchive_lib STATIC
  third_party/zarchive/zarchivereader.cpp
  third_party/zarchive/zarchivewriter.cpp
  third_party/zarchive/sha_256.c
)
target_include_directories(zarchive_lib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(zarchive_lib PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/zarchive
)
target_link_libraries(zarchive_lib PUBLIC libzstd_static)
target_compile_definitions(zarchive_lib PRIVATE NOMINMAX)
if(MSVC)
  target_compile_options(zarchive_lib PRIVATE /EHsc)
  set_property(TARGET zarchive_lib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_library(ZArchive7z SHARED
  src/dllmain.cpp
  src/ZarHandler.cpp
)
target_include_directories(ZArchive7z PRIVATE
  sdk/CPP/7zip/Archive
  sdk/CPP/7zip
  sdk/CPP/Common
  sdk/CPP
  sdk/C
  sdk
  src
  include
)
target_link_libraries(ZArchive7z PRIVATE zarchive_lib)
target_compile_definitions(ZArchive7z PRIVATE NOMINMAX)
if(WIN32)
  target_link_libraries(ZArchive7z PRIVATE oleaut32)
endif()
if(MSVC)
  target_sources(ZArchive7z PRIVATE src/Archive2.def)
  target_compile_options(ZArchive7z PRIVATE /EHsc)
  set_property(TARGET ZArchive7z PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set_target_properties(ZArchive7z PROPERTIES
  OUTPUT_NAME "ZArchive"
  PREFIX ""
)
