cmake_minimum_required(VERSION 3.16)

project(MFMTools VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

configure_file (globals.h.in ${CMAKE_CURRENT_SOURCE_DIR}/globals.h @ONLY)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS Core5Compat Widgets)

qt_add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
	${PROJECT_RESOURCES}
    aboutdlg.ui
    converter.cpp converter.h
    diskparamsdlg.cpp diskparamsdlg.h diskparamsdlg.ui
    libs/config.h
    libs/crc16.c libs/crc16.h
    libs/disk_formats.cpp libs/disk_formats.h
    libs/hcombo.cpp libs/hcombo.h
    libs/loader.cpp libs/loader.h
    libs/loader_raw.cpp libs/loader_raw.h
    libs/loader_td0.cpp libs/loader_td0.h
    libs/lzss.c libs/lzss.h
    libs/mfm_formats.h
    libs/writer.cpp libs/writer.h
    libs/writer_mfm.cpp libs/writer_mfm.h
    libs/writer_raw.cpp libs/writer_raw.h
    loggerdlg.cpp loggerdlg.h loggerdlg.ui
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
)

set(resources_resource_files
    "libs/icons/2leftarrow.png"
    "libs/icons/2rightarrow.png"
    "libs/icons/5floppy_mount.png"
    "libs/icons/5floppy_mount_32x32.png"
    "libs/icons/Emuverse3.png"
    "libs/icons/cache.png"
    "libs/icons/help.png"
    "libs/icons/messagebox_info.png"
)

qt6_add_resources(MFMTools "resources"
    PREFIX
        "/icons"
    FILES
        ${resources_resource_files}
)

target_link_libraries(MFMTools PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

if((QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(MFMTools PUBLIC
        Qt::Core5Compat
    )
endif()

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

qt_add_translations(${PROJECT_NAME} TS_FILES
    deploy/languages/en_US.ts
    deploy/languages/ru_RU.ts
)
