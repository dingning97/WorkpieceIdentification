/*
 * This file is used to calibrate a new camera to get its
 * corresponding intrinsic and extrinsic parameters.
 * Four files will be saved at the same path as this file
 * if the calibration process turns out to be successful.
*/

#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>

using namespace std;


void writeMatToFile(const cv::Mat& m, const string& filename);

// Defining the dimensions of checkerboard
int CHECKERBOARD[2]{ 9,6 };

// Define length of a grid (mm)
double gridLength = 143. / 5.;

int __my_main()
{
	// Creating vector to store vectors of 3D points for each checkerboard image
	// 创建矢量以存储每个棋盘图像的三维点矢量
	vector<vector<cv::Point3f> > objpoints;

	// Creating vector to store vectors of 2D points for each checkerboard image
	// 创建矢量以存储每个棋盘图像的二维点矢量
	vector<vector<cv::Point2f> > imgpoints;

	// Defining the world coordinates for 3D points
	// 为三维点定义世界坐标系
	vector<cv::Point3f> objp;
	for (int i{ 0 }; i < CHECKERBOARD[1]; i++)
	{
		for (int j{ 0 }; j < CHECKERBOARD[0]; j++)
		{
			objp.push_back(cv::Point3f(j*gridLength, i*gridLength, 0));
		}
	}

	// Extracting path of individual image stored in a given directory
	// 提取存储在给定目录中的单个图像的路径
	vector<cv::String> images;

	// Path of the folder containing checkerboard images
	// 包含棋盘图像的文件夹的路径, 根据实际情况修改文件夹路径，
	// 标定过程需相机拍摄10~12张包含完整棋盘格的不同的图片。
	string path = ".\\imagesForCalib\\*.bmp";

	// 使用glob函数读取所有图像的路径
	cv::glob(path, images);
	for (int i = 0; i < images.size(); i++)
		cout << images[i] << endl;


	cv::Mat frame, gray;

	// vector to store the pixel coordinates of detected checker board corners
	// 存储检测到的棋盘转角像素坐标的矢量
	vector<cv::Point2f> corner_pts;
	bool success;

	// Looping over all the images in the directory
	// 循环读取图像
	for (int i{ 0 }; i < images.size(); i++)
	{
		frame = cv::imread(images[i]);
		cv::resize(frame, frame, cv::Size(int(frame.size().width / 3), int(frame.size().height / 3)));
		if (frame.empty())
		{
			continue;
		}
		if (i == 40)
		{
			int b = 1;
		}
		cout << "the current image is " << i << "th" << endl;
		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

		// Finding checker board corners
		// 寻找角点
		// If desired number of corners are found in the image then success = true
		// 如果在图像中找到所需数量的角，则success = true
		// opencv4以下版本，flag参数为CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE
		success = cv::findChessboardCorners(gray, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, 
			cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);

		/*
		 * If desired number of corner are detected,
		 * we refine the pixel coordinates and display
		 * them on the images of checker board
		*/

		// 如果检测到所需数量的角点，我们将细化像素坐标并将其显示在棋盘图像上
		if (success)
		{
			// 如果是OpenCV4以下版本，第一个参数为CV_TERMCRIT_EPS | CV_TERMCRIT_ITER
			cv::TermCriteria criteria(cv::TermCriteria::EPS | cv::TermCriteria::Type::MAX_ITER, 30, 0.001);

			// refining pixel coordinates for given 2d points.
			// 为给定的二维点细化像素坐标
			cv::cornerSubPix(gray, corner_pts, cv::Size(11, 11), cv::Size(-1, -1), criteria);

			// Displaying the detected corner points on the checker board
			// 在棋盘上显示检测到的角点
			cv::drawChessboardCorners(frame, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, success);

			objpoints.push_back(objp);
			imgpoints.push_back(corner_pts);
		}

		string winName = "Image" + to_string(i);
		//cv::Mat showFame;
		//cv::Size showSize(int(frame.size().width / 3), int(frame.size().height / 3));
		//cv::resize(frame, showFame, showSize);
		cv::imshow(winName, frame);
	}

	cv::Mat cameraMatrix, distCoeffs, R, T;

	/*
	 * Performing camera calibration by
	 * passing the value of known 3D points (objpoints)
	 * and corresponding pixel coordinates of the
	 * detected corners (imgpoints)
	*/
	// 通过传递已知3D点（objpoints）的值和检测到的角点（imgpoints）的相应像素坐标来执行相机校准
	cv::calibrateCamera(objpoints, imgpoints, cv::Size(gray.rows, gray.cols), cameraMatrix, distCoeffs, R, T);

	cout << endl;
	// 内参矩阵
	cout << "cameraMatrix : " << endl << cameraMatrix << endl << endl;
	// 透镜畸变系数
	cout << "distCoeffs : " << endl << distCoeffs << endl << endl;
	// rvecs
	cout << "Rotation vector : " << R.channels() << endl << R << endl << endl;
	// tvecs
	cout << "Translation vector : " << endl << T << endl << endl;

	// write MATs to file
	string filename = ".\\intrinsicParam.txt";
	writeMatToFile(cameraMatrix, filename);
	filename = ".\\distCoeffs.txt";
	writeMatToFile(distCoeffs, filename);
	filename = ".\\rVect.txt";
	writeMatToFile(R.reshape(1, 0), filename);
	filename = ".\\tVect.txt";
	writeMatToFile(T.reshape(1, 0), filename);

	// show images
	cv::waitKey(0);
	cv::destroyAllWindows();
	return 0;
}


//int main()
//{
//	__my_main();
//	return 0;
//}
//
//
//void writeMatToFile(const cv::Mat& m, const string& filename)
//{
//	ofstream fout(filename);
//
//	if (!fout)
//	{
//		cout << "File Not Opened" << endl;
//		return;
//	}
//
//	for (int i = 0; i < m.rows; i++)
//	{
//		for (int j = 0; j < m.cols; j++)
//		{
//			fout << m.at<double>(i, j) << " ";
//		}
//		fout <<endl;
//	}
//
//	fout.close();
//	cout << "File Written Succes." << endl;
//}