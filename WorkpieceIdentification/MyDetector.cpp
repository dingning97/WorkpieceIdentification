#include "MyDetector.h"
#include <cmath>
#include <iostream>

double euc_distance(double arr1[], double arr2[], int len)
{
	double euc_dis = 0, sum = 0;
	for (int i = 0; i < len; i++)
	{
		sum += (arr1[i] - arr2[i])*(arr1[i] - arr2[i]);
	}
	euc_dis = sqrt(sum);
	return euc_dis;
}

cv::Mat MyDetector::unevenLightCompensate(cv::Mat img, int blockSize)
{
	cv::Mat a;
	return a;
}

std::vector<Workpiece> MyDetector::segmentAndGetInstance(cv::Mat img, bool debug)
{
	std::vector<Workpiece>				instances;
	std::vector<std::vector<cv::Point>> contours;

	if (flag_unevenLightCompensate)
		img = unevenLightCompensate(img);
	contours = instanceSegment(img, debug = debug);
	if (contours.size() != 0)
	{
		for (int i = 0; i < contours.size(); i++)
		{
			Workpiece info = getInstanceInfo(img, contours[i]);
			if (info.cls != -1) instances.push_back(info);
		}
	}
	return instances;
}

std::vector<std::vector<cv::Point>> MyDetector::instanceSegment(cv::Mat img, bool debug)
{
	if (cannyThreshA >= cannyThreshB) { cannyThreshA = 30; cannyThreshB = 70; }
	int img_size = img.size[0];
	int ksize = int(close_ksize_percentage * double(img_size));
	cv::Mat gray_img, gau_img, canny_img, kernel, close_img;
	std::vector<std::vector<cv::Point>> contours;

	cv::cvtColor(img, gray_img, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(gray_img, gau_img, cv::Size(9, 9), -1, -1);
	cv::Canny(gau_img, canny_img, cannyThreshA, cannyThreshB);
	kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(ksize, ksize));
	cv::morphologyEx(canny_img, close_img, cv::MORPH_CLOSE, kernel);
	cv::findContours(close_img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

	return removeFalseContours(contours);
}

cv::Point MyDetector::getCentroid(std::vector<cv::Point> contour)
{
	throw std::exception("Not Implemented yet.");
	return cv::Point();
}


int MyDetector::sampleRadiusAndClassify(cv::Mat roi)
{
	int width = roi.size().width;
	int height = roi.size().height;
	if (width > height)
	{
		cv::transpose(roi, roi);
		width = roi.size().width;
		height = roi.size().height;
	}
	double  radius[6];
	for (int i = 0; i < 6; i++)
	{
		int y = int(height * positions[i]);
		cv::Mat row = roi.rowRange(y, y + 1).clone();
		cv::Scalar num_pixels = cv::sum(row);
		radius[i] = num_pixels[0] / 255;
	}
	double max_radius = *std::max_element(radius, radius + 6);
	if (max_radius == 0) return -1;
	else
	{
		double distances[3][4], radius_inv[6];
		for (int i = 0; i < 6; i++)
		{
			radius[i] /= max_radius;
			radius_inv[5 - i] = radius[i];
		}
		for (int i = 0; i < 4; i++)
		{
			distances[0][i] = euc_distance(radius_template[i], radius, 6);
			distances[1][i] = euc_distance(radius_template[i], radius_inv, 6);
			distances[2][i] = distances[0][i] < distances[1][i] ? distances[0][i] : distances[1][i];
		}
		int cls = std::min_element(distances[2], distances[2] + 4) - distances[2];
		return (cls + 1);
	}
}

cv::Mat MyDetector::drawInstances(cv::Mat img, std::vector<Workpiece> instances, bool show)
{
	for (int i = 0; i < instances.size(); i++)
	{
		cv::circle(img, instances[i].centroid, 4, CV_RGB(0, 0, 255), -1);
		cv::Point2f pts[4];
		instances[i].minAreaRect.points(pts);
		cv::line(img, pts[0], pts[1], CV_RGB(0, 255, 0), 2);
		cv::line(img, pts[0], pts[3], CV_RGB(0, 255, 0), 2);
		cv::line(img, pts[2], pts[1], CV_RGB(0, 255, 0), 2);
		cv::line(img, pts[2], pts[3], CV_RGB(0, 255, 0), 2);
		cv::drawContours(img, cv::Mat(instances[i].contour), -1, CV_RGB(0, 0,255), 2);
		cv::putText(img, std::to_string(instances[i].cls), instances[i].centroid, cv::FONT_HERSHEY_COMPLEX, 1, CV_RGB(255, 0, 0));
	}
	if (show)
		cv::imshow("show detect result", img);
	return img;
}

std::vector<std::vector<cv::Point>> MyDetector::removeFalseContours(std::vector<std::vector<cv::Point>> contours)
{
	int num_false = 0, totalSize = contours.size();
	for (int i = 0; i < totalSize; i++)
	{
		if (contours[i - num_false].size() < contourFilterThresh)
		{
			contours.erase(contours.begin() + i - num_false);
			num_false += 1;
		}
	}
	return contours;
}

MyDetector::MyDetector()
{
	using namespace std;
	string template_path = "D:\\1__GraduationProject\\radius_template.txt";
	vector<double> sample;
	ifstream ifstr_data(template_path);
	double d;
	while (ifstr_data >> d)
		sample.push_back(d);
	ifstr_data.close();

	for (int j = 0; j < 4; j++)
		for (int k = 0; k < 6; k++)
			radius_template[j][k] = sample[6 * j + k];
}


Workpiece MyDetector::getInstanceInfo(cv::Mat img, std::vector<cv::Point> contour)
{
	using namespace std;
	using namespace cv;
	Workpiece info;
	Mat contour_img(img.size(), CV_8UC1, Scalar::all(0));
	RotatedRect roRect = minAreaRect(Mat(contour));
	Point2f pts[4];
	roRect.points(pts);
	Point center = roRect.center;
	double angle = roRect.angle;
	double roWidth = roRect.size.width;
	double roHeight = roRect.size.height;
	vector<vector<Point>> tmp_c;
	tmp_c.push_back(contour);
	drawContours(contour_img, tmp_c, -1, Scalar(255), -1);

	if (45 < abs(angle) && abs(angle) < 90)
	{
		angle = roRect.angle + 90;
		roHeight = roRect.size.width;
		roWidth = roRect.size.height;
	}

	// Decide whether to discard this target by its side ratio.
	float biggerSide = roRect.size.width > roRect.size.height ? roRect.size.width : roRect.size.height;
	float smallerSide = roRect.size.width < roRect.size.height ? roRect.size.width : roRect.size.height;
	float ratio = biggerSide / smallerSide;
	if (ratio < 3.5 || ratio > 5)
	{
		info.cls = -1;
		return info;
	}

	// Judge whether the rotated rect is out of bound.
	if (center.x - roWidth / 2 - 1 < 0 || center.y - roHeight / 2 - 1 < 0 ||
		center.x + roWidth / 2 + 2 >= img.size().width || center.y + roHeight / 2 + 2 >= img.size().height)
	{
		info.cls = -1;
		return info;
	}

	Mat ROI;
	if (angle == 0)
	{
		ROI = contour_img(Rect(center.x - roWidth / 2, center.y - roHeight / 2, roWidth + 2, roHeight + 2));
	}
	else
	{
		Mat M = getRotationMatrix2D(center, angle, 1), warpped;
		warpAffine(contour_img, warpped, M, contour_img.size());
		ROI = warpped(Rect(center.x - roWidth / 2, center.y - roHeight / 2, roWidth + 2, roHeight + 2));
	}

	// Calculate the angle between the horizontal line and the major axis of roRect
	if (roRect.size.width >= roRect.size.height)
		info.angle = - roRect.angle;
	else
		info.angle = 90 - roRect.angle;
	info.contour = contour;
	info.centroid = center;
	info.minAreaRect = roRect;
	info.resolution = img.size();
	info.cls = sampleRadiusAndClassify(ROI);

	return info;
}
