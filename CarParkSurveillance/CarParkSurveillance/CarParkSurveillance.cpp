// CarCounting.cpp : Defines the entry point for the console application.
//



#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/features2d/features2d.hpp>
#include<opencv2/nonfree/features2d.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/nonfree/nonfree.hpp>
#include<opencv2/gpu/gpu.hpp>

#include<stdio.h>
#include<iostream>
#include<conio.h>           // it may be necessary to change or remove this line if not using Windows

#include "Blob.h"
#include "AdaptiveBackgroundLearning.h"
#include "FramedifferenceBGS.h"

#define SHOW_STEPS            // un-comment or comment this line to show steps or not

// global variables ///////////////////////////////////////////////////////////////////////////////
const cv::Scalar SCALAR_BLACK = cv::Scalar(0.0, 0.0, 0.0);
const cv::Scalar SCALAR_WHITE = cv::Scalar(255.0, 255.0, 255.0);
const cv::Scalar SCALAR_YELLOW = cv::Scalar(0.0, 255.0, 255.0);
const cv::Scalar SCALAR_GREEN = cv::Scalar(0.0, 200.0, 0.0);
const cv::Scalar SCALAR_RED = cv::Scalar(0.0, 0.0, 255.0);

// function prototypes ////////////////////////////////////////////////////////////////////////////
void matchCurrentFrameBlobsToExistingBlobs(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs);
void addBlobToExistingBlobs(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex);
void addNewBlob(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs);
double distanceBetweenPoints(cv::Point point1, cv::Point point2);
void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName);
void drawAndShowContours(cv::Size imageSize, std::vector<Blob> blobs, std::string strImageName, cv::Mat colourImage);
bool checkIfBlobsCrossedTheLine(std::vector<Blob> &blobs, int &intHorizontalLinePosition, int &carCount);
void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy);
void drawCarCountOnImage(int &carCount, cv::Mat &imgFrame2Copy);
void drawCarDensityOnImage(double &carCount, cv::Mat &imgFrame2Copy);
void drawRegion(cv::Size imageSize, cv::vector<cv::Point2f> points, cv::Mat imageCopy);
void matchCurrentFrameBlobsToExistingBlobs2(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs);
void CallBackFunc(int event, int x, int y, int flags, void* userdata);
void printNumberofCar(int entrance, bool entExt);

int carDensity = 0;

int carNumberCounter = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////

int minHessian = 10;


cv::Point drawingPointHorizontal[9];
cv::Point drawingPointVertical[9];
std::vector<int> countingfeatures;

cv::gpu::ORB_GPU descriptorGPU;

cv::BFMatcher matcher(cv::NORM_L2);
//std::vector<cv::DMatch> matches;

//std::vector<std::vector<cv::DMatch>> matches;

cv::Mat zoneA;
cv::Mat zoneB;
cv::Mat zoneC;
cv::Mat zoneD;

cv::Mat entrance1;
cv::Mat entrance2;
cv::Mat entrance3;
cv::Mat entrance4;
cv::Mat entrance5;
cv::Mat entrance6;

cv::Mat carParkZone;


int unitObjCounter = 1;



#if defined(_MSC_VER) || defined(WIN32)  || defined(_WIN32) || defined(__WIN32__) \
    || defined(WIN64)    || defined(_WIN64) || defined(__WIN64__) 

#include <windows.h>
bool _qpcInited = false;
double PCFreq = 0.0;
__int64 CounterStart = 0;
void InitCounter() {
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li))
	{
		std::cout << "QueryPerformanceFrequency failed!\n";
	}
	PCFreq = double(li.QuadPart) / 1000.0f;
	_qpcInited = true;
}
double CLOCK()
{
	if (!_qpcInited) InitCounter();
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart) / PCFreq;
}

#endif

#if defined(unix)        || defined(__unix)      || defined(__unix__) \
    || defined(linux)       || defined(__linux)     || defined(__linux__) \
    || defined(sun)         || defined(__sun) \
    || defined(BSD)         || defined(__OpenBSD__) || defined(__NetBSD__) \
    || defined(__FreeBSD__) || defined __DragonFly__ \
    || defined(sgi)         || defined(__sgi) \
    || defined(__MACOSX__)  || defined(__APPLE__) \
    || defined(__CYGWIN__) 
double CLOCK() {
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (t.tv_sec * 1000) + (t.tv_nsec*1e-6);
}
#endif

double _avgdur = 0;
double _fpsstart = 0;
double _avgfps = 0;
double _fps1sec = 0;

double avgdur(double newdur) {
	_avgdur = 0.98*_avgdur + 0.02*newdur;
	return _avgdur;
}

double avgfps() {
	if (CLOCK() - _fpsstart>1000) {
		_fpsstart = CLOCK();
		_avgfps = 0.7*_avgfps + 0.3*_fps1sec;
		_fps1sec = 0;
	}
	_fps1sec++;
	return _avgfps;
}

void process(cv::Mat& frame) {
	Sleep(3);
}

int main(void) {

	//cv::Ptr<cv::FastFeatureDetector> detectorr = cv::FastFeatureDetector::create("FASTdetector");
	//std::vector<cv::KeyPoint> keyPoint;



	for (int i = 0; i < 25; i++) {
		countingfeatures.push_back(0);
	}


	IBGS *bgs;
	bgs = new AdaptiveBackgroundLearning;

	IBGS *bgs2;
	bgs2 = new FrameDifferenceBGS;



	std::cout << CV_VERSION << "\n";



	cv::VideoCapture capVideo;

	cv::Mat imgFrame1;
	cv::Mat imgFrame2;

	std::vector<Blob> blobs;

	cv::Point crossingLine[2];

	cv::Point crossingLine2[2];

	cv::Point crossingLine3[2];

	cv::Point crossingLine4[2];

	cv::Point crossingLine5[2];

	cv::Point crossingLine6[2];

	int carCount = 0;

	cv::FastFeatureDetector detector(minHessian);
	std::vector<cv::KeyPoint> keypoints_new, keypoints_old;
	cv::Mat newImg, oldImg;

	cv::SiftDescriptorExtractor descriptor;
	cv::Mat newDes, oldDes;




	cv::Mat mask;
	mask = cv::imread("mask.jpg");




	zoneA = cv::imread("zoneAmask.png");
	zoneB = cv::imread("zoneBmask.png");
	zoneC = cv::imread("zoneCmask.png");
	zoneD = cv::imread("zoneDmask.png");

	entrance1 = cv::imread("e1.png");
	entrance2 = cv::imread("e2.png");
	entrance3 = cv::imread("e3.png");
	entrance4 = cv::imread("e4.png");
	entrance5 = cv::imread("e5.png");
	entrance6 = cv::imread("e6.png");

	carParkZone = cv::imread("carParkZone.png");


	//cv::namedWindow("maskk", 1);
	//cv::setMouseCallback("maskk", CallBackFunc, NULL);
	//imshow("maskk", mask);


	cv::Mat ROImask;
	ROImask = cv::imread("ROImask.png");
	


	capVideo.open("20161018_084200.mp4");

	if (!capVideo.isOpened()) {                                                 // if unable to open video file
		std::cout << "error reading video file" << std::endl << std::endl;      // show error message
		_getch();                   // it may be necessary to change or remove this line if not using Windows
		return(0);                                                              // and exit program
	}

	if (capVideo.get(CV_CAP_PROP_FRAME_COUNT) < 2) {
		std::cout << "error: video file must have at least two frames";
		_getch();                   // it may be necessary to change or remove this line if not using Windows
		return(0);
	}

	capVideo.read(imgFrame1);
	capVideo.read(imgFrame2);

	

	int intHorizontalLinePosition = (int)std::round((double)imgFrame1.rows * 0.45);
	int intHorizontalLinePosition2 = (int)std::round((double)imgFrame1.rows * 0.42);
	int intHorizontalLinePosition3 = (int)std::round((double)imgFrame1.rows * 0.6);




	int intHorizontalLinePosition4 = (int)std::round((double)imgFrame1.rows * 0.8);




	int intVerticalLinePosition1 = (int)std::round((double)imgFrame1.rows * 0.82);
	int intVerticalLinePosition2 = (int)std::round((double)imgFrame1.rows * 0.87);

	int intVerticalLinePosition3 = (int)std::round((double)imgFrame1.rows * 0.94);
	int intVerticalLinePosition4 = (int)std::round((double)imgFrame1.rows * 0.98);




	crossingLine[0].x = 530;
	crossingLine[0].y = intHorizontalLinePosition;
	crossingLine[1].x = imgFrame1.cols - 90;
	crossingLine[1].y = intHorizontalLinePosition;

	crossingLine2[0].x = 310;
	crossingLine2[0].y = intHorizontalLinePosition2;
	crossingLine2[1].x = imgFrame1.cols - 310;
	crossingLine2[1].y = intHorizontalLinePosition2;

	crossingLine3[0].x = 50;
	crossingLine3[0].y = intHorizontalLinePosition3;
	crossingLine3[1].x = imgFrame1.cols - 570;
	crossingLine3[1].y = intHorizontalLinePosition3;

	crossingLine4[0].x = 15;
	crossingLine4[0].y = intVerticalLinePosition1;
	crossingLine4[1].x = 15;
	crossingLine4[1].y = intVerticalLinePosition2;

	crossingLine5[0].x = 610;
	crossingLine5[0].y = intHorizontalLinePosition4;
	crossingLine5[1].x = 630;
	crossingLine5[1].y = intHorizontalLinePosition4;

	crossingLine6[0].x = 560;
	crossingLine6[0].y = intVerticalLinePosition3;
	crossingLine6[1].x = 560;
	crossingLine6[1].y = intVerticalLinePosition4;







	char chCheckForEscKey = 0;

	bool blnFirstFrame = true;

	bool firstTime = true;

	int frameCount = 2;

	//testong testing
	cv::Mat copy_image;
	bool dontshow = true;

	int dontshowcounter = 0;

	std::vector<cv::Point2f> features_prev, features_next;
	//test


	while (capVideo.isOpened() && chCheckForEscKey != 27) {

		double start = CLOCK();

		cv::Mat structuringElement3x3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
		cv::Mat structuringElement5x5 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
		cv::Mat structuringElement7x7 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
		cv::Mat structuringElement15x15 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15));

		std::vector<Blob> currentFrameBlobs;



		cv::Mat ROImask1 = cv::Mat::zeros(ROImask.size(), ROImask.type());
		cv::Mat ROImask2 = cv::Mat::zeros(ROImask.size(), ROImask.type());


		cv::Mat imgFrame1Copy = imgFrame1.clone();
		cv::Mat imgFrame2Copy = imgFrame2.clone();

		imgFrame1Copy.copyTo(ROImask1, ROImask);
		imgFrame2Copy.copyTo(ROImask2, ROImask);

		imgFrame1Copy = ROImask1;
		imgFrame2Copy = ROImask2;

		//cv::imshow("preimg1", ROImask1);



		/*cv::Mat flow, cflow, frame;
		cv::Mat gray, prevgray, uflow;
		cv::Mat xFlow, yFlow;*/

		//cvtColor(imgFrame1Copy, prevgray, COLOR_BGR2GRAY);
		//cvtColor(imgFrame2Copy, gray, COLOR_BGR2GRAY);

		//imgFrame1Copy.convertTo(prevgray, CV_32FC1, 1.0 / 255.0);
		//imgFrame2Copy.convertTo(gray, CV_32FC1, 1.0 / 255.0);

		/*imgFrame1Copy.convertTo(prevgray, CV_32F);
		imgFrame2Copy.convertTo(gray, CV_32F);

		cvtColor(prevgray, prevgray, COLOR_BGR2GRAY);
		cvtColor(gray, gray, COLOR_BGR2GRAY);



		cv::gpu::GpuMat GPUgray(gray);
		cv::gpu::GpuMat GPUpreygray(prevgray);
		cv::gpu::GpuMat GPUxFlow, GPUyFlow;

		cv::gpu::BroxOpticalFlow test =	cv::gpu::BroxOpticalFlow(0.197f, 0.8f, 50.0f, 10, 77, 10);

		std::cout << GPUpreygray.type() << "\n";
		std::cout << GPUgray.type() << "\n";

		std::cout << "pre gray col : " << GPUpreygray.cols << " " << GPUpreygray.rows << "\n";
		std::cout << "gray col : " << GPUgray.cols << " " << GPUgray.rows << "\n";*/




		//cv::gpu::BroxOpticalFlow test(0.197f, 0.8f, 50.0f, 10, 77, 10);
		//test.dense(GPUpreygray, GPUgray, GPUxFlow, GPUyFlow);
		//test.operator()(GPUpreygray, GPUgray, GPUxFlow, GPUyFlow);




		//GPUxFlow.download(xFlow);
		//GPUyFlow.download(yFlow);

		//	cv::Mat temp = xFlow + yFlow;
		//	cv::imshow("temp", temp);


		//cv::threshold(xFlow, xFlow, 20, 255, cv::THRESH_BINARY);
		//cv::threshold(yFlow, yFlow, 20, 255, cv::THRESH_BINARY);

		//cv::imshow("x", xFlow);
		//cv::imshow("y", yFlow);



		//::calcOpticalFlowFarneback(prevgray, gray, uflow, 0.5, 3, 15, 3, 5, 1.2, 0);

		//std::cout << uflow.channels() << "\n";
		//std::cout << uflow.type() << "\n";

		////extraxt x and y channels
		//cv::Mat xy[2]; //X,Y
		//cv::split(uflow, xy);

		////calculate angle and magnitude
		//cv::Mat magnitude, angle;
		//cv::cartToPolar(xy[0], xy[1], magnitude, angle, true);

		////translate magnitude to range [0;1]
		//double mag_max;
		//cv::minMaxLoc(magnitude, 0, &mag_max);
		//magnitude.convertTo(magnitude, -1, 1.0 / mag_max);

		////build hsv image
		//cv::Mat _hsv[3], hsv;
		//_hsv[0] = angle;
		//_hsv[1] = cv::Mat::ones(angle.size(), CV_32F);
		//_hsv[2] = magnitude;
		//cv::merge(_hsv, 3, hsv);

		////convert to BGR and show
		//cv::Mat bgr;//CV_32FC3 matrix
		//cv::cvtColor(hsv, bgr, cv::COLOR_HSV2BGR);
		//cv::imshow("optical flow", bgr);





		//imgFrame1Copy = imgFrame1Copy + cv::Scalar(100, 100, 100);




		//BGS
		cv::Mat img_mask;
		cv::Mat img_bkgmodel;
		bgs->process(imgFrame1Copy, img_mask, img_bkgmodel);
		//BGS

		//BGS2

		cv::Mat img_mask2;
		cv::Mat img_bkgmodel2;
		bgs2->process(imgFrame1Copy, img_mask2, img_bkgmodel2);


		cv::imshow("original mask", img_mask);


		//BGS2
		cv::Mat fusion;
		//
		if (img_mask2.cols > 0) {
			cv::bitwise_and(img_mask, img_mask2, fusion);
			cv::imshow("fusion", fusion);
			img_mask = fusion;
		}




		cv::Mat imgDifference;
		cv::Mat imgThresh;

		cv::Mat colorForeground = cv::Mat::zeros(img_mask.size(), img_mask.type());
		imgFrame1Copy.copyTo(colorForeground, img_mask);




		//	cv::imshow("frame different", img_mask2);


		/*cv::cvtColor(imgFrame1Copy, imgFrame1Copy, CV_BGR2GRAY);
		cv::cvtColor(imgFrame2Copy, imgFrame2Copy, CV_BGR2GRAY);

		cv::GaussianBlur(imgFrame1Copy, imgFrame1Copy, cv::Size(5, 5), 0);
		cv::GaussianBlur(imgFrame2Copy, imgFrame2Copy, cv::Size(5, 5), 0);

		cv::absdiff(imgFrame1Copy, imgFrame2Copy, imgDifference);

		cv::threshold(imgDifference, imgThresh, 15, 255.0, CV_THRESH_BINARY);*/



		//Threshold (Grey Scale Image)
		cv::threshold(img_mask, imgThresh, 30, 255.0, CV_THRESH_BINARY);
		cv::imshow("imgThresh", imgThresh);
		//Threshold (Grey Scale Image)


		// dilate + find contours


		//cv::dilate(imgThresh, imgThresh, structuringElement3x3);
		//cv::dilate(imgThresh, imgThresh, structuringElement3x3);
		cv::erode(imgThresh, imgThresh, structuringElement3x3);

		cv::dilate(imgThresh, imgThresh, structuringElement7x7);
		cv::erode(imgThresh, imgThresh, structuringElement3x3);

		cv::dilate(imgThresh, imgThresh, structuringElement7x7);
		cv::erode(imgThresh, imgThresh, structuringElement5x5);




		cv::erode(colorForeground, colorForeground, structuringElement3x3);

		cv::dilate(colorForeground, colorForeground, structuringElement7x7);
		cv::erode(colorForeground, colorForeground, structuringElement5x5);

		cv::dilate(colorForeground, colorForeground, structuringElement7x7);
		cv::erode(colorForeground, colorForeground, structuringElement5x5);

		//cv::dilate(colorForeground, colorForeground, structuringElement5x5);

		cv::imshow("ss", colorForeground);

		///////////////////////////////////////////////////////////////////////////////////direct get different between mask and image
		cv::Mat motion;
		cv::absdiff(mask, imgFrame2Copy, motion);
		cv::threshold(motion, motion, 60, 255, cv::THRESH_BINARY);
		cv::erode(motion, motion, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
		cv::Mat foregroundMask = cv::Mat::zeros(motion.rows, motion.cols, CV_8UC1);

		/*float threshold = 120.0f;
		float dist;

		for (int j = 0; j<motion.rows; ++j)
		for (int i = 0; i<motion.cols; ++i)
		{
		cv::Vec3b pix = motion.at<cv::Vec3b>(j, i);

		dist = (pix[0] * pix[0] + pix[1] * pix[1] + pix[2] * pix[2]);
		dist = sqrt(dist);

		if (dist>threshold)
		{
		foregroundMask.at<unsigned char>(j, i) = 255;
		}

		}*/




		//cv::imshow("ssssddd", motion);

		////////////////////////////////////////////////////////////////////////////////////////direct get different between mask and image

		////////////////////////////////////////////////////////////////////////////////////// remove shadow
		//cv::Mat grays;
		//cvtColor(imgFrame2Copy, grays, CV_BGR2GRAY);

		//double minGray, maxGray;
		//cv::minMaxLoc(grays, &minGray, &maxGray);

		//bool useRelative = true;
		//if (useRelative)
		//{
		//	// Relative clipping dark range to black
		//	double clipPercent = 10;
		//	minGray = cvRound(minGray * (1 + clipPercent / 100.0));
		//	//all below minGray+10% will become black
		//}
		//else
		//{
		//	//absolute clipping. Use a fixed lower bound for dark regions
		//	double minGrayWanted = 50;
		//	minGray = minGrayWanted;
		//	//all below 50 will become black
		//}

		//// current range
		//float inputRange = maxGray - minGray;
		//double alpha, beta;
		//alpha = 255.0 / inputRange; // alpha expands current range. MaxGray will be 255
		//beta = -minGray * alpha;    // beta shifts current range so that minGray will go to 0
		//cv::Mat hohoho;
		//imgFrame2Copy.convertTo(hohoho, -1, alpha, beta);
		//cv::imshow("shadow removel", hohoho);
		/////////////////////////////////////////////////////////////////////////////////////// remove shadow

		/////////////////////////////////////////////////////////////////////////////////////
		//cv::Mat sss;
		//bg_sub->operator()(imgFrame2Copy, sss, 0);
		//	cv::threshold(sss, sss, 100, 255, THRESH_BINARY);
		//cv::imshow("gfds", sss);

		/*	cv::Mat fg = *bgfg.fg(imgFrame2Copy);
		cv::imshow("amam", fg);
		*/


		/////////////////////////////////////////////////////////////////////////////////////


		cv::Mat imgThreshCopy = imgThresh.clone();


		imgThreshCopy.convertTo(imgThreshCopy, CV_8UC1);
		//std::cout << imgThreshCopy.type() << "\n";
		std::vector<std::vector<cv::Point> > contours;

		cv::findContours(imgThreshCopy, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);



		//drawAndShowContours(imgThresh.size(), contours, "imgContours");
		// dilate + find contours


		//convexHull
		std::vector<std::vector<cv::Point> > convexHulls(contours.size());

		for (unsigned int i = 0; i < contours.size(); i++) {
			cv::convexHull(contours[i], convexHulls[i]);
		}

		//drawAndShowContours(imgThresh.size(), convexHulls, "imgConvexHulls");
		//convexHull

		//filter convexHull
		for (auto &convexHull : convexHulls) {
			Blob possibleBlob(convexHull);

			if (possibleBlob.currentBoundingRect.area() > 550 &&
				possibleBlob.dblCurrentAspectRatio > 0.2 &&
				possibleBlob.dblCurrentAspectRatio < 4.0 &&
				possibleBlob.currentBoundingRect.width > 25 &&
				possibleBlob.currentBoundingRect.height > 25 &&
				possibleBlob.dblCurrentDiagonalSize > 20.0 &&
				(cv::contourArea(possibleBlob.currentContour) / (double)possibleBlob.currentBoundingRect.area()) > 0.50) {
				//cv::cvtColor(colorForeground, colorForeground, CV_BGR2GRAY);
				//std::cout << imgFrame1Copy.type() << "\n";

				possibleBlob.storeImage(imgFrame1Copy);

				//	std::cout << "hehe\n";
				currentFrameBlobs.push_back(possibleBlob);


			}
			/*		if (possibleBlob.currentBoundingRect.area() > 1000) {
			currentFrameBlobs.push_back(possibleBlob);
			}*/
		}

		//drawAndShowContours(imgThresh.size(), currentFrameBlobs, "imgCurrentFrameBlobs");
		//filter convexHull
	

		drawAndShowContours(imgThresh.size(), currentFrameBlobs, "imgCurrentFrameBlobs", imgFrame1Copy);

		//match blob
		if (blnFirstFrame == true) {
			for (auto &currentFrameBlob : currentFrameBlobs) {
				blobs.push_back(currentFrameBlob);
			}
			std::cout << "first time\n";
		}
		else {
			/*if (blobs.size() == 0) {
			for (auto &currentFrameBlob : currentFrameBlobs) {
			blobs.push_back(currentFrameBlob);
			}
			}
			else {*/
			//   std::cout << blobs.size() << "\n";
			matchCurrentFrameBlobsToExistingBlobs2(blobs, currentFrameBlobs);
			/*}*/
		}

		drawAndShowContours(imgThresh.size(), blobs, "imgBlobs", imgFrame1Copy);


		//match blob





		//cv::Mat image(imgThresh.size(), CV_8UC3, SCALAR_BLACK);

		//std::vector<std::vector<cv::Point> > contoursss;

		//for (auto &blob : blobs) {
		//	if (blob.blnStillBeingTracked == true) {
		//		contoursss.push_back(blob.currentContour);
		//	}
		//}

		//cv::drawContours(image, contoursss, -1, SCALAR_WHITE, -1);
		//cv::cvtColor(image, image, CV_BGR2GRAY);





		/*	cv::gpu::GpuMat abc(imgThresh);



		cv::gpu::GpuMat rawResult;*/







		//std::vector<cv::gpu::GpuMat> features_next;
		/*cv::gpu::GoodFeaturesToTrackDetector_GPU aa;
		aa.operator()(abc, rawResult);
		std::cout << rawResult.size() << " ";*/
		//aa.operator()(abc, rawResult);

		//abc.download(image);

		//cv::gpu::goodfeaturestotrackdetector_gpu(image, // the image 
		//	features_next,   // the output detected features
		//	10000,  // the maximum number of features 
		//	0.1,     // quality level
		//	10);

		//cv::Mat result(rawResult);

		//if (dontshow == true) {
		//	dontshow = false;
		//}
		//else {
		//	
		//}
		//	cv::Mat hoho(rawResult);
		//	
		//	//extraxt x and y channels
		//	cv::Mat xy[2]; //X,Y
		//	cv::split(hoho, xy);

		//	//calculate angle and magnitude
		//	cv::Mat magnitude, angle;
		//	cv::cartToPolar(xy[0], xy[1], magnitude, angle, true);

		//	//translate magnitude to range [0;1]
		//	double mag_max;
		//	cv::minMaxLoc(magnitude, 0, &mag_max);
		//	magnitude.convertTo(magnitude, -1, 1.0 / mag_max);

		//	//build hsv image
		//	cv::Mat _hsv[3], hsv;
		//	_hsv[0] = angle;
		//	_hsv[1] = cv::Mat::ones(angle.size(), CV_32F);
		//	_hsv[2] = magnitude;
		//	cv::merge(_hsv, 3, hsv);

		//	//convert to BGR and show
		//	cv::Mat bgr;//CV_32FC3 matrix
		//	cv::cvtColor(hsv, bgr, cv::COLOR_HSV2BGR);
		//	cv::imshow("optical flow", bgr);
		//}
		//

		//contours.clear();








		cv::Mat imgFrame2Copy2 = imgFrame2.clone();



		// Vehicle counting

		imgFrame2Copy = imgFrame2.clone();          // get another copy of frame 2 since we changed the previous frame 2 copy in the processing above

		drawBlobInfoOnImage(blobs, imgFrame2Copy);

		bool blnAtLeastOneBlobCrossedTheLine = checkIfBlobsCrossedTheLine(blobs, intHorizontalLinePosition2, carCount);

		if (blnAtLeastOneBlobCrossedTheLine == true) {
			cv::line(imgFrame2Copy, crossingLine[0], crossingLine[1], SCALAR_GREEN, 2);
			cv::line(imgFrame2Copy, crossingLine2[0], crossingLine2[1], SCALAR_GREEN, 2);
			cv::line(imgFrame2Copy, crossingLine3[0], crossingLine3[1], SCALAR_GREEN, 2);
			cv::line(imgFrame2Copy, crossingLine4[0], crossingLine4[1], SCALAR_GREEN, 2);
			cv::line(imgFrame2Copy, crossingLine5[0], crossingLine5[1], SCALAR_GREEN, 2);

			cv::line(imgFrame2Copy, crossingLine6[0], crossingLine6[1], SCALAR_GREEN, 2);
		}
		else {
			cv::line(imgFrame2Copy, crossingLine[0], crossingLine[1], SCALAR_RED, 2);
			cv::line(imgFrame2Copy, crossingLine2[0], crossingLine2[1], SCALAR_RED, 2);
			cv::line(imgFrame2Copy, crossingLine3[0], crossingLine3[1], SCALAR_RED, 2);
			cv::line(imgFrame2Copy, crossingLine4[0], crossingLine4[1], SCALAR_RED, 2);
			cv::line(imgFrame2Copy, crossingLine5[0], crossingLine5[1], SCALAR_RED, 2);

			cv::line(imgFrame2Copy, crossingLine6[0], crossingLine6[1], SCALAR_RED, 2);
		}




		drawCarCountOnImage(carCount, imgFrame2Copy);
		
		// Vehicle counting

		/*std::vector<cv::Point> region;
		region.push_back(cv::Point(35, 330));
		region.push_back(cv::Point(537, 384));
		region.push_back(cv::Point(547, 433));
		region.push_back(cv::Point(2, 370));

		const cv::Point *pts = (const cv::Point*) cv::Mat(region).data;
		int npts = cv::Mat(region).rows;

		polylines(imgFrame2Copy, &pts, &npts, 1, true, cv::Scalar(250, 0, 0), 2);


		std::vector<cv::Point> region2;
		region2.push_back(cv::Point(118, 295));
		region2.push_back(cv::Point(525, 337));
		region2.push_back(cv::Point(535, 372));
		region2.push_back(cv::Point(88, 324));

		const cv::Point *pts2 = (const cv::Point*) cv::Mat(region2).data;
		int npts2 = cv::Mat(region2).rows;

		polylines(imgFrame2Copy, &pts2, &npts2, 1, true, cv::Scalar(0, 250, 0), 2);





		std::vector<cv::Point> region3;
		region3.push_back(cv::Point(208, 251));
		region3.push_back(cv::Point(508, 279));
		region3.push_back(cv::Point(514, 306));
		region3.push_back(cv::Point(188, 272));


		const cv::Point *pts3 = (const cv::Point*) cv::Mat(region3).data;
		int npts3 = cv::Mat(region3).rows;

		polylines(imgFrame2Copy, &pts3, &npts3, 1, true, cv::Scalar(0, 0, 250), 2);




		std::vector<cv::Point> region4;
		region4.push_back(cv::Point(257, 230));
		region4.push_back(cv::Point(502, 254));
		region4.push_back(cv::Point(504, 273));
		region4.push_back(cv::Point(247, 246));


		const cv::Point *pts4 = (const cv::Point*) cv::Mat(region4).data;
		int npts4 = cv::Mat(region4).rows;

		polylines(imgFrame2Copy, &pts4, &npts4, 1, true, cv::Scalar(0, 250, 0), 2);

		std::vector<cv::Point> region5;
		region5.push_back(cv::Point(333, 208));
		region5.push_back(cv::Point(492, 219));
		region5.push_back(cv::Point(496, 233));
		region5.push_back(cv::Point(327, 221));


		const cv::Point *pts5 = (const cv::Point*) cv::Mat(region5).data;
		int npts5 = cv::Mat(region5).rows;

		polylines(imgFrame2Copy, &pts5, &npts5, 1, true, cv::Scalar(0, 250, 0), 2);*/





		// keypoints
		//detector.detect(imgFrame1Copy, keypoints_1);
		//cv::Mat img_keypoints_1;
		//drawKeypoints(imgFrame1Copy, keypoints_1, imgFrame2Copy);
		// keypoints




		//copy_image = imgFrame2.clone();
		//cv::cvtColor(copy_image, copy_image, CV_BGR2GRAY);



		//cv::goodFeaturesToTrack(imgThresh, // the image 
		//	features_next,   // the output detected features
		//	200,  // the maximum number of features 
		//	0.1,     // quality level
		//	10);
		//
		////std::cout << features_next.size() << "\n";

		//int r = 4;
		//for (int i = 0; i < features_next.size(); i++) {
		//	cv::circle(imgFrame2Copy, features_next[i], r, cv::Scalar(0, 0, 255), -1, 8, 0);
		//}


		//

		//
		//drawRegion(imgFrame2Copy.size(), features_next, imgFrame2Copy2);

		//if (firstTime) {

		//	cv::cvtColor(colorForeground, colorForeground, CV_BGR2GRAY);
		//	cv::gpu::GpuMat colourForegroundGPU(colorForeground);

		//	cv::gpu::FAST_GPU::FAST_GPU(15).operator() (colourForegroundGPU, colourForegroundGPU, keypoints_new);


		////	//detector.detect(colorForeground, keypoints_new);
		////	//std::cout << keypoints_1.size() << "\n";
		////	//cv::drawKeypoints(imgFrame2Copy, keypoints_1, imgFrame2Copy);
		//	newImg = colorForeground.clone();
		//	firstTime = false;
		//}
		//else {

		cv::cvtColor(colorForeground, colorForeground, CV_BGR2GRAY);

		//	keypoints_old = keypoints_new;
		//	keypoints_new.clear();
		//	oldimg = newimg.clone();
		//	newimg = colorforeground.clone();

		//	






		/////////////////////////////////////////////////////////////////remove temporary

		//cv::gpu::GpuMat colourForegroundGPU(colorForeground);
		//cv::gpu::FAST_GPU::FAST_GPU(15).operator() (colourForegroundGPU, colourForegroundGPU, keypoints_new);

		////	//detector.detect(colorForeground, keypoints_new);

		//cv::drawKeypoints(imgFrame2Copy, keypoints_new, imgFrame2Copy);

		//for (int i = 0; i < keypoints_new.size(); i++) {
		//	features_prev.push_back(keypoints_new[i].pt);
		//}


		//drawRegion(imgFrame2Copy.size(), features_prev, imgFrame2Copy2);
		//features_prev.clear();


		///////////////////////////////////////////////////////////////////remove temporary





		//
		//	cv::gpu::GpuMat newImgGPU(newImg);
		//	cv::gpu::GpuMat oldImgGPU(oldImg);
		//	cv::gpu::GpuMat newDesGPU, oldDesGPU;


		//	descriptorGPU.operator() (newImgGPU, newImgGPU, keypoints_new, newDesGPU);
		//	descriptorGPU.operator() (oldImgGPU, oldImgGPU, keypoints_old, oldDesGPU);

		//	
		//	if (!newDesGPU.empty())
		//		newDesGPU.download(newDes);
		//	if (!oldDesGPU.empty())
		//		oldDesGPU.download(oldDes);
		//	




		//	descriptor.compute(newImg, keypoints_new, newDes);
		//	descriptor.compute(oldImg, keypoints_old, oldDes);

		////	//std::cout <<"sdsd : " << newDes.size();
		////	//cv::gpu::BruteForceMatcher_GPU_base().match(newDesGPU, oldDesGPU, matches);
		//matcher.match(oldDes, newDes, matches);
		////	//std::cout << matches.size() << "\n";
		//	cv::Mat img_matches;
		//	cv::drawMatches(oldImg, keypoints_old, newImg, keypoints_new, matches, img_matches);

		//	cv::imshow("matcher", img_matches);

		//}
		double dur = CLOCK() - start;
	//	std::cout << "fps : " << avgfps() << "\n";
		double fps = avgfps();
		drawCarDensityOnImage(fps, imgFrame2Copy);

		cv::imshow("imgFrame2Copy", imgFrame2Copy);


		//cv::waitKey(0);                 // uncomment this line to go frame by frame for debugging

		// now we prepare for the next iteration

		currentFrameBlobs.clear();


		imgFrame1 = imgFrame2.clone();           // move frame 1 up to where frame 2 is

		if ((capVideo.get(CV_CAP_PROP_POS_FRAMES) + 1) < capVideo.get(CV_CAP_PROP_FRAME_COUNT)) {

			capVideo.read(imgFrame2);




			////BGS
			//cv::Mat img_mask2;
			//cv::Mat img_bkgmodel2;
			//bgs2->process(imgFrame2, img_mask2, img_bkgmodel2);
			////BGS

			//cv::Mat imgThresh2;

			////Threshold (Grey Scale Image)
			//cv::threshold(img_mask2, imgThresh2, 50, 255.0, CV_THRESH_BINARY);
			//cv::dilate(imgThresh2, imgThresh2, structuringElement5x5);
			//cv::dilate(imgThresh2, imgThresh2, structuringElement5x5);
			////Threshold (Grey Scale Image)

			//std::vector<uchar> status;
			//std::vector<float> err;

			//features_prev = features_next;

			//cv::goodFeaturesToTrack(imgThresh2, // the image 
			//	features_next,   // the output detected features
			//	10000,  // the maximum number of features 
			//	0.1,     // quality level
			//	10);

			////std::cout << features_prev.size() << " " << features_next.size() << "\n";

			////imshow("dsds", imgThresh2);
			//
			//if (features_next.size() != 0) {
			//	cv::calcOpticalFlowPyrLK(
			//		imgThresh, imgThresh2, // 2 consecutive images
			//		features_prev, // input point positions in first im
			//		features_next, // output point positions in the 2nd
			//		status,    // tracking success	
			//		err      // tracking error
			//		);
			//}


		}
		else {
			std::cout << "end of video\n";
			break;
		}

		blnFirstFrame = false;
		frameCount++;
		chCheckForEscKey = cv::waitKey(1);

		
	}





	if (chCheckForEscKey != 27) {               // if the user did not press esc (i.e. we reached the end of the video)
		cv::waitKey(0);                         // hold the windows open to allow the "end of video" message to show
	}
	// note that if the user did press esc, we don't need to hold the windows open, we can simply let the program end which will close the windows

	return(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawRegion(cv::Size imageSize, cv::vector<cv::Point2f> points, cv::Mat imageCopy) {

	//cv::Mat countingRegion(imageSize, CV_8UC3, SCALAR_BLACK);

	cv::Mat countingRegion = imageCopy.clone();

	//bool firstTime = true;
	int dividedWidth = imageSize.width / 5;
	int dividedHeight = imageSize.height / 5;
	int counter = 1;

	for (int i = 1; i < 10; i = i + 2) {
		int test = i - 1;
		drawingPointVertical[test].x = counter * dividedWidth;
		drawingPointVertical[test].y = 0;

		drawingPointVertical[i].x = counter * dividedWidth;
		drawingPointVertical[i].y = imageSize.height;

		drawingPointHorizontal[test].x = 0;
		drawingPointHorizontal[test].y = counter * dividedHeight;

		drawingPointHorizontal[i].x = imageSize.width;
		drawingPointHorizontal[i].y = counter * dividedHeight;

		counter++;
	}

	for (int i = 0; i < 9; i += 2) {
		cv::line(countingRegion, drawingPointVertical[i], drawingPointVertical[i + 1], SCALAR_GREEN, 2);
		cv::line(countingRegion, drawingPointHorizontal[i], drawingPointHorizontal[i + 1], SCALAR_GREEN, 2);
	}



	for (int i = 0; i < 25; i++) {
		countingfeatures[i] = 0;
	}




	int r = 4;
	for (int i = 0; i < points.size(); i++) {
		//
		if (points[i].x < dividedWidth) {
			if (points[i].y < dividedHeight) {
				countingfeatures[0]++;
			}
			else if (points[i].y < dividedHeight * 2) {
				countingfeatures[5]++;
			}
			else if (points[i].y < dividedHeight * 3) {
				countingfeatures[10]++;
			}
			else if (points[i].y < dividedHeight * 4) {
				countingfeatures[15]++;
			}
			else {
				countingfeatures[20]++;
			}
		}
		//
		else if (points[i].x < dividedWidth * 2) {
			if (points[i].y < dividedHeight) {
				countingfeatures[1]++;
			}
			else if (points[i].y < dividedHeight * 2) {
				countingfeatures[6]++;
			}
			else if (points[i].y < dividedHeight * 3) {
				countingfeatures[11]++;
			}
			else if (points[i].y < dividedHeight * 4) {
				countingfeatures[16]++;
			}
			else {
				countingfeatures[21]++;
			}
		}
		//
		else if (points[i].x < dividedWidth * 3) {
			if (points[i].y < dividedHeight) {
				countingfeatures[2]++;
			}
			else if (points[i].y < dividedHeight * 2) {
				countingfeatures[7]++;
			}
			else if (points[i].y < dividedHeight * 3) {
				countingfeatures[12]++;
			}
			else if (points[i].y < dividedHeight * 4) {
				countingfeatures[17]++;
			}
			else {
				countingfeatures[22]++;
			}
		}
		//
		else if (points[i].x < dividedWidth * 4) {
			if (points[i].y < dividedHeight) {
				countingfeatures[3]++;
			}
			else if (points[i].y < dividedHeight * 2) {
				countingfeatures[8]++;
			}
			else if (points[i].y < dividedHeight * 3) {
				countingfeatures[13]++;
			}
			else if (points[i].y < dividedHeight * 4) {
				countingfeatures[18]++;
			}
			else {
				countingfeatures[23]++;
			}
		}
		//
		else {
			if (points[i].y < dividedHeight) {
				countingfeatures[4]++;
			}
			else if (points[i].y < dividedHeight * 2) {
				countingfeatures[9]++;
			}
			else if (points[i].y < dividedHeight * 3) {
				countingfeatures[14]++;
			}
			else if (points[i].y < dividedHeight * 4) {
				countingfeatures[19]++;
			}
			else {
				countingfeatures[24]++;
			}
		}

		//cv::circle(countingRegion, points[i], r, cv::Scalar(0, 0, 255), -1, 8, 0);
	}


	/////////////////////////////////text
	int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
	double dblFontScale = (countingRegion.rows * countingRegion.cols) / 300000.0;
	int intFontThickness = (int)std::round(dblFontScale * 1.5);

	///////////////////////////////text

	int x = dividedWidth / 2;
	int y = dividedHeight / 2;

	cv::Point ptTextBottomLeftPosition;
	for (int i = 0; i < 25; i++) {
		ptTextBottomLeftPosition.x = x + dividedWidth * (i % 5);

		if (i < 5) {
			ptTextBottomLeftPosition.y = y;
		}
		else if (i < 10) {
			ptTextBottomLeftPosition.y = y + dividedHeight;
		}
		else if (i < 15) {
			ptTextBottomLeftPosition.y = y + dividedHeight * 2;
		}
		else if (i < 20) {
			ptTextBottomLeftPosition.y = y + dividedHeight * 3;
		}
		else {
			ptTextBottomLeftPosition.y = y + dividedHeight * 4;
		}


		if (countingfeatures[i] < 1)
			cv::putText(countingRegion, std::to_string(countingfeatures[i]), ptTextBottomLeftPosition, intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);
		else
			cv::putText(countingRegion, std::to_string(countingfeatures[i]), ptTextBottomLeftPosition, intFontFace, dblFontScale, SCALAR_RED, intFontThickness);
	}






	cv::imshow("sdsd", countingRegion);
}

void matchCurrentFrameBlobsToExistingBlobs2(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs) {



	for (auto &existingBlob : existingBlobs) {

		existingBlob.blnCurrentMatchFoundOrNewBlob = false;

		existingBlob.predictNextPosition();
	}




	for (int j = 0; j < currentFrameBlobs.size(); j++) {

		int intIndexOfLeastDistance = 0;
		double dblLeastDistance = 100000.0;

		for (unsigned int i = 0; i < existingBlobs.size(); i++) {

			if (existingBlobs[i].blnStillBeingTracked == true) {

				double dblDistance = distanceBetweenPoints(currentFrameBlobs[j].centerPositions.back(), existingBlobs[i].predictedNextPosition);

				if (dblDistance < dblLeastDistance) {
					dblLeastDistance = dblDistance;
					intIndexOfLeastDistance = i;
				}
			}
		}

		if (dblLeastDistance < currentFrameBlobs[j].dblCurrentDiagonalSize * 1.5) {
			addBlobToExistingBlobs(currentFrameBlobs[j], existingBlobs, intIndexOfLeastDistance);
		}
		else {
			addNewBlob(currentFrameBlobs[j], existingBlobs);
		}

	}



	for (int i = 0; i < existingBlobs.size(); i++) {

		if (existingBlobs[i].blnCurrentMatchFoundOrNewBlob == false) {
			existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch++;
		}

		if (existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch >= 10 || (existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch >= 5 && existingBlobs[i].exit == true)) {
			existingBlobs[i].blnStillBeingTracked = false;
		}

	}



}

void matchCurrentFrameBlobsToExistingBlobs(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs) {

	for (auto &existingBlob : existingBlobs) {

		existingBlob.blnCurrentMatchFoundOrNewBlob = false;

		existingBlob.predictNextPosition();
	}

	//for (auto &currentFrameBlob : currentFrameBlobs) {

	//	int intIndexOfLeastDistance = 0;
	//	double dblLeastDistance = 100000.0;

	//	for (unsigned int i = 0; i < existingBlobs.size(); i++) {

	//		if (existingBlobs[i].blnStillBeingTracked == true) {

	//			double dblDistance = distanceBetweenPoints(currentFrameBlob.centerPositions.back(), existingBlobs[i].predictedNextPosition);

	//			if (dblDistance < dblLeastDistance) {












	//				dblLeastDistance = dblDistance;
	//				intIndexOfLeastDistance = i;
	//			}
	//		}
	//		else {
	//			if (existingBlobs[i].isAdded == true) {
	//				carDensity = carDensity - existingBlobs[i].currentBoundingRect.area();
	//				existingBlobs[i].isAdded = false;
	//			}
	//		}
	//	}




	//	if (dblLeastDistance < currentFrameBlob.dblCurrentDiagonalSize * 0.5) {
	//		//addBlobToExistingBlobs(currentFrameBlob, existingBlobs, intIndexOfLeastDistance);
	//	}
	//	else {
	//		//addNewBlob(currentFrameBlob, existingBlobs);
	//	}

	//}
	//cv::gpu::BruteForceMatcher_GPU< cv::L2<float> > matcher;


	int highestMatch = 0;
	int highestMatchPosition = 0;

	int SecondhighestMatch = 0;
	int SecondhighestMatchPosition = 0;
	std::vector<int> currentJ;
	std::vector<int> currenK;

	int gg = existingBlobs.size();


	for (int j = 0; j < currentFrameBlobs.size(); j++) {

		//////////////////
		int intIndexOfLeastDistance = 0;
		double dblLeastDistance = 100000.0;
		/////////////////

		highestMatch = 0;
		highestMatchPosition = 0;
		SecondhighestMatch = 0;
		SecondhighestMatchPosition = 0;
		bool istrack = false;



		for (int k = 0; k < gg; k++) {
			
			if (existingBlobs[k].blnStillBeingTracked == true && existingBlobs[k].blnCurrentMatchFoundOrNewBlob == false) {

			
					/////////////////////////////////////////////////////////////////////////////////////
					double dblDistance = distanceBetweenPoints(currentFrameBlobs[j].centerPositions.back(), existingBlobs[k].predictedNextPosition);

					if (dblDistance < dblLeastDistance) {
						dblLeastDistance = dblDistance;
						intIndexOfLeastDistance = k;
					}
					else {
						if (existingBlobs[k].isAdded == true) {
							carDensity = carDensity - existingBlobs[k].currentBoundingRect.area();
							existingBlobs[k].isAdded = false;
						}
					}
					////////////////////////////////////////////////////////////////////////////////////

					

					/*std::vector< std::vector<cv::DMatch> > matches;
					cv::gpu::BruteForceMatcher_GPU< cv::HammingLUT > matching;
					matching.knnMatch(existingBlobs[k].getGpuDes(), currentFrameBlobs[j].getGpuDes(), matches, 1);

					if (matches.size() > highestMatch) {

						SecondhighestMatch = highestMatch;
						SecondhighestMatchPosition = highestMatchPosition;

						highestMatch = matches.size();
						highestMatchPosition = k;
					}
					else if (matches.size() > SecondhighestMatch) {
						SecondhighestMatch = matches.size();
						SecondhighestMatchPosition = k;
					}


					
					matches.clear();*/
				
			}
		}



		//std::cout << "k1 = " << highestMatchPosition << "k2 = " << intIndexOfLeastDistance << "\n";

		//if (highestMatch != 0 && dblLeastDistance < currentFrameBlobs[j].dblCurrentDiagonalSize * 0.3) {
		if (dblLeastDistance < currentFrameBlobs[j].dblCurrentDiagonalSize * 1.5) {
			//currentJ.push_back(j);
			//currenK.push_back(highestMatchPosition);
		

			

			/*std::vector<std::vector<cv::Point> > contourVec;
			contourVec.push_back(currentFrameBlobs[j].currentContour);
			cv::Mat te(zoneA.size(), CV_8UC3, SCALAR_BLACK);
			cv::drawContours(te, contourVec, -1, SCALAR_WHITE, -1);
			int countaa;

			cv::Mat tet;
			cv::Mat ttett;

			cv::bitwise_and(zoneA, te, tet);

			cv::cvtColor(tet, ttett, cv::COLOR_BGR2GRAY);

			countaa = cv::countNonZero(ttett);
			if (countaa > 0 && currentFrameBlobs[j].park == false) {
				std::cout << "car entering zoneA";
				currentFrameBlobs[j].park = true;
			}
			else {

				cv::bitwise_and(zoneB, te, tet);
				cv::cvtColor(tet, ttett, cv::COLOR_BGR2GRAY);
				countaa = cv::countNonZero(ttett);
				if (countaa > 0 && currentFrameBlobs[j].park == false) {
					std::cout << "car entering zoneB";
					currentFrameBlobs[j].park = true;
				}
				else {

					cv::bitwise_and(zoneC, te, tet);
					cv::cvtColor(tet, ttett, cv::COLOR_BGR2GRAY);
					countaa = cv::countNonZero(ttett);
					if (countaa > 0 && currentFrameBlobs[j].park == false) {
						std::cout << "car entering zoneC";
					
						currentFrameBlobs[j].park = true;
					}
					else {

						cv::bitwise_and(zoneD, te, tet);
						cv::cvtColor(tet, ttett, cv::COLOR_BGR2GRAY);
						countaa = cv::countNonZero(ttett);
						if (countaa > 0 && currentFrameBlobs[j].park == false) {
							 std::cout << "car entering zoneD";
							currentFrameBlobs[j].park = true;
						}
					}
				}
			}*/

			addBlobToExistingBlobs(currentFrameBlobs[j], existingBlobs, intIndexOfLeastDistance);




			//contourVec.clear();
		}

		else {
			addNewBlob(currentFrameBlobs[j], existingBlobs);
			
		}




	}

	
	currentJ.clear();
	currenK.clear();


	for (int z = 0; z < existingBlobs.size(); z++) {
		if (existingBlobs[z].blnCurrentMatchFoundOrNewBlob == false) {
			existingBlobs[z].intNumOfConsecutiveFramesWithoutAMatch++;
		}
		if (existingBlobs[z].intNumOfConsecutiveFramesWithoutAMatch >= 20 || (existingBlobs[z].intNumOfConsecutiveFramesWithoutAMatch >= 5 && existingBlobs[z].exit == true)) {
			existingBlobs[z].blnStillBeingTracked = false;
			//existingBlobs.erase(existingBlobs.begin() + z);
			//z--;
			
		}
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void addBlobToExistingBlobs(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex) {

	carDensity = carDensity - existingBlobs[intIndex].currentBoundingRect.area();
	carDensity = carDensity + currentFrameBlob.currentBoundingRect.area();

	currentFrameBlob.addornot = true;


	existingBlobs[intIndex].currentContour = currentFrameBlob.currentContour;
	existingBlobs[intIndex].currentBoundingRect = currentFrameBlob.currentBoundingRect;

	existingBlobs[intIndex].centerPositions.push_back(currentFrameBlob.centerPositions.back());

	existingBlobs[intIndex].dblCurrentDiagonalSize = currentFrameBlob.dblCurrentDiagonalSize;
	existingBlobs[intIndex].dblCurrentAspectRatio = currentFrameBlob.dblCurrentAspectRatio;

	existingBlobs[intIndex].blnStillBeingTracked = true;
	existingBlobs[intIndex].blnCurrentMatchFoundOrNewBlob = true;

	existingBlobs[intIndex].rawImage = currentFrameBlob.rawImage;
	existingBlobs[intIndex].maskImage = currentFrameBlob.maskImage;
	existingBlobs[intIndex].points = currentFrameBlob.points;
	existingBlobs[intIndex].keypoints_new = currentFrameBlob.keypoints_new;
	existingBlobs[intIndex].des = currentFrameBlob.des;
	existingBlobs[intIndex].desNoGpu = currentFrameBlob.desNoGpu;

	existingBlobs[intIndex].existInSceen++;
	

	if (existingBlobs[intIndex].existInSceen > 15 && existingBlobs[intIndex].unitID == 0) {
		existingBlobs[intIndex].unitID = unitObjCounter;
		//std::cout << "Number : " << existingBlobs[intIndex].unitID << "\n";
		unitObjCounter++;
	}

	





}

///////////////////////////////////////////////////////////////////////////////////////////////////
void addNewBlob(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs) {

	currentFrameBlob.blnCurrentMatchFoundOrNewBlob = true;
	carDensity = carDensity + currentFrameBlob.currentBoundingRect.area();
	existingBlobs.push_back(currentFrameBlob);

}

///////////////////////////////////////////////////////////////////////////////////////////////////
double distanceBetweenPoints(cv::Point point1, cv::Point point2) {

	int intX = abs(point1.x - point2.x);
	int intY = abs(point1.y - point2.y);

	return(sqrt(pow(intX, 2) + pow(intY, 2)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName) {
	cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

	cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);

	cv::imshow(strImageName, image);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawAndShowContours(cv::Size imageSize, std::vector<Blob> blobs, std::string strImageName, cv::Mat colourImage) {

	cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

	std::vector<std::vector<cv::Point> > contours;

	for (auto &blob : blobs) {
		if (blob.blnStillBeingTracked == true) {
			contours.push_back(blob.currentContour);
		}
	}

	cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);


	//cv::Mat colorForegrounds = cv::Mat::zeros(image.size(), image.type());
	//colourImage.copyTo(colorForegrounds, image);


	cv::imshow(strImageName, image);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool checkIfBlobsCrossedTheLine(std::vector<Blob> &blobs, int &intHorizontalLinePosition, int &carCount) {
	
	bool blnAtLeastOneBlobCrossedTheLine = false;

	//for (auto blob : blobs) {
	for(int i = 0; i < blobs.size(); i ++) {

		if (blobs[i].blnStillBeingTracked == true && blobs[i].centerPositions.size() >= 2) {
			int prevFrameIndex = (int)blobs[i].centerPositions.size() - 2;
			int currFrameIndex = (int)blobs[i].centerPositions.size() - 1;



			//////////////////////////////////////////////////////////////////////////////////////////////////////
			std::vector<std::vector<cv::Point> > contourVec;
			contourVec.push_back(blobs[i].currentContour);
			cv::Mat ctr(entrance1.size(), CV_8UC3, SCALAR_BLACK);
			cv::drawContours(ctr, contourVec, -1, SCALAR_WHITE, -1);


			int counter;
			cv::Mat bitwise;
			cv::Mat bwInt;

			cv::bitwise_and(entrance1, ctr, bitwise);
			cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
			counter = cv::countNonZero(bwInt);

			if (counter > 0) {
				if (blobs[i].centerPositions[prevFrameIndex].y < blobs[i].centerPositions[currFrameIndex].y && blobs[i].enter == false) {	
					printNumberofCar(1, true);
					blobs[i].enter = true;
				}
				else if (blobs[i].centerPositions[prevFrameIndex].y > blobs[i].centerPositions[currFrameIndex].y && blobs[i].enter == true && blobs[i].exit == false) {
					printNumberofCar(1, false);
					blobs[i].exit = true;
				}
			}
			else {

				cv::bitwise_and(entrance5, ctr, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counter = cv::countNonZero(bwInt);

				if (counter > 0) {
					if (blobs[i].centerPositions[prevFrameIndex].y < blobs[i].centerPositions[currFrameIndex].y && blobs[i].enter == false) {
						printNumberofCar(2, true);
						blobs[i].enter = true;
					}
					else if (blobs[i].centerPositions[prevFrameIndex].y > blobs[i].centerPositions[currFrameIndex].y && blobs[i].enter == true && blobs[i].exit == false) {
						printNumberofCar(2, false);
						blobs[i].exit = true;
					}
				}
				else {

					cv::bitwise_and(entrance6, ctr, bitwise);
					cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
					counter = cv::countNonZero(bwInt);

					if (counter > 0) {
						if (blobs[i].centerPositions[prevFrameIndex].x < blobs[i].centerPositions[currFrameIndex].x && blobs[i].enter == false) {
							printNumberofCar(3, true);
							blobs[i].enter = true;
						}
						else if (blobs[i].centerPositions[prevFrameIndex].x > blobs[i].centerPositions[currFrameIndex].x && blobs[i].enter == true && blobs[i].exit == false) {
							printNumberofCar(3, false);
							blobs[i].exit = true;
						}
					}
					else {

						cv::bitwise_and(entrance2, ctr, bitwise);
						cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
						counter = cv::countNonZero(bwInt);

						if (counter > 0) {

							if (blobs[i].centerPositions[prevFrameIndex].y > blobs[i].centerPositions[currFrameIndex].y && blobs[i].enter == false) {
								printNumberofCar(4, true);
								blobs[i].enter = true;
							}
							else if (blobs[i].centerPositions[prevFrameIndex].y < blobs[i].centerPositions[currFrameIndex].y && blobs[i].enter == true && blobs[i].exit == false) {
								printNumberofCar(4, false);
								blobs[i].exit = true;
							}

						}
						else {

							cv::bitwise_and(entrance3, ctr, bitwise);
							cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
							counter = cv::countNonZero(bwInt);

							if (counter > 0) {
								if (blobs[i].centerPositions[prevFrameIndex].x > blobs[i].centerPositions[currFrameIndex].x && blobs[i].enter == false) {
									printNumberofCar(5, true);
									blobs[i].enter = true;
								}
								else if (blobs[i].centerPositions[prevFrameIndex].x < blobs[i].centerPositions[currFrameIndex].x && blobs[i].enter == true && blobs[i].exit == false) {
									printNumberofCar(5, false);
									blobs[i].exit = true;
								}
							}

							else {

								cv::bitwise_and(entrance4, ctr, bitwise);
								cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
								counter = cv::countNonZero(bwInt);

								if (counter > 0) {
									if (blobs[i].centerPositions[prevFrameIndex].x < blobs[i].centerPositions[currFrameIndex].x && blobs[i].enter == false) {
										printNumberofCar(6, true);
										blobs[i].enter = true;
									}
									else if (blobs[i].centerPositions[prevFrameIndex].x > blobs[i].centerPositions[currFrameIndex].x && blobs[i].enter == true && blobs[i].exit == false) {
										printNumberofCar(6, false);
										blobs[i].exit = true;
									}
								}
							}
						}
					}

				}

			}



		



			//////////////////////////////////////////////////////////////////////////////////////////////////////////

			cv::bitwise_and(zoneA, ctr, bitwise);
			cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
			counter = cv::countNonZero(bwInt);


			if (counter > 0 && blobs[i].park == false) {
				if (blobs[i].parkLocation != 1) {
					blobs[i].parkLocation = 1;
					blobs[i].parkframe = 1;
				}
				else if (blobs[i].parkLocation == 1) {
					blobs[i].parkframe++;
				}
				

			}
			else {

				cv::bitwise_and(zoneB, ctr, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counter = cv::countNonZero(bwInt);

				if (counter > 0 && blobs[i].park == false) {
					if (blobs[i].parkLocation != 2) {
						blobs[i].parkLocation = 2;
						blobs[i].parkframe = 1;
					}
					else if (blobs[i].parkLocation == 2) {
						blobs[i].parkframe++;
					}

					
				}
				else {

					cv::bitwise_and(zoneC, ctr, bitwise);
					cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
					counter = cv::countNonZero(bwInt);

					if (counter > 0 && blobs[i].park == false) {
						
						if (blobs[i].parkLocation != 3) {
							blobs[i].parkLocation = 3;
							blobs[i].parkframe = 1;
						}
						else if (blobs[i].parkLocation == 3) {
							blobs[i].parkframe++;
						}

						
						
					}
					else {

						cv::bitwise_and(zoneD, ctr, bitwise);
						cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
						counter = cv::countNonZero(bwInt);

						if (counter > 0 && blobs[i].park == false) {

							if (blobs[i].parkLocation != 4) {
								blobs[i].parkLocation = 4;
								blobs[i].parkframe = 1;
							}
							else if (blobs[i].parkLocation == 4) {
								blobs[i].parkframe++;
							}


							
						}
					}
				}
			}

			if (blobs[i].parkframe > 20) {
				
				int a = blobs[i].parkLocation;

				if (a == 1 && blobs[i].park == false) {
					std::cout << "Park: Zone A\n";
					blobs[i].park = true;
				}
				else if (a == 2 && blobs[i].park == false) {
					std::cout << "Park: Zone B\n";
					blobs[i].park = true;
				}
				else if (a == 3 && blobs[i].park == false) {
					std::cout << "Park: Zone C\n";
					blobs[i].park = true;
				}
				else if (a == 4&& blobs[i].park == false) {
					std::cout << "Park: Zone D\n";
					blobs[i].park = true;
				}

			}




			///////////////////////////////////////////////////////////////////////////////////////////////////////////



			if (blobs[i].centerPositions[prevFrameIndex].y > intHorizontalLinePosition && blobs[i].centerPositions[currFrameIndex].y <= intHorizontalLinePosition) {
				carCount++;
				blnAtLeastOneBlobCrossedTheLine = true;
			}










		}

	}

	return blnAtLeastOneBlobCrossedTheLine;








}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy) {

	for (unsigned int i = 0; i < blobs.size(); i++) {

		if (blobs[i].blnStillBeingTracked == true) {
			cv::rectangle(imgFrame2Copy, blobs[i].currentBoundingRect, SCALAR_RED, 2);

			int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
			double dblFontScale = blobs[i].dblCurrentDiagonalSize / 60.0;
			int intFontThickness = (int)std::round(dblFontScale * 1.0);

			cv::putText(imgFrame2Copy, std::to_string(blobs[i].unitID), blobs[i].centerPositions.back(), intFontFace, dblFontScale, SCALAR_GREEN, 2);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawCarCountOnImage(int &carCount, cv::Mat &imgFrame2Copy) {

	int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
	double dblFontScale = (imgFrame2Copy.rows * imgFrame2Copy.cols) / 300000.0;
	int intFontThickness = (int)std::round(dblFontScale * 1.5);

	cv::Size textSize = cv::getTextSize(std::to_string(carCount), intFontFace, dblFontScale, intFontThickness, 0);

	cv::Point ptTextBottomLeftPosition;



	//cv::Point density;


	ptTextBottomLeftPosition.x = imgFrame2Copy.cols - 1 - (int)((double)textSize.width * 1.25);
	ptTextBottomLeftPosition.y = (int)((double)textSize.height * 1.25);

	//density.x = 50;
	//density.y = 50;

	cv::putText(imgFrame2Copy, std::to_string(carCount), ptTextBottomLeftPosition, intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);

	//cv::putText(imgFrame2Copy, std::to_string(carCount), density, intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);

}

void drawCarDensityOnImage(double &carDensity, cv::Mat &imgFrame2Copy) {

	int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
	double dblFontScale = (imgFrame2Copy.rows * imgFrame2Copy.cols) / 300000.0;
	int intFontThickness = (int)std::round(dblFontScale * 1.5);

	cv::Size textSize = cv::getTextSize(std::to_string(carDensity), intFontFace, dblFontScale, intFontThickness, 0);

	cv::Point density;

	density.x = 50;
	density.y = 50;

	cv::putText(imgFrame2Copy, std::to_string(carDensity), density, intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);

}

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if (flags == (cv::EVENT_FLAG_LBUTTON))
	{
		std::cout << "Left mouse button is clicked while pressing CTRL key - position (" << x << ", " << y << ")" << "\n";
	}
	else if (flags == (cv::EVENT_FLAG_RBUTTON + cv::EVENT_FLAG_SHIFTKEY))
	{
		std::cout << "Right mouse button is clicked while pressing SHIFT key - position (" << x << ", " << y << ")" << "\n";
	}
	else if (event == cv::EVENT_MOUSEMOVE && flags == cv::EVENT_FLAG_ALTKEY)
	{
		std::cout << "Mouse is moved over the window while pressing ALT key - position (" << x << ", " << y << ")" << "\n";
	}
}








void printNumberofCar(int entrance, bool entExt) {
	std::cout << "***************************************************\n";
	if (entExt == true) {
		std::cout << "Enter : " << entrance << "\n";
		carNumberCounter++;
		std::cout << "Number of car in Car Park : " << carNumberCounter << "\n";
	}
	else {
		std::cout << "Exit : " << entrance << "\n";
		carNumberCounter--;
		std::cout << "Number of car in Car Park : " << carNumberCounter << "\n";
	}
	std::cout << "***************************************************\n";
}