
#ifndef _RVISION_CAMERA_MONOCAMERACALIBRATOR_
#define _RVISION_CAMERA_MONOCAMERACALIBRATOR_

#include "AbstractCameraCalibrator.h"
#include "CameraCalibratorHelper.h"


namespace RCamera {
;

class MonoCameraCalibrator : public AbstractCameraCalibrator
{
public:
	
	MonoCameraCalibrator();
	explicit MonoCameraCalibrator(const CalibratorConfiguration& configuration);
	~MonoCameraCalibrator();
	
	
	// Set next image for calibration.
	CameraCalibrationStatus setImage(const unsigned char* image, int width, int height, int bytesPerPixel, int numRowbytes);

	
	void saveParametersToJSON(nlohmann::json* json) const override;
	void setConfiguration(const CalibratorConfiguration& configuration) override;
	void getParameters(std::vector<double>& intrinsic, std::vector<double>& distortion);
	void getDebugParameters(double& coveragePercentage, double& lastRmsError);


	#if defined(RVISIONLIB_HAVE_QT)
		inline QImage displayImage() const {return mHelper.displayImage();}
	#endif


private:
	

	bool _calibrateCamera();


private:

	CameraCalibratorHelper mHelper;
};

}; // end namespace RCamera

#endif // _RVISION_CAMERA_MONOCAMERACALIBRATOR_
