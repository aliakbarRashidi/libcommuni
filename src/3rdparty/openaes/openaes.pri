######################################################################
# Communi
######################################################################

INCLUDEPATH += $$PWD/inc $$PWD/src/isaac
DEPENDPATH += $$PWD/inc $$PWD/src $$PWD/src/isaac

HEADERS += $$PWD/inc/oaes_base64.h
HEADERS += $$PWD/inc/oaes_common.h
HEADERS += $$PWD/inc/oaes_config.h
HEADERS += $$PWD/inc/oaes_lib.h
HEADERS += $$PWD/src/isaac/rand.h
HEADERS += $$PWD/src/isaac/standard.h

SOURCES += $$PWD/src/oaes_base64.c
SOURCES += $$PWD/src/oaes_lib.c
SOURCES += $$PWD/src/oaes.c
SOURCES += $$PWD/src/isaac/rand.c
