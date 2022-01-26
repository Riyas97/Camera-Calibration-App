
#ifndef _RVISION_CAMERA_STEREOCAMERACALIBRATOR_
#define _RVISION_CAMERA_STEREOCAMERACALIBRATOR_

#include "AbstractCameraCalibrator.h"
#include "CameraCalibratorHelper.h"


namespace RCamera {
;

class StereoCameraCalibrator : public AbstractCameraCalibrator
{
public:
	
	StereoCameraCalibrator();
	explicit StereoCameraCalibrator(const CalibratorConfiguration& configuration);
	~StereoCameraCalibrator();
	
	
	// Set next image for calibration.
	CameraCalibrationStatus setImage(const unsigned char* leftImage, const unsigned char* rightImage, 
		                             int width, int height, int bytesPerPixel, int numRowbytes);

	
	void saveParametersToJSON(nlohmann::json* json) const override;
	void setConfiguration(const CalibratorConfiguration& configuration) override;
	void getParameters(std::vector<double>& intrinsicLeft, std::vector<double>& distortionLeft, std::vector<double>& intrinsicRight, std::vector<double>& distortionRight);
	void getDebugParameters(double& leftCoverage, double& leftRmsError, double& rightCoverage, double& rightRmsError);


	#if defined(RVISIONLIB_HAVE_QT)
		inline QImage leftDisplayImage()  const {return mLeftHelper.displayImage(); }
		inline QImage rightDisplayImage() const {return mRightHelper.displayImage(); }
	#endif


private:
	

	bool _calibrateCamera();


private:

	CameraCalibratorHelper mLeftHelper;
	CameraCalibratorHelper mRightHelper;
	cv::Mat                mRotationMatrix;
	cv::Mat                mTranslationMatrix;
	cv::Mat                mEssentialMatrix;
	cv::Mat                mFundamentalMatrix;
};

}; // end namespace RCamera

#endif // _RVISION_CAMERA_STEREOCAMERACALIBRATOR_
