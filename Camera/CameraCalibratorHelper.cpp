
#include "CameraCalibratorHelper.h"

#include "fmt/format.h"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"


namespace RCamera {
;


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
CameraCalibratorHelper::CameraCalibratorHelper()
	: CameraCalibratorHelper(CalibratorConfiguration())
{
}
CameraCalibratorHelper::CameraCalibratorHelper(const CalibratorConfiguration& parameters)
	: mConfiguration(parameters),
	  mInputImageSize(-1, -1),
	  mCoveragePercentage(0),
	  mLastRmsError(std::numeric_limits<double>::max()),
	  mCameraParamersValid(false)
{
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
bool CameraCalibratorHelper::checkImageSize(int width, int height)
{
	// This is the first image.
	if(mInputImageSize.width == -1 && mInputImageSize.height == -1)
	{
		mInputImageSize = cv::Size(width, height);
		return true;
	}
	else
	{
		return mInputImageSize.width==width && mInputImageSize.height==height;
	}
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
void CameraCalibratorHelper::createCoverageMask(int width, int height)
{
	if(mCoverageMask.empty())
	{
		mCoverageMask = cv::Mat(height, width, CV_8UC1, cv::Scalar(50)); // 20% tint.
	}
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
std::vector<cv::Point2f> CameraCalibratorHelper::findChessboardCorners(const cv::Mat& inputImage) const
{
	const cv::Size2i _boardSize(mConfiguration.boardWidth(), mConfiguration.boardHeight());
	const int        _cornerDetectionFlagsFast = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE|cv::CALIB_CB_FAST_CHECK;
	const int        _cornerDetectionFlags     = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE;

	
	// Use smaller image for quickly rejecting an image when there is no chess board pattern.
	cv::Mat      _resizedImage;
	const double _scale = 0.5;
	cv::resize(inputImage, _resizedImage, cv::Size(), _scale, _scale, cv::INTER_LINEAR_EXACT);
	
	std::vector<cv::Point2f> _corners;
	if(cv::findChessboardCorners(_resizedImage, _boardSize, _corners, _cornerDetectionFlagsFast))
	{
		_corners.clear();
		if(cv::findChessboardCorners(inputImage, _boardSize, _corners, _cornerDetectionFlags))
		{
			cv::TermCriteria _termCriteria = cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1);
			cv::cornerSubPix(inputImage, _corners, cv::Size(11, 11), cv::Size(-1, -1), _termCriteria);
		}
		else
		{
			_corners.clear();
		}
	}
	else
	{
		_corners.clear();
	}
	
	if(_corners.size() != size_t(_boardSize.height) * size_t(_boardSize.width))
	{
		_corners.clear();
	}
	return _corners;
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
void CameraCalibratorHelper::updateCorners(const cv::Mat&           inputImage, 
	                                       std::vector<cv::Point2f> corners)
{
	mAllChessBoardCorners.emplace_back(corners);
	_updateCoverageMask(corners);
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
void CameraCalibratorHelper::updateDisplayImage(const cv::Mat& inputImage)
{
	// Add coverage to the display image.
	cv::cvtColor(inputImage, mDisplayImage, cv::COLOR_GRAY2RGB);

	cv::Mat _redChannel;
	cv::extractChannel(mDisplayImage, _redChannel, 0);
	cv::add(_redChannel, mCoverageMask, _redChannel);
	cv::insertChannel(_redChannel, mDisplayImage, 0);
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
void CameraCalibratorHelper::drawChessboardCorners(const std::vector<cv::Point2f>& corners)
{
	const cv::Size2i _boardSize(mConfiguration.boardWidth(), mConfiguration.boardHeight());
	cv::drawChessboardCorners(mDisplayImage, _boardSize, cv::Mat(corners), true);
}
void CameraCalibratorHelper::saveChessboardCorners(int imageIndex, const std::string& cameraStr)
{
	std::string _fileName = fmt::format("{}{}_{:04d}.png", mConfiguration.chessboardCornersImageFilePrefix(), cameraStr, imageIndex);
	cv::cvtColor(mDisplayImage, mDisplayImage, cv::COLOR_RGB2BGR);
	cv::imwrite(_fileName, mDisplayImage);
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
std::vector<cv::Point3f> CameraCalibratorHelper::calculateChessboard3DCornerPositions() const
{
	std::vector<cv::Point3f> _chessBoard3DCornerPositions;
	for(int i = 0 ; i<mConfiguration.boardHeight() ; ++i)
	{
		for(int j = 0 ; j<mConfiguration.boardWidth() ; ++j)
		{
			_chessBoard3DCornerPositions.emplace_back(j*mConfiguration.squareSize(), i*mConfiguration.squareSize(), 0.0f);
		}
	}
	return _chessBoard3DCornerPositions;
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
double CameraCalibratorHelper::calculateReprojectionErrors(const std::vector<std::vector<cv::Point3f>>& allChessBoard3DCornerPositions,
	                                                       const std::vector<cv::Mat>&                  rotationVectors,
	                                                       const std::vector<cv::Mat>&                  translationVectors) const
{
	const size_t _numImages  = allChessBoard3DCornerPositions.size();
	const size_t _numCorners = allChessBoard3DCornerPositions[0].size();

	size_t _totalPoints = 0;
	double _totalError = 0;
	for(size_t i=0 ; i<_numImages ; ++i)
	{
		std::vector<cv::Point2f> _projectImagePoints;
		cv::projectPoints(allChessBoard3DCornerPositions[i], rotationVectors[i], translationVectors[i], 
			              mIntrinsicMatrix, mDistortionCoeffs, _projectImagePoints);

		double _error = cv::norm(mAllChessBoardCorners[i], _projectImagePoints, cv::NORM_L2);

		_totalError += _error*_error;
		_totalPoints += _numCorners;
	}

	return std::sqrt(_totalError/_totalPoints);
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
void CameraCalibratorHelper::saveParameters(nlohmann::json* json, const std::string& prefix) const
{
	std::vector<double> _intrinsicMatrix;
	for(int i = 0 ; i<mIntrinsicMatrix.rows ; i++)
	{
		for(int j = 0 ; j<mIntrinsicMatrix.cols ; j++)
		{
			_intrinsicMatrix.emplace_back(mIntrinsicMatrix.at<double>(i, j));
		}
	}

	std::vector<double> _distortionCoeffs;
	for(int m = 0 ; m<mDistortionCoeffs.rows ; m++)
	{
		for(int n = 0 ; n<mDistortionCoeffs.cols ; n++)
		{
			_distortionCoeffs.emplace_back(mDistortionCoeffs.at<double>(m, n));
		}
	}

	(*json)[prefix + "IntrinsicMatrix" ] = _intrinsicMatrix;
	(*json)[prefix + "DistortionCoeffs"] = _distortionCoeffs;
	(*json)[prefix + "RmsError"        ] = mLastRmsError;
	(*json)[prefix + "Coverage"        ] = mCoveragePercentage;
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
void CameraCalibratorHelper::initializeCameraParameters()
{
	// Initialize camera matrix.
	mIntrinsicMatrix = cv::Mat::eye(3, 3, CV_64F);
	if(mConfiguration.calibFixAspectRatio())
	{
		mIntrinsicMatrix.at<double>(0, 0) = mConfiguration.calibAspectRatio();
	}

	// Initialize distortion matrix.
	mDistortionCoeffs = cv::Mat::zeros(8, 1, CV_64F);
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
#if defined(RVISIONLIB_HAVE_QT)
	QImage CameraCalibratorHelper::displayImage() const
	{
		QImage _image((uchar*)mDisplayImage.data, mDisplayImage.cols, mDisplayImage.rows, QImage::Format_RGB888);
		return _image;
	}
#endif
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
void CameraCalibratorHelper::_updateCoverageMask(const std::vector<cv::Point2f>& corners)
{
	if(!mCoverageMask.empty())
	{
		size_t _width = mConfiguration.boardWidth();
		size_t _height = mConfiguration.boardHeight();

		std::vector<cv::Point> _points;
		_points.emplace_back(corners[0]);
		_points.emplace_back(corners[_width - 1]);
		_points.emplace_back(corners[_height * _width - 1]);
		_points.emplace_back(corners[(_height - 1) * _width]);

		cv::fillPoly(mCoverageMask, _points, cv::Scalar(0));
		mCoveragePercentage = 1.0 - (cv::countNonZero(mCoverageMask) / (double(mCoverageMask.rows) * double(mCoverageMask.cols)));
	}
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //

}; // end namespace RCamera.
