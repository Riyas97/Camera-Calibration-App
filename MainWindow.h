
#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_
#define RVISIONLIB_HAVE_QT

#include "Camera/CalibratorConfiguration.h"
#include "Camera/MonoCameraCalibrator.h"
#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include <opencv2/core/mat.hpp>
#include <QWidget>
#include <QGraphicsView>
#include <qgraphicsitem.h>
#include <qgraphicsscene.h>
#include <QLabel>

/*
 * This class is responsible for the main window interface. 
 * It contains various ui elements and calls respective functions when these elements are invoked by the user. 
 */
class MainWindow : public QMainWindow, public Ui::MainWindowClass
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = nullptr);
    virtual QString getLogTime(); /* to obtain date and time for log messages */
    void setLoadingIcon(QGraphicsView* graphicsView); /* to set loading icon while loading images */
    void displayMessageBox(QString type, QString msg); /* to display message boxes when there is an error/info to be displayed */
    void saveUserConfigurations(); /* save camera calibration configurations entered by user */
    void initializeGraphicsView(QGraphicsView* graphicsView, QString msg, bool isOrigPics); /* to initialize graphics view that displays images */
    void changeNoOfPicsPerRowDisplayed(); /* to change the number of images displayed per row in multiview */
    void setCameraParamsLabels(QLabel* label, double val); /* to set obtained camera parameters in the ui */
    void clearCameraParamsLabels(); /* to clear camera parameters in the ui */
    
private slots:
    void setUISettings(); /* set initial ui settings and initializations */
    void loadConfiguration(); /* load default camera calibration configurations */
    void onStartCalibButtonClicked(); /* invoked when 'Start Calibration' button is clicked to start camera calibration */
    void onFileOpen(); /* invoked when 'Open' button is clicked to load configurations from JSON file */
    void onSave(); /* invoked when user wants to save camera configurations into already saved JSON file */
    void onSaveAs(); /* invoked when user wants to save camera configurations into new JSON file */
    void onBrowseCalibImgButtonClicked(); /* invoked when 'Browse' button is clicked to upload images */
    void onPrevOrigPicButtonClicked(); /* invoked when user clicks button to navigate to previous original image in single view */
    void onNextOrigPicButtonClicked(); /* invoked when user clicks button to navigate to next original image in single view */
    void displayOrigImagesSingleView(); /* to display original images in single view mode */
    void onOrigPicSingleViewButtonClicked(); /* invoked when user clicks the 'Single View' button to view images in single view mode */
    void onOrigPicMultiViewButtonClicked(); /* invoked when user clicks the 'MultiView' button to view images in multiple view mode */
    void displayOrigImagesMultiView(QList<QPixmap> PixList); /* to display original images in multi view mode */
    void obtainOrigImages(QList<QPixmap> PixList); /* obtain list of orig images in pixmap from worker thread */
    void obtainCalibratedImages(QList<QPixmap> PixList, QList<QString> CoverageParams, QList<QString> RMSErrorList); /* obtain list of calibrated images in pixmap from worker thread */
    void displayCalibImagesSingleView(); /* to display calibrated images in single view mode */
    void displayCalibratedImagesMultiView(QList<QPixmap> PixList); /* to display calibrated images in multi view mode */
    void onCalibPicSingleViewButtonClicked(); /* invoked when user clicks the 'Single View' button to view calibrated images in single view mode */
    void onCalibPicMultiViewButtonClicked(); /* invoked when user clicks the 'Single View' button to view calibrated images in multi view mode */
    void onPrevCalibPicButtonClicked(); /* invoked when user clicks button to navigate to previous calibrated image in single view */
    void onNextCalibPicButtonClicked(); /* invoked when user clicks button to navigate to next calibrated image in single view */
    void addLogMsg(QString msg); /* to add debug log messages */
    void obtainCameraParams(std::vector<double> intrinsic, std::vector<double> distortion, double _coverage, double _rmsError); /* to obtain generated camera parameters from worker thread*/
    void onOrigPicSelectionChanged(); /* invoked when user clicks an orig image in multiview */
    void onClickCalibPicInMultiView(); /* invoked when user clicks a calib image in multiview */
    void onSelect3PicPerRow(); /* invoked when user selects 3 images to be displayed in a row */
    void onSelect4PicPerRow(); /* invoked when user selects 4 images to be displayed in a row */
    void onSelect5PicPerRow(); /* invoked when user selects 5 images to be displayed in a row */
    void deleteCurrOrigImg(bool isDisplayInMultiView); /* to delete a particular original image when invoked by custom graphics item class when user enters the delete key */
    void onDisplaySelectedOrigPicInSingleView(); /* invoked when user clicks on the context menu option to view an orig image in single view mode */
    void onDeleteSelectedOrigPic(); /* invoked when user clicks on the context menu option to delete an orig image */

private:
    RCamera::CalibratorConfiguration mCalibratorConfiguration; /* defines the configuration of the CameraCalibrator */
    RCamera::MonoCameraCalibrator mMonoCameraCalibrator; /* contains the camera calibration algorithm */
    std::string saveFilePath = ""; /* to store the filepath of the saved JSON file */
    int currOrigImageCount = 1; /* to store the current original image being displayed in single view mode */
    int currCalibImageCount = 1; /* to store the current calibrated image being displayed in single view mode */
    QStringList matChessPics; /* to store filepaths of original images */
    QThread* thread; /* thread to do time-consuming tasks such as camera calibration */
    QList<QPixmap> origImages; /* to store the original images in pixmap */
    QList<QPixmap> calibratedImages; /* to store the calibrated images in pixmap */
    QList<QString> coverageParams; /* to store the coverage params */
    QList<QString> rmsValList; /* to store the RMS Error values */
    QString imagesDirName; /* to store the directory from which original images were uploaded from user */
    QGraphicsScene* origPicScene = new QGraphicsScene(this); /* scene for graphics view that will display uploaded images */
    QGraphicsScene* calibPicScene = new QGraphicsScene(this); /* scene for graphics view that will display calibrated images */

signals:
    void obtainImagesPixListThread(QStringList); /* to call worker thread to obtain list of uploaded images in pixmap to be displayed */
    void monoCalibrationTestThread(QStringList, RCamera::CalibratorConfiguration); /* to call the worker thread to start the camera calibration algorithm*/
};


#endif // _MAINWINDOW