
#include "MonoCameraCalibrator.h"

#include "fmt/format.h"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>
#include <chrono>


namespace RCamera {
;

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
MonoCameraCalibrator::MonoCameraCalibrator()
	: MonoCameraCalibrator(CalibratorConfiguration())
{
}
MonoCameraCalibrator::MonoCameraCalibrator(const CalibratorConfiguration& configuration)
	: AbstractCameraCalibrator(configuration), 
	  mHelper(configuration)
{
}
MonoCameraCalibrator::~MonoCameraCalibrator()
{
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
CameraCalibrationStatus MonoCameraCalibrator::setImage(const unsigned char* imageData, int width, int height, int bytesPerPixel, int numRowbytes)
{
	// Size of the input image must be same as previously added images.
	if(!mHelper.checkImageSize(width, height))
	{
		return CameraCalibrationStatus::ImageSizeInvalid;
	}

	// Don't accept images after the maximum number number of images has been accepted.
	if(mNumImagesAccepted > mHelper.mConfiguration.maxNumImages())
	{
		return CameraCalibrationStatus::CalibrationFailed;
	}


	if(imageData)
	{
		cv::Mat _image;
		if(bytesPerPixel == 1)
		{
			_image = cv::Mat(mHelper.mInputImageSize, CV_8UC1, (void*)imageData, numRowbytes);
		}
		else if(bytesPerPixel == 2)
		{
			_image = cv::Mat(mHelper.mInputImageSize, CV_16UC1, (void*)imageData, numRowbytes);
			_image.convertTo(_image, CV_8UC1, 1.0/256.0);
		}
		else
		{
			return CameraCalibrationStatus::InvalidBytesPerPixel;
		}
		
		mNumImagesAdded++;

		// If this is the first image, create coverage mask.
		mHelper.createCoverageMask(width, height);
		
		if(mConfiguration.flipVertically())
		{
			cv::flip(_image, _image, 0);
		}

		std::vector<cv::Point2f> _corners = mHelper.findChessboardCorners(_image);
		if(!_corners.empty())
		{
			mHelper.mAcceptedImages.push_back(_image.clone());
			mNumImagesAccepted++;

			if(mConfiguration.drawAcceptedImage())
			{
				std::string _fileName = fmt::format("{}{:04d}.png", mConfiguration.acceptedImageFilePrefix(), mNumImagesAccepted);
				cv::imwrite(_fileName, _image);
			}

			// The following order of functions must not be changed.
			mHelper.updateCorners(_image, _corners);
			mHelper.updateDisplayImage(_image);
			mHelper.drawChessboardCorners(_corners);

			if(mConfiguration.drawChessboardCorners() && !mConfiguration.saveOnlyLastChessboardImage())
			{
				mHelper.saveChessboardCorners(mNumImagesAccepted, "");
			}

			bool b1 = mNumImagesAccepted          >= mConfiguration.minNumImages();
			bool b2 = mHelper.mCoveragePercentage > mConfiguration.minCoverage();
			int  b3 = mNumImagesAccepted          % mConfiguration.imageBatchSize();
			if(b1 && b2 && b3==0)
			{
				if(_calibrateCamera() && mHelper.mLastRmsError < mConfiguration.maxRmsError())
				{
					if(mConfiguration.drawChessboardCorners() && mConfiguration.saveOnlyLastChessboardImage())
					{
						mHelper.saveChessboardCorners(mNumImagesAccepted, "");
					}

					return CameraCalibrationStatus::Calibrated;
				}
				else
				{
					return CameraCalibrationStatus::ImageAccepted;
				}
			}
			else
			{
				return CameraCalibrationStatus::ImageAccepted;
			}
		}
		else
		{
			mHelper.updateDisplayImage(_image);
			return CameraCalibrationStatus::ImageRejected;
		}
	}
	else
	{
		// Passing in a null image forces camera calibration without RMS or coverage check.
		return _calibrateCamera() ? CameraCalibrationStatus::Calibrated : CameraCalibrationStatus::CalibrationFailed;
	}
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
void MonoCameraCalibrator::saveParametersToJSON(nlohmann::json* json) const
{
	// Create empty object.
	(*json)["MonoCameraParameters"] =
	{
	};
	
	mHelper.saveParameters(&(*json)["MonoCameraParameters"], "");
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
void MonoCameraCalibrator::setConfiguration(const CalibratorConfiguration& configuration)
{
	mHelper = CameraCalibratorHelper(configuration);
	AbstractCameraCalibrator::setConfiguration(configuration);
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
void MonoCameraCalibrator::getParameters(std::vector<double>& intrinsic, std::vector<double>& distortion)
{
	for (int i = 0; i < mHelper.mIntrinsicMatrix.rows; i++)
	{
		for (int j = 0; j < mHelper.mIntrinsicMatrix.cols; j++)
		{
			intrinsic.emplace_back(mHelper.mIntrinsicMatrix.at<double>(i, j));
		}
	}

	for (int m = 0; m < mHelper.mDistortionCoeffs.rows; m++)
	{
		for (int n = 0; n < mHelper.mDistortionCoeffs.cols; n++)
		{
			distortion.emplace_back(mHelper.mDistortionCoeffs.at<double>(m, n));
		}
	}
}

void MonoCameraCalibrator::getDebugParameters(double& coveragePercentage, double& lastRmsError)
{
	coveragePercentage = mHelper.mCoveragePercentage;
	lastRmsError = mHelper.mLastRmsError;
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
bool MonoCameraCalibrator::_calibrateCamera()
{
	mHelper.initializeCameraParameters();
	int _calibrationFlag = mConfiguration.calibrationFlag();
	
	std::vector<cv::Point3f>              _chessBoard3DCornerPositions = mHelper.calculateChessboard3DCornerPositions();
	std::vector<std::vector<cv::Point3f>> _allChessBoard3DCornerPositions(mNumImagesAccepted, _chessBoard3DCornerPositions);

	// Find intrinsic and extrinsic camera parameters	
	std::vector<cv::Mat> _rotationVectors;
	std::vector<cv::Mat> _translationVectors;
	auto _startTime = std::chrono::high_resolution_clock::now();
	mHelper.mLastRmsError = cv::calibrateCamera(_allChessBoard3DCornerPositions, mHelper.mAllChessBoardCorners, mHelper.mInputImageSize,
		                                        mHelper.mIntrinsicMatrix, mHelper.mDistortionCoeffs, _rotationVectors, _translationVectors, _calibrationFlag);
	auto _endTime = std::chrono::high_resolution_clock::now();
	double _timeMS = std::chrono::duration_cast<std::chrono::milliseconds>(_endTime - _startTime).count();

	std::cout << fmt::format("Time taken for calibration: {}\n", _timeMS);
	std::cout << fmt::format("Re-projection Error={:.5f}, Coverage={:.3f}\n", mHelper.mLastRmsError, mHelper.mCoveragePercentage);
	std::cout << fmt::format("Calibration Flag is ={:1d}\n",_calibrationFlag);
	std::cout << "Distortion Coeff   are "<< mHelper.mDistortionCoeffs << "\n";
	std::cout << "Camera Matrix are "<< mHelper.mIntrinsicMatrix << "\n";

	return cv::checkRange(mHelper.mIntrinsicMatrix) && cv::checkRange(mHelper.mDistortionCoeffs);
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //

}; // end namespace RCamera.
