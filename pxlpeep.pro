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
}

# For Catalina:
macx {
    QMAKE_CXXFLAGS += -Xpreprocessor -fopenmp
    INCLUDEPATH += /usr/local/opt/freeimage/include
    INCLUDEPATH += /usr/local/opt/libomp/include
    LIBS += -L/usr/local/lib -lfreeimage -lomp
}

win32 {
    LIBS += -L$$PWD/../../FreeImageDLL/Dist/x32/ -lFreeImage
    INCLUDEPATH += $$PWD/../../FreeImageDLL/Dist/x32
    DEPENDPATH += $$PWD/../../FreeImageDLL/Dist/x32
}
