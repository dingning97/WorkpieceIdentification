#pragma once
#include <QTimer>
#include <QCloseEvent>
#include <QtWidgets/QMainWindow>
#include "ui_WorkpieceIdentification.h"
#include "MyCamera.h"
#include "MyDetector.h"


class WorkpieceIdentification : public QMainWindow
{
	Q_OBJECT

public:
	//MyCamera cam = MyCamera(1200, 1200);//original
	MyCamera_2 cam; //new for test
	MyDetector detector;
	QTimer* timer_camera = new QTimer(this);
	bool flag_cameraOpened = false;
	bool flag_startDetection = false;
	bool flag_unevenLightCompensate = false;
	WorkpieceIdentification(QWidget *parent = Q_NULLPTR);
	
	void setWidgetsEnabled();
	void setWidgetsDisabled();
	cv::Mat detectOnFrame(cv::Mat frame);
	void updateDetectionResult(std::vector<Workpiece> instances);

private slots:
	void on_timer_camera_timeout();
	void on_pushButton_exit_clicked();
	void on_pushButton_openCamera_clicked();
	void on_pushButton_savePicture_clicked();
	void on_pushButton_balanceWhite_clicked();
	void on_pushButton_startDetection_clicked();
	void on_horizontalSlider_closeKSize_valueChanged();
	void on_horizontalSlider_exposureTime_valueChanged();
	void on_spinBox_cannyA_valueChanged();
	void on_spinBox_cannyB_valueChanged();
	void on_spinBox_cannyA_editingFinished();
	void on_spinBox_cannyB_editingFinished();


private:
	Ui::WorkpieceIdentificationClass ui;

protected:
	void closeEvent(QCloseEvent*event);
};
