#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>


class MyLocator
{
public:
	int		 CHECKERBOARD[2]{ 9,6 };
	double	 gridLength = 143. / 5.;
	double	 sCoeff;
	cv::Size resolution = cv::Size(2248, 2048);
	cv::Mat	 intrinsicMat;
	cv::Mat  distCoeffs;
	cv::Mat  rVec, tVec, rotMat;

	static cv::Mat loadMatFromFile(const std::string& filename, int nRows, int nCols);
	bool	calibrateCameraInit(const std::string& imgPath);
	bool	calibrateSingleImage(cv::Mat img);
	cv::Mat calcRealCoor(cv::Point2f imgCoor);

private:
	void estimateS(const std::vector<cv::Point2f> &imgPts);
};