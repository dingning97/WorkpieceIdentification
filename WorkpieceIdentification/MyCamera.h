#pragma once
#include"GalaxyIncludes.h"
#include<iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>  


class MyCamera
{
public:
	cv::Mat m_frame;
	std::string modelName = "";
	std::string MAC = "";
	bool bIsDeviceLoaded = false;
	bool bIsDeviceOpen = false;
	bool bIsStreamOpen = false;
	int resWidth, resHeight;
	int minWidth = 64, minHeight = 64;
	int maxWidth = 2448, maxHeight = 2048;

	CGXDevicePointer		 objDevicePtr;
	CGXStreamPointer		 objStreamPtr;
	CImageDataPointer        objImageDataPtr;
	CGXFeatureControlPointer objFeatureControlPtr;
	
	void configCameraParamOnInit();
	cv::Mat getSingleFrame();
	void balanceWhite();
	void setExposureTime(float val);
	void setResolution(int width, int height, bool on_init = true);
	std::string saveCurrentFrame();

	MyCamera(int initWidth, int initHeight);
	~MyCamera();
};


/**************************************/
//This Camera Class is for openCV

//class MyCamera
//{
//public:
//	cv::Mat frame;
//	VideoCapture cap;
//	void getSingleFrame();
//	void balanceWhite();
//	void setExposureTime();
//	void saveCurrentFrame();
//	MyCamera();
//	~MyCamera();
//};
