
#include "AbstractCameraCalibrator.h"
#include <fstream>


namespace RCamera {
;

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
std::string toString(CameraCalibrationStatus status)
{
	switch(status)
	{
		case RCamera::CameraCalibrationStatus::ImageSizeInvalid:
			return "ImageSizeInvalid";
		case RCamera::CameraCalibrationStatus::ImageAccepted:
			return "ImageAccepted";
		case RCamera::CameraCalibrationStatus::ImageRejected:
			return "ImageRejected";
		case RCamera::CameraCalibrationStatus::Calibrated:
			return "Calibrated";
		case RCamera::CameraCalibrationStatus::CalibrationFailed:
			return "CalibrationFailed";
		default:
			return "Invalid Camera Calibration Status";
	}
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
AbstractCameraCalibrator::AbstractCameraCalibrator()
	: AbstractCameraCalibrator(CalibratorConfiguration())
{
}
AbstractCameraCalibrator::AbstractCameraCalibrator(const CalibratorConfiguration& configuration)
	: mConfiguration(configuration),
	  mNumImagesAdded(0),
	  mNumImagesAccepted(0)
{
}
AbstractCameraCalibrator::~AbstractCameraCalibrator()
{
	
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
void AbstractCameraCalibrator::saveParametersToFile(const std::string& fileName) const
{
	nlohmann::json j;
	saveParametersToJSON(&j);

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
void AbstractCameraCalibrator::setConfiguration(const CalibratorConfiguration& configuration)
{
	mConfiguration     = configuration;
	mNumImagesAdded    = 0;
	mNumImagesAccepted = 0;
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


}; // end namespace RCamera.
