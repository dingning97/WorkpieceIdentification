#include "WorkpieceIdentification.h"
#include <QtWidgets/qmessagebox.h>
#include <string>

using namespace std;


WorkpieceIdentification::WorkpieceIdentification(QWidget *parent): QMainWindow(parent)
{
	ui.setupUi(this);
	ui.spinBox_cannyA->setValue(30);
	ui.spinBox_cannyA->setRange(1, 254);
	ui.spinBox_cannyB->setValue(70);
	ui.spinBox_cannyB->setRange(1, 254);
	ui.horizontalSlider_closeKSize->setMinimum(1);
	ui.horizontalSlider_closeKSize->setMaximum(10);
	ui.horizontalSlider_closeKSize->setValue(5);
	ui.horizontalSlider_exposureTime->setMinimum(2);
	ui.horizontalSlider_exposureTime->setMaximum(30);
	ui.horizontalSlider_exposureTime->setValue(11);
	QObject::connect(timer_camera, SIGNAL(timeout()), this, SLOT(on_timer_camera_timeout()));
	setWidgetsDisabled();
	//checkIsCameraLoaded();// check camera's availability and try to reload if not
}

void WorkpieceIdentification::on_pushButton_openCamera_clicked()
{/***********   Original Code Block   *********
	if (!timer_camera->isActive())
	{
		flag_cameraOpened = true;
		ui.pushButton_openCamera->setText("Close Camera");
		setWidgetsEnabled();
		timer_camera->start(40);
	}
	else
	{
		flag_cameraOpened = false;
		timer_camera->stop();
		setWidgetsDisabled();
		ui.pushButton_openCamera->setText("Open Camera");
		ui.label_display->clear();
		ui.label_display->setText("Camera not running.");
		ui.label_showResult->setText("Camera not running.");
		if(flag_startDetection)
		{
			flag_startDetection = false;
			ui.pushButton_startDetection->setText("Start Detecting");
		}
	}*/
/**************************************************************/
//***** new testing block
	if (!timer_camera->isActive())
	{
		cam.openDevice();
		if (cam.m_bIsOpen)
		{
			cam.startSnap();
			if (cam.m_bIsSnap)
			{
				string info = "Camera Is Succeddfully Loaded.\nModel:" + cam.modelName + "\nMAC:" + cam.MAC;
				ui.label_cameraInfo->setText(info.c_str());
				flag_cameraOpened = true;
				ui.pushButton_openCamera->setText("Close Camera");
				setWidgetsEnabled();
				timer_camera->start(40);
			}
		}
	}
	else
	{
		timer_camera->stop();
		cam.stopSnap();
		cam.closeDevice();
		flag_cameraOpened = false;
		ui.pushButton_openCamera->setText("Open Camera");
		ui.label_display->clear();
		ui.label_display->setText("Camera not running.");
		ui.label_showResult->setText("Camera not running.");
		string info = "Camera Closed.\nModel:Unknown\nMAC:Unknown";
		ui.label_cameraInfo->setText(info.c_str());
		ui.horizontalSlider_exposureTime->setValue(11);
		if (flag_startDetection)
		{
			flag_startDetection = false;
			ui.pushButton_startDetection->setText("Start Detecting");
		}
		setWidgetsDisabled();
	}
}

void WorkpieceIdentification::on_timer_camera_timeout()
{
	cv::Mat frame = cam.getSingleFrame();
	if(!frame.empty())
	{
		cv::resize(frame, frame, cv::Size(640, 640));
		if (flag_startDetection) 
			frame = detectOnFrame(frame);
		cvtColor(frame, frame, CV_BGR2RGB);
		QImage showFrame = QImage((const uchar*)(frame.data), frame.cols, frame.rows, QImage::Format_RGB888);
		ui.label_display->setPixmap(QPixmap::fromImage(showFrame));
	}
}


cv::Mat WorkpieceIdentification::detectOnFrame(cv::Mat frame)
{
	vector<Workpiece> instances = detector.segmentAndGetInstance(frame, false);
	if (instances.size() != 0)
	{
		updateDetectionResult(instances);
		return detector.drawInstances(frame, instances);
	}
	else
		return frame;
}


void WorkpieceIdentification::updateDetectionResult(vector<Workpiece> instances)
{
	string str = "Number of items : " + to_string(instances.size()) + "\n\n\n";
	for (int i = 0; i < instances.size(); i++)
	{
		str += "class : " + to_string(instances[i].cls) + "\n";
		str += "centroid : (" + to_string(instances[i].centroid.x) + ", " + to_string(instances[i].centroid.y) + ")\n\n";
	}
	ui.label_showResult->setText(str.c_str());
}


void WorkpieceIdentification::on_pushButton_startDetection_clicked()
{
	if (flag_cameraOpened)
	{
		if (flag_startDetection)
		{
			flag_startDetection = false;
			ui.pushButton_startDetection->setText("Start Detection");
		}
		else
		{
			flag_startDetection = true;
			ui.pushButton_startDetection->setText("Stop Detection");
		}
	}
	else QMessageBox::information(NULL, "Error", "Please Open Camera First.", QMessageBox::Ok);
}

void WorkpieceIdentification::on_pushButton_savePicture_clicked()
{
	if (flag_cameraOpened)
	{
		string str = cam.saveCurrentFrame();
		if (str == "") QMessageBox::information(NULL, "Error", "Fail to save.", QMessageBox::Ok);
		else
		{
			QString info = QString::fromStdString("Saved at " + str);
			QMessageBox::information(NULL, "OK", info, QMessageBox::Ok);
		}
		
	}
	else QMessageBox::information(NULL, "Error", "Camera Not Running.", QMessageBox::Ok);
}

void WorkpieceIdentification::on_pushButton_balanceWhite_clicked()
{
	if (flag_cameraOpened) cam.balanceWhite();
	else QMessageBox::information(NULL, "Error", "Camera Not Running.", QMessageBox::Ok);
}

void WorkpieceIdentification::on_pushButton_exit_clicked()
{
	close();
}

void WorkpieceIdentification::on_horizontalSlider_closeKSize_valueChanged()
{
	int val = ui.horizontalSlider_closeKSize->value();
	std::string sval = "Close Op K size:" + std::to_string(val) + "%";
	ui.label_5->setText(sval.c_str());
	detector.close_ksize_percentage = val / 100.0;
}

void WorkpieceIdentification::on_horizontalSlider_exposureTime_valueChanged()
{
	if (flag_cameraOpened)
	{
		int exp_time;
		int val = ui.horizontalSlider_exposureTime->value();
		if (2 <= val && val <= 10) exp_time = (val - 1) * 1000;
		else exp_time = (val - 10) * 10000;
		std::string sval = "Exposure:" + std::to_string(exp_time) + "ms";
		ui.label_6->setText(sval.c_str());
		cam.setExposureTime(float(exp_time));
	}

}

void WorkpieceIdentification::on_spinBox_cannyA_valueChanged()
{
	int oldVal = detector.cannyThreshA;
	int inVal = ui.spinBox_cannyA->value();
	if (abs(oldVal - inVal) < 2)
	{
		if (inVal < detector.cannyThreshB)
		{
			std::string sval = "Canny Thresh A: " + std::to_string(inVal);
			ui.label->setText(QString::fromStdString(sval));
			detector.cannyThreshA = inVal;
		}
		else
		{
			QMessageBox::information(NULL, "Error", "Canny thresh A has to be less than Canny thresh B.", QMessageBox::Ok);
			ui.spinBox_cannyA->setValue(oldVal);
		}
	}
}

void WorkpieceIdentification::on_spinBox_cannyA_editingFinished()
{
	if (ui.spinBox_cannyA->hasFocus())
	{
		int oldVal = detector.cannyThreshA;
		int inVal = ui.spinBox_cannyA->value();
		if (inVal < detector.cannyThreshB)
		{
			std::string sval = "Canny Thresh A: " + std::to_string(inVal);
			ui.label->setText(QString::fromStdString(sval));
			detector.cannyThreshA = inVal;
		}
		else
		{
			QMessageBox::information(NULL, "Error", "Canny thresh A has to be less than Canny thresh B.", QMessageBox::Ok);
			ui.spinBox_cannyA->setValue(oldVal);
		}
	}
}

void WorkpieceIdentification::on_spinBox_cannyB_valueChanged()
{
	int oldVal = detector.cannyThreshB;
	int inVal = ui.spinBox_cannyB->value();
	if (abs(oldVal - inVal) < 2)
	{
		if (inVal > detector.cannyThreshA)
		{
			std::string sval = "Canny Thresh B: " + std::to_string(inVal);
			ui.label_2->setText(QString::fromStdString(sval));
			detector.cannyThreshB = inVal;
		}
		else
		{
			QMessageBox::information(NULL, "Error", "Canny thresh B has to be greater than Canny thresh A.", QMessageBox::Ok);
			ui.spinBox_cannyB->setValue(oldVal);
		}
	}
}

void WorkpieceIdentification::on_spinBox_cannyB_editingFinished()
{
	if (ui.spinBox_cannyB->hasFocus())
	{
		int oldVal = detector.cannyThreshB;
		int inVal = ui.spinBox_cannyB->value();
		if (inVal > detector.cannyThreshA)
		{
			std::string sval = "Canny Thresh B: " + std::to_string(inVal);
			ui.label_2->setText(QString::fromStdString(sval));
			detector.cannyThreshB = inVal;
		}
		else
		{
			QMessageBox::information(NULL, "Error", "Canny thresh B has to be greater than Canny thresh A.", QMessageBox::Ok);
			ui.spinBox_cannyB->setValue(oldVal);
		}
	}
}

void WorkpieceIdentification::setWidgetsDisabled()
{
	ui.radioButton_unevenLight->setDisabled(true);

	ui.pushButton_savePicture->setDisabled(true);
	ui.pushButton_balanceWhite->setDisabled(true);
	ui.pushButton_startDetection->setDisabled(true);

	ui.horizontalSlider_3->setDisabled(true);
	ui.horizontalSlider_closeKSize->setDisabled(true);
	ui.horizontalSlider_exposureTime->setDisabled(true);

	ui.spinBox_cannyA->setDisabled(true);
	ui.spinBox_cannyB->setDisabled(true);
	ui.spinBox_3->setDisabled(true);
	ui.spinBox_4->setDisabled(true);
}

void WorkpieceIdentification::setWidgetsEnabled()
{
	ui.radioButton_unevenLight->setEnabled(true);

	ui.pushButton_savePicture->setEnabled(true);
	ui.pushButton_balanceWhite->setEnabled(true);
	ui.pushButton_startDetection->setEnabled(true);

	ui.horizontalSlider_3->setEnabled(true);
	ui.horizontalSlider_closeKSize->setEnabled(true);
	ui.horizontalSlider_exposureTime->setEnabled(true);

	ui.spinBox_cannyA->setEnabled(true);
	ui.spinBox_cannyB->setEnabled(true);
	ui.spinBox_3->setEnabled(true);
	ui.spinBox_4->setEnabled(true);
}


void WorkpieceIdentification::closeEvent(QCloseEvent* event)
{
	IGXFactory::GetInstance().Uninit();
}

