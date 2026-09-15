include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${CLICK_INCLUDE_DIRS})
include_directories(${PAM_INCLUDE_DIR})
include_directories(${SNAPD_QT_INCLUDE_DIRS})
# Hardcode's last
include_directories("/usr/include/snapd-qt-2")
include_directories("/usr/include/snapd-qt")

add_definitions(-DQT_NO_KEYWORDS)

set(
    SRC
    main.cpp
    models/categoriesmodel.cpp
    installers/clickinstaller.cpp
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
    packageitems/clickpackage.cpp
    packageitems/snappackage.cpp
    packagescache.cpp
    cachingnetworkmanagerfactory.cpp
    models/reviewsmodel.cpp
    review.cpp
)

add_definitions(-DBUILD_VERSION="${BUILD_VERSION}")

if(ENABLE_DEB_SUPPORT)
    add_definitions(-DENABLE_DEB_SUPPORT)
    include_directories(${PACKAGEKIT_INCLUDE_DIRS})
    # packagekitqt6 headers include <packagekitqt_global.h> without a
    # PackageKit/ prefix; that header lives in this sibling subdir, so the
    # pkg-config include path alone does not resolve it.
    include_directories("/usr/include/PackageKitQt/PackageKit")
    include_directories(${XAPIAN_INCLUDE_DIRS})
    set(
        DEB_SRC
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
if(ENABLE_DEB_SUPPORT)
    add_executable(${PROJECT_NAME} ${SRC} ${DEB_SRC} ${QT_RESOURCES})
else()
    add_executable(${PROJECT_NAME} ${SRC} ${QT_RESOURCES})
endif()
target_link_libraries(${PROJECT_NAME}
                      Qt${QT_VERSION_MAJOR}::Gui
                      Qt${QT_VERSION_MAJOR}::Qml
                      Qt${QT_VERSION_MAJOR}::Quick
                      Qt${QT_VERSION_MAJOR}::QuickControls2
                      Qt${QT_VERSION_MAJOR}::DBus
                      ${GLIB_LDFLAGS} ${GIO_LDFLAGS} ${CLICK_LDFLAGS}
                      ${PAM_LIBRARIES} ${SNAPD_QT_LIBRARIES})
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${PROJECT_NAME} Qt6::WebEngineQuick)
endif()
if(ENABLE_DEB_SUPPORT)
    target_link_libraries(${PROJECT_NAME} ${PACKAGEKIT_QT_LIBRARIES} ${XAPIAN_LDFLAGS} AppStreamQt)
endif()
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

