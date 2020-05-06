#pragma once
#include <vector>
#include <fstream> 
#include <iostream> 
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>  



class Workpiece
{
public:
	cv::Size resolution;
	cv::Point centroid;
	std::vector<cv::Point> contour;
	cv::RotatedRect minAreaRect;
	double angle; //in degree
	int cls;
};

class MyDetector
{
public:
	bool    flag_unevenLightCompensate = false;
	int	cannyThreshA = 30;
	int	cannyThreshB = 70;
	int contourFilterThresh = 400;
	double  close_ksize_percentage = 5. / 100.;
	double  positions[6] = { 1. / 12., 3. / 12., 5. / 12., 7. / 12., 9. / 12., 11. / 12. };
	double	radius_template[4][6];

public:
	MyDetector();
	cv::Mat								unevenLightCompensate(cv::Mat img, int blockSize = 16);
	std::vector<Workpiece>				segmentAndGetInstance(cv::Mat img, bool debug = false);
	std::vector<std::vector<cv::Point>> instanceSegment(cv::Mat img, bool debug = false);
	Workpiece							getInstanceInfo(cv::Mat img, std::vector<cv::Point> contour);
	cv::Point							getCentroid(std::vector<cv::Point> contour);
	int									sampleRadiusAndClassify(cv::Mat roi);
	cv::Mat								drawInstances(cv::Mat img, std::vector<Workpiece> instances, bool show = false);
	std::vector<std::vector<cv::Point>> removeFalseContours(std::vector<std::vector<cv::Point>> contours);

};