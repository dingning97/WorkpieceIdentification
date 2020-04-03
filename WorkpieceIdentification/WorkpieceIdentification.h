#pragma once
#include <QTimer>
#include <QCloseEvent>
#include <QtWidgets/QMainWindow>
#include "ui_WorkpieceIdentification.h"
#include "MyCamera.h"
#include "MyLocator.h"
#include "MyDetector.h"


class WorkpieceIdentification : public QMainWindow
{
	Q_OBJECT

public:
	MyCamera cam;
	MyDetector detector;
	MyLocator locator;
	QTimer* timer_camera = new QTimer(this);
	cv::Mat currentFrame;
	int displayRes = 640;
	bool flag_cameraOpened = false;
	bool flag_startDetection = false;
	bool flag_unevenLightCompensate = false;
	WorkpieceIdentification(QWidget *parent = Q_NULLPTR);
	
	void setWidgetsEnabled();
	void setWidgetsDisabled();
	cv::Mat detectOnFrame(const cv::Mat &frame, double scaleFactor);
	void updateDetectionResult(const std::vector<Workpiece> &instances, double scaleFactor);

private slots:
	void on_timer_camera_timeout();
	void on_pushButton_exit_clicked();
	void on_pushButton_openCamera_clicked();
	void on_pushButton_savePicture_clicked();
	void on_pushButton_balanceWhite_clicked();
	void on_pushButton_calibrateCoor_clicked();
	void on_pushButton_startDetection_clicked();
	void on_horizontalSlider_closeKSize_valueChanged();
	void on_horizontalSlider_exposureTime_valueChanged();
	void on_horizontalSlider_contourThresh_valueChanged();
	void on_spinBox_cannyA_valueChanged();
	void on_spinBox_cannyB_valueChanged();
	void on_spinBox_cannyA_editingFinished();
	void on_spinBox_cannyB_editingFinished();


private:
	Ui::WorkpieceIdentificationClass ui;

protected:
	void closeEvent(QCloseEvent* event);
};
