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
    clickinstaller.cpp
    models/discovermodel.cpp
    models/localpackagesmodel.cpp
    models/searchmodel.cpp
    openstorenetworkmanager.cpp
    platformintegration.cpp
    package.cpp
    clickpackage.cpp
    snappackage.cpp
    packagescache.cpp
    cachingnetworkmanagerfactory.cpp
    models/reviewsmodel.cpp
    review.cpp
)

add_definitions(-DBUILD_VERSION="${BUILD_VERSION}")

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Qml Quick QuickControls2 DBus)

if(QT_VERSION_MAJOR EQUAL 6)
    qt6_add_resources(QT_RESOURCES ../qml/qml.qrc)
else()
    qt5_add_resources(QT_RESOURCES ../qml/qml.qrc)
endif()
add_executable(${PROJECT_NAME} ${SRC} ${QT_RESOURCES})
target_link_libraries(${PROJECT_NAME} Qt::Gui Qt::Qml Qt::Quick Qt::QuickControls2 Qt::DBus
                      ${GLIB_LDFLAGS} ${GIO_LDFLAGS} ${CLICK_LDFLAGS}
                      ${PAM_LIBRARIES} ${SNAPD_QT_LIBRARIES})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
