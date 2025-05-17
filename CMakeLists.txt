cmake_minimum_required(VERSION 3.12)
include(GNUInstallDirs)

project(qtfm VERSION 6.3.0)

add_definitions(-DAPP="${PROJECT_NAME}")
add_definitions(-DAPP_NAME="QtFM")
add_definitions(-DAPP_VERSION="${PROJECT_VERSION}${PROJECT_VERSION_EXTRA}")

#set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
include_directories(${CMAKE_BINARY_DIR})

if(CMAKE_BUILD_TYPE MATCHES "^(release|Release|RELEASE)$")
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
else()
    message("-- DEBUG BUILD")
endif()
add_compile_options(-Wall -Wextra)

# configure
set(ENABLE_MAGICK true CACHE BOOL "Enable support for ImageMagick")
set(ENABLE_FFMPEG true CACHE BOOL "Enable support for FFmpeg")
set(ENABLE_DBUS true CACHE BOOL "Enable support for D-Bus")
set(ENABLE_UDISKS true CACHE BOOL "Enable support for udisks")
set(ENABLE_TRAY true CACHE BOOL "Enable support for qtfm-tray")

# magick
if(ENABLE_MAGICK)
    add_definitions(-DWITH_MAGICK)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MAGICK REQUIRED Magick++)
    # ffmpeg
    if(ENABLE_FFMPEG)
        pkg_check_modules(AVDEVICE REQUIRED libavdevice)
        pkg_check_modules(SWSCALE REQUIRED libswscale)
        pkg_check_modules(AVFORMAT REQUIRED libavformat)
        pkg_check_modules(AVCODEC REQUIRED libavcodec)
        pkg_check_modules(AVUTIL REQUIRED libavutil)
        add_definitions(-DWITH_FFMPEG)
        add_compile_options(-fpermissive) # a fix for ffmpeg 5
    endif()
endif()

# dbus
if(NOT ENABLE_DBUS)
    add_definitions(-DNO_DBUS -DNO_UDISKS)
endif()
if(NOT ENABLE_UDISKS AND ENABLE_DBUS)
    add_definitions(-DNO_UDISKS)
endif()

# qt
find_package(QT NAMES Qt5 COMPONENTS Core REQUIRED)
if(ENABLE_DBUS)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS DBus REQUIRED)
endif()
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui Widgets Concurrent REQUIRED)

# inotify
if(NOT "${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(NOTIFY REQUIRED libinotify)
endif()

# libfm
if(ENABLE_DBUS)
    add_library(
        QtFMBus
        STATIC
        libfm/disks.cpp
        libfm/service.h
        libfm/udisks2.cpp
    )
    target_link_libraries(QtFMBus Qt${QT_VERSION_MAJOR}::DBus)
    target_include_directories(QtFMBus PRIVATE libfm)
endif()
add_library(
    QtFM
    SHARED
    libfm/applicationdialog.cpp
    libfm/bookmarkmodel.cpp
    libfm/common.cpp
    libfm/completer.cpp
    libfm/customactionsmanager.cpp
    libfm/desktopfile.cpp
    libfm/fileutils.cpp
    libfm/fm.cpp
    libfm/icondlg.cpp
    libfm/iconlist.cpp
    libfm/iconview.cpp
    libfm/mimeutils.cpp
    libfm/mymodel.cpp
    libfm/mymodelitem.cpp
    libfm/processdialog.cpp
    libfm/properties.cpp
    libfm/propertiesdlg.cpp
    libfm/sortmodel.cpp
    libfm/dfmqtreeview.cpp
    libfm/dfmqstyleditemdelegate.cpp
    libfm/qtcopydialog/qtcopydialog.cpp
    libfm/qtcopydialog/qtfilecopier.cpp
    libfm/qtcopydialog/qtcopydialog.ui
    libfm/qtcopydialog/qtotherdialog.ui
    libfm/qtcopydialog/qtoverwritedialog.ui
)
set_target_properties(
    QtFM
    PROPERTIES
    VERSION
    ${PROJECT_VERSION}
)
set_target_properties(
    QtFM
    PROPERTIES
    SOVERSION
    ${PROJECT_VERSION_MAJOR}
)
if(ENABLE_MAGICK)
    string(REGEX MATCH "\\-DMAGICKCORE_HDRI_ENABLE=[^ $]" MAGICK_HDRI ${MAGICK_CFLAGS})
    add_definitions(-DMAGICKCORE_QUANTUM_DEPTH=16)
    add_definitions(${MAGICK_HDRI})
    target_include_directories(QtFM PRIVATE ${MAGICK_INCLUDE_DIRS})
    target_link_libraries(QtFM ${MAGICK_LIBRARIES})
    if(ENABLE_FFMPEG)
        target_include_directories(QtFM PRIVATE ${AVUTIL_INCLUDEDIR})
        target_link_libraries(QtFM
                              ${AVDEVICE_LIBRARIES}
                              ${SWSCALE_LIBRARIES}
                              ${AVFORMAT_LIBRARIES}
                              ${AVCODEC_LIBRARIES}
                              ${AVUTIL_LIBRARIES})
    endif()
endif()
target_include_directories(QtFM PRIVATE libfm libfm/qtcopydialog ${NOTIFY_INCLUDEDIR})

target_link_libraries(
    QtFM
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
)
if(ENABLE_DBUS)
    target_link_libraries(QtFM Qt${QT_VERSION_MAJOR}::DBus QtFMBus)
endif()
if(NOT "${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    target_link_libraries(QtFM ${NOTIFY_LIBRARIES})
endif()
if(APPLE)
    target_link_libraries(QtFM -framework CoreFoundation -framework CoreServices)
endif()

# qtfm
add_executable(
    ${PROJECT_NAME}
    fm/src/main.cpp
    fm/src/mainwindow.cpp
    fm/src/bookmarks.cpp
    fm/src/tabbar.cpp
    fm/src/settingsdialog.cpp
    fm/src/actiondefs.cpp
    fm/src/actiontriggers.cpp
    share/${PROJECT_NAME}.qrc
)
target_link_libraries(
    ${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    QtFM
)
target_include_directories(${PROJECT_NAME} PRIVATE fm/src libfm libfm/qtcopydialog)
if(ENABLE_DBUS)
    target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::DBus)
endif()
if(NOT "${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    target_link_libraries(${PROJECT_NAME} ${NOTIFY_LIBRARIES})
endif()
if(APPLE)
    target_link_libraries(${PROJECT_NAME} -framework CoreFoundation -framework CoreServices)
endif()

# qtfm-tray
if(ENABLE_DBUS AND ENABLE_TRAY)
    add_executable(${PROJECT_NAME}-tray tray/main.cpp tray/systray.cpp tray/tray.qrc)
    target_include_directories(${PROJECT_NAME}-tray PRIVATE tray libfm)
    target_link_libraries(
        ${PROJECT_NAME}-tray
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        QtFM
    )
endif()

# docs
install(
    FILES
    LICENSE
    AUTHORS
    ChangeLog
    README.md
    DESTINATION
    ${CMAKE_INSTALL_DOCDIR}/${PROJECT_NAME}-${PROJECT_VERSION}${PROJECT_VERSION_EXTRA}
)

# etc
set(DEFAULT_SYSCONFDIR "${CMAKE_INSTALL_PREFIX}/etc")
if(CMAKE_INSTALL_PREFIX STREQUAL "/usr")
    set(DEFAULT_SYSCONFDIR "/etc")
endif()

# libdir
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
else()
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

# install
install(
    TARGETS
    QtFM
    DESTINATION
    ${LIB_INSTALL_DIR}
)
install(
    TARGETS
    ${PROJECT_NAME}
    DESTINATION
    ${CMAKE_INSTALL_BINDIR}
)
install(
    FILES
    fm/${PROJECT_NAME}.1
    DESTINATION
    ${CMAKE_INSTALL_MANDIR}/man1
)
install(
    FILES
    fm/${PROJECT_NAME}.desktop
    DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/applications
)
install(
    DIRECTORY
    share/hicolor
    DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/icons
)
if(ENABLE_DBUS AND ENABLE_TRAY)
    install(
        TARGETS
        ${PROJECT_NAME}-tray
        DESTINATION
        ${CMAKE_INSTALL_BINDIR}
    )
    install(
        FILES
        tray/${PROJECT_NAME}-tray.1
        DESTINATION
        ${CMAKE_INSTALL_MANDIR}/man1
    )
endif()
