#include "MainWindow.h"
#include "WorkerThread.h"
#include "Camera/CalibratorConfiguration.h"
#include "Camera/MonoCameraCalibrator.h"
#include "Camera/CameraCalibratorHelper.h"
#include <QtWidgets/QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include "fmt/format.h"
#include <QDir>
#include <QListWidgetItem>
#include <QScrollArea>
#include <vector>
#include <QTimer>
#include <QMovie>
#include <QThread>
#include <QStringList>
#include <QWindow>
#include <QStringListModel>
#include <QGraphicsScene>
#include <QGraphicsItemGroup>
#include <qscrollbar.h>
#include <QtWidgets>
#include <QGraphicsView>
#include <QMatrix>
#include <QGraphicsItem>
#include "GraphicsViewZoom.h"
#include <QGraphicsItem>
#include <opencv2/calib3d.hpp>
#include "CustomGraphicsItemClass.h"

using namespace cv;
using namespace std;

// constants
QString MS_SHELL_FONT = "MS Shell dlg 2";
QString ARIAL_FONT = "Arial";
int PREFERRED_FONT_SIZE = 11;
QString ORIG_PIC_INIT_MSG = "There are currently no images to be displayed. Add images by selecting a folder in the control panel on the left.";
QString CALIB_PIC_INIT_MSG = "There are currently no calibrated images to be displayed.";
QString NO_IMAGES_FOR_CALIB_ERROR_MSG = "Cannot start image calibration because no images were uploaded. Please upload the images by clicking the 'Browse' button on the control panel on the left.";
QString LOADING_ICON_PATH = ":/gif/loader.gif";
QString TICK_IMG_PATH = ":/gif/tick.png";
Qt::KeyboardModifiers KEY_MODIFIER_FOR_ZOOM = Qt::ControlModifier;
int PREFERRED_DP_FOR_CAM_PARAMS = 6;
int PREFERRED_MARGIN_BTW_IMGS = 5;
QString ERROR_TYPE = "ERROR";
QString INFO_TYPE = "Information Alert";

// common variables
QDateTime logTime; // to print date and time for log messages
GraphicsViewZoom* origPicGraphicViewZoom;
GraphicsViewZoom* calibPicGraphicViewZoom;
QIcon tickIcon = QIcon();
int noOfPicsPerRow = 3;
int selectedImage = 0;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    // load the UI and initalize the config params
    setupUi(this);
    setUISettings();
    
    // create a new thread to handle time consuming functions later
    thread = new QThread();
    Workerthread* worker = new Workerthread();
    
    // define QMetaType 
    qRegisterMetaType< QList<QString> >("QList<QString>");
    qRegisterMetaType< QList<QPixmap> >("QList<QPixmap>");
    qRegisterMetaType< RCamera::CalibratorConfiguration >("RCamera::CalibratorConfiguration");
    qRegisterMetaType< std::vector<double> >("std::vector<double>");

    // signals and slots for menubar
    connect(mFileOpen, &QAction::triggered, this, &MainWindow::onFileOpen);
    connect(mSave, &QAction::triggered, this, &MainWindow::onSave);
    connect(mSaveAs, &QAction::triggered, this, &MainWindow::onSaveAs);
    connect(mSelect3PicPerRow, &QAction::triggered, this, &MainWindow::onSelect3PicPerRow);
    connect(mSelect4PicPerRow, &QAction::triggered, this, &MainWindow::onSelect4PicPerRow);
    connect(mSelect5PicPerRow, &QAction::triggered, this, &MainWindow::onSelect5PicPerRow);
    
    // signals and slots for buttons in control panel
    connect(mStartCalibButton, &QPushButton::clicked, this, &MainWindow::onStartCalibButtonClicked);
    connect(mClearButton, &QPushButton::clicked, this, &MainWindow::loadConfiguration);
    connect(mBrowseCalibImgButton, &QPushButton::clicked, this, &MainWindow::onBrowseCalibImgButtonClicked);
   
    // signals and slots for buttons in original images display tab
    connect(mPrevOrigPicButton, &QPushButton::clicked, this, &MainWindow::onPrevOrigPicButtonClicked);
    connect(mNextOrigPicButton, &QPushButton::clicked, this, &MainWindow::onNextOrigPicButtonClicked);
    connect(mOrigPicSingleViewButton, &QPushButton::clicked, this, &MainWindow::onOrigPicSingleViewButtonClicked);
    connect(mOrigPicMultiViewButton, &QPushButton::clicked, this, &MainWindow::onOrigPicMultiViewButtonClicked);

    // signals and slots for buttons in calibrated images display tab
    connect(mPrevCalibPicButton, &QPushButton::clicked, this, &MainWindow::onPrevCalibPicButtonClicked);
    connect(mNextCalibPicButton, &QPushButton::clicked, this, &MainWindow::onNextCalibPicButtonClicked);
    connect(mCalibSingleViewButton, &QPushButton::clicked, this, &MainWindow::onCalibPicSingleViewButtonClicked);
    connect(mCalibMultiViewButton, &QPushButton::clicked, this, &MainWindow::onCalibPicMultiViewButtonClicked);

    // signals and slots between main thread and worker thread
    connect(this, SIGNAL(obtainImagesPixListThread(QStringList)), worker, SLOT(obtainImagesPixList(QStringList)));
    connect(worker, SIGNAL(sendImagesPixList(QList<QPixmap>)), this, SLOT(obtainOrigImages(QList<QPixmap>)));   
    connect(this, SIGNAL(monoCalibrationTestThread(QStringList, RCamera::CalibratorConfiguration)), worker, SLOT(monoCalibrationTest(QStringList, RCamera::CalibratorConfiguration)));
    connect(worker, SIGNAL(sendCalibratedImages(QList<QPixmap>, QList<QString>, QList<QString>)), this, SLOT(obtainCalibratedImages(QList<QPixmap>, QList<QString>, QList<QString>)));
    connect(worker, SIGNAL(sendLogMsg(QString)), this, SLOT(addLogMsg(QString)));
    connect(worker, SIGNAL(startExtractCamParams(std::vector<double>, std::vector<double>, double, double)), this, SLOT(obtainCameraParams(std::vector<double>, std::vector<double>, double, double)));

    // to mark worker thread for deletion once the thread is stopped
    connect(thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    // move worker to thread and start the thread
    worker->moveToThread(thread);
    thread->start();
}



void MainWindow::setUISettings()
{
    // set log tab to be read-only
    mLogTE->setReadOnly(true);

    // initialize graphics view display
    initializeGraphicsView(mOrigPicGraphicsView, ORIG_PIC_INIT_MSG, true);
    initializeGraphicsView(mCalibPicGraphicsView, CALIB_PIC_INIT_MSG, false);

    // setup graphics view zoom
    origPicGraphicViewZoom = new GraphicsViewZoom(mOrigPicGraphicsView);
    calibPicGraphicViewZoom = new GraphicsViewZoom(mCalibPicGraphicsView);

    // add keyboard modifier
    origPicGraphicViewZoom->setModifiers(KEY_MODIFIER_FOR_ZOOM);
    calibPicGraphicViewZoom->setModifiers(KEY_MODIFIER_FOR_ZOOM);

    // set tick img for menubar options
    tickIcon = QIcon(TICK_IMG_PATH);

    // clear all camera parameters
    clearCameraParamsLabels();

    // maximise window
    QMainWindow::showMaximized();

    // load the default configurations
    loadConfiguration();
}

void MainWindow::initializeGraphicsView(QGraphicsView* graphicsView, QString msg, bool isOrigPics)
{
    // Display messages in respective graphics view
    QLabel* label = new QLabel;
    label->setText(msg);
    QFont newFont(MS_SHELL_FONT, PREFERRED_FONT_SIZE);
    QFontMetrics nfm(newFont);
    label->setFont(newFont);
    QGraphicsScene* scene = new QGraphicsScene(this);
    scene->addWidget(label);
    graphicsView->setScene(scene);

    // disable drag mode
    graphicsView->setDragMode(QGraphicsView::DragMode::NoDrag);

    if (isOrigPics) {
        // disable buttons in single image display tab
        mPrevOrigPicButton->setVisible(false);
        mOrigPicsCountLabel->setVisible(false);
        mNextOrigPicButton->setVisible(false);
        mOrigPicSingleViewButton->setVisible(false);
        mOrigPicMultiViewButton->setVisible(false);
    }
    else {
        // disable buttons in calibrated image display tab
        mPrevCalibPicButton->setVisible(false);
        mCalibImagesCountLabel->setVisible(false);
        mNextCalibPicButton->setVisible(false);
        mCalibSingleViewButton->setVisible(false);
        mCalibMultiViewButton->setVisible(false);
    }
    
}

void MainWindow::loadConfiguration()
{
    // load camera calibrator configurations from CalibratorConfigurations class
    addLogMsg("INFO Loading camera calibrator configuations");
    mFlipVerticalCB->setChecked(mCalibratorConfiguration.flipVertically());
    mBoardWidthSB->setValue(mCalibratorConfiguration.boardWidth());
    mBoardHeightSB->setValue(mCalibratorConfiguration.boardHeight());
    mSquareSizeDSB->setValue(mCalibratorConfiguration.squareSize());
    mDrawChessboardCornersCB->setChecked(mCalibratorConfiguration.drawChessboardCorners());
    mSaveOnlyLastChessboardImageCB->setChecked(mCalibratorConfiguration.saveOnlyLastChessboardImage());
    mChessboardCornersImageFilePrefixLE->setText(QString::fromStdString(mCalibratorConfiguration.chessboardCornersImageFilePrefix()));
    mDrawAcceptedImageCB->setChecked(mCalibratorConfiguration.drawAcceptedImage());
    mAcceptedImageFilePrefixLE->setText(QString::fromStdString(mCalibratorConfiguration.acceptedImageFilePrefix()));
    mMinNumImagesSB->setValue(mCalibratorConfiguration.minNumImages());
    mMaxNumImagesSB->setValue(mCalibratorConfiguration.maxNumImages());
    mMaxRmsErrorDSB->setValue(mCalibratorConfiguration.maxRmsError());
    mMinCoverageDSB->setValue(mCalibratorConfiguration.minCoverage());
    mImageBatchSizeSB->setValue(mCalibratorConfiguration.imageBatchSize());
    mCalibFixPrincipalPointCB->setChecked(mCalibratorConfiguration.calibFixPrincipalPoint());
    mCalibZeroTangentDistCB->setChecked(mCalibratorConfiguration.calibZeroTangentDist());
    mCalibFixAspectRatioCB->setChecked(mCalibratorConfiguration.calibFixAspectRatio());
    mCalibAspectRatioDSB->setValue(mCalibratorConfiguration.calibAspectRatio());
    mCalibFixK1CB->setChecked(mCalibratorConfiguration.calibFixK1());
    mCalibFixK2CB->setChecked(mCalibratorConfiguration.calibFixK2());
    mCalibFixK3CB->setChecked(mCalibratorConfiguration.calibFixK3());
    mCalibFixK4CB->setChecked(mCalibratorConfiguration.calibFixK4());
    mCalibFixK5CB->setChecked(mCalibratorConfiguration.calibFixK5());
}

QString MainWindow::getLogTime()
{
    // return log time for debug log messages
    return logTime.currentDateTime().toString("dd-MM-yyyy HH:mm:ss");
}

void MainWindow::addLogMsg(QString msg)
{
    // set preferred font
    QFont newFont(ARIAL_FONT, PREFERRED_FONT_SIZE);
    QFontMetrics nfm(newFont);
    mLogTE->setFont(newFont);

    // display debug log messages
    mLogTE->append(getLogTime() + "  " + msg);
}

void MainWindow::displayMessageBox(QString type, QString msg)
{
    // display message boxes to show errors
    QMessageBox messageBox;
    if (type == ERROR_TYPE) {
        messageBox.critical(0, "ERROR: Application cannot process the request", msg);
    }
    else if (type == INFO_TYPE) {
        messageBox.information(0, INFO_TYPE, msg);
    }
    messageBox.setFixedSize(600, 300);
}

void MainWindow::onBrowseCalibImgButtonClicked()
{
    addLogMsg("INFO Browse folder button clicked");
    imagesDirName = QFileDialog::getExistingDirectory(this,
        "Select folder containing images to calibrate the camera", QDir::currentPath());

    if (imagesDirName == "") {
        return;
    } else {
        // reset list of images
        origImages.clear();

        // change tab to display original images tab
        mDisplayTab->setCurrentIndex(0);

        // set the folder path in UI
        mPathToCalibImgLE->setText(imagesDirName);

        // set the directory
        QDir dir(imagesDirName);
        dir.setNameFilters(QStringList({ "*.png", "*.jpg" }));
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

        // extract files
        addLogMsg("INFO Extracting all png and jpg image files from " + dir.path());
        QFileInfoList fileList = dir.entryInfoList();
        matChessPics.clear();
        origImages.clear();

        for (int i = 0; i < fileList.count(); i++) {
            matChessPics.push_back(fileList[i].absoluteFilePath());
        }
        currOrigImageCount = 1;

        // set graphicsview to default size
        origPicGraphicViewZoom->setDefaultSize();

        // set loading icon while waiting to display images in multiview
        setLoadingIcon(mOrigPicGraphicsView);

        // call qthread function to extract images
        emit obtainImagesPixListThread(matChessPics);
    }
}

void MainWindow::saveUserConfigurations()
{
    // set camera calibrator configurations in CalibratorConfiguration class 
    mCalibratorConfiguration.setFlipVertically(mFlipVerticalCB->isChecked());
    mCalibratorConfiguration.setBoardWidth(mBoardWidthSB->value());
    mCalibratorConfiguration.setBoardHeight(mBoardHeightSB->value());
    mCalibratorConfiguration.setSquareSize(mSquareSizeDSB->value());
    mCalibratorConfiguration.setDrawChessboardCorners(mDrawChessboardCornersCB->isChecked());
    mCalibratorConfiguration.setSaveOnlyLastChessboardImage(mSaveOnlyLastChessboardImageCB->isChecked());
    mCalibratorConfiguration.setChessboardCornersImageFilePrefix(mChessboardCornersImageFilePrefixLE->text().toStdString());
    mCalibratorConfiguration.setDrawAcceptedImage(mDrawAcceptedImageCB->isChecked());
    mCalibratorConfiguration.setAcceptedImageFilePrefix(mAcceptedImageFilePrefixLE->text().toStdString());
    mCalibratorConfiguration.setMinNumImages(mMinNumImagesSB->value());
    mCalibratorConfiguration.setMaxNumImages(mMaxNumImagesSB->value());
    mCalibratorConfiguration.setMaxRmsError(mMaxRmsErrorDSB->value());
    mCalibratorConfiguration.setMinCoverage(mMinCoverageDSB->value());
    mCalibratorConfiguration.setImageBatchSize(mImageBatchSizeSB->value());
    mCalibratorConfiguration.setCalibFixPrincipalPoint(mCalibFixPrincipalPointCB->isChecked());
    mCalibratorConfiguration.setCalibZeroTangentDist(mCalibZeroTangentDistCB->isChecked());
    mCalibratorConfiguration.setCalibFixAspectRatio(mCalibFixAspectRatioCB->isChecked());
    mCalibratorConfiguration.setAspectRatio(mCalibAspectRatioDSB->value());
    mCalibratorConfiguration.setCalibFixK1(mCalibFixK1CB->isChecked());
    mCalibratorConfiguration.setCalibFixK2(mCalibFixK2CB->isChecked());
    mCalibratorConfiguration.setCalibFixK3(mCalibFixK3CB->isChecked());
    mCalibratorConfiguration.setCalibFixK4(mCalibFixK4CB->isChecked());
    mCalibratorConfiguration.setCalibFixK5(mCalibFixK5CB->isChecked());
    addLogMsg("INFO Successfully saved all camera calibrator configuations in CalibratorConfiguration class");
}

void MainWindow::onStartCalibButtonClicked() 
{
    addLogMsg("INFO Start Calibration button clicked");

    // if image directory not set, cannot do image calibration
    if (imagesDirName == "") {
        displayMessageBox(ERROR_TYPE, NO_IMAGES_FOR_CALIB_ERROR_MSG);
        addLogMsg("ERROR Error in calibrating images - " + NO_IMAGES_FOR_CALIB_ERROR_MSG);
        return;
    }

    // save user configurations first
    saveUserConfigurations();
    
    // initialize calibrated image list, count, coverage params list & rms error list
    calibratedImages.clear();
    coverageParams.clear();
    rmsValList.clear();
    currCalibImageCount = 1;
    
    // clear camera parameters labels
    clearCameraParamsLabels();

    // change tab to display log tab
    mDisplayTab->setCurrentIndex(1);
    mDisplayTab->setCurrentIndex(4);
    
    // set loading icon while loading the calibrated images
    setLoadingIcon(mCalibPicGraphicsView);

    // call qthread function to run the calibration algorithm
    addLogMsg("INFO Calling thread to start MonoCalibration algorithm");
    emit monoCalibrationTestThread(matChessPics, mCalibratorConfiguration);
}

void MainWindow::setLoadingIcon(QGraphicsView* graphicsArea) 
{
    // set loading icon in the respective graphics view
    QGraphicsScene* scene = new QGraphicsScene(this);
    graphicsArea->setScene(scene);
    QLabel* gif_anim = new QLabel();
    QMovie* loadingIcon = new QMovie(LOADING_ICON_PATH);
    gif_anim->setMovie(loadingIcon);
    loadingIcon->start();
    scene->addWidget(gif_anim);
}

void MainWindow::onFileOpen()
{
    addLogMsg("INFO File open button clicked");
    
    // open file dialog to allow users to upload images
    QString _filePath = QFileDialog::getOpenFileName(this, "Choose a calibration settings file", "Desktop/", "JSON Files (*.json)");
    
    // to capture any errors while extracting params from uploaded JSON file
    std::string error = "";
    
    // extract params from uploaded JSON file
    qDebug() << "Opening input configurations from " << _filePath << "filepath";
    addLogMsg("INFO Attemping to open input configuarations from " + _filePath);
    if (_filePath != "") {
        mCalibratorConfiguration.loadConfiguration(_filePath.toStdString(), &error);
        if (error != "") {
            qCritical() << "Critical Error Message: Unable to fetch parameters from JSON file";
            displayMessageBox(ERROR_TYPE, "The application was unable to extract parameters from the given JSON file. Please use a valid JSON file!");
            addLogMsg("ERROR Error in opening file - " + QString::fromStdString(error));
        }
        loadConfiguration();
    }
}

void MainWindow::onSave() 
{
    addLogMsg("INFO Save button clicked");
    
    // if user is clicking save for first time
    if (saveFilePath == "") {
        onSaveAs();
    }
    else {
        // save all the user inputs first
        saveUserConfigurations();
        
        // save params to last saved file
        mCalibratorConfiguration.saveConfiguration(saveFilePath);
        addLogMsg("INFO All input configurations have been saved in " + QString::fromStdString(saveFilePath));
    }   
}

void MainWindow::onSaveAs() 
{
    addLogMsg("INFO Save As button clicked");

    // save all the user inputs first
    saveUserConfigurations();
    
    // open file dialog to select directory to save file
    QString _filePath = QFileDialog::getSaveFileName(this, tr("Save File"), "", "JSON Files (*.json)");
    qDebug() << "Saving contents to " << _filePath << "filepath";
    if (_filePath != "") {
        saveFilePath = _filePath.toStdString();
        mCalibratorConfiguration.saveConfiguration(saveFilePath);
        addLogMsg("INFO All input configurations have been saved in " + QString::fromStdString(saveFilePath));
    }
}

void MainWindow::onPrevOrigPicButtonClicked() {
    addLogMsg("INFO Previous Orig Image View button clicked");

    // reset matrix to rescale back if user had zoomed
    origPicGraphicViewZoom->setDefaultSize();
    
    // check for edge case
    if (currOrigImageCount > 1) {
        currOrigImageCount -= 1;
        displayOrigImagesSingleView();
    }
}

void MainWindow::onNextOrigPicButtonClicked() {
    addLogMsg("INFO Next Orig Image View button clicked");

    // reset matrix to rescale back if user had zoomed
    origPicGraphicViewZoom->setDefaultSize();
    
    // check for edge case
    if (currOrigImageCount < origImages.size()) {
        currOrigImageCount += 1;
        displayOrigImagesSingleView();
    }
}

void MainWindow::onOrigPicSingleViewButtonClicked() {
    addLogMsg("INFO Orig Pic Single View button clicked");
    
    // display images in single view
    displayOrigImagesSingleView();
}

void MainWindow::displayOrigImagesSingleView() 
{
    // obtain image to be displayed
    QPixmap pix = origImages[currOrigImageCount - 1];
     
    // display respective image in the graphics view
    QGraphicsScene* scene = new QGraphicsScene(this);
    mOrigPicGraphicsView->setScene(scene);
    QPixmap scaledPix = pix.scaled(mOrigPicGraphicsView->width(), mOrigPicGraphicsView->height(), Qt::KeepAspectRatio);

    // create custom graphicsitem class to enable keypress event detection 
    CustomGraphicsItemClass* pixmapItem = new CustomGraphicsItemClass();
    pixmapItem->setPixmap(scaledPix);
    pixmapItem->setFlag(QGraphicsItem::ItemIsFocusable);
    pixmapItem->setFocus();
    connect(pixmapItem, SIGNAL(startDeleteCurrentImage(bool)), this, SLOT(deleteCurrOrigImg(bool)));
    scene->addItem(pixmapItem);

    // fit the scene to scale
    mOrigPicGraphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

    // enable buttons and other user options
    mPrevOrigPicButton->setVisible(true);
    mOrigPicsCountLabel->setVisible(true);
    mNextOrigPicButton->setVisible(true);
    mOrigPicSingleViewButton->setVisible(true);
    mOrigPicMultiViewButton->setVisible(true);
    mOrigPicsCountLabel->setText(QString::number(currOrigImageCount) + " / "
        + QString::number(origImages.size()));

    // disable prev or next button if at first or last image respectively
    if (currOrigImageCount == origImages.size() && currOrigImageCount == 1) {
        mPrevOrigPicButton->setDisabled(true);
        mNextOrigPicButton->setDisabled(true);
    }
    else if (currOrigImageCount == 1) {
        mPrevOrigPicButton->setDisabled(true);
        mNextOrigPicButton->setDisabled(false);
    }
    else if (currOrigImageCount == origImages.size()) {
        mPrevOrigPicButton->setDisabled(false);
        mNextOrigPicButton->setDisabled(true);
    }
    else {
        mPrevOrigPicButton->setDisabled(false);
        mNextOrigPicButton->setDisabled(false);
    }

    // enable drag mode
    mOrigPicGraphicsView->setDragMode(QGraphicsView::DragMode::ScrollHandDrag);

    // enable zooming
    origPicGraphicViewZoom->setZoomConfig(true);

    addLogMsg("INFO Displaying original images in single view - Current image no. displayed: " + QString::number(currOrigImageCount));
}

void MainWindow::deleteCurrOrigImg(bool isDisplayInMultiView)
{
    addLogMsg("INFO Detected the delete key");

    // display loading icon while deleting orig image
    setLoadingIcon(mOrigPicGraphicsView);

    // delete selected image
    QString msg_to_display = "Image " + QString::number(currOrigImageCount) + " (" + matChessPics[currOrigImageCount - 1] + ") was deleted";
    origImages.removeAt(currOrigImageCount - 1);
    QFile(matChessPics[currOrigImageCount - 1]).remove();
    matChessPics.removeAt(currOrigImageCount - 1);
    addLogMsg("INFO " + msg_to_display);

    // update display view 
    if (origImages.size() == 0) {
        // if no more images to display, show default view
        currCalibImageCount = 1;
        initializeGraphicsView(mOrigPicGraphicsView, ORIG_PIC_INIT_MSG, true);
    }
    else {
        // update current image index
        if (currOrigImageCount == origImages.size() + 1) {
            // previously last image was deleted
            // dislpay the now last image
            currOrigImageCount = origImages.size();
        }
        if (isDisplayInMultiView) {
            // display updated multiview
            displayOrigImagesMultiView(origImages);
        }
        else {
            // display updated single view
            displayOrigImagesSingleView();
        }
    }

    // notify users that image has been deleted
    displayMessageBox(INFO_TYPE, msg_to_display);
}

void MainWindow::onOrigPicMultiViewButtonClicked() 
{
    addLogMsg("INFO Orig Pic Multi View button clicked");
    
    // reset matrix to rescale back if user had zoomed
    origPicGraphicViewZoom->setDefaultSize();

    // set loading icon while loading images
    setLoadingIcon(mOrigPicGraphicsView);

    // display image in multiview
    displayOrigImagesMultiView(origImages);
}

void MainWindow::obtainOrigImages(QList<QPixmap> PixList)
{
    foreach(QPixmap pix, PixList) {
        origImages.append(pix);
    }

    // set graphics view to default size
    origPicGraphicViewZoom->setDefaultSize();

    // display image in multiview 
    displayOrigImagesMultiView(origImages);
}

void MainWindow::displayOrigImagesMultiView(QList<QPixmap> PixList) 
{
    // set default size
    origPicGraphicViewZoom->setDefaultSize();
    
    // initialize params
    origPicScene = new QGraphicsScene(this);
    mOrigPicGraphicsView->setRenderHints(QPainter::Antialiasing);
    mOrigPicGraphicsView->setScene(origPicScene);

    int row = 0;
    int col = 0;
    int index = 0;
    
    foreach(QPixmap pix, PixList) {
        index += 1;
        // put the pixmaps next to the other
        QPixmap scaledPix = pix.scaled(mOrigPicGraphicsView->width() / noOfPicsPerRow - PREFERRED_MARGIN_BTW_IMGS, mOrigPicGraphicsView->height(), Qt::KeepAspectRatio);
        QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(scaledPix);

        // make the images clickable
        pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
        // add the image number as index (so that image can be identified when user clickes on it)
        pixmapItem->setData(0, index);;
        pixmapItem->setCursor(Qt::PointingHandCursor);
        origPicScene->addItem(pixmapItem);
        
        // arrange the images by spacing them out approprately
        if (row != 0) {
            pixmapItem->moveBy(0, row * (scaledPix.height() + PREFERRED_MARGIN_BTW_IMGS));
        }

        if (col != 0) {
            pixmapItem->moveBy(col * (scaledPix.width() + PREFERRED_MARGIN_BTW_IMGS), 0);
        }
        col += 1;
        if (col == noOfPicsPerRow) {
            row += 1;
            col = 0;
        }
    }

    // add signal and slot so that images can be clicked
    connect(origPicScene, SIGNAL(selectionChanged()), this, SLOT(onOrigPicSelectionChanged()));

    // enable or disable buttons accordingly
    mPrevOrigPicButton->setVisible(false);
    mOrigPicsCountLabel->setVisible(false);
    mNextOrigPicButton->setVisible(false);
    mOrigPicSingleViewButton->setVisible(true);
    mOrigPicMultiViewButton->setVisible(true);
    
    // disable drag mode
    mOrigPicGraphicsView->setDragMode(QGraphicsView::DragMode::NoDrag);

    // disable zooming
    origPicGraphicViewZoom->setZoomConfig(false);
    
    addLogMsg("INFO Displaying original images in multiview");

}

void MainWindow::onOrigPicSelectionChanged() {
    
    // obtain selected items
    if (origPicScene->selectedItems().size() != 0) {
        QGraphicsItem* item = origPicScene->selectedItems().at(0);
        int index = 1;
        foreach(QGraphicsItem * it, origPicScene->items()) {
            // check which picture was selected
            if (it->data(0) == item->data(0)) {
                // display selected image
                int val = origPicScene->items().size() - index + 1;
                addLogMsg("INFO Image " + QString::number(val) + " has been clicked");    
                selectedImage = val;
                
                // create context menu
                QAction* viewAction = new QAction("Display in single view", this);
                QAction* deleteAction = new QAction("Delete image", this);
                QMenu* menu = new QMenu(this);
                menu->addAction(viewAction);
                menu->addAction(deleteAction);
               
                // add signals and slots for the context menu options
                connect(viewAction, SIGNAL(triggered()), this, SLOT(onDisplaySelectedOrigPicInSingleView()));
                connect(deleteAction, SIGNAL(triggered()), this, SLOT(onDeleteSelectedOrigPic()));
                
                // display context menu
                menu->exec(QCursor::pos());
                
                break;
            }
            index += 1;
        }
    }
}

void MainWindow::onDisplaySelectedOrigPicInSingleView() 
{
    currOrigImageCount = selectedImage;
    displayOrigImagesSingleView();
}

void MainWindow::onDeleteSelectedOrigPic()
{
    currOrigImageCount = selectedImage;
    deleteCurrOrigImg(true);
}


void MainWindow::onPrevCalibPicButtonClicked() {
    addLogMsg("INFO Prev Calib Image View button clicked");

    // reset matrix to rescale back if user had zoomed
    calibPicGraphicViewZoom->setDefaultSize();
    
    // check for edge case
    if (currCalibImageCount > 1) {
        currCalibImageCount--;
        displayCalibImagesSingleView();
    }
}

void MainWindow::onNextCalibPicButtonClicked() 
{
    addLogMsg("INFO Next Calib Image View button clicked");

    // reset matrix to rescale back if user had zoomed
    calibPicGraphicViewZoom->setDefaultSize();

    // check for edge case
    if (currCalibImageCount < calibratedImages.size()) {
        currCalibImageCount++;
        displayCalibImagesSingleView();
    }
}

void MainWindow::onCalibPicSingleViewButtonClicked() 
{
    addLogMsg("INFO Calib Pic Single View button clicked");
    
    // display images in single view
    displayCalibImagesSingleView();
}

void MainWindow::displayCalibImagesSingleView() 
{
    
    // obtain image to be displayed
    QPixmap pix = calibratedImages[currCalibImageCount - 1];

    // display respective image in the graphics view 
    QGraphicsScene* scene = new QGraphicsScene(this);
    mCalibPicGraphicsView->setScene(scene);
    QPixmap scaledPix = pix.scaled(mCalibPicGraphicsView->width(), mCalibPicGraphicsView->height(), Qt::KeepAspectRatio);
    QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(scaledPix);
    scene->addItem(pixmapItem);

    // fit the scene to scale
    mCalibPicGraphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

    // enable buttons and other user options
    mPrevCalibPicButton->setVisible(true);
    mCalibImagesCountLabel->setVisible(true);
    mNextCalibPicButton->setVisible(true);
    mCalibSingleViewButton->setVisible(true);
    mCalibMultiViewButton->setVisible(true);
    mCalibImagesCountLabel->setText(QString::number(currCalibImageCount) + " / "
        + QString::number(calibratedImages.size()));

    // disable prev or next button if at first or last image respectively
    if (currCalibImageCount == 1) {
        mPrevCalibPicButton->setDisabled(true);
        mNextCalibPicButton->setDisabled(false);
    }
    else if (currCalibImageCount == calibratedImages.size()) {
        mPrevCalibPicButton->setDisabled(false);
        mNextCalibPicButton->setDisabled(true);
    }
    else {
        mPrevCalibPicButton->setDisabled(false);
        mNextCalibPicButton->setDisabled(false);
    }

    // enable zooming
    calibPicGraphicViewZoom->setZoomConfig(true);   

    // enable drag mode
    mCalibPicGraphicsView->setDragMode(QGraphicsView::DragMode::ScrollHandDrag);

    addLogMsg("INFO Displaying calibrated images in single view - Current image no. displayed: " + QString::number(currCalibImageCount));
}

void MainWindow::onCalibPicMultiViewButtonClicked() 
{
    addLogMsg("INFO Calib Pic Multi View button clicked");

    // reset matrix to rescale back if user had zoomed
    calibPicGraphicViewZoom->setDefaultSize();

    // set loading icon while loading images
    setLoadingIcon(mCalibPicGraphicsView);

    displayCalibratedImagesMultiView(calibratedImages);
}

void MainWindow::obtainCalibratedImages(QList<QPixmap> PixList, QList<QString> CoverageList, QList<QString> RMSErrorList)
{
    foreach(QPixmap pix, PixList) {
        calibratedImages.append(pix);
    }

    foreach(QString coverage, CoverageList) {
        coverageParams.append(coverage);
    }

    foreach(QString rmsError, RMSErrorList) {
        rmsValList.append(rmsError);
    }

    // set graphics view to default size
    //calibPicGraphicViewZoom->setDefaultSize();
    
    // display image in multiview 
    displayCalibratedImagesMultiView(calibratedImages);
}

void MainWindow::displayCalibratedImagesMultiView(QList<QPixmap> PixList)
{ 
    // set graphics view to default size
    calibPicGraphicViewZoom->setDefaultSize();
    
    // initialize params
    calibPicScene = new QGraphicsScene(this);
    mCalibPicGraphicsView->setRenderHints(QPainter::Antialiasing);
    mCalibPicGraphicsView->setScene(calibPicScene);
    int row = 0;
    int col = 0;
    int index = 0;

    // set graphics view to default size
    calibPicGraphicViewZoom->setDefaultSize();

    foreach(QPixmap pix, PixList) {
        index += 1;

        // put the pixmaps next to the other
        QPixmap scaledPix = pix.scaled(mCalibPicGraphicsView->width() / noOfPicsPerRow - PREFERRED_MARGIN_BTW_IMGS, mCalibPicGraphicsView->height(), Qt::KeepAspectRatio);
        QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(scaledPix);
      
        // make the images clickable
        pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
        // add the image number as index (so that image can be identified when user clickes on it)
        pixmapItem->setData(0, index);;
        pixmapItem->setCursor(Qt::PointingHandCursor);
        pixmapItem->setToolTip("Coverage: " + coverageParams.at(index - 1));
        calibPicScene->addItem(pixmapItem);
        
        // arrange the images by spacing them out approprately
        if (row != 0) {
            pixmapItem->moveBy(0, row * (scaledPix.height() + PREFERRED_MARGIN_BTW_IMGS));
        }
        if (col != 0) {
            pixmapItem->moveBy(col * (scaledPix.width() + PREFERRED_MARGIN_BTW_IMGS), 0);
        }
        col += 1;
        if (col == noOfPicsPerRow) {
            row += 1;
            col = 0;
        }
    }

    // add signal and slot so that images can be clicked
    connect(calibPicScene, SIGNAL(selectionChanged()), this, SLOT(onClickCalibPicInMultiView()));

    // enable or disable buttons accordingly
    mPrevCalibPicButton->setVisible(false);
    mCalibImagesCountLabel->setVisible(false);
    mNextCalibPicButton->setVisible(false);
    mCalibSingleViewButton->setVisible(true);
    mCalibMultiViewButton->setVisible(true);

    // disable drag mode
    mCalibPicGraphicsView->setDragMode(QGraphicsView::DragMode::NoDrag);

    // disable zooming
    calibPicGraphicViewZoom->setZoomConfig(false);

    addLogMsg("INFO Displaying calibrated images in multiview");
}

void MainWindow::onClickCalibPicInMultiView()
{
    // obtain selected items
    if (calibPicScene->selectedItems().size() != 0) {
        QGraphicsItem* item = calibPicScene->selectedItems().at(0);
        int index = 1;
        foreach(QGraphicsItem * it, calibPicScene->items()) {
            // check which picture was selected
            if (it->data(0) == item->data(0)) {
                // display selected calib image in single view
                int val = calibPicScene->items().size() - index + 1;
                addLogMsg("INFO Image " + QString::number(val) + " has been clicked");
                currCalibImageCount = val;
                displayCalibImagesSingleView();
            }
            index += 1;
        }
    }
}


void MainWindow::obtainCameraParams(std::vector<double> intrinsic, std::vector<double> distortion, 
    double _coverage, double _rmsError) 
{   
    // set intrinsic parameters in ui 
    setCameraParamsLabels(labelCamRow1_1, intrinsic.at(0));
    setCameraParamsLabels(labelCamRow1_2, intrinsic.at(1));
    setCameraParamsLabels(labelCamRow1_3, intrinsic.at(2));
    setCameraParamsLabels(labelCamRow2_1, intrinsic.at(3));
    setCameraParamsLabels(labelCamRow2_2, intrinsic.at(4));
    setCameraParamsLabels(labelCamRow2_3, intrinsic.at(5));
    setCameraParamsLabels(labelCamRow3_1, intrinsic.at(6));
    setCameraParamsLabels(labelCamRow3_2, intrinsic.at(7));
    setCameraParamsLabels(labelCamRow3_3, intrinsic.at(8));

    // set distortion parameters in ui 
    setCameraParamsLabels(labelDistCoeff1, distortion.at(0));
    setCameraParamsLabels(labelDistCoeff2, distortion.at(1));
    setCameraParamsLabels(labelDistCoeff3, distortion.at(2));
    setCameraParamsLabels(labelDistCoeff4, distortion.at(3));
    setCameraParamsLabels(labelDistCoeff5, distortion.at(4));

    // set coverage and RMS error value in ui
    setCameraParamsLabels(mCoverageLabel, _coverage);
    setCameraParamsLabels(mRMSErrorLabel, _rmsError); 

    addLogMsg("INFO Camera Parameters have been set. You can view them under the 'Camera Parameters' tab.");
}

void MainWindow::clearCameraParamsLabels()
{
    // clear intrinsic parameters in ui 
    labelCamRow1_1->setText("-");
    labelCamRow1_2->setText("-");
    labelCamRow1_3->setText("-");
    labelCamRow2_1->setText("-");
    labelCamRow2_2->setText("-");
    labelCamRow2_3->setText("-");
    labelCamRow3_1->setText("-");
    labelCamRow3_2->setText("-");
    labelCamRow3_3->setText("-");
    
    // clear distortion parameters in ui 
    labelDistCoeff1->setText("-");
    labelDistCoeff2->setText("-");
    labelDistCoeff3->setText("-");
    labelDistCoeff4->setText("-");
    labelDistCoeff5->setText("-");

    // set coverage and RMS error value in ui
    mCoverageLabel->setText("-");
    mRMSErrorLabel->setText("-");

    addLogMsg("INFO Camera Parameters have been cleared.");
}

void MainWindow::onSelect3PicPerRow()
{
    addLogMsg("INFO Switching to display 3 images per row");

    // change params accordingly
    noOfPicsPerRow = 3;
    mSelect3PicPerRow->setIcon(tickIcon);
    mSelect4PicPerRow->setIcon(QIcon());
    mSelect5PicPerRow->setIcon(QIcon());
    
    // update UI
    changeNoOfPicsPerRowDisplayed();
}

void MainWindow::onSelect4PicPerRow()
{
    addLogMsg("INFO Switching to display 4 images per row");

    // change params accordingly
    noOfPicsPerRow = 4;
    mSelect3PicPerRow->setIcon(QIcon());
    mSelect4PicPerRow->setIcon(tickIcon);
    mSelect5PicPerRow->setIcon(QIcon());

    // update UI
    changeNoOfPicsPerRowDisplayed();
}

void MainWindow::onSelect5PicPerRow() 
{
    addLogMsg("INFO Switching to display 5 images per row");

    // change params accordingly
    noOfPicsPerRow = 5;
    mSelect3PicPerRow->setIcon(QIcon());
    mSelect4PicPerRow->setIcon(QIcon());
    mSelect5PicPerRow->setIcon(tickIcon);

    // update UI
    changeNoOfPicsPerRowDisplayed();
}

void MainWindow::changeNoOfPicsPerRowDisplayed()
{
    // if user had already calibrated images
    if (calibratedImages.size() != 0) {
        // update both the original and calibrated images tab
        setLoadingIcon(mOrigPicGraphicsView);
        setLoadingIcon(mCalibPicGraphicsView);
        displayCalibratedImagesMultiView(calibratedImages);
        displayOrigImagesMultiView(origImages);
       
    // if user had only uploaded images
    } else if (origImages.size() != 0) {
        // only update the original images tab
        setLoadingIcon(mOrigPicGraphicsView);
        displayOrigImagesMultiView(origImages);
    }
}

void MainWindow::setCameraParamsLabels(QLabel* label, double val)
{
    
    // set up to respective floating point
    QString str_val = QString::number(val, 'f', PREFERRED_DP_FOR_CAM_PARAMS);

    // Remove any number of trailing 0's
    str_val.remove(QRegExp("0+$")); 
    
    // If the last character is just a '.' then remove it
    str_val.remove(QRegExp("\\.$")); 

    // set the respective label
    label->setText(str_val);
}
