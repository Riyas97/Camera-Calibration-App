
#ifndef _RVISION_CAMERA_CAMERACALIBRATORHELPER_H_
#define _RVISION_CAMERA_CAMERACALIBRATORHELPER_H_

#include "CalibratorConfiguration.h"

#include "nlohmann/json.hpp"
#include "opencv2/core.hpp"

#if defined(RVISIONLIB_HAVE_QT)
	#include <QtGui/QImage>
#endif
#include <string>


namespace RCamera {
;

struct CameraCalibratorHelper
{
public:

	CameraCalibratorHelper();
	explicit CameraCalibratorHelper(const CalibratorConfiguration& parameters);
	
	bool                     checkImageSize(int width, int height);
	void                     createCoverageMask(int width, int height);
	std::vector<cv::Point2f> findChessboardCorners(const cv::Mat& inputImage) const;
	void                     updateCorners(const cv::Mat& inputImage, std::vector<cv::Point2f> _corners);
	void                     updateDisplayImage(const cv::Mat& inputImage);
	void                     drawChessboardCorners(const std::vector<cv::Point2f>& corners);
	void                     saveChessboardCorners(int imageIndex, const std::string& cameraStr = "");

	std::vector<cv::Point3f> calculateChessboard3DCornerPositions() const;
	double                   calculateReprojectionErrors(const std::vector<std::vector<cv::Point3f>>& allChessBoard3DCornerPositions,
		                                                 const std::vector<cv::Mat>&                  rotationVectors,
		                                                 const std::vector<cv::Mat>&                  translationVectors) const;
	void                     saveParameters(nlohmann::json* json, const std::string& prefix) const;
	void                     initializeCameraParameters();


	#if defined(RVISIONLIB_HAVE_QT)
		QImage displayImage() const;
	#endif
	

private:
	
	void _updateCoverageMask(const std::vector<cv::Point2f>& corners);
	

public:

	CalibratorConfiguration               mConfiguration;        // The configuration used to calibrate camera.
	cv::Size                              mInputImageSize;       // The size of the input image.
	cv::Mat                               mIntrinsicMatrix;         // The camera calibration matrix.
	cv::Mat                               mDistortionCoeffs;     // The camera distortion coefficients.
	cv::Mat                               mCoverageMask;         // Binary mask used to show the current coverage of chessboard pattern.
	cv::Mat                               mDisplayImage;         // The last color image showing coverage which can be displayed on UI.
	double                                mCoveragePercentage;   // The percentage of image covered by chess board pattern so far.
	std::vector<std::vector<cv::Point2f>> mAllChessBoardCorners; // Collection of chessboard corners from all images.
	double                                mLastRmsError;         // The RMS error after last time camera was calibrated.
	bool                                  mCameraParamersValid;  // True if mCameraMatrix and mDistortionCoeffs doesn't contain NAN or INF.
	std::vector<cv::Mat>                  mAcceptedImages;
};

}; // end namespace RCamera

#endif // _RVISION_CAMERA_CAMERACALIBRATORHELPER_H_
