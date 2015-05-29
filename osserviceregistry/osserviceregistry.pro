TARGET = osserviceregistry
CONFIG   += console

load(ubuntu-click)

QT       += core
QT       -= gui


TEMPLATE = app


SOURCES += main.cpp \
    serviceregistry.cpp

HEADERS += \
    serviceregistry.h

# Default rules for deployment.
target.path = $${UBUNTU_CLICK_BINARY_PATH}
INSTALLS+=target
