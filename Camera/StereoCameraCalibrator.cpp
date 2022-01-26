
#include "StereoCameraCalibrator.h"

#include "fmt/format.h"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>


namespace RCamera {
;

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
StereoCameraCalibrator::StereoCameraCalibrator()
	: StereoCameraCalibrator(CalibratorConfiguration())
{
}
StereoCameraCalibrator::StereoCameraCalibrator(const CalibratorConfiguration& configuration)
	: AbstractCameraCalibrator(configuration), 
	  mLeftHelper(configuration),
	  mRightHelper(configuration)
{
}
StereoCameraCalibrator::~StereoCameraCalibrator()
{
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
CameraCalibrationStatus StereoCameraCalibrator::setImage(const unsigned char* leftImage, 
	                                                     const unsigned char* rightImage, 
	                                                     int width, int height, int bytesPerPixel, int numRowbytes)
{
	// Size of the input image must be same as previously added images.
	if(!mLeftHelper.checkImageSize(width, height) && !mRightHelper.checkImageSize(width, height))
	{
		return CameraCalibrationStatus::ImageSizeInvalid;
	}

	// Don't accept images after the maximum number number of images has been accepted.
	if(mNumImagesAccepted > mLeftHelper.mConfiguration.maxNumImages())
	{
		return CameraCalibrationStatus::CalibrationFailed;
	}


	if(leftImage && rightImage)
	{
		cv::Mat _leftImage;
		cv::Mat _rightImage;
		if(bytesPerPixel == 1)
		{
			_leftImage  = cv::Mat(mLeftHelper.mInputImageSize, CV_8UC1, (void*)leftImage, numRowbytes);
			_rightImage = cv::Mat(mLeftHelper.mInputImageSize, CV_8UC1, (void*)rightImage, numRowbytes);

		}
		else if(bytesPerPixel == 2)
		{
			_leftImage  = cv::Mat(mLeftHelper.mInputImageSize, CV_16UC1, (void*)leftImage, numRowbytes);
			_rightImage = cv::Mat(mLeftHelper.mInputImageSize, CV_16UC1, (void*)rightImage, numRowbytes);

			_leftImage.convertTo (_leftImage , CV_8UC1, 1.0/256.0);
			_rightImage.convertTo(_rightImage, CV_8UC1, 1.0/256.0);
		}
		else
		{
			return CameraCalibrationStatus::InvalidBytesPerPixel;
		}

		mNumImagesAdded++;
		
		
		// If this is the first image, create coverage mask.
		mLeftHelper.createCoverageMask(width, height);
		mRightHelper.createCoverageMask(width, height);


		if(mConfiguration.flipVertically())
		{
			cv::flip(_leftImage , _leftImage , 0);
			cv::flip(_rightImage, _rightImage, 0);
		}


		std::vector<cv::Point2f> _leftCorners  = mLeftHelper.findChessboardCorners(_leftImage);
		std::vector<cv::Point2f> _rightCorners = mRightHelper.findChessboardCorners(_rightImage);
		if(!_leftCorners.empty() && !_rightCorners.empty())
		{
			mLeftHelper.mAcceptedImages.push_back(_leftImage.clone());
			mRightHelper.mAcceptedImages.push_back(_rightImage.clone());
			mNumImagesAccepted++;

			if(mConfiguration.drawAcceptedImage())
			{
				std::string _leftFileName = fmt::format("{}L_{:04d}.png", mConfiguration.acceptedImageFilePrefix(), mNumImagesAccepted);
				cv::imwrite(_leftFileName, _leftImage);

				std::string _rightFileName = fmt::format("{}R_{:04d}.png", mConfiguration.acceptedImageFilePrefix(), mNumImagesAccepted);
				cv::imwrite(_rightFileName, _rightImage);
			}

			mLeftHelper.updateCorners (_leftImage , _leftCorners);
			mLeftHelper.updateDisplayImage(_leftImage);
			mLeftHelper.drawChessboardCorners(_leftCorners);

			mRightHelper.updateCorners(_rightImage, _rightCorners);
			mRightHelper.updateDisplayImage(_rightImage);
			mRightHelper.drawChessboardCorners(_rightCorners);

			if(mConfiguration.drawChessboardCorners() && !mConfiguration.saveOnlyLastChessboardImage())
			{
				mLeftHelper.saveChessboardCorners(mNumImagesAccepted, "Left");
				mRightHelper.saveChessboardCorners(mNumImagesAccepted, "Right");
			}
			
			bool b1 = mNumImagesAccepted               >= mConfiguration.minNumImages();
			bool b2 = mLeftHelper.mCoveragePercentage  >  mConfiguration.minCoverage();
			bool b3 = mRightHelper.mCoveragePercentage >  mConfiguration.minCoverage();
			int  b4 = mNumImagesAccepted               %  mConfiguration.imageBatchSize();
			if(b1 && b2 && b3 && b4 ==0)
			{
				if(_calibrateCamera() && 
					mLeftHelper.mLastRmsError < mConfiguration.maxRmsError() &&
					mRightHelper.mLastRmsError < mConfiguration.maxRmsError())
				{
					if(mConfiguration.drawChessboardCorners() && mConfiguration.saveOnlyLastChessboardImage())
					{
						mLeftHelper.saveChessboardCorners(mNumImagesAccepted, "Left");
						mRightHelper.saveChessboardCorners(mNumImagesAccepted, "Right");
					}

					return CameraCalibrationStatus::Calibrated;
				}
				else
				{
					return CameraCalibrationStatus::CalibrationFailed;
				}
			}
			else
			{
				return CameraCalibrationStatus::ImageAccepted;
			}
		}
		else
		{
			mLeftHelper.updateDisplayImage(_leftImage);
			mRightHelper.updateDisplayImage(_rightImage);
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
void StereoCameraCalibrator::saveParametersToJSON(nlohmann::json* json) const
{
	(*json)["StereoCameraParameters"] =
	{
	};
	
	nlohmann::json* _stereoCameraParameters = &(*json)["StereoCameraParameters"];


	mLeftHelper.saveParameters(_stereoCameraParameters, "Left");
	mRightHelper.saveParameters(_stereoCameraParameters, "Right");
	
	
	std::vector<double> _rotation;
	for(int i = 0 ; i<mRotationMatrix.rows ; i++)
	{
		for(int j = 0 ; j<mRotationMatrix.cols ; j++)
		{
			_rotation.emplace_back(mRotationMatrix.at<double>(i, j));
		}
	}
	(*_stereoCameraParameters)["RotationMatrix"] = _rotation;

	
	std::vector<double> _translation;
	for(int m = 0 ; m<mTranslationMatrix.rows ; m++)
	{
		for(int n = 0 ; n<mTranslationMatrix.cols ; n++)
		{
			_translation.emplace_back(mTranslationMatrix.at<double>(m, n));
		}
	}
	(*_stereoCameraParameters)["TranslationMatrix"] = _translation;


	std::vector<double> _essential;
	for(int m = 0 ; m<mEssentialMatrix.rows ; m++)
	{
		for(int n = 0 ; n<mEssentialMatrix.cols ; n++)
		{
			_essential.emplace_back(mEssentialMatrix.at<double>(m, n));
		}
	}
	(*_stereoCameraParameters)["EssentialMatrix"] = _essential;


	std::vector<double> _fundamental;
	for(int m = 0 ; m<mFundamentalMatrix.rows ; m++)
	{
		for(int n = 0 ; n<mFundamentalMatrix.cols ; n++)
		{
			_fundamental.emplace_back(mFundamentalMatrix.at<double>(m, n));
		}
	}
	(*_stereoCameraParameters)["FundamentalMatrix"] = _fundamental;
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
void StereoCameraCalibrator::setConfiguration(const CalibratorConfiguration& configuration)
{
	mLeftHelper  = CameraCalibratorHelper(configuration);
	mRightHelper = CameraCalibratorHelper(configuration);
	AbstractCameraCalibrator::setConfiguration(configuration);
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
void StereoCameraCalibrator::getParameters(std::vector<double>& intrinsicLeft, std::vector<double>& distortionLeft, std::vector<double>& intrinsicRight, std::vector<double>& distortionRight)
{
	for (int i = 0; i < mLeftHelper.mIntrinsicMatrix.rows; i++)
	{
		for (int j = 0; j < mLeftHelper.mIntrinsicMatrix.cols; j++)
		{
			intrinsicLeft.emplace_back(mLeftHelper.mIntrinsicMatrix.at<double>(i, j));
		}
	}

	for (int m = 0; m < mLeftHelper.mDistortionCoeffs.rows; m++)
	{
		for (int n = 0; n < mLeftHelper.mDistortionCoeffs.cols; n++)
		{
			distortionLeft.emplace_back(mLeftHelper.mDistortionCoeffs.at<double>(m, n));
		}
	}

	for (int i = 0; i < mRightHelper.mIntrinsicMatrix.rows; i++)
	{
		for (int j = 0; j < mRightHelper.mIntrinsicMatrix.cols; j++)
		{
			intrinsicRight.emplace_back(mRightHelper.mIntrinsicMatrix.at<double>(i, j));
		}
	}

	for (int m = 0; m < mRightHelper.mDistortionCoeffs.rows; m++)
	{
		for (int n = 0; n < mRightHelper.mDistortionCoeffs.cols; n++)
		{
			distortionRight.emplace_back(mRightHelper.mDistortionCoeffs.at<double>(m, n));
		}
	}
}

void StereoCameraCalibrator::getDebugParameters(double& leftCoverage, double& leftRmsError, double& rightCoverage, double& rightRmsError)
{
	leftCoverage = mLeftHelper.mCoveragePercentage;
	leftRmsError = mLeftHelper.mLastRmsError;
	rightCoverage = mRightHelper.mCoveragePercentage;
	rightRmsError = mRightHelper.mLastRmsError;
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
bool StereoCameraCalibrator::_calibrateCamera()
{
	mLeftHelper.initializeCameraParameters();
	mRightHelper.initializeCameraParameters();
	int _calibrationFlag = mConfiguration.calibrationFlag();
	
	std::vector<cv::Point3f>              _chessBoard3DCornerPositions = mLeftHelper.calculateChessboard3DCornerPositions();
	std::vector<std::vector<cv::Point3f>> _allChessBoard3DCornerPositions(mNumImagesAccepted, _chessBoard3DCornerPositions);

	// Find intrinsic and extrinsic camera parameters	
	cv::Mat _rotationMatrix;
	cv::Mat _translationMatrix;
	cv::Mat _essentialMatrix;
	cv::Mat _fundamentalMatrix;

	double rmsError = cv::stereoCalibrate(_allChessBoard3DCornerPositions, 
		                                  mLeftHelper.mAllChessBoardCorners,
		                                  mRightHelper.mAllChessBoardCorners,           
                                          mLeftHelper.mIntrinsicMatrix, mLeftHelper.mDistortionCoeffs,
		                                  mRightHelper.mIntrinsicMatrix, mRightHelper.mDistortionCoeffs,
		                                  mLeftHelper.mInputImageSize,
										  _rotationMatrix, _translationMatrix, 
		                                  _essentialMatrix, _fundamentalMatrix,
		                                  _calibrationFlag);
	
	mLeftHelper.mLastRmsError  = rmsError;
	mRightHelper.mLastRmsError = rmsError;
	mRotationMatrix            = _rotationMatrix.clone();
	mTranslationMatrix         = _translationMatrix.clone();
	mEssentialMatrix           = _essentialMatrix.clone();
	mFundamentalMatrix         = _fundamentalMatrix.clone();

	std::cout << fmt::format("Re-projection Error={:.5f}, Left Coverage={:.3f}, Right Coverage={:.3f}\n", rmsError, mLeftHelper.mCoveragePercentage, mRightHelper.mCoveragePercentage);
	std::cout << fmt::format("Calibration Flag is ={:1d}\n", _calibrationFlag);
	std::cout << "Left Camera Matrix    : "<< mLeftHelper.mIntrinsicMatrix << "\n";
	std::cout << "Left Distortion Coeff : "<< mLeftHelper.mDistortionCoeffs << "\n";
	std::cout << "Right Camera Matrix   : "<< mRightHelper.mIntrinsicMatrix << "\n";
	std::cout << "Right Distortion Coeff: "<< mRightHelper.mDistortionCoeffs << "\n";
	
	return cv::checkRange(mLeftHelper.mIntrinsicMatrix) && 
		   cv::checkRange(mRightHelper.mIntrinsicMatrix) && 
		   cv::checkRange(mLeftHelper.mDistortionCoeffs) &&
		   cv::checkRange(mRightHelper.mDistortionCoeffs);
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //

}; // end namespace RCamera.
