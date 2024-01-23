QT       += core gui network concurrent serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/customhighlightdialog.cpp \
    src/mainwindow.cpp \
    src/searchdock.cpp \
    src/tracemanager.cpp \
    src/traceserver.cpp \
    src/traceview.cpp \
    src/main.cpp

HEADERS += \
    inc/constants.h \
    inc/customhighlightdialog.h \
    inc/mainwindow.h \
    inc/searchdock.h \
    inc/tracemanager.h \
    inc/traceserver.h \
    inc/traceview.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = img/favicon.ico
RESOURCES += \
    images.qrc
