cmake_minimum_required(VERSION 3.16)

project(watermark_qt6 VERSION 0.1 LANGUAGES CXX)

if (MSVC)
  set(CMAKE_BUILD_TYPE Release) # Debug MSVC has bug

  if (POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
  endif (POLICY CMP0091)
  # set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

option(USE_QEXIF_LIB "use qexif lib rather than exiv2" OFF) # all platform but has bugs (qexif is not good enough)
option(USE_EXIV2_SOURCE "if use exiv2, open this to build from source (cross platform) " ON) # allow all platform build, not only MSVC

if (ANDROID)
  option(BUILD_FOR_ANDROID "build android version" ON)
  message(STATUS "building android")
  add_compile_definitions("-DBUILD_FOR_ANDROID") # for cpp use
endif()

if (USE_QEXIF_LIB)
  message(STATUS "using qexif lib")
  add_compile_definitions("-DUSE_QEXIF_LIB") # for cpp use
endif ()

set(TS_FILES watermark_qt6_zh_CN.ts)

set(PROJECT_SOURCES
        app/main.cpp
        app/mainwindow.cpp
        app/mainwindow.h
        app/mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(watermark_qt6
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        src/WaterMarkGenerator.h src/WaterMarkGenerator.cpp
        src/WaterMarkDesc.h
        src/Utils.h src/Utils.cpp
        resource/resource.qrc
        resource/icon.rc
        src/WaterMarkDesc.cpp
        src/WaterMarkHolder.h src/WaterMarkHolder.cpp
        app/MainWindowAndroid.h app/MainWindowAndroid.cpp app/MainWindowAndroid.ui
        resource/AndroidManifest.xml
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET watermark_qt6 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(watermark_qt6 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(watermark_qt6
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

if (USE_QEXIF_LIB)
  find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core5Compat)
  find_package(Qt${QT_VERSION_MAJOR} REQUIRED Core5Compat)
  add_library(qexif STATIC
    ${PROJECT_SOURCE_DIR}/third_party/qexif/qexifimageheader.cpp
    ${PROJECT_SOURCE_DIR}/third_party/qexif/qexifimageheader.h
    ${PROJECT_SOURCE_DIR}/third_party/qexif/qmetadata.cpp
    ${PROJECT_SOURCE_DIR}/third_party/qexif/qmetadata.h
  )
  target_link_libraries(qexif PUBLIC
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Core5Compat
  )
  target_include_directories(watermark_qt6 PRIVATE
    ${PROJECT_SOURCE_DIR}/third_party/qexif
  )
target_link_libraries(watermark_qt6 PRIVATE
  qexif
)
else()
  if (USE_EXIV2_SOURCE)
    message(STATUS "build exiv2 from source ...")
    include_directories(${PROJECT_BINARY_DIR})
    include_directories(${PROJECT_SOURCE_DIR}/third_party/exiv2-0.28.5/)
    add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/exiv2-0.28.5/)
  else()
    message(STATUS "use exiv2 binaries ...")
    add_subdirectory(${PROJECT_SOURCE_DIR}/third_party)
    find_package(exiv2 REQUIRED CONFIG NAMES exiv2)    # search ${CMAKE_INSTALL_PREFIX}/lib/cmake/exiv2/
  endif()
  target_link_libraries(watermark_qt6 PRIVATE
    Exiv2::exiv2lib
  )
endif()

target_link_libraries(watermark_qt6 PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Core
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.watermark_qt6)
endif()
set_target_properties(watermark_qt6 PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS watermark_qt6
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(watermark_qt6)
endif()
