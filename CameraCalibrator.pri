# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Tools.
# ------------------------------------------------------

# This is a reminder that you are using a generated .pro file.
# Remove it when you are finished editing this file.
message("You are running qmake on a generated .pro file. This may not work!")


HEADERS += ./qcustomplot.h \
    ./Camera.h \
    ./resource.h \
    ./MainWindow.h \
    ./GraphicsViewZoom.h \
    ./GraphicsSceneClass.h \
    ./Workerthread.h \
    ./Camera/AbstractCameraCalibrator.h \
    ./Camera/CalibratorConfiguration.h \
    ./Camera/CameraCalibratorHelper.h \
    ./Camera/MonoCameraCalibrator.h \
    ./Camera/StereoCameraCalibrator.h
SOURCES += ./Camera.cpp \
    ./GraphicsSceneClass.cpp \
    ./GraphicsViewZoom.cpp \
    ./Workerthread.cpp \
    ./MainWindow.cpp \
    ./Main.cpp \
    ./qcustomplot.cpp \
    ./Camera/AbstractCameraCalibrator.cpp \
    ./Camera/CalibratorConfiguration.cpp \
    ./Camera/CameraCalibratorHelper.cpp \
    ./Camera/MonoCameraCalibrator.cpp \
    ./Camera/StereoCameraCalibrator.cpp
FORMS += ./MainWindow.ui
RESOURCES += CameraCalibrator.qrc \
    loader.qrc