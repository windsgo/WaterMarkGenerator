cmake_minimum_required(VERSION 3.16)

project(watermark_qt6 VERSION 3.1.2 LANGUAGES CXX)

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

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Concurrent )
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Concurrent )

# 弃用了，不用qexif了现在先
# 虽然exiv2在安卓上似乎也读取不到文件，这个以后再说 TODO
option(USE_QEXIF_LIB "use qexif lib rather than exiv2" OFF) # all platform but has bugs (qexif is not good enough)

# 不打开这个是构建不了安卓的，不打开只能用MSVC构建，因为库是MSVC二进制的库
option(USE_EXIV2_SOURCE "if use exiv2, open this to build from source (cross platform) " ON) # allow all platform build, not only MSVC

if (ANDROID)
  option(BUILD_FOR_ANDROID "build android version" ON)
  message(STATUS "building android")
  add_compile_definitions("-DBUILD_FOR_ANDROID") # 在CPP代码中检测用
  add_compile_definitions("-DANDROID_IMAGE_SAVE_USE_JNI") # 使用JNI保存到图库
endif()

if (USE_QEXIF_LIB)
  message(STATUS "using qexif lib")
  add_compile_definitions("-DUSE_QEXIF_LIB") # 在CPP代码中检测用
endif ()

set(TS_FILES watermark_qt6_zh_CN.ts)

# project版本映射到version.h中
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(PROJECT_SOURCES
        app/main.cpp
        src/WaterMarkGenerator.h src/WaterMarkGenerator.cpp
        src/WaterMarkDesc.h
        src/Utils.h src/Utils.cpp
        resource/resource.qrc
        resource/icon.rc
        src/WaterMarkDesc.cpp
        src/WaterMarkHolder.h src/WaterMarkHolder.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
        ${TS_FILES}
)

# 不同生成方式，用不同的ui界面（就是对安卓apk特殊化一下）
if (ANDROID)
  list(APPEND PROJECT_SOURCES
    app/MainWindowAndroid.h
    app/MainWindowAndroid.cpp
    app/MainWindowAndroid.ui
  )
else ()
  list(APPEND PROJECT_SOURCES
    app/mainwindow.cpp
    app/mainwindow.h
    app/mainwindow.ui
  )
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(watermark_qt6
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )

    # 重要 安卓上的相关文件路径
    if (ANDROID)
      # 生成 versionCode: 1.2.3 → 10203
      math(EXPR VERSION_CODE "${PROJECT_VERSION_MAJOR} * 10000 + ${PROJECT_VERSION_MINOR} * 100 + ${PROJECT_VERSION_PATCH}")
      message(STATUS "App Version: ${PROJECT_VERSION}, Code: ${VERSION_CODE}")
      # 配置 AndroidManifest.xml（模板在 android/AndroidManifest.xml.in）
      configure_file(
          ${CMAKE_CURRENT_SOURCE_DIR}/android/AndroidManifest.xml.in
          ${CMAKE_CURRENT_SOURCE_DIR}/android/AndroidManifest.xml
          @ONLY
      )
      set_property(TARGET watermark_qt6 APPEND PROPERTY
          QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
      )
      message(STATUS "**** VERSION_CODE: ${VERSION_CODE}")
      get_target_property(PACKAGE_DIR watermark_qt6 QT_ANDROID_PACKAGE_SOURCE_DIR)
      message(STATUS "QT_ANDROID_PACKAGE_SOURCE_DIR = ${PACKAGE_DIR}")

      # qt6_android_generate_deployment_settings(watermark_qt6)
      # qt_android_add_apk_target(watermark_qt6)
    endif()
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
  Qt${QT_VERSION_MAJOR}::Concurrent
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
