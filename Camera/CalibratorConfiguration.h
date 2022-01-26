
#ifndef _RVISION_CAMERA_CALIBRATORCONFIGURATION_H_
#define _RVISION_CAMERA_CALIBRATORCONFIGURATION_H_

#include "nlohmann/json.hpp"
#include <string>


namespace RCamera {
;

// The CalibratorConfiguration class defines the configuration of the CameraCalibrator.
class CalibratorConfiguration
{
public:

	CalibratorConfiguration();
	
	bool loadConfiguration(const std::string& fileName, std::string* error = nullptr);
	bool loadConfiguration(const nlohmann::json& json, std::string* error = nullptr);

	void saveConfiguration(const std::string& fileName) const;
	void saveConfiguration(nlohmann::json* json) const;

	int calibrationFlag() const;

	inline bool        flipVertically()                   const {return mFlipVertically;}
	inline int         boardWidth()                       const {return mBoardWidth;}
	inline int         boardHeight()                      const {return mBoardHeight;}
	inline float       squareSize()                       const {return mSquareSize;}
	inline bool        drawChessboardCorners()            const {return mDrawChessboardCorners;}
	inline bool        saveOnlyLastChessboardImage()      const {return mSaveOnlyLastChessboardImage;}
	inline std::string chessboardCornersImageFilePrefix() const {return mChessboardCornersImageFilePrefix;}
	inline bool        drawAcceptedImage()                const {return mDrawAcceptedImage;}
	inline std::string acceptedImageFilePrefix()          const {return mAcceptedImageFilePrefix;}
	inline int         minNumImages()                     const {return mMinNumImages;}
	inline int         maxNumImages()                     const {return mMaxNumImages;}
	inline double      maxRmsError()                      const {return mMaxRmsError;}
	inline double      minCoverage()                      const {return mMinCoverage;}
	inline int         imageBatchSize()                   const {return mImageBatchSize;}
	inline bool        calibFixPrincipalPoint()           const {return mCalibFixPrincipalPoint;}
	inline bool        calibZeroTangentDist()             const {return mCalibZeroTangentDist;}
	inline bool        calibFixAspectRatio()              const {return mCalibFixAspectRatio;}
	inline float       calibAspectRatio()                 const {return mCalibAspectRatio;}
	inline bool        calibFixK1()                       const {return mCalibFixK1;}
	inline bool        calibFixK2()                       const {return mCalibFixK2;}
	inline bool        calibFixK3()                       const {return mCalibFixK3;}
	inline bool        calibFixK4()                       const {return mCalibFixK4;}
	inline bool        calibFixK5()                       const {return mCalibFixK5;}

	inline void setFlipVertically(bool x)                                 {mFlipVertically = x;}
	inline void setBoardWidth(int x)                                      {mBoardWidth = x;}
	inline void setBoardHeight(int x)                                     {mBoardHeight = x;}
	inline void setSquareSize(float x)                                    {mSquareSize = x;}
	inline void setDrawChessboardCorners(bool x)                          {mDrawChessboardCorners = x;}
	inline void setSaveOnlyLastChessboardImage(bool x)                    {mSaveOnlyLastChessboardImage = x;}
	inline void setChessboardCornersImageFilePrefix(const std::string& x) {mChessboardCornersImageFilePrefix = x;}
	inline void setDrawAcceptedImage(bool x)                              {mDrawAcceptedImage = x;}
	inline void setAcceptedImageFilePrefix(const std::string& x)          {mAcceptedImageFilePrefix = x;}
	inline void setMinNumImages(int x)                                    {mMinNumImages = x;}
	inline void setMaxNumImages(int x)                                    {mMaxNumImages = x;}
	inline void setMaxRmsError(double x)                                  {mMaxRmsError = x;}
	inline void setMinCoverage(double x)                                  {mMinCoverage = x;}
	inline void setImageBatchSize(int x)                                  {mImageBatchSize = x;}
	inline void setCalibFixPrincipalPoint(bool x)                         {mCalibFixPrincipalPoint = x;}
	inline void setCalibZeroTangentDist(bool x)                           {mCalibZeroTangentDist = x;}
	inline void setCalibFixAspectRatio(bool x)                            {mCalibFixAspectRatio = x;}
	inline void setAspectRatio(float x)                                   {mCalibAspectRatio = x;}
	inline void setCalibFixK1(bool x)                                     {mCalibFixK1 = x;}
	inline void setCalibFixK2(bool x)                                     {mCalibFixK2 = x;}
	inline void setCalibFixK3(bool x)                                     {mCalibFixK3 = x;}
	inline void setCalibFixK4(bool x)                                     {mCalibFixK4 = x;}
	inline void setCalibFixK5(bool x)                                     {mCalibFixK5 = x;}


private:
	
	bool        mFlipVertically;                   // If true, flip images vertically before processing.
	int         mBoardWidth;                       // The number of inner corners along the width in the chess board pattern.
	int         mBoardHeight;                      // The number of inner corners along the height in the chess board pattern.
	float       mSquareSize;                       // The size of the each square in chess board pattern in used defined coordinate system.
	bool        mDrawChessboardCorners;            // If true, save an image with detected chess board corners.
	bool        mSaveOnlyLastChessboardImage;      // If true, only the last chessboard image is saved.
	std::string mChessboardCornersImageFilePrefix; // The prefix of the file name of the chess board corners image.
	bool        mDrawAcceptedImage;                // If true, save an the original accepted image.
	std::string mAcceptedImageFilePrefix;          // The prefix of the file name of the chess board corners image.
	int         mMinNumImages;                     // The minimum number of image to use for calibration.
	int         mMaxNumImages;                     // The maximum number of image to use for calibration.
	double      mMaxRmsError;                      // The maximum RMS error for calibration to be successful.
	double      mMinCoverage;                      // The coverage of the image required for calibration to be successful.
	int         mImageBatchSize;                   // The number of images to collect before running calibration.

	// OpenCV camera calibration flags.
	bool  mCalibFixPrincipalPoint;
	bool  mCalibZeroTangentDist;
	bool  mCalibFixAspectRatio;
	float mCalibAspectRatio;
	bool  mCalibFixK1;
	bool  mCalibFixK2;
	bool  mCalibFixK3;
	bool  mCalibFixK4;
	bool  mCalibFixK5;
};

}; // end namespace RCamera

#endif // _RVISION_CAMERA_CALIBRATORCONFIGURATION_H_
