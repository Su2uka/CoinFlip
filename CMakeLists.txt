cmake_minimum_required(VERSION 3.21)

project(CoinFlip VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 可由 CI 覆盖：cmake -DAPP_VERSION=v1.2.3
set(APP_VERSION "1.0.0" CACHE STRING "Application version string")

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Multimedia)

qt_standard_project_setup()

set(COINFLIP_SOURCES
    main.cpp
    src/mainwindow.cpp
    src/mainmenuwidget.cpp
    src/chooselevelwidget.cpp
    src/playwidget.cpp
    src/iconbutton.cpp
    src/coinbutton.cpp
    src/soundplayer.cpp
    src/leveldatabase.cpp
    src/progressmanager.cpp
    src/recordmanager.cpp
)

set(COINFLIP_HEADERS
    include/mainwindow.h
    include/mainmenuwidget.h
    include/chooselevelwidget.h
    include/playwidget.h
    include/iconbutton.h
    include/coinbutton.h
    include/soundplayer.h
    include/leveldatabase.h
    include/progressmanager.h
    include/recordmanager.h
)

qt_add_executable(CoinFlip WIN32
    ${COINFLIP_SOURCES}
    ${COINFLIP_HEADERS}
    resource/res.qrc
)

target_include_directories(CoinFlip PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(CoinFlip PRIVATE APP_VERSION="${APP_VERSION}")

if(MSVC)
    # 源码为 UTF-8（含中文字符串字面量）
    target_compile_options(CoinFlip PRIVATE /utf-8)
endif()

target_link_libraries(CoinFlip PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
)

if(WIN32)
    # Windows 版本信息 + 应用图标（.ico 路径写入生成的 rc，避免 windres 相对路径问题）
    string(REPLACE "." "," COINFLIP_VERSION_COMMA ${PROJECT_VERSION})
    configure_file(resource/CoinFlip.rc.in ${CMAKE_CURRENT_BINARY_DIR}/CoinFlip.rc @ONLY)
    target_sources(CoinFlip PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/CoinFlip.rc)
endif()

install(TARGETS CoinFlip
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION .
)
