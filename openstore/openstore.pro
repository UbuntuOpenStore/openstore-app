TEMPLATE = app
TARGET = openstore

load(ubuntu-click)

QT += qml quick
CONFIG += c++11

DEFINES += QT_NO_KEYWORDS
testing {
    DEFINES += TESTING
}

INCLUDEPATH += /usr/include/click-0.4/ \
    /usr/include/glib-2.0/ \
    /usr/lib/x86_64-linux-gnu/glib-2.0/include \
    /usr/lib/arm-linux-gnueabihf/glib-2.0/include \
    /usr/include/json-glib-1.0 \

LIBS += -lglib-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lclick-0.4

SOURCES += main.cpp \
    categoriesmodel.cpp \
    clickinstaller.cpp \
    discovermodel.cpp \
    packagesmodel.cpp \
    searchmodel.cpp \
    serviceregistry.cpp \
    openstorenetworkmanager.cpp \
    platformintegration.cpp \
    package.cpp \
    packagescache.cpp \
    CachingNetworkManagerFactory.cpp


RESOURCES += openstore.qrc

OTHER_FILES += openstore.apparmor \
               openstore.desktop \
               openstore.svg \
               openstore.url-dispatcher \
               openstore-contenthub.json

#specify where the config files are installed to
config_files.path = /openstore
config_files.files += $${OTHER_FILES}
message($$config_files.files)
INSTALLS+=config_files

# Default rules for deployment.
target.path = $${UBUNTU_CLICK_BINARY_PATH}
INSTALLS+=target

HEADERS += \
    categoriesmodel.h \
    clickinstaller.h \
    discovermodel.h \
    packagesmodel.h \
    searchmodel.h \
    serviceregistry.h \
    openstorenetworkmanager.h \
    platformintegration.h \
    packagescache.h \
    package.h \
    CachingNetworkManagerFactory.h \
    apiconstants.h
