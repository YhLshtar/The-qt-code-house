QT       += core gui
QT       += network
QT       += serialport
QT       += concurrent
QT       += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    common/common.cpp \
    common/configurationfile.cpp \
    common/myqmessage.cpp \
    include/AerialCamera.cpp \
    include/AxleCamera.cpp \
    include/IOcontrol.cpp \
    include/McuControl.cpp \
    include/camerapreview1.cpp \
    include/camerapreview2.cpp \
    include/screentest.cpp \
    libraries/CameraControl.cpp \
    libraries/VideoCapture.cpp \
    libraries/AxleVideoCapture.cpp \
    libraries/VirtualKeyboard.cpp \
    main.cpp \
    mmitest.cpp

HEADERS += \
    common/common.h \
    common/configurationfile.h \
    common/myqmessage.h \
    include/AerialCamera.h \
    include/AxleCamera.h \
    include/IOcontrol.h \
    include/McuControl.h \
    include/camerapreview1.h \
    include/camerapreview2.h \
    include/screentest.h \
    libraries/CameraControl.h \
    libraries/VideoCapture.h \
    libraries/AxleVideoCapture.h \
    libraries/VirtualKeyboard.h \
    mmitest.h

FORMS += \
    include/camerapreview1.ui \
    include/camerapreview2.ui \
    include/screentest.ui \
    libraries/VirtualKeyboard.ui \
    mmitest.ui

QT_CONFIG -= no-pkg-config
CONFIG    += sdk_no_version_check
CONFIG    += link_pkgconfig
PKGCONFIG += opencv4

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc
