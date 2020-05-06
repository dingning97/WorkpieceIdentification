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
	// ����ʸ���Դ洢ÿ������ͼ�����ά��ʸ��
	vector<vector<cv::Point3f> > objpoints;

	// Creating vector to store vectors of 2D points for each checkerboard image
	// ����ʸ���Դ洢ÿ������ͼ��Ķ�ά��ʸ��
	vector<vector<cv::Point2f> > imgpoints;

	// Defining the world coordinates for 3D points
	// Ϊ��ά�㶨����������ϵ
	vector<cv::Point3f> objp;
	for (int i{ 0 }; i < CHECKERBOARD[1]; i++)
	{
		for (int j{ 0 }; j < CHECKERBOARD[0]; j++)
		{
			objp.push_back(cv::Point3f(j*gridLength, i*gridLength, 0));
		}
	}

	// Extracting path of individual image stored in a given directory
	// ��ȡ�洢�ڸ���Ŀ¼�еĵ���ͼ���·��
	vector<cv::String> images;

	// Path of the folder containing checkerboard images
	// ��������ͼ����ļ��е�·��, ����ʵ������޸��ļ���·����
	// �궨�������������10~12�Ű����������̸�Ĳ�ͬ��ͼƬ��
	string path = ".\\imagesForCalib\\*.bmp";

	// ʹ��glob������ȡ����ͼ���·��
	cv::glob(path, images);
	for (int i = 0; i < images.size(); i++)
		cout << images[i] << endl;


	cv::Mat frame, gray;

	// vector to store the pixel coordinates of detected checker board corners
	// �洢��⵽������ת�����������ʸ��
	vector<cv::Point2f> corner_pts;
	bool success;

	// Looping over all the images in the directory
	// ѭ����ȡͼ��
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
		// Ѱ�ҽǵ�
		// If desired number of corners are found in the image then success = true
		// �����ͼ�����ҵ����������Ľǣ���success = true
		// opencv4���°汾��flag����ΪCV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE
		success = cv::findChessboardCorners(gray, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, 
			cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);

		/*
		 * If desired number of corner are detected,
		 * we refine the pixel coordinates and display
		 * them on the images of checker board
		*/

		// �����⵽���������Ľǵ㣬���ǽ�ϸ���������겢������ʾ������ͼ����
		if (success)
		{
			// �����OpenCV4���°汾����һ������ΪCV_TERMCRIT_EPS | CV_TERMCRIT_ITER
			cv::TermCriteria criteria(cv::TermCriteria::EPS | cv::TermCriteria::Type::MAX_ITER, 30, 0.001);

			// refining pixel coordinates for given 2d points.
			// Ϊ�����Ķ�ά��ϸ����������
			cv::cornerSubPix(gray, corner_pts, cv::Size(11, 11), cv::Size(-1, -1), criteria);

			// Displaying the detected corner points on the checker board
			// ����������ʾ��⵽�Ľǵ�
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
	// ͨ��������֪3D�㣨objpoints����ֵ�ͼ�⵽�Ľǵ㣨imgpoints������Ӧ����������ִ�����У׼
	cv::calibrateCamera(objpoints, imgpoints, cv::Size(gray.rows, gray.cols), cameraMatrix, distCoeffs, R, T);

	cout << endl;
	// �ڲξ���
	cout << "cameraMatrix : " << endl << cameraMatrix << endl << endl;
	// ͸������ϵ��
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