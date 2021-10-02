#include <stdio.h>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <iostream>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "iostream"

using namespace cv;
using namespace std;

Mat getMask(const Mat aftercvt);

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

	for (;;) {
		//wait for a new frame from camera and store it into 'frame'
		cap.read(frame);
		//check if we succeeded
		if (frame.empty()) {
			cerr << "ERROR! blank frame grabbed\n";
			break;
		}

		/*Mat gray;
		cvtColor(frame, gray, COLOR_BGR2GRAY);
		cvtColor(gray, gray, COLOR_GRAY2BGR);

		imshow("grayvideo", gray);*/

		Mat YCbCr;
		cvtColor(frame, YCbCr, COLOR_BGR2YCrCb);

		Mat mask = getMask(YCbCr);

		//show live and wait for a key with timeout long enough to show images
		imshow("mask video", mask);
		imshow("Live", frame);
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

	Mat mask(aftercvt.size(), CV_8U, Scalar(0));

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