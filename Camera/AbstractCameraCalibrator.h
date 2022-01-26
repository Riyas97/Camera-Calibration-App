
#ifndef _RVISION_CAMERA_ABSTRACTCAMERACALIBRATOR_
#define _RVISION_CAMERA_ABSTRACTCAMERACALIBRATOR_

#include "CalibratorConfiguration.h"
#include "nlohmann/json.hpp"

#include <string>


namespace RCamera {
;

// The IntrinsicCalibrationStatus enumeration defines the state of the CameraIntrinsicCalibration after setImage().
enum class CameraCalibrationStatus
{
	InvalidBytesPerPixel,
	ImageSizeInvalid,
	ImageAccepted,
	ImageRejected,
	Calibrated,
	CalibrationFailed
};

std::string toString(CameraCalibrationStatus status);


// The AbstractCameraCalibrator calibrates intrinsic and extrinsic parameters of a camera from a set of images.
class AbstractCameraCalibrator
{
public:
	
	AbstractCameraCalibrator();
	explicit AbstractCameraCalibrator(const CalibratorConfiguration& configuration);
	virtual ~AbstractCameraCalibrator();
	
	void saveParametersToFile(const std::string& fileName) const;
	
	virtual void saveParametersToJSON(nlohmann::json* json) const = 0;

	// Setting parameters in middle of calibration will reset current calibration.
	virtual void setConfiguration(const CalibratorConfiguration& parameters);
	

public:
	
	inline int                            numAcceptedImages() const {return mNumImagesAccepted;}
	inline const CalibratorConfiguration& configuration()     const {return mConfiguration;}


protected:
	
	CalibratorConfiguration mConfiguration;
	int                     mNumImagesAdded;     // The number of images added for calibration so far.
	int                     mNumImagesAccepted;  // The number of images accepted for calibration so far.
};

}; // end namespace RCamera

#endif // _RVISION_CAMERA_ABSTRACTCAMERACALIBRATOR_
