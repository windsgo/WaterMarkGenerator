if (WIN32)
  message(STATUS "Platform: WIN32")

  # 仅允许 MSVC 构建
  if(NOT MSVC)
    message(FATAL_ERROR "Only support MSVC On WIN32")
  endif()

  set(EXIV2_CMAKE_DIR ${PROJECT_SOURCE_DIR}/third_party/exiv2-0.28.5-2022msvc-AMD64/lib/cmake/exiv2/)
elseif(APPLE)
  # 判断架构
  if(CMAKE_OSX_ARCHITECTURES)
    message(STATUS "CMAKE_OSX_ARCHITECTURES: ${CMAKE_OSX_ARCHITECTURES}")
  endif()

  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    message(STATUS "Architecture: macOS arm64 (Apple Silicon)")
    # Apple Silicon
    set(EXIV2_CMAKE_DIR ${PROJECT_SOURCE_DIR}/third_party/exiv2-0.28.5-Darwin-arm64/lib/cmake/exiv2/)
  elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    message(STATUS "Architecture: macOS x86_64 (Intel)")
    #Intel
    set(EXIV2_CMAKE_DIR ${PROJECT_SOURCE_DIR}/third_party/exiv2-0.28.5-Darwin-x86_64/lib/cmake/exiv2/)
  else()
    message(FATAL_ERROR "Unknown macOS Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
  endif()
else()
  message(FATAL_ERROR "Only support Windows and macOS Platforms")
endif()

message(STATUS "EXIV2_CMAKE_DIR: ${EXIV2_CMAKE_DIR}")
list(APPEND CMAKE_PREFIX_PATH ${EXIV2_CMAKE_DIR})
find_package(exiv2 REQUIRED CONFIG NAMES exiv2)    # search ${CMAKE_INSTALL_PREFIX}/lib/cmake/exiv2/
