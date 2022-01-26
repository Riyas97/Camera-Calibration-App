#include "Workerthread.h"
#include <iostream>
#include <QMutexLocker>
#include <ui_MainWindow.h>
#include <vector>
#include <QDebug>
#include <QPixmap>
#include "Camera/CalibratorConfiguration.h"
#include "Camera/MonoCameraCalibrator.h"
#include <opencv2/imgcodecs.hpp>
#include "fmt/format.h"

Workerthread::Workerthread(QObject* parent) : QObject(parent)
{
}

void Workerthread::obtainImagesPixList(QStringList matChessPics) 
{
    // list to store pixmap images
    QList<QPixmap> PixList;

    // appending images to Pixmap list
    emit(sendLogMsg("INFO Calling thread to convert images to QPixmap"));
    for (auto& _fileName : matChessPics)
    {
        QPixmap pix(_fileName);
        PixList.append(pix);
    }

    // send Pixmap list back to main thread
    emit sendImagesPixList(PixList);
}

void Workerthread::monoCalibrationTest(QStringList matChessPics, RCamera::CalibratorConfiguration _config)
{
    qDebug() << "Starting MonoCalibration";
    RCamera::MonoCameraCalibrator _calibrator(_config);

    int imageIndex = 0; /* to store the image index number */
    QList<QPixmap> PixList; /* to store calibrated images */
    std::vector<double> intrinsic; /* to store intrinsic parameters */
    std::vector<double> distortion; /* to store distortion parameters */
    QList<QString> CoverageParams; /* to store respective calibrated images coverage value */
    QList<QString> RMSErrorList; /* to store respective calibrated images RMS Error value */

    for (const auto& it : matChessPics)
    {
        imageIndex += 1;
        emit(sendLogMsg("INFO Found file: " + it));
        qDebug() << "Found file: " << it;
        
        // loads image from specified file
        cv::Mat _image = cv::imread(it.toStdString(), cv::IMREAD_GRAYSCALE);
        
        if (!_image.empty())
        {
            int _width = _image.cols;
            int _height = _image.rows;
            unsigned char* _imageData = _image.data;
            int _rowLength = int(_image.step[0]);
            double _coverage = 0;
            double _rmsError = 0;

            emit(sendLogMsg("INFO File: " + it + "- Processing Image (Accept) " + QString::number(_calibrator.numAcceptedImages())));
            qDebug() << "Processing image (Accept) " << _calibrator.numAcceptedImages();
            
            // get calibration status
            RCamera::CameraCalibrationStatus _status = _calibrator.setImage(_imageData, _width, _height, 1, _rowLength);

            // do respective actions wrt status
            switch (_status)
            {
            case RCamera::CameraCalibrationStatus::ImageSizeInvalid:
            {
                emit(sendLogMsg("INFO File: " + it + "- Image size is not valid"));
                qDebug() << "Image size is not valid";
                break;
            }
            case RCamera::CameraCalibrationStatus::ImageAccepted:
            {
                // obtain image and append to list 
                QImage image = _calibrator.displayImage();
                QPixmap pix = QPixmap::fromImage(image);
                PixList.append(QPixmap::fromImage(image));
                
                // obtain coverage and rms error values 
                _calibrator.getDebugParameters(_coverage, _rmsError);
                CoverageParams.append(QString::number(_coverage));
                RMSErrorList.append(QString::number(_rmsError));
                emit(sendLogMsg("INFO File: " + it + "- Image accepted. Coverage is  " + QString::number(_coverage)));
                qDebug() << "Image accepted. Coverage is: " << _coverage;
                break;
            }
            case RCamera::CameraCalibrationStatus::ImageRejected:
            {
                emit(sendLogMsg("INFO File: " + it + "- Image rejected"));
                qDebug() << "Image rejected";
                break;
            }
            case RCamera::CameraCalibrationStatus::Calibrated:
            {
                // save camera parameters
                emit(sendLogMsg("INFO File: " + it + "- Camera calibrated. Saving parameters to file."));
                qDebug() << "Image calibrated";
                _calibrator.saveParametersToFile(std::string("CameraParameters.json"));
                
                // obtain various camera parameters and send them to main thread
                intrinsic.clear();
                distortion.clear();
                _coverage = 0;
                _rmsError = 0;
                _calibrator.getParameters(intrinsic, distortion);
                _calibrator.getDebugParameters(_coverage, _rmsError);
                emit (startExtractCamParams(intrinsic, distortion, _coverage, _rmsError));
                break;
            }

            case RCamera::CameraCalibrationStatus::CalibrationFailed:
            {
                // save failed camera parameters
                emit(sendLogMsg("INFO File: " + it + "- Camera calibration failed."));
                qDebug() << "Camera calibration failed";
                _calibrator.saveParametersToFile(std::string("CameraParametersFailed.json"));     
                break;
            }
            default:
            {
                emit(sendLogMsg("INFO File: " + it + "- Invalid calibration."));
                qDebug() << "Invalid calibration status";
            }
            };
        }
    }

    // send list of calibrated images back to main thread
    emit(sendLogMsg("INFO End of MonoCalibrationTest. Redirecting to main thread."));
    qDebug() << "End of MonoCalibrationTest";
    emit sendCalibratedImages(PixList, CoverageParams, RMSErrorList);
}
