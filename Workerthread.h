#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <vector>
#include <QPixmap>
#include "Camera/CalibratorConfiguration.h"
#include "Camera/MonoCameraCalibrator.h"
#include <opencv2/core/mat.hpp>

class Workerthread : public QObject
{
    Q_OBJECT

public:
    explicit Workerthread(QObject* parent = 0);

public slots:
    void obtainImagesPixList(QStringList matFiles); /* converts original uploaded images uploaded pixmap & places all of them in a list */
    void monoCalibrationTest(QStringList matFiles, RCamera::CalibratorConfiguration _config); /* does the camera calibration algorithm and generates respective results */

signals:
    void sendImagesPixList(QList<QPixmap> PixList); /* sends list of original images in pixmap back to main thread to be displayed in the ui */
    void sendCalibratedImages(QList<QPixmap> PixList, QList<QString> CoverageParams, QList<QString> RMSErrorList); /* sends list of calibrated images in pixmap to main thread to be displayed in the ui */
    void sendLogMsg(QString msg); /* sends log messages to be displayed in the debug log */
    void startExtractCamParams(std::vector<double> intrinsic, std::vector<double> distortion, double _coverage, double _rmsError); /* sends generated camera parameters if any to be displayed in the ui */

private:
     
};

#endif // WORKERTHREAD_H
