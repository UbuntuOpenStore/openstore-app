# NOTE: This is not finished yet... for now, it only uses the service registry inside the app.
# This would be for future use if we'd want to monitor the file system for newly installed packages
# and pick up serviec entries from there. That would allow us to support the services hook even
# when not installing an app through openstore itself.
# Let's see where the journey to snappy/systemd goes, and decide if we want to finish this or drop it.

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
