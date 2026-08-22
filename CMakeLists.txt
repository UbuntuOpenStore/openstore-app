cmake_minimum_required(VERSION 3.16.0)
project(openstore C CXX)

# Default to whichever Qt major version is actually available on this
# system, so the same clickable.yaml/CI matrix (focal still on Qt5,
# 26.04+/next already on Qt6) builds the right one without extra config.
# Pass -DENABLE_QT6=ON/OFF explicitly to override the detection.
if(NOT DEFINED ENABLE_QT6)
    find_package(QT NAMES Qt6 QUIET COMPONENTS Core)
    if(QT_FOUND)
        set(ENABLE_QT6_DEFAULT ON)
    else()
        set(ENABLE_QT6_DEFAULT OFF)
    endif()
    unset(QT_FOUND)
    unset(QT_VERSION_MAJOR)
endif()
option(ENABLE_QT6 "Enable building with Qt6" ${ENABLE_QT6_DEFAULT})

if(ENABLE_QT6)
    set(QT_VERSION_MAJOR 6)
else()
    set(QT_VERSION_MAJOR 5)
endif()

message(STATUS "Qt version is: ${QT_VERSION_MAJOR}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(QT NAMES Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Qml Quick DBus)
find_package(PAM REQUIRED)

include(FindPkgConfig)
pkg_search_module(GOBJECT REQUIRED gobject-2.0)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0 gio-unix-2.0)
pkg_search_module(CLICK REQUIRED click-0.4)
pkg_search_module(SNAPD_QT REQUIRED snapd-qt snapd-qt-2)

# Automatically create moc files
set(CMAKE_AUTOMOC ON)

# Components PATH
execute_process(
    COMMAND dpkg-architecture -qDEB_HOST_MULTIARCH
    OUTPUT_VARIABLE ARCH_TRIPLET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(QT_IMPORTS_DIR "lib/${ARCH_TRIPLET}")
set(PROJECT_NAME "openstore")
set(FULL_PROJECT_NAME "openstore.openstore-team")
set(CMAKE_INSTALL_PREFIX /)
set(DATA_DIR /)
set(DESKTOP_FILE_NAME ${PROJECT_NAME}.desktop)

# This command figures out the target architecture for use in the manifest file
# Either via the environment variable ARCH (set by Clickable) or dpkg
if(DEFINED ENV{ARCH})
    set(CLICK_ARCH "$ENV{ARCH}")
else()
    execute_process(
        COMMAND dpkg-architecture -qDEB_HOST_ARCH
        OUTPUT_VARIABLE CLICK_ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

# Sets BUILD_VERSION: Either tag of the current git HEAD or devel build version with git hash
execute_process(
  COMMAND git describe --tags --abbrev=0 --exact-match
  OUTPUT_VARIABLE BUILD_VERSION_RAW
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
  )
if(NOT BUILD_VERSION_RAW)
  execute_process(
    COMMAND git describe --tags --abbrev=0
    OUTPUT_VARIABLE LAST_VERSION_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    )
  string(TIMESTAMP BUILD_VERSION_RAW "${LAST_VERSION_RAW}.%y%m%d%H%M%S" UTC)
endif(NOT BUILD_VERSION_RAW)
string(SUBSTRING ${BUILD_VERSION_RAW} 1 -1 BUILD_VERSION)
message(STATUS "Build version is: ${BUILD_VERSION}")

if(DEFINED ENV{CLICK_FRAMEWORK_BASE})
    set(BUILD_VERSION "${BUILD_VERSION}-$ENV{CLICK_FRAMEWORK_BASE}")
    message(STATUS "Build version with framework is: ${BUILD_VERSION}")
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    set(LOMIRI_COMPONENTS_VERSION 2.0)
    set(LOMIRI_CONTENT_VERSION 2.0)
    set(LOMIRI_CONNECTIVITY_VERSION 2.0)
else()
    set(LOMIRI_COMPONENTS_VERSION 1.3)
    set(LOMIRI_CONTENT_VERSION 1.3)
    set(LOMIRI_CONNECTIVITY_VERSION 1.0)
endif()

# Generate the QML sources and the resource file from their *.in templates.
# The generated files keep the same relative layout inside the build tree,
file(RELATIVE_PATH QML_SOURCE_DIR_RELATIVE ${CMAKE_BINARY_DIR}/qml ${CMAKE_CURRENT_SOURCE_DIR}/qml)

file(GLOB_RECURSE QML_TEMPLATE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/qml/*.qml.in)
list(SORT QML_TEMPLATE_FILES)
foreach(QML_TEMPLATE_FILE ${QML_TEMPLATE_FILES})
    string(REGEX REPLACE "\\.in$" "" QML_GENERATED_FILE ${QML_TEMPLATE_FILE})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${QML_TEMPLATE_FILE} ${CMAKE_BINARY_DIR}/${QML_GENERATED_FILE} @ONLY)
endforeach()

configure_file(qml/qml.qrc.in ${CMAKE_BINARY_DIR}/qml/qml.qrc @ONLY)

# Config files & the logo
configure_file(manifest.json.in ${CMAKE_CURRENT_BINARY_DIR}/manifest.json)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/manifest.json DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${PROJECT_NAME}.apparmor DESTINATION ${DATA_DIR})
install(FILES ${PROJECT_NAME}.url-dispatcher DESTINATION ${DATA_DIR})
install(FILES ${PROJECT_NAME}-contenthub.json DESTINATION ${DATA_DIR})
install(FILES qml/Assets/${PROJECT_NAME}.svg DESTINATION ${DATA_DIR})
install(FILES qml/Assets/${PROJECT_NAME}splash.svg DESTINATION ${DATA_DIR})

# Translations
file(GLOB_RECURSE I18N_SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/po qml/*.qml.in qml/*.js)
list(APPEND I18N_SRC_FILES ${DESKTOP_FILE_NAME}.in.h)

find_program(INTLTOOL_MERGE intltool-merge)
if(NOT INTLTOOL_MERGE)
    message(FATAL_ERROR "Could not find intltool-merge, please install the intltool package")
endif()
find_program(INTLTOOL_EXTRACT intltool-extract)
if(NOT INTLTOOL_EXTRACT)
    message(FATAL_ERROR "Could not find intltool-extract, please install the intltool package")
endif()

add_custom_target(${DESKTOP_FILE_NAME} ALL
    COMMENT "Merging translations into ${DESKTOP_FILE_NAME}..."
    COMMAND LC_ALL=C ${INTLTOOL_MERGE} -d -u ${CMAKE_SOURCE_DIR}/po ${CMAKE_SOURCE_DIR}/${DESKTOP_FILE_NAME}.in ${DESKTOP_FILE_NAME}
    COMMAND sed -i 's/${PROJECT_NAME}-//g' ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME} DESTINATION ${DATA_DIR})

# Make source files visible in qtcreator
file(GLOB_RECURSE PROJECT_SRC_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    qml/*.qml.in
    qml/*.js
    *.json
    *.json.in
    *.apparmor
    *.desktop.in
    *.url-dispatcher
)

add_custom_target(${PROJECT_NAME}_FILES ALL SOURCES ${PROJECT_SRC_FILES})

add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(plugins)
