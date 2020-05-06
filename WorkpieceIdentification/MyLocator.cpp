#include"MyLocator.h"

using namespace std;

////int direction:
////1-upward / 2-downward / 3-leftward / 4-rightward
//void drawArrow(cv::Mat& img, cv::Point2f& startPt, cv::Point2f& endPt, int direction)
//{
//	cv::Point2f end1, end2;
//	if (1 == direction)
//	{
//		end1 = cv::Point2f(endPt.x - 30, endPt.y + 30);
//		end2 = cv::Point2f(endPt.x + 30, endPt.y + 30);
//	}
//	else if (2 == direction)
//	{
//		end1 = cv::Point2f(endPt.x - 30, endPt.y - 30);
//		end2 = cv::Point2f(endPt.x + 30, endPt.y - 30);
//	}
//	else if (3 == direction)
//	{
//		end1 = cv::Point2f(endPt.x + 30, endPt.y - 30);
//		end2 = cv::Point2f(endPt.x + 30, endPt.y + 30);
//	}
//	else if (4 == direction)
//	{
//		end1 = cv::Point2f(endPt.x - 30, endPt.y - 30);
//		end2 = cv::Point2f(endPt.x - 30, endPt.y + 30);
//	}
//	else return;
//
//	cv::line(img, startPt, endPt, CV_RGB(255, 255, 0), 2);
//	cv::line(img, endPt, end1, CV_RGB(255, 255, 0), 2);
//	cv::line(img, endPt, end2, CV_RGB(255, 255, 0), 2);
//}


void writeMatToFile(const cv::Mat& m, const string& filename)
{
	ofstream fout(filename);
	if (!fout)
	{
		cout << "File Not Opened" << endl;
		return;
	}
	for (int i = 0; i < m.rows; i++)
	{
		for (int j = 0; j < m.cols; j++)
		{
			fout << m.at<double>(i, j) << " ";
		}
		fout << endl;
	}

	fout.close();
	cout << "File Written Succes." << endl;
}

//string imgPath = "D:\\1GraduationPrj\\cproject\\*.bmp"; 
bool MyLocator::calibrateCameraInit(const string& imgPath)
{
	vector<cv::String> images;
	cv::glob(imgPath, images);
	for (int i = 0; i < images.size(); i++)
		cout << images[i] << endl;

	vector<vector<cv::Point3f> > objpoints;
	vector<vector<cv::Point2f> > imgpoints;
	vector<cv::Point3f> objPt;
	vector<cv::Point2f> cornerPt;
	cv::Mat frame, gray;
	bool success;

	for (int i{ 0 }; i < CHECKERBOARD[1]; i++)
	{
		for (int j{ 0 }; j < CHECKERBOARD[0]; j++)
		{
			objPt.push_back(cv::Point3f(j*gridLength, i*gridLength, 0));
		}
	}

	for (int i{ 0 }; i < images.size(); i++)
	{
		frame = cv::imread(images[i]);
		if (frame.empty())
		{
			continue;
		}
		cout << "the current image is " << i << "th" << endl;
		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

		success = cv::findChessboardCorners(gray, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), cornerPt,
			cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);
		if (success)
		{
			cv::TermCriteria criteria(cv::TermCriteria::EPS | cv::TermCriteria::Type::MAX_ITER, 30, 0.001);
			cv::cornerSubPix(gray, cornerPt, cv::Size(11, 11), cv::Size(-1, -1), criteria);
			cv::drawChessboardCorners(frame, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), cornerPt, success);
			objpoints.push_back(objPt);
			imgpoints.push_back(cornerPt);
		}

		string winName = "Image_" + to_string(i);
		cv::Mat showFame;
		cv::Size showSize(int(frame.size().width / 3), int(frame.size().height / 3));
		cv::resize(frame, showFame, showSize);
		cv::imshow(winName, showFame);
	}

	cv::calibrateCamera(objpoints, imgpoints, cv::Size(gray.rows, gray.cols), intrinsicMat, distCoeffs, rVec, tVec);
	cv::Rodrigues(tVec, rotMat);

	string filename = ".\\intrinsicParam.txt";
	writeMatToFile(intrinsicMat, filename);
	filename = ".\\distCoeffs.txt";
	writeMatToFile(distCoeffs, filename);
	filename = ".\\rVect.txt";
	writeMatToFile(rVec.reshape(1, 0), filename);
	filename = ".\\tVect.txt";
	writeMatToFile(tVec.reshape(1, 0), filename);

	cv::waitKey(0);
	cv::destroyAllWindows();
	return 1;
}


bool MyLocator::calibrateSingleImage(cv::Mat img)
{
	if (intrinsicMat.empty() || distCoeffs.empty()) {
		return false;
	}

	cv::Mat gray;
	vector<cv::Point3f> objPts, worldPts;
	vector<cv::Point2f> imgPts, cornerPts;
	for (int i = 0; i < CHECKERBOARD[1]; i++)
		for (int j = 0; j < CHECKERBOARD[0]; j++)
		{
			cv::Point3f pt(gridLength*j, gridLength*i, 0);
			worldPts.push_back(pt);
		}
	objPts.push_back(cv::Point3f(0, 0, 0));
	objPts.push_back(cv::Point3f(gridLength * 5, 0, 0));
	objPts.push_back(cv::Point3f(0, gridLength * 3, 0));
	objPts.push_back(cv::Point3f(gridLength * 5, gridLength * 3, 0));

	cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
	bool success = cv::findChessboardCorners(gray, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), cornerPts,
		cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);
	if (success)
	{
		cv::TermCriteria criteria(cv::TermCriteria::EPS | cv::TermCriteria::Type::MAX_ITER, 30, 0.001);
		cv::cornerSubPix(gray, cornerPts, cv::Size(11, 11), cv::Size(-1, -1), criteria);
		cv::drawChessboardCorners(img, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), cornerPts, success);
		cv::solvePnPRansac(worldPts, cornerPts, intrinsicMat, distCoeffs, rVec, tVec);
		cv::Rodrigues(rVec, rotMat);//Transform rVec to rotMat
		cv::projectPoints(objPts, rVec, tVec, intrinsicMat, distCoeffs, imgPts);
		estimateS(imgPts); //estimate S coeff

		//Draw info for visualization
		string drawText[4];
		drawText[0] = "(" + to_string(0) + "," + to_string(0) +  ")";
		drawText[1] = "(" + to_string(int(gridLength * 5)) + "," + to_string(0) + ")";
		drawText[2] = "(" + to_string(0) + "," + to_string(int(gridLength * 3)) + ")";
		drawText[3] = "(" + to_string(int(gridLength * 5)) + "," + to_string(int(gridLength * 3)) + ")";

		for (int i = 0; i < imgPts.size(); i++) {
			cv::circle(img, imgPts[i], 20, CV_RGB(255, 0, 0), -1);
			cv::putText(img, drawText[i], imgPts[i], cv::FONT_HERSHEY_PLAIN, 6, CV_RGB(255, 0, 0), 6);
		}
		cv::line(img, imgPts[0], imgPts[1], CV_RGB(255, 255, 0), 10);
		cv::line(img, imgPts[0], imgPts[2], CV_RGB(255, 255, 0), 10);
		cv::Mat showImg;
		cv::resize(img, showImg, cv::Size(resolution.width / 3, resolution.height / 3));

		cv::putText(showImg, "Coordinates are displayed in (X, Y).", cv::Point2f(10, 30), cv::FONT_HERSHEY_PLAIN, 2, CV_RGB(255, 255, 0), 2);
		cv::imwrite("result_calibrateSingleImage.jpg", showImg);
		return true;
	}
	else
		return false;
}

void MyLocator::estimateS(const vector<cv::Point2f> &imgPts)
{
	sCoeff = 0;
	//vector<cv::Point3f> objPts;
	//vector<cv::Point2f> imgPts;
	//cv::Mat rotationMatrix;
	//objPts.push_back(cv::Point3f(0, 0, 0));
	//objPts.push_back(cv::Point3f(143, 0, 0));
	//objPts.push_back(cv::Point3f(0, 143, 0));
	//objPts.push_back(cv::Point3f(143, 143, 0));
	//cv::Rodrigues(rVec, rotationMatrix);
	//cv::projectPoints(objPts, rVec, tVec, intrinsicMat, distCoeffs, imgPts);
	for (int i = 0; i < imgPts.size(); i++)
	{
		cv::Mat imagePoint = (cv::Mat_<double>(3, 1) << double(imgPts[i].x), double(imgPts[i].y), 1);
		cv::Mat tempMat = rotMat.inv() * intrinsicMat.inv() * imagePoint;
		cv::Mat tempMat2 = rotMat.inv() * tVec.reshape(0, 3);
		sCoeff += (tempMat2.at<double>(2, 0)) / tempMat.at<double>(2, 0);
	}
	sCoeff /= imgPts.size();
}

cv::Mat MyLocator::calcRealCoor(cv::Point2f imgCoor) //The image operated on has to be undistorted already.
{
	cv::Mat imagePoint = (cv::Mat_<double>(3, 1) << double(imgCoor.x), double(imgCoor.y), 1);
	cv::Mat worldPoint;
	worldPoint = rotMat.inv() * (sCoeff * intrinsicMat.inv() * imagePoint - tVec.reshape(0, 3));
	return worldPoint;
}


cv::Mat MyLocator::loadMatFromFile(const string& filename, int nRows, int nCols)
{
	vector<double> sample;
	ifstream ifstr_data(filename);
	if (ifstr_data.is_open())
	{
		cv::Mat mat(nRows, nCols, cv::DataType<double>::type);
		double d;
		while (ifstr_data >> d)
			sample.push_back(d);
		ifstr_data.close();
		if (nRows*nCols == sample.size())
		{
			for (int j = 0; j < nRows; j++)
				for (int k = 0; k < nCols; k++)
					mat.at<double>(j, k) = sample[nRows * j + k];
			return mat;
		}
		else
		{
			return cv::Mat();
		}
	}
	else
	{
		return cv::Mat();
	}
}
