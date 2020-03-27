#include "MyCamera.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>

using namespace std;


MyCamera::MyCamera(int initWidth, int initHeight)
{
	try {
		//IGXFactory::GetInstance().Init();
		GxIAPICPP::gxdeviceinfo_vector vectorDeviceInfo;
		IGXFactory::GetInstance().UpdateDeviceList(1000, vectorDeviceInfo);
		if (vectorDeviceInfo.size() >= 1)
		{
			bIsDeviceLoaded = true;
			MAC = vectorDeviceInfo[0].GetMAC();
			modelName = vectorDeviceInfo[0].GetModelName();
			// Open Device
			objDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(vectorDeviceInfo[0].GetSN(), GX_ACCESS_EXCLUSIVE);
			bIsDeviceOpen = true;
			// Get Feature Control Pointer And Initialize Features
			objFeatureControlPtr = objDevicePtr->GetRemoteFeatureControl();
			configCameraParamOnInit();
			// Set Resolution
			setResolution(initWidth, initHeight);
			// Open Stream And Get Stream Pointer
			objStreamPtr = objDevicePtr->OpenStream(0);
			bIsStreamOpen = true;
			// Start Acquisition
			if (objDevicePtr->GetStreamCount() > 0)
			{
				objStreamPtr->StartGrab();
				objFeatureControlPtr->GetCommandFeature("AcquisitionStart")->Execute();
			}
		}
		else
		{
			bIsDeviceLoaded = false;
			//IGXFactory::GetInstance().Uninit();
		}
	}
	catch (CGalaxyException& e) { cout << e.what() << "\nthis CGalaxyException is from init"; }
	catch (exception& se) { cout << "\nthis exception is from init"; }
}


void MyCamera::configCameraParamOnInit()
{
	objFeatureControlPtr->GetEnumFeature("TriggerMode")->SetValue("On");
	objFeatureControlPtr->GetEnumFeature("TriggerSource")->SetValue("Software");
	objFeatureControlPtr->GetFloatFeature("ExposureTime")->SetValue(10000.0);
	objFeatureControlPtr->GetFloatFeature("Gain")->SetValue(10.0);
	objFeatureControlPtr->GetEnumFeature("BalanceWhiteAuto")->SetValue("Once");
}


MyCamera::~MyCamera()
{
	try
	{
		if (bIsDeviceLoaded)
		{
			objFeatureControlPtr->GetCommandFeature("AcquisitionStop")->Execute();
			if (bIsStreamOpen)
			{
				objStreamPtr->StopGrab();
				objStreamPtr->UnregisterCaptureCallback();
				objStreamPtr->Close();
			}
			if (bIsDeviceOpen) objDevicePtr->Close();
			//IGXFactory::GetInstance().Uninit();
		}
	}
	catch (CGalaxyException& e) { cout << "\nthis is from ~init CGalaxyException\n" << e.what(); }
	catch (exception& e) { cout << "\nthis is from ~init\nstd::exception"; }
}


cv::Mat MyCamera::getSingleFrame()
{
	//objFeatureControlPtr->GetCommandFeature("TriggerSoftware")->Execute();
	try { objFeatureControlPtr->GetCommandFeature("TriggerSoftware")->Execute(); }
	catch (CGalaxyException& e)
	{
		string se = string(e.what());
		ofstream OutFile = ofstream("D:\\1__GraduationProject\\QtCppProj\\log.txt", ios::app);
		OutFile << se;
		OutFile.close();
	}
	cv::Mat frame;
	try {objImageDataPtr = objStreamPtr->GetImage(100);}
	catch (CGalaxyException& e) { return frame; }
	if (objImageDataPtr->GetStatus() == GX_FRAME_STATUS_SUCCESS)
	{
		frame.create(objImageDataPtr->GetHeight(), objImageDataPtr->GetWidth(), CV_8UC3);
		void* pRGB24Buffer = NULL;
		pRGB24Buffer = objImageDataPtr->ConvertToRGB24(GX_BIT_0_7, GX_RAW2RGB_NEIGHBOUR, true);
		memcpy(frame.data, pRGB24Buffer, 3 * (objImageDataPtr->GetHeight())*(objImageDataPtr->GetWidth()));
		cv::flip(frame, frame, 0);
	}
	return frame;
}


void MyCamera::balanceWhite()
{
	if (bIsDeviceOpen) objFeatureControlPtr->GetEnumFeature("BalanceWhiteAuto")->SetValue("Once");
}

void MyCamera::setExposureTime(float val)
{
	if (bIsDeviceOpen && 20.0 < val && val < 1000000.0)
		objFeatureControlPtr->GetFloatFeature("ExposureTime")->SetValue(val);
}

void MyCamera::setResolution(int width, int height, bool on_init)
{
	if (minWidth <= width && minHeight <= height && width <= maxWidth && height <= maxHeight)
	{
		int offset_x = (maxWidth - width) / 2;
		int offset_y = (maxHeight - height) / 2;

		if (on_init)
		{
			objFeatureControlPtr->GetIntFeature("Width")->SetValue(width);
			objFeatureControlPtr->GetIntFeature("Height")->SetValue(height);
			objFeatureControlPtr->GetIntFeature("OffsetX")->SetValue(offset_x);
			objFeatureControlPtr->GetIntFeature("OffsetY")->SetValue(offset_y);
		}
		else
		{
			;
		}
	}
	else {}
}

std::string MyCamera::saveCurrentFrame()
{
	if (!m_frame.empty())
	{
		auto t = chrono::system_clock::to_time_t(chrono::system_clock::now());
		stringstream ss;
		ss << put_time(localtime(&t), "%f_%h-%m-%s");
		string str = "d:\\1__graduationproject\\" + ss.str() + "_cpp.jpg";
		cv::imwrite(str, m_frame);
		return str;
	}
	else
	{
		string str = "";
		return str;
	}
}




#ifdef CAMERA_TEST
int main()
{
	cv::Mat showFrame;
	MyCamera* cam = new MyCamera(1200, 1200);
	while (1)
	{
		showFrame = cam->getSingleFrame();
		if (!showFrame.empty())
		{
			cv::resize(showFrame, showFrame, cv::Size(512, 512));
			cv::imshow("from camera", showFrame);
			if (cv::waitKey(20) > 0)
				break;
		}
	}

	cout << "while loop broken";
	cv::destroyAllWindows();
	delete cam;
}
#endif





/**************************************/
//This Camera Class is for openCV

//MyCamera::MyCamera()
//{
//	cap.open(0);
//}
//
//MyCamera::~MyCamera()
//{
//	if (cap.isOpened()) cap.release();
//
//	destroyAllWindows();
//}
//
//
//void MyCamera::getSingleFrame()
//{
//	cap >> frame;
//	resize(frame, frame, Size(640, 640));
//}
//
//void MyCamera::saveCurrentFrame()
//{
//	using namespace std;
//	auto t = chrono::system_clock::to_time_t(chrono::system_clock::now());
//	stringstream ss;
//	ss << put_time(localtime(&t), "%F_%H-%M-%S");
//	string str = "D:\\1__GraduationProject\\" + ss.str() + ".jpg";
//	if (!frame.empty()) imwrite(str, frame);
//}
/////////////////////////////////////////////////////////////
//
//int main()
//{
//	MyCamera cam(0);
//	while (1)
//	{
//		cam.getFrame();
//		if (cam.frame.empty())
//			break;
//		imshow("video", cam.frame);
//		if (waitKey(20) > 0)
//			break;
//	}
//
//}