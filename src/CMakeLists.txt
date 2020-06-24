include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${CLICK_INCLUDE_DIRS})
include_directories(${PAM_INCLUDE_DIR})
add_definitions(-DQT_NO_KEYWORDS)

set(
    SRC
    main.cpp
    categoriesmodel.cpp
    clickinstaller.cpp
    discovermodel.cpp
    packagesmodel.cpp
    searchmodel.cpp
    serviceregistry.cpp
    openstorenetworkmanager.cpp
    platformintegration.cpp
    package.cpp
    packagescache.cpp
    pamauthentication.cpp
    cachingnetworkmanagerfactory.cpp
    reviewsmodel.cpp
    review.cpp
)

qt5_add_resources(QT_RESOURCES ../qml/qml.qrc)
add_executable(${PROJECT_NAME} ${SRC} ${QT_RESOURCES})
qt5_use_modules(${PROJECT_NAME} Gui Qml Quick QuickControls2 DBus)
target_link_libraries(${PROJECT_NAME} ${GLIB_LDFLAGS} ${GIO_LDFLAGS} ${CLICK_LDFLAGS} ${PAM_LIBRARIES})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
