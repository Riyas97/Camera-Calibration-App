
#include "CalibratorConfiguration.h"

#include "opencv2/calib3d.hpp"

#include <fstream>


namespace RCamera {
;

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
CalibratorConfiguration::CalibratorConfiguration()
	: mFlipVertically(false),
	  mBoardWidth(11),
	  mBoardHeight(8),
	  mSquareSize(35),
	  mDrawChessboardCorners(true),
	  mSaveOnlyLastChessboardImage(true),
	  mChessboardCornersImageFilePrefix("Corners"),
	  mDrawAcceptedImage(true),
	  mAcceptedImageFilePrefix("Image"),
	  mMinNumImages(10),
	  mMaxNumImages(40),
	  mMaxRmsError(0.95),
	  mMinCoverage(0.10),
	  mImageBatchSize(4),
	  mCalibFixPrincipalPoint(false),
	  mCalibZeroTangentDist(false),
	  mCalibFixAspectRatio(true),
	  mCalibAspectRatio(1),
	  mCalibFixK1(false),
	  mCalibFixK2(false),
	  mCalibFixK3(false),
	  mCalibFixK4(true),
	  mCalibFixK5(true)
{
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
bool CalibratorConfiguration::loadConfiguration(const std::string& fileName, std::string* error)
{
	try
	{
		std::ifstream _stream(fileName);
		if(_stream)
		{
			nlohmann::json j = nlohmann::json::parse(_stream);
			return loadConfiguration(j, error);
		}
		else
		{
			if(error)
			{
				*error = "Cannot open " + fileName;
			}
			return false;
		}
	}
	catch(const std::exception& e)
	{
		if(error)
		{
			*error = e.what();
		}
		return false;
	}
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
bool CalibratorConfiguration::loadConfiguration(const nlohmann::json& json, std::string* error)
{
	try
	{
		if(json.contains("CalibratorConfiguration"))
		{
			nlohmann::json p = json.at("CalibratorConfiguration");

			mFlipVertically                   = p["FlipVertically"];
			mBoardWidth                       = p["BoardWidth"];
			mBoardHeight                      = p["BoardHeight"];
			mSquareSize                       = p["SquareSize"];
			mDrawChessboardCorners            = p["DrawChessboardCorners"];
			mSaveOnlyLastChessboardImage      = p["SaveOnlyLastChessboardImage"];
			mChessboardCornersImageFilePrefix = p["ChessboardCornersImageFilePrefix"];
			mDrawAcceptedImage                = p["DrawAcceptedImage"];
			mAcceptedImageFilePrefix          = p["AcceptedImageFilePrefix"];
			mMinNumImages                     = p["MinNumImages"];
			mMaxNumImages                     = p["MaxNumImages"];
			mMaxRmsError                      = p["MaxRmsError"];
			mMinCoverage                      = p["MinCoverage"];
			mImageBatchSize                   = p["ImageBatchSize"];
			mCalibFixPrincipalPoint           = p["CalibFixPrincipalPoint"];
			mCalibZeroTangentDist             = p["CalibZeroTangentDist"];
			mCalibFixAspectRatio              = p["CalibFixAspectRatio"];
			mCalibAspectRatio                 = p["CalibAspectRatio"];
			mCalibFixK1                       = p["CalibFixK1"];
			mCalibFixK2                       = p["CalibFixK2"];
			mCalibFixK3                       = p["CalibFixK3"];
			mCalibFixK4                       = p["CalibFixK4"];
			mCalibFixK5                       = p["CalibFixK5"];
			
			return true;
		}
		else
		{
			return false;
		}
	}
	catch(const std::exception& e)
	{
		if(error)
		{
			*error = e.what();
		}
		return false;
	}
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
void CalibratorConfiguration::saveConfiguration(const std::string& fileName) const
{
	nlohmann::json j;
	saveConfiguration(&j);

	std::ofstream _file;
	_file.open(fileName);
	if(_file)
	{
		_file << j.dump(2);
		_file.close();
	}
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
void CalibratorConfiguration::saveConfiguration(nlohmann::json* json) const
{
	std::string s = json->dump();
	(*json)["CalibratorConfiguration"] =
	{
		{"FlipVertically"                   , mFlipVertically},
		{"BoardWidth"                       , mBoardWidth},
		{"BoardHeight"                      , mBoardHeight},
		{"SquareSize"                       , mSquareSize},
		{"DrawChessboardCorners"            , mDrawChessboardCorners},
		{"SaveOnlyLastChessboardImage"      , mSaveOnlyLastChessboardImage},
		{"ChessboardCornersImageFilePrefix" , mChessboardCornersImageFilePrefix},
		{"DrawAcceptedImage"                , mDrawAcceptedImage},
		{"AcceptedImageFilePrefix"          , mAcceptedImageFilePrefix},
		{"MinNumImages"                     , mMinNumImages},
		{"MaxNumImages"                     , mMaxNumImages},
		{"MaxRmsError"                      , mMaxRmsError},
		{"MinCoverage"                      , mMinCoverage},
		{"ImageBatchSize"                   , mImageBatchSize},
		{"CalibFixPrincipalPoint"           , mCalibFixPrincipalPoint},
		{"CalibZeroTangentDist"             , mCalibZeroTangentDist},
		{"CalibFixAspectRatio"              , mCalibFixAspectRatio},
		{"CalibAspectRatio"                 , mCalibAspectRatio},
		{"CalibFixK1"                       , mCalibFixK1},
		{"CalibFixK2"                       , mCalibFixK2},
		{"CalibFixK3"                       , mCalibFixK3},
		{"CalibFixK4"                       , mCalibFixK4},
		{"CalibFixK5"                       , mCalibFixK5}
	};
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
// The camera calibration flag used by OpenCV.
int CalibratorConfiguration::calibrationFlag() const
{
	int _calibrationFlag = 0;
	if(calibFixPrincipalPoint()) {_calibrationFlag |= cv::CALIB_FIX_PRINCIPAL_POINT;}
	if(calibZeroTangentDist())   {_calibrationFlag |= cv::CALIB_ZERO_TANGENT_DIST;}
	if(calibFixAspectRatio())    {_calibrationFlag |= cv::CALIB_FIX_ASPECT_RATIO;}
	if(calibFixK1())             {_calibrationFlag |= cv::CALIB_FIX_K1;}
	if(calibFixK2())             {_calibrationFlag |= cv::CALIB_FIX_K2;}
	if(calibFixK3())             {_calibrationFlag |= cv::CALIB_FIX_K3;}
	if(calibFixK4())             {_calibrationFlag |= cv::CALIB_FIX_K4;}
	if(calibFixK5())             {_calibrationFlag |= cv::CALIB_FIX_K5;}
	return _calibrationFlag;
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //

}; // end namespace RCamera.
