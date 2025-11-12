#----------------------------------------------------------------------------------
# Project     : contactless-mifare
# Description : Example of connection with contactless reader and reading record
#               from MIFARE Ultralight cards or MIFARE Classic cards where
#               authentication keys has not been configured yet
#----------------------------------------------------------------------------------

# Set runtime library search path for target device
# Embed library paths in the executable
unix {
    QMAKE_LFLAGS += -Wl,-rpath,/home/dart/apps/lib
    QMAKE_LFLAGS += -Wl,-rpath,/usr/lib
    QMAKE_LFLAGS += -Wl,-rpath,'$$ORIGIN/lib'
    QMAKE_LFLAGS += -Wl,--enable-new-dtags
}

x86 {
  error("Impossible to build for architecture $$SDL_ARCH")
}

TARGET      = demoapp
TEMPLATE    = app
QT         += core gui widgets concurrent
CONFIG     += cmdline
CONFIG     += link_pkgconfig
CONFIG     += c++11
SOURCES    += main.cpp mainwindow.cpp \
    api_client.cpp \
    card_reader.cpp \
    signature_helper.cpp

HEADERS    += mainwindow.h \
    api_client.hpp \
    card_reader.hpp \
    config.hpp \
    scanworker.hpp \
    signature_helper.hpp

FORMS      += mainwindow.ui

# Library paths
LIBS_PATH = /opt/aep-cdb4v2

# Include paths
INCLUDEPATH += $$LIBS_PATH/curl/include
INCLUDEPATH += $$LIBS_PATH/openssl/include
INCLUDEPATH += $$LIBS_PATH/json-c/usr/local/include
INCLUDEPATH += $$LIBS_PATH/yaml-cpp/usr/local/include

# Library linking - order matters!
# Link curl first
LIBS += -L$$LIBS_PATH/curl/lib -lcurl

# Link OpenSSL libraries (crypto and ssl)
LIBS += -L$$LIBS_PATH/openssl/lib -lcrypto -lssl

# Link json-c
LIBS += -L$$LIBS_PATH/json-c/usr/local/lib -ljson-c

# Link yaml-cpp
LIBS += -L$$LIBS_PATH/yaml-cpp/usr/local/lib -lyaml-cpp

# Add system libraries that OpenSSL/curl might need
LIBS += -ldl -lpthread


CONFIG(debug, debug|release) {
  PKGCONFIG  += als-debug coupler-debug
} else {
  PKGCONFIG  += als coupler
}

# Default rules for deployment.
target.path = /tmp
INSTALLS += target

# Install config file
config.path = /home/dart/program-files
config.files = card_config.ini
INSTALLS += config

DISTFILES += \
    card_config.ini
