#include "MyCamera_2.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>

using namespace std;


MyCamera_2::MyCamera_2(int initWidth, int initHeight)
{

}


void MyCamera_2::configCameraParamOnInit()
{
	m_objFeatureControlPtr->GetEnumFeature("TriggerMode")->SetValue("On");
	m_objFeatureControlPtr->GetEnumFeature("TriggerSource")->SetValue("Software");
	m_objFeatureControlPtr->GetFloatFeature("ExposureTime")->SetValue(10000.0);
	m_objFeatureControlPtr->GetFloatFeature("Gain")->SetValue(10.0);
	m_objFeatureControlPtr->GetEnumFeature("BalanceWhiteAuto")->SetValue("Once");
}


MyCamera_2::~MyCamera_2()
{
	;
}


cv::Mat MyCamera_2::getSingleFrame()
{
	//objFeatureControlPtr->GetCommandFeature("TriggerSoftware")->Execute();
	try { m_objFeatureControlPtr->GetCommandFeature("TriggerSoftware")->Execute(); }
	catch (CGalaxyException& e)
	{
		string se = string(e.what());
		ofstream OutFile = ofstream("D:\\1__GraduationProject\\QtCppProj\\log.txt", ios::app);
		OutFile << se;
		OutFile.close();
	}
	cv::Mat frame;
	try { m_objImageDataPtr = m_objStreamPtr->GetImage(100); }
	catch (CGalaxyException& e) { return frame; }
	if (m_objImageDataPtr->GetStatus() == GX_FRAME_STATUS_SUCCESS)
	{
		frame.create(m_objImageDataPtr->GetHeight(), m_objImageDataPtr->GetWidth(), CV_8UC3);
		void* pRGB24Buffer = NULL;
		pRGB24Buffer = m_objImageDataPtr->ConvertToRGB24(GX_BIT_0_7, GX_RAW2RGB_NEIGHBOUR, true);
		memcpy(frame.data, pRGB24Buffer, 3 * (m_objImageDataPtr->GetHeight())*(m_objImageDataPtr->GetWidth()));
		cv::flip(frame, frame, 0);
	}
	return frame;
}


void MyCamera_2::balanceWhite()
{
	if (m_bIsOpen) m_objFeatureControlPtr->GetEnumFeature("BalanceWhiteAuto")->SetValue("Once");
}

void MyCamera_2::setExposureTime(float val)
{
	if (m_bIsOpen && 20.0 < val && val < 1000000.0)
		m_objFeatureControlPtr->GetFloatFeature("ExposureTime")->SetValue(val);
}

void MyCamera_2::setResolution(int width, int height, bool on_init)
{
	if (minWidth <= width && minHeight <= height && width <= maxWidth && height <= maxHeight)
	{
		int offset_x = (maxWidth - width) / 2;
		int offset_y = (maxHeight - height) / 2;

		if (on_init)
		{
			m_objFeatureControlPtr->GetIntFeature("Width")->SetValue(width);
			m_objFeatureControlPtr->GetIntFeature("Height")->SetValue(height);
			m_objFeatureControlPtr->GetIntFeature("OffsetX")->SetValue(offset_x);
			m_objFeatureControlPtr->GetIntFeature("OffsetY")->SetValue(offset_y);
		}
		else
		{
			;
		}
	}
	else {}
}

std::string MyCamera_2::saveCurrentFrame()
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


void MyCamera_2::openDevice()
{
	// TODO: Add your control notification handler code here
	bool bIsDeviceOpen = false;         ///< �豸�Ƿ�򿪱�־
	bool bIsStreamOpen = false;         ///< �豸���Ƿ�򿪱�־

	try
	{
		GxIAPICPP::gxdeviceinfo_vector vectorDeviceInfo;
		IGXFactory::GetInstance().UpdateDeviceList(1000, vectorDeviceInfo);
		if (vectorDeviceInfo.size() <= 0)
		{
			//Found No Device
			return;
		}

		m_objDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(vectorDeviceInfo[0].GetSN(), GX_ACCESS_EXCLUSIVE);
		bIsDeviceOpen = true;
		m_objFeatureControlPtr = m_objDevicePtr->GetRemoteFeatureControl();

		int nStreamCount = m_objDevicePtr->GetStreamCount();
		if (nStreamCount > 0)
		{
			m_objStreamPtr = m_objDevicePtr->OpenStream(0);
			bIsStreamOpen = true;
		}
		else
			throw exception("δ�����豸��!");

		// �����û��ڴ��������֮�󣬸��ݵ�ǰ���绷�������������ͨ������ֵ���������������Ĳɼ�����,���÷����ο����´��롣
		GX_DEVICE_CLASS_LIST objDeviceClass = m_objDevicePtr->GetDeviceInfo().GetDeviceClass();
		if (GX_DEVICE_CLASS_GEV == objDeviceClass)
		{
			// �ж��豸�Ƿ�֧����ͨ�����ݰ�����
			if (true == m_objFeatureControlPtr->IsImplemented("GevSCPSPacketSize"))
			{
				// ��ȡ��ǰ���绷�������Ű���ֵ
				int nPacketSize = m_objStreamPtr->GetOptimalPacketSize();
				// �����Ű���ֵ����Ϊ��ǰ�豸����ͨ������ֵ
				m_objFeatureControlPtr->GetIntFeature("GevSCPSPacketSize")->SetValue(nPacketSize);
			}
		}

		configCameraParamOnInit();
		m_bIsOpen = true;
		//TODO ����ui���� enable and disable buttons.
	}
	catch (CGalaxyException& e)
	{
		if (bIsStreamOpen)
		{
			m_objStreamPtr->Close();
		}
		if (bIsDeviceOpen)
		{
			m_objDevicePtr->Close();
		}
		return;
	}
	catch (std::exception& e)
	{
		if (bIsStreamOpen)
		{
			m_objStreamPtr->Close();
		}
		if (bIsDeviceOpen)
		{
			m_objDevicePtr->Close();
		}
		return ;
	}
}



void MyCamera_2::closeDevice()
{

	try
	{
		if (m_bIsSnap)
		{
			m_objFeatureControlPtr->GetCommandFeature("AcquisitionStop")->Execute();
			m_objStreamPtr->StopGrab();
		}
	}
	catch (CGalaxyException) { ; }

	try
	{
		m_objStreamPtr->Close();
	}
	catch (CGalaxyException) { ; }

	try
	{
		m_objDevicePtr->Close();
	}
	catch (CGalaxyException) { ; }
	//update UI
	m_bIsOpen = false;
	m_bIsSnap = false;
}

void MyCamera_2::startSnap()
{
	try
	{
		m_objStreamPtr->StartGrab();
		m_objFeatureControlPtr->GetCommandFeature("AcquisitionStart")->Execute();
		m_objFeatureControlPtr->GetEnumFeature("TriggerMode")->SetValue("On");
		m_objFeatureControlPtr->GetEnumFeature("TriggerSource")->SetValue("Software");
		m_bIsSnap = true;
		//���½���
	}
	catch (CGalaxyException& e) { ; }
	catch (std::exception& e) { ; }
}

void MyCamera_2::stopSnap()
{
	try
	{
		m_objFeatureControlPtr->GetCommandFeature("AcquisitionStop")->Execute();
		m_objStreamPtr->StopGrab();
		m_bIsSnap = false;
		//���½���
	}
	catch (CGalaxyException& e) { ; }
	catch (std::exception& e) { ; }
}