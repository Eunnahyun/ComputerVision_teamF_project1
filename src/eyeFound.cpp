#include <stdio.h>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <iostream>
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

Mat getMask(const Mat aftercvt);
void pre_irisFount(const Mat src, const Mat origin);

int main(int, char**) {
	
	Mat frame;
	// --- INITIALIZE VIDIEOCAPTURE
	VideoCapture cap;
	//open the default camera using default API
	//cap.open(0);
	//OR advance usage: select any API backend
	int deviceID = 0;   // 0 = open default camera
	int apiID = cv::CAP_ANY;	//0 = autodetect default API
	//open selected camera using selected API
	cap.open(deviceID + apiID);
	//check if we succeeded
	if (!cap.isOpened()) {
		cerr << "ERROR! Unable to open camera\n";
		return -1;
	}

	//--GRAB AND WRITE LOOP
	cout << "Start grabbing" << endl << "Press any key to terminate" << endl;

	Mat mask;

	for (;;) {
		//wait for a new frame from camera and store it into 'frame'
		cap.read(frame);
		//check if we succeeded
		if (frame.empty()) {
			cerr << "ERROR! blank frame grabbed\n";
			break;
		}

		Mat YCbCr;
		cvtColor(frame, YCbCr, COLOR_BGR2YCrCb);

		mask = getMask(YCbCr); //mask is Gray type

		Mat grayFrame;
		cvtColor(frame, grayFrame, COLOR_BGR2GRAY);

		Mat equ_grayFrame;
		equalizeHist(grayFrame, equ_grayFrame);

		pre_irisFount(equ_grayFrame, frame);

		//show live and wait for a key with timeout long enough to show images
		
		if (waitKey(5) >= 0) break;
	}

	//the camera will be deinitialized automatically in VideoCapture destructor 
	return 0;
}

Mat getMask(const Mat aftercvt) {
	int Cr_min = 128;
	int Cr_max = 170;

	int Cb_min = 73;
	int Cb_max = 158;

	vector<Mat> planes;
	split(aftercvt, planes);

	Mat mask(aftercvt.size(), CV_8U, Scalar(0)); //result mask

	int result_video_row = aftercvt.rows;
	int result_video_col = aftercvt.cols;

	for (int i = 0; i < result_video_row; i++) {
		uchar* Plane_Y = planes[0].ptr<uchar>(i);
		uchar* Plane_Cr = planes[1].ptr<uchar>(i); 
		uchar* Plane_Cb = planes[2].ptr<uchar>(i);
		
		for (int j = 0; j < result_video_col; j++) {

			if (Plane_Y[j] <= 128) mask.at<uchar>(i, j) = 0; 

			else if ((Cr_min < Plane_Cr[j]) && (Plane_Cr[j] < Cr_max) && (Cb_min < Plane_Cb[j]) && (Plane_Cb[j] < Cb_max)) {
				mask.at<uchar>(i, j) = 255;
			}
		}
	}

	//erode(mask, mask, Mat(3, 3, CV_8U, Scalar(1)), Point(-1, -1), 2);

	return mask;
}


void pre_irisFount(const Mat src, const Mat origin) {

	Mat tmp;

	double minVal = 0;
	minMaxLoc(src, &minVal, NULL, NULL, NULL); //최솟값 찾기

	threshold(src, tmp, minVal + 10, 255, THRESH_BINARY_INV); //src에서 tmp보다 크면 


	vector<Vec4i> hierarchy;
	vector<vector<Point2i> > contours;
	findContours(tmp, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE); //윤곽선 찾기

	//contour
	int maxArea = 0;
	Rect maxContourRect;
	for (int i = 0; i < contours.size(); i++)
	{
		int area = contourArea(contours[i]); //컨투어로 둘러쌓인 면적
		Rect rect = boundingRect(contours[i]); //컨투어영역에 외접하는 직사각형
		double squareKoef = ((double)rect.width) / rect.height;

		//contour shape
#define SQUARE_KOEF 1.5

		if (area > maxArea && squareKoef < SQUARE_KOEF && squareKoef > 1.0 / SQUARE_KOEF)
		{
			maxArea = area;
			maxContourRect = rect;
		}
	}

	if (maxArea == 0)
	{
		cout << "Iris not found!" << endl;
	}
	else
	{
		Rect drawRect = Rect(maxContourRect.x - maxContourRect.width, maxContourRect.y - maxContourRect.height, maxContourRect.width * 3, maxContourRect.height * 3);

		rectangle(origin, drawRect, Scalar(255,0,0), 2);

		imshow("result video", origin);
	}
}