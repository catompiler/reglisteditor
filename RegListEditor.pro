QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    regdelegate.cpp \
    regentry.cpp \
    regentrydlg.cpp \
    reglisteditorwin.cpp \
    reglistmodel.cpp \
    regobject.cpp \
    regselectdlg.cpp \
    regselectmodel.cpp \
    regtypes.cpp \
    regutils.cpp \
    regvar.cpp \
    regvarmodel.cpp \
    sellineedit.cpp

HEADERS += \
    is_base_creator.hpp \
    regdelegate.h \
    regentry.h \
    regentrydlg.h \
    reglisteditorwin.h \
    reglistmodel.h \
    regobject.h \
    regselectdlg.h \
    regselectmodel.h \
    regtypes.h \
    regutils.h \
    regvar.h \
    regvarmodel.h \
    sellineedit.h

FORMS += \
    regentrydlg.ui \
    reglisteditorwin.ui \
    regselectdlg.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    reglisteditor.qrc
