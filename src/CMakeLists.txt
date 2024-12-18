cmake_minimum_required(VERSION 3.21.1)

project(MFMTools VERSION 1.0.0 LANGUAGES C CXX)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

configure_file (globals.h.in ${CMAKE_CURRENT_SOURCE_DIR}/globals.h @ONLY)

set(RESOURCE_FILES resources.qrc)

if(${QT_VERSION} VERSION_LESS 5.15.0)
    qt5_add_resources(PROJECT_RESOURCES ${RESOURCE_FILES})
else()
    qt_add_resources(PROJECT_RESOURCES ${RESOURCE_FILES})
endif()


set(PROJECT_SOURCES
    ../deploy/config.json

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

set(TRANSLATION_FILES
    translations/app_en.ts
    translations/app_ru.ts
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${PROJECT_RESOURCES}
    )
    qt_add_translations(${PROJECT_NAME} TS_FILES
        ${TRANSLATION_FILES}
    )
else()
    add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
        ${PROJECT_SOURCES}
        ${PROJECT_RESOURCES}
    )
    qt5_add_translation(QM_FILES ${TRANSLATION_FILES})

    find_program(LUPDATE_EXECUTABLE lupdate)
    find_program(LRELEASE_EXECUTABLE lrelease)

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/languages/)

    foreach(_ts_file ${TRANSLATION_FILES})

        # execute_process(
        #     COMMAND ${LUPDATE_EXECUTABLE} -recursive ${CMAKE_SOURCE_DIR} -ts ${_ts_file})
        get_filename_component(_qt_id ${_ts_file} NAME_WE)
        execute_process(
            COMMAND ${LRELEASE_EXECUTABLE} "${CMAKE_SOURCE_DIR}/${_ts_file}" -qm ${CMAKE_CURRENT_BINARY_DIR}/languages/${_qt_id}.qm )

    endforeach()
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${PROJECT_NAME})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if((QT_VERSION_MAJOR GREATER 5))
    find_package(Qt6 REQUIRED COMPONENTS Core5Compat)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core5Compat)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_source_files_properties(${TRANSLATION_FILES} PROPERTIES OUTPUT_LOCATION "MFMTools.app/Contents/MacOS/languages")
    configure_file(../deploy/config.json MFMTools.app/Contents/MacOS/config.json COPYONLY)
    configure_file(../deploy/mfmtools.ini MFMTools.app/Contents/MacOS/mfmtools.ini COPYONLY)
    configure_file(../deploy/languages/ru.png MFMTools.app/Contents/MacOS/languages/ru.png COPYONLY)
    configure_file(../deploy/languages/en.png MFMTools.app/Contents/MacOS/languages/en.png COPYONLY)

else()
    set_source_files_properties(${TRANSLATION_FILES} PROPERTIES OUTPUT_LOCATION "languages")
    configure_file(../deploy/config.json config.json COPYONLY)
    configure_file(../deploy/mfmtools.ini mfmtools.ini COPYONLY)
    configure_file(../deploy/languages/ru.png languages/ru.png COPYONLY)
    configure_file(../deploy/languages/en.png languages/en.png COPYONLY)
endif()

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
