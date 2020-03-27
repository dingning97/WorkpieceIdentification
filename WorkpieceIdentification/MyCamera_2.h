#pragma once
#pragma once
#include"GalaxyIncludes.h"
#include<iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>  


class MyCamera_2
{
public:
	cv::Mat m_frame;
	std::string modelName = "";
	std::string MAC = "";
	//bool bIsDeviceLoaded = false;
	//bool bIsDeviceOpen = false;
	//bool bIsStreamOpen = false;
	bool m_bIsOpen = false;
	bool m_bIsSnap = false;
	int resWidth, resHeight;
	int minWidth = 64, minHeight = 64;
	int maxWidth = 2448, maxHeight = 2048;

	CGXDevicePointer		 m_objDevicePtr;
	CGXStreamPointer		 m_objStreamPtr;
	CImageDataPointer        m_objImageDataPtr;
	CGXFeatureControlPointer m_objFeatureControlPtr;


	void openDevice();
	void closeDevice();
	void startSnap();
	void stopSnap();

	void configCameraParamOnInit();
	cv::Mat getSingleFrame();
	void balanceWhite();
	void setExposureTime(float val);
	void setResolution(int width, int height, bool on_init = true);
	std::string saveCurrentFrame();

	MyCamera_2(int initWidth, int initHeight);
	~MyCamera_2();
};
