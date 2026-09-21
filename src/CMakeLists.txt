include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${PAM_INCLUDE_DIR})
if(ENABLE_CLICK_SUPPORT)
    include_directories(${CLICK_INCLUDE_DIRS})
endif()
if(ENABLE_SNAP_SUPPORT)
    include_directories(${SNAPD_QT_INCLUDE_DIRS})
    # Hardcode's last
    include_directories("/usr/include/snapd-qt-2")
    include_directories("/usr/include/snapd-qt")
endif()

add_definitions(-DQT_NO_KEYWORDS)

set(
    SRC
    main.cpp
    models/categoriesmodel.cpp
    packageSource.cpp
    packagebackendmanager.cpp
    backendinstaller.cpp
    sources/remoteapisource.cpp
    models/discovermodel.cpp
    models/localpackagesmodel.cpp
    models/searchmodel.cpp
    openstorenetworkmanager.cpp
    platformintegration.cpp
    packageitems/package.cpp
    packagescache.cpp
    cachingnetworkmanagerfactory.cpp
    models/reviewsmodel.cpp
    review.cpp
)

add_definitions(-DBUILD_VERSION="${BUILD_VERSION}")

if(ENABLE_CLICK_SUPPORT)
    add_definitions(-DENABLE_CLICK_SUPPORT)
    set(
        CLICK_SRC
        installers/clickinstaller.cpp
        packageitems/clickpackage.cpp
    )
endif()

if(ENABLE_SNAP_SUPPORT)
    add_definitions(-DENABLE_SNAP_SUPPORT)
    set(
        SNAP_SRC
        packageitems/snappackage.cpp
    )
endif()

if(ENABLE_PACKAGEKIT_SUPPORT)
    add_definitions(-DENABLE_PACKAGEKIT_SUPPORT)
    include_directories(${PACKAGEKIT_INCLUDE_DIRS})
    # packagekitqt6 headers include <packagekitqt_global.h> without a
    # PackageKit/ prefix; that header lives in this sibling subdir, so the
    # pkg-config include path alone does not resolve it.
    include_directories("/usr/include/PackageKitQt/PackageKit")
    include_directories(${XAPIAN_INCLUDE_DIRS})
    set(
        PACKAGEKIT_SRC
        installers/packagekitinstaller.cpp
        appstreampool.cpp
        indexstatus.cpp
        packageindex.cpp
        xapianindex.cpp
        packageitems/packagekitpackage.cpp
        sources/packagekitsource.cpp
    )
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Qml Quick QuickControls2 DBus)

if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 COMPONENTS WebEngineQuick REQUIRED)
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt6_add_resources(QT_RESOURCES ${CMAKE_BINARY_DIR}/qml/qml.qrc)
else()
    qt5_add_resources(QT_RESOURCES ${CMAKE_BINARY_DIR}/qml/qml.qrc)
endif()
if(ENABLE_PACKAGEKIT_SUPPORT)
    add_executable(${PROJECT_NAME} ${SRC} ${PACKAGEKIT_SRC} ${QT_RESOURCES})
else()
    add_executable(${PROJECT_NAME} ${SRC} ${CLICK_SRC} ${SNAP_SRC} ${QT_RESOURCES})
endif()
target_link_libraries(${PROJECT_NAME}
                      Qt${QT_VERSION_MAJOR}::Gui
                      Qt${QT_VERSION_MAJOR}::Qml
                      Qt${QT_VERSION_MAJOR}::Quick
                      Qt${QT_VERSION_MAJOR}::QuickControls2
                      Qt${QT_VERSION_MAJOR}::DBus
                      ${GLIB_LDFLAGS} ${GIO_LDFLAGS} ${PAM_LIBRARIES})
if(ENABLE_CLICK_SUPPORT)
    target_link_libraries(${PROJECT_NAME} ${CLICK_LDFLAGS})
endif()
if(ENABLE_SNAP_SUPPORT)
    target_link_libraries(${PROJECT_NAME} ${SNAPD_QT_LIBRARIES})
endif()
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${PROJECT_NAME} Qt6::WebEngineQuick)
endif()
if(ENABLE_PACKAGEKIT_SUPPORT)
    target_link_libraries(${PROJECT_NAME} ${PACKAGEKIT_QT_LIBRARIES} ${XAPIAN_LDFLAGS} AppStreamQt)
endif()
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

