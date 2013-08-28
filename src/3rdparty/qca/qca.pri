######################################################################
# Communi
######################################################################

INCLUDEPATH += $$PWD/include/QtCrypto $$PWD/src
DEPENDPATH += $$PWD/include/QtCrypto $$PWD/src

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000

HEADERS += $$PWD/include/QtCrypto/qca_basic.h
HEADERS += $$PWD/include/QtCrypto/qca_cert.h
HEADERS += $$PWD/include/QtCrypto/qca_core.h
HEADERS += $$PWD/include/QtCrypto/qca_export.h
HEADERS += $$PWD/include/QtCrypto/qca_keystore.h
HEADERS += $$PWD/include/QtCrypto/qca_publickey.h
HEADERS += $$PWD/include/QtCrypto/qca_securelayer.h
HEADERS += $$PWD/include/QtCrypto/qca_securemessage.h
HEADERS += $$PWD/include/QtCrypto/qca_support.h
HEADERS += $$PWD/include/QtCrypto/qca_textfilter.h
HEADERS += $$PWD/include/QtCrypto/qca_tools.h
HEADERS += $$PWD/include/QtCrypto/qca.h
HEADERS += $$PWD/include/QtCrypto/qcaprovider.h
HEADERS += $$PWD/include/QtCrypto/qpipe.h

HEADERS += $$PWD/src/qca_plugin.h
HEADERS += $$PWD/src/qca_safeobj.h
HEADERS += $$PWD/src/qca_systemstore.h

SOURCES += $$PWD/src/support/console.cpp
SOURCES += $$PWD/src/support/dirwatch.cpp
SOURCES += $$PWD/src/support/logger.cpp
SOURCES += $$PWD/src/support/qpipe.cpp
SOURCES += $$PWD/src/support/synchronizer.cpp
SOURCES += $$PWD/src/support/syncthread.cpp

SOURCES += $$PWD/src/qca_basic.cpp
SOURCES += $$PWD/src/qca_cert.cpp
SOURCES += $$PWD/src/qca_core.cpp
SOURCES += $$PWD/src/qca_default.cpp
SOURCES += $$PWD/src/qca_keystore.cpp
SOURCES += $$PWD/src/qca_plugin.cpp
SOURCES += $$PWD/src/qca_publickey.cpp
SOURCES += $$PWD/src/qca_safeobj.cpp
SOURCES += $$PWD/src/qca_securelayer.cpp
SOURCES += $$PWD/src/qca_securemessage.cpp
SOURCES += $$PWD/src/qca_textfilter.cpp
SOURCES += $$PWD/src/qca_tools.cpp

win32 {
    SOURCES += $$PWD/src/qca_systemstore_win.cpp
    LIBS += -lcrypt32
} else:mac {
    SOURCES += $$PWD/src/qca_systemstore_mac.cpp
    LIBS += -framework Carbon -framework Security
} else:unix {
    SOURCES += $$PWD/src/qca_systemstore_flatfile.cpp
}

include(src/botantools/botantools.pri)
