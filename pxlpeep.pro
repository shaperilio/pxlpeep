#-------------------------------------------------
#
# Project created by QtCreator 2015-05-30T16:11:06
#
#-------------------------------------------------

QT += core gui
QT += opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = pxlpeep
TEMPLATE = app


SOURCES +=\
    main.cpp \
    ImageWindow.cpp \
    ImageData.cpp \
    MainDialog.cpp

HEADERS  += \
    colormapper.h \
    ImageWindow.h \
    ImageData.h \
    definitions.h \
    MainDialog.h
FORMS    += \
    MainDialog.ui

RESOURCES += \
    pxlpeep.qrc

#http://stackoverflow.com/a/14999452/149506
CONFIG(debug, release|debug):DEFINES += _DEBUG

# For linux:
unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp -lfreeimage

    # Statically link qt components so that we don't get weird errors when trying to load platform plugins.
    # Doesn't work on Mac.
    CONFIG += qt static
}

# For Catalina:
macx {
    QMAKE_CXXFLAGS += -Xpreprocessor -fopenmp
    INCLUDEPATH += /usr/local/opt/freeimage/include
    INCLUDEPATH += /usr/local/opt/libomp/include
    LIBS += -L/usr/local/lib -lfreeimage -lomp
    ICON = loupe.icns
    DEFINES += Q_OS_MACOS
}

win32 {
    LIBS += -L$$PWD/../../FreeImageDLL/Dist/x32/ -lFreeImage
    INCLUDEPATH += $$PWD/../../FreeImageDLL/Dist/x32
    DEPENDPATH += $$PWD/../../FreeImageDLL/Dist/x32
}

# Note. To debug this on Mac, you may have to do the following:
# Thanks to this guy for saving me: https://stackoverflow.com/a/35070568/149506
#
# mv FILE.dylib FILE.dylib.old
# ln -s /System/Library/Frameworks/ImageIO.framework/Versions/A/ImageIO/FILE.dylib /usr/local/lib/FILE.dylib
#
# and tab completion doesn't work for some reason? (hidden files?)
# I had to do this for the following values of FILE:
# libJPEG, libGIF, libTIFF, libPnG

DISTFILES += \
    pxlpeep.desktop \
    pxlpeep.sh
