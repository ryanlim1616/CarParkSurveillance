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
#include<cmath>

#include<stdio.h>
#include<iostream>
#include<conio.h>           // it may be necessary to change or remove this line if not using Windows

#include "Blob.h"
#include "AdaptiveBackgroundLearning.h"
#include "FramedifferenceBGS.h"
#include "ParkingLot.h"


//clarence added the following:
#include <string>
#include "sqlite3.h" 
#include "CarParkTrackExporter.h"
#include "Logger.h"
#include "SQLiteManager.h"
#include "StringHelpers.h"
#include <sstream>
#include "GlobalClass.h"
#include <regex>
#include "dirent.h"
#include <vector>

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
cv::Mat drawAndShowContoursProccess(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName);
void drawAndShowContours(cv::Size imageSize, std::vector<Blob> blobs, std::string strImageName, cv::Mat colourImage);

//clarence changed the following function to include write to db:
//bool checkIfBlobsCrossedTheLine(std::vector<Blob> &blobs, int &intHorizontalLinePosition, int &carCount);
bool checkIfBlobsCrossedTheLine(std::vector<Blob> &blobs, int &intHorizontalLinePosition, int &carCount, CarParkTrackExporter &openDB, int &frameCount, int &vidLength);

//clarence changed the following function to include write to db:
//void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy);
void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy, CarParkTrackExporter &openDB, int &frameCount, int &vidLength);
//void vehicleIO(std::vector<Blob> &blobs, CarParkTrackExporter &openDB);

void drawCarCountOnImage(int &carCount, cv::Mat &imgFrame2Copy);
void drawCarDensityOnImage(double &carCount, cv::Mat &imgFrame2Copy);
void drawRegion(cv::Size imageSize, cv::vector<cv::Point2f> points, cv::Mat imageCopy);
void matchCurrentFrameBlobsToExistingBlobs2(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs);
void CallBackFunc(int event, int x, int y, int flags, void* userdata);
void printNumberofCar(int entrance, bool entExt);
void addBlobToGroupState(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex, int &intIndex2);
void splitBlob(Blob &currentFrameBlob1, Blob &currentFrameBlob2, std::vector<Blob> &existingBlobs, int &intIndex);
void checkLeaveWithNoEnter();
void addBack(std::vector<Blob> &blobs);
void addBlobToExistingBlobsMissMatch(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex);

int carDensity = 0;

int carNumberCounter = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////

int minHessian = 10;

std::vector<std::vector<cv::Point> > parkContours;

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
cv::Mat zoneE;


cv::Mat entrance1;
cv::Mat entrance2;
cv::Mat entrance3;
cv::Mat entrance4;
cv::Mat entrance5;
cv::Mat entrance6;

cv::Mat carParkZone;

cv::Mat parkImg;
cv::Mat minusMask;

cv::Mat allParking;
cv::Mat allParking2;
cv::Mat dangerZone;
cv::Mat nonTrackZone;
cv::Mat nonTrackZone2;

cv::Mat firstAll;

std::vector<Blob> missMatchBlob;

std::vector<Blob> nTrackzone1;
std::vector<Blob> nTrackzone2;


int unitObjCounter = 1;

std::vector < ParkingLot > zoneAlot;
std::vector < ParkingLot > zoneBlot;
std::vector < ParkingLot > zoneClot;
std::vector < ParkingLot > zoneDlot;
std::vector < ParkingLot > zoneElot;



//IBGS *bgs2;

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

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	int i;
	for (i = 0; i<argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

GlobalClass *GlobalClass::s_instance = 0;

//recursive function: https://github.com/paulrehkugler/ExtensionSearch
std::vector<std::string> results;				// holds search results
												//std::vector<std::string> results_FN;				// file name only

												// recursive search algorithm
void search(std::string curr_directory, std::string extension) {
	DIR* dir_point = opendir(curr_directory.c_str());
	dirent* entry = readdir(dir_point);
	while (entry) {									// if !entry then end of directory
		if (entry->d_type == DT_DIR) {				// if entry is a directory
			std::string fname = entry->d_name;
			if (fname != "." && fname != "..")
				search(entry->d_name, extension);	// search through it
		}
		else if (entry->d_type == DT_REG) {		// if entry is a regular file
			std::string fname = entry->d_name;	// filename
												// if filename's last characters are extension
			if (fname.find(extension, (fname.length() - extension.length())) != std::string::npos)
				results.push_back(curr_directory + fname);		// add path + filename to results vector
																//results_FN.push_back(fname);					// add filename to results vector
		}
		entry = readdir(dir_point);
	}
	return;
}


int main(void) {

	//initialize DB
	CarParkTrackExporter openDB;
	openDB.run();

	for (int i = 0; i < 25; i++) {
		countingfeatures.push_back(0);
	}


	IBGS *bgs;
	bgs = new AdaptiveBackgroundLearning;

	IBGS *bgs2;
	bgs2 = new FrameDifferenceBGS;


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
	zoneE = cv::imread("zoneEmask.png");


	entrance1 = cv::imread("e1.png");
	entrance2 = cv::imread("e2.png");
	entrance3 = cv::imread("e3.png");
	entrance4 = cv::imread("e4.png");
	entrance5 = cv::imread("e5.png");
	entrance6 = cv::imread("e6.png");

	carParkZone = cv::imread("carParkZone.png");

	firstAll = cv::imread("firstAll.png");

	dangerZone = cv::imread("DangerZone.png");
	nonTrackZone = cv::imread("nonTrackParkingZone.png");
	nonTrackZone2 = cv::imread("nonTrackParkingZone2.png");

	minusMask = cv::imread("minusmask.png");
	cv::transform(minusMask, minusMask, cv::Matx13f(1, 1, 1));






	cv::Mat ROImask;
	ROImask = cv::imread("ROImask.png");

	/////////////////////////////////////////////////////////////////// Load parking lot
	for (int i = 0; i < 11; i++) {
		std::string tempName = "zoneA" + std::to_string(i + 1) + ".png";
		cv::Mat temp = cv::imread(tempName);

		cv::Mat tempImages;
		cv::cvtColor(temp, tempImages, cv::COLOR_BGR2GRAY);
		int counter = cv::countNonZero(tempImages);

		ParkingLot newLot(1, i + 1, false, temp, counter);
		std::cout << counter << " ";
		zoneAlot.push_back(newLot);
	}
	std::cout << "\n";
	for (int i = 0; i < 10; i++) {
		std::string tempName = "zoneB" + std::to_string(i + 1) + ".png";
		cv::Mat temp = cv::imread(tempName);

		cv::Mat tempImages;
		cv::cvtColor(temp, tempImages, cv::COLOR_BGR2GRAY);
		int counter = cv::countNonZero(tempImages);

		ParkingLot newLot(2, i + 1, false, temp, counter);
		std::cout << counter << " ";
		zoneBlot.push_back(newLot);
	}
	std::cout << "\n";
	for (int i = 0; i < 9; i++) {
		std::string tempName = "zoneC" + std::to_string(i + 1) + ".png";
		cv::Mat temp = cv::imread(tempName);

		cv::Mat tempImages;
		cv::cvtColor(temp, tempImages, cv::COLOR_BGR2GRAY);
		int counter = cv::countNonZero(tempImages);

		ParkingLot newLot(3, i + 1, false, temp, counter);
		std::cout << counter << " ";
		zoneClot.push_back(newLot);
	}
	std::cout << "\n";
	for (int i = 0; i < 8; i++) {
		std::string tempName = "zoneD" + std::to_string(i + 1) + ".png";
		cv::Mat temp = cv::imread(tempName);

		cv::Mat tempImages;
		cv::cvtColor(temp, tempImages, cv::COLOR_BGR2GRAY);
		int counter = cv::countNonZero(tempImages);

		ParkingLot newLot(4, i + 1, false, temp, counter);
		std::cout << counter << " ";
		zoneDlot.push_back(newLot);
	}
	std::cout << "\n";
	for (int i = 0; i < 6; i++) {
		std::string tempName = "zoneE" + std::to_string(i + 1) + ".png";
		cv::Mat temp = cv::imread(tempName);

		cv::Mat tempImages;
		cv::cvtColor(temp, tempImages, cv::COLOR_BGR2GRAY);
		int counter = cv::countNonZero(tempImages);

		ParkingLot newLot(5, i + 1, false, temp, counter);
		std::cout << counter << " ";
		zoneElot.push_back(newLot);
	}
	std::cout << "\n";
	////////////////////////////////////////////////////// done 


	//get list of files in directory
	DIR *pd = NULL;


	std::string cinDate;
	std::string myRoot = "D:\\Videos Database\\Carpark Data\\";

	std::cout << "Please enter processing date (CCYYMMDD): ";
	std::cin >> cinDate;

	//check if root directory is correct
	std::cout << "Kindly confirm your video database root directory: \nie: " << myRoot << "CCYYMMDD\\c2 (Y/n)";
	//bbcc!!!!: else update 
	//std::cout << "TO BE IMPLEMENTED!\n";


	std::string myRoot2 = myRoot + cinDate + "\\c2\\";

	pd = opendir(myRoot2.c_str());
	if (pd == NULL) {
		//return error code 
		std::cout << "Error(" << errno << ") opening " << myRoot << std::endl;
		std::cout << "ERROR IN LOCATING THE ROOT DIRECTORY FOR VIDEOS!\n";
		exit(1);

	}

	std::string extension = ".mp4";
	search(myRoot2, extension);

	bool first_video = true;
	bool firstTime = true;

	//for debugging purpose
	bool debug_on = false;

	// output results
	if (results.size()) {
		std::cout << results.size() << " files were found:" << std::endl;
		for (unsigned int i = 0; i < results.size(); ++i)	// used unsigned to appease compiler warnings
		{

			//UPDATE i to change video, eg:
			//i = 2;

			//performing loop over all 100 videos to keep the obj_ID
			//obtain the time difference between 2 videos as well.
			std::cout << i + 1 << ": " << results[i] << std::endl;
			int vidLength = stoi(results[i + 1].substr(53, 4)) - stoi(results[i].substr(53, 4));

			//BBCC! temp use 2nd video to start and try to reproduce error
			GlobalClass::instance()->set_InputFileName(results[i+1].c_str());
			std::string InputFile = GlobalClass::instance()->get_InputFileName();

			capVideo.open(InputFile.c_str());
			std::cout << "Processing file: " << InputFile.c_str() << std::endl;


			//commented out
			//capVideo.open("20170228_084200.mp4");

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



			int frameCount = 2;




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

				if (debug_on) std::cout << "Did it break here?" << std::endl;


				//BBCC!! clarence left this here to see if it will affect
				//if (blnFirstFrame == true && first_video == true) {
				//	
				//	

				//	cv::Mat ROImaskParking = cv::Mat::zeros(firstAll.size(), firstAll.type());


				//	/*imgFrame1Copy.copyTo(ROImaskZoneA, firstzoneA);
				//	imgFrame1Copy.copyTo(ROImaskZoneB, firstzoneB);
				//	imgFrame1Copy.copyTo(ROImaskZoneC, firstzoneC);
				//	imgFrame1Copy.copyTo(ROImaskZoneD, firstzoneD);*/

				//	imgFrame1Copy.copyTo(ROImaskParking, firstAll);

				//	/*	cv::imshow("a", ROImaskZoneA);
				//		cv::imshow("b", ROImaskZoneB);
				//		cv::imshow("c", ROImaskZoneC);
				//		cv::imshow("d", ROImaskZoneD);*/



				//		//cv::cvtColor(ROImaskParking, ROImaskParking, CV_RGB2GRAY);
				//		//ROImaskParking = ROImaskParking + cv::Scalar(50, 50, 50);
				//		//cv::threshold(ROImaskParking, ROImaskParking, 65, 255, CV_THRESH_BINARY);
				//	cv::imshow("a", ROImaskParking);


				//	///////////////////////////////////////////////////// Get features point of first frame
				//	cv::FastFeatureDetector detector(5);


				//	std::vector<cv::KeyPoint> keypoints_1;

				//	detector.detect(ROImaskParking, keypoints_1);
				//	std::cout << keypoints_1.size() << " pouint1 \n";


				//	for (int h = 0; h < keypoints_1.size(); h++) {
				//		cv::Point pp = keypoints_1[h].pt;
				//		cv::circle(features, pp, 2.5, cv::Scalar(255, 255, 255), CV_FILLED, 8, 0);
				//	}

				//	keypoints_1.clear();
				//	/*detector.detect(ROImaskZoneB, keypoints_1);
				//	std::cout << keypoints_1.size() << " pouint2 \n";

				//	for (int h = 0; h < keypoints_1.size(); h++) {
				//		cv::Point pp = keypoints_1[h].pt;
				//		cv::circle(features, pp, 2.5, cv::Scalar(255, 255, 255), CV_FILLED, 8, 0);
				//	}
				//	keypoints_1.clear();
				//	detector.detect(ROImaskZoneC, keypoints_1);
				//	std::cout << keypoints_1.size() << " pouint3 \n";
				//	for (int h = 0; h < keypoints_1.size(); h++) {
				//		cv::Point pp = keypoints_1[h].pt;
				//		cv::circle(features, pp, 2.5, cv::Scalar(255, 255, 255), CV_FILLED, 8, 0);
				//	}
				//	keypoints_1.clear();
				//	detector.detect(ROImaskZoneD, keypoints_1);
				//	std::cout << keypoints_1.size() << " pouint4 \n";
				//	for (int h = 0; h < keypoints_1.size(); h++) {
				//		cv::Point pp = keypoints_1[h].pt;
				//		cv::circle(features, pp, 2.5, cv::Scalar(255, 255, 255), CV_FILLED, 8, 0);
				//	}*/
				//	cv::imshow("sds", features);



				//	////////////////////////// Get parked lot
				//	int counter;
				//	cv::Mat bitwise;
				//	cv::Mat bwInt;
				//	for (int i = 0; i < zoneAlot.size(); i++) {
				//		cv::bitwise_and(zoneAlot[i].image, features, bitwise);
				//		cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				//		counter = cv::countNonZero(bwInt);
				//		zoneAlot[i].featurePoint = counter;
				//		std::cout << counter << " ";
				//	}
				//	std::cout << "\n";

				//	for (int i = 0; i < zoneBlot.size(); i++) {
				//		cv::bitwise_and(zoneBlot[i].image, features, bitwise);
				//		cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				//		counter = cv::countNonZero(bwInt);
				//		zoneBlot[i].featurePoint = counter;
				//		std::cout << counter << " ";
				//	}
				//	std::cout << "\n";
				//	for (int i = 0; i < zoneClot.size(); i++) {
				//		cv::bitwise_and(zoneClot[i].image, features, bitwise);
				//		cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				//		counter = cv::countNonZero(bwInt);
				//		zoneClot[i].featurePoint = counter;
				//		std::cout << counter << " ";
				//	}
				//	std::cout << "\n";
				//	for (int i = 0; i < zoneDlot.size(); i++) {
				//		cv::bitwise_and(zoneDlot[i].image, features, bitwise);
				//		cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				//		counter = cv::countNonZero(bwInt);
				//		zoneDlot[i].featurePoint = counter;
				//		std::cout << counter << " ";
				//	}
				//	std::cout << "\n";

				//	for (int i = 0; i < zoneElot.size(); i++) {
				//		cv::bitwise_and(zoneElot[i].image, features, bitwise);
				//		cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				//		counter = cv::countNonZero(bwInt);
				//		zoneElot[i].featurePoint = counter;
				//		std::cout << counter << " ";
				//	}
				//	std::cout << "\n";


				//	////////////////////////// Done





				//	//first_video = false;


				//}

				///////////////////////////////////////////////////// Done



				//BGS
				cv::Mat img_mask;
				cv::Mat img_bkgmodel;
				bgs->process(imgFrame1Copy, img_mask, img_bkgmodel);
				//BGS

				//BGS2
				cv::Mat img_mask2;
				cv::Mat img_bkgmodel2;
				bgs2->process(imgFrame1Copy, img_mask2, img_bkgmodel2);

				if (debug_on)std::cout << "Did it break here? 2.3" << std::endl;


				cv::imshow("original mask", img_mask);

				if (unitObjCounter % 5 == 0) {
					//bgs2->updatemask();
				}

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








				//Threshold (Grey Scale Image)
				cv::threshold(img_mask, imgThresh, 30, 255.0, CV_THRESH_BINARY);
				cv::imshow("imgThresh", imgThresh);
				//Threshold (Grey Scale Image)


				// dilate + find contours



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


				cv::imshow("ss", colorForeground);

				///////////////////////////////////////////////////////////////////////////////////direct get different between mask and image


				cv::Mat imgThreshCopy = imgThresh.clone();
				imgThreshCopy.convertTo(imgThreshCopy, CV_8UC1);


				///////////////////////////// process contours
				std::vector<std::vector<cv::Point> > contours;

				cv::findContours(imgThreshCopy, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);


				/////////////////////////// process contours




				//convexHull
				std::vector<std::vector<cv::Point> > convexHulls(contours.size());

				for (unsigned int i = 0; i < contours.size(); i++) {
					cv::convexHull(contours[i], convexHulls[i]);
				}


				//convexHull

				//filter convexHull
				for (auto &convexHull : convexHulls) {
					Blob possibleBlob(convexHull);

					if (possibleBlob.currentBoundingRect.area() > 650 &&
						possibleBlob.dblCurrentAspectRatio > 0.2 &&
						possibleBlob.dblCurrentAspectRatio < 4.0 &&
						possibleBlob.currentBoundingRect.width > 25 &&
						possibleBlob.currentBoundingRect.height > 25 &&
						possibleBlob.dblCurrentDiagonalSize > 45.0 && possibleBlob.dblCurrentDiagonalSize < 200.0 &&
						(cv::contourArea(possibleBlob.currentContour) / (double)possibleBlob.currentBoundingRect.area()) > 0.50) {
						//cv::cvtColor(colorForeground, colorForeground, CV_BGR2GRAY);


						possibleBlob.storeImage(imgFrame1Copy);

						currentFrameBlobs.push_back(possibleBlob);


					}

				}


				//filter convexHull


				drawAndShowContours(imgThresh.size(), currentFrameBlobs, "imgCurrentFrameBlobs", imgFrame1Copy);



				//match blob
				if (blnFirstFrame == true && first_video == true) {
					first_video = false;
					for (auto &currentFrameBlob : currentFrameBlobs) {
						blobs.push_back(currentFrameBlob);
					}
					//std::cout << "first time\n";
				}
				else {
					if (blobs.size() == 0) {
						for (auto &currentFrameBlob : currentFrameBlobs) {
							blobs.push_back(currentFrameBlob);
						}
					}
					else {
						if (currentFrameBlobs.size() == 0) {
							bgs2->updatemask();

						}

						matchCurrentFrameBlobsToExistingBlobs2(blobs, currentFrameBlobs);

					}
				}






				drawAndShowContours(imgThresh.size(), blobs, "imgBlobs", imgFrame1Copy);


				//match blob

				cv::Mat imgFrame2Copy2 = imgFrame2.clone();


				// Vehicle counting

				imgFrame2Copy = imgFrame2.clone();          // get another copy of frame 2 since we changed the previous frame 2 copy in the processing above



				//write blob info into DB as well when tracked.
				//drawBlobInfoOnImage(blobs, imgFrame2Copy);
				drawBlobInfoOnImage(blobs, imgFrame2Copy, openDB, frameCount, vidLength);




				//bool blnAtLeastOneBlobCrossedTheLine = checkIfBlobsCrossedTheLine(blobs, intHorizontalLinePosition2, carCount);
				bool blnAtLeastOneBlobCrossedTheLine = checkIfBlobsCrossedTheLine(blobs, intHorizontalLinePosition2, carCount, openDB, frameCount, vidLength);


				addBack(blobs);

				cv::line(imgFrame2Copy, crossingLine[0], crossingLine[1], SCALAR_RED, 2);
				cv::line(imgFrame2Copy, crossingLine2[0], crossingLine2[1], SCALAR_RED, 2);
				cv::line(imgFrame2Copy, crossingLine3[0], crossingLine3[1], SCALAR_RED, 2);
				cv::line(imgFrame2Copy, crossingLine4[0], crossingLine4[1], SCALAR_RED, 2);
				cv::line(imgFrame2Copy, crossingLine5[0], crossingLine5[1], SCALAR_RED, 2);
				cv::line(imgFrame2Copy, crossingLine6[0], crossingLine6[1], SCALAR_RED, 2);





				drawCarCountOnImage(carCount, imgFrame2Copy);




				cv::cvtColor(colorForeground, colorForeground, CV_BGR2GRAY);




				double dur = CLOCK() - start;

				double fps = avgfps();
				drawCarDensityOnImage(fps, imgFrame2Copy);



				//draw position of video
				cv::Point Bar1, Bar2, VidPos;
				Bar1.x = 56;
				Bar1.y = 10;
				Bar2.x = 164;
				Bar2.y = 10;

				cv::line(imgFrame2Copy, Bar1, Bar2, SCALAR_BLACK, 5);

				VidPos.y = 10;
				VidPos.x = 60 + ((capVideo.get(CV_CAP_PROP_POS_FRAMES) / capVideo.get(CV_CAP_PROP_FRAME_COUNT)) * 100);

				cv::line(imgFrame2Copy, Bar1, VidPos, SCALAR_GREEN, 2);

				//end





				cv::imshow("imgFrame2Copy", imgFrame2Copy);


				//cv::waitKey(0);                 // uncomment this line to go frame by frame for debugging

				// now we prepare for the next iteration

				currentFrameBlobs.clear();


				imgFrame1 = imgFrame2.clone();           // move frame 1 up to where frame 2 is

														 //cv::Mat temp_frame;
														 //capVideo >> temp_frame;
														 //if (temp_frame.empty())
														 //{
														 //	std::cout << "EOF: Processing next video\n";
														 //	chCheckForEscKey = 27;
														 //	break;
														 //}
														 //else
														 //{
														 //	capVideo.read(imgFrame2);
														 //	if (imgFrame2.empty())
														 //	{
														 //		std::cout << "imgFrame2.empty() - breaking from loop\n";
														 //		chCheckForEscKey = 27;
														 //		break;
														 //	}

														 //	//std::cout << "next frame:" << capVideo.get(CV_CAP_PROP_POS_FRAMES) << "/" << capVideo.get(CV_CAP_PROP_FRAME_COUNT) << std::endl;
														 //}


														 //clarence commented out, using the above method instead
				if ((capVideo.get(CV_CAP_PROP_POS_FRAMES) + 1) < (capVideo.get(CV_CAP_PROP_FRAME_COUNT))) {

					capVideo.read(imgFrame2);
					if (imgFrame2.empty())
					{
						std::cout << "imgFrame2.empty() - breaking from loop\n";
						chCheckForEscKey = 27;
						break;
					}
				}
				else {
					std::cout << "end of video\n";
					std::cout << "EOF: Processing next video\n";
					chCheckForEscKey = 27;
					break;
				}

				if (blnFirstFrame == true)
				{
					GlobalClass::instance()->set_TotalFrames(capVideo.get(CV_CAP_PROP_FRAME_COUNT));
					GlobalClass::instance()->get_TotalFrames();
				}

				blnFirstFrame = false;
				frameCount++;
				chCheckForEscKey = cv::waitKey(1);


			}





			if (chCheckForEscKey != 27) {               // if the user did not press esc (i.e. we reached the end of the video)
				cv::waitKey(0);                         // hold the windows open to allow the "end of video" message to show
			}
			// note that if the user did press esc, we don't need to hold the windows open, we can simply let the program end which will close the windows
		}
	}
	else
	{
		std::cout << "No files ending in '" << extension << "' were found." << std::endl;
		std::cin.get();
	}
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



	std::vector<Blob> tempBlob;
	for (int g = 0; g < currentFrameBlobs.size(); g++) {
		tempBlob.push_back(currentFrameBlobs[g]);
	}


	for (int i = 0; i < existingBlobs.size(); i++) {
		existingBlobs[i].blnCurrentMatchFoundOrNewBlob = false;
		if (existingBlobs[i].park == true)
			existingBlobs[i].predictNextPositionPark();
		else
			existingBlobs[i].predictNextPosition();
	}

	for (int j = 0; j < currentFrameBlobs.size(); j++) {

		int intIndexOfLeastDistance = 0;
		double dblLeastDistance = 100000.0;

		int intIndexOfLeastDistance2 = 0;
		double dblLeastDistance2 = 100000.0;
		for (int i = 0; i < existingBlobs.size(); i++) {

			if (existingBlobs[i].blnStillBeingTracked == true) {

				double dblDistance = distanceBetweenPoints(currentFrameBlobs[j].centerPositions.back(), existingBlobs[i].predictedNextPosition);

				if (dblDistance < dblLeastDistance) {
					dblLeastDistance2 = dblLeastDistance;
					intIndexOfLeastDistance2 = intIndexOfLeastDistance;

					dblLeastDistance = dblDistance;
					intIndexOfLeastDistance = i;
				}
				else if (dblDistance < dblLeastDistance2) {
					dblLeastDistance2 = dblDistance;
					intIndexOfLeastDistance2 = i;
				}
			}
		}

		bool matchParked = false;


		if (existingBlobs[intIndexOfLeastDistance].park == true && dblLeastDistance < currentFrameBlobs[j].dblCurrentDiagonalSize * 0.1) {
			//	std::cout << existingBlobs[intIndexOfLeastDistance].unitID << " : Testing\n";
			cv::Mat bitwise;
			cv::Mat bwInt;
			int counterww = 0;
			int defaultCount = 0;
			//int counterww2 = 0;
			//int counterww3 = 0;

			std::vector<std::vector<cv::Point> > contourVec4;
			contourVec4.push_back(currentFrameBlobs[j].currentContour);
			cv::Mat ctr4(entrance1.size(), CV_8UC3, SCALAR_BLACK);
			cv::drawContours(ctr4, contourVec4, -1, SCALAR_WHITE, -1);
			contourVec4.clear();

			if (existingBlobs[intIndexOfLeastDistance].parkLocation == 1) {
				cv::bitwise_and(ctr4, zoneAlot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].image, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counterww = cv::countNonZero(bwInt);
				defaultCount = zoneAlot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].countNZero;


			}
			else if (existingBlobs[intIndexOfLeastDistance].parkLocation == 2) {
				cv::bitwise_and(ctr4, zoneBlot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].image, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counterww = cv::countNonZero(bwInt);
				defaultCount = zoneBlot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].countNZero;


			}
			else if (existingBlobs[intIndexOfLeastDistance].parkLocation == 3) {
				cv::bitwise_and(ctr4, zoneClot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].image, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counterww = cv::countNonZero(bwInt);
				defaultCount = zoneClot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].countNZero;

			}
			else if (existingBlobs[intIndexOfLeastDistance].parkLocation == 4) {
				cv::bitwise_and(ctr4, zoneDlot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].image, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counterww = cv::countNonZero(bwInt);
				defaultCount = zoneDlot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].countNZero;


			}
			else if (existingBlobs[intIndexOfLeastDistance].parkLocation == 5) {
				cv::bitwise_and(ctr4, zoneElot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].image, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counterww = cv::countNonZero(bwInt);
				defaultCount = zoneElot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].countNZero;

			}

			else if (existingBlobs[intIndexOfLeastDistance].parkLocation == 6) {
				cv::bitwise_and(ctr4, dangerZone, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counterww = cv::countNonZero(bwInt);
			}

			else if (existingBlobs[intIndexOfLeastDistance].parkLocation == 7) {
				cv::bitwise_and(ctr4, nonTrackZone, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counterww = cv::countNonZero(bwInt);
			}

			ctr4.release();
			bitwise.release();
			bwInt.release();



			std::cout << existingBlobs[intIndexOfLeastDistance].unitID << " : counterww : " << counterww << " + " << defaultCount << "\n";
			std::cout << dblLeastDistance << " < " << existingBlobs[intIndexOfLeastDistance].dblCurrentDiagonalSize * 0.1 << "\n";
			//std::cout << currentFrameBlobs[j].currentBoundingRect.width << " >= " << existingBlobs[intIndexOfLeastDistance].currentBoundingRect.width * 0.8 << "\n";

			if (counterww >= defaultCount * 0.5) {

				matchParked = true;

			}

		}


		if (matchParked == true) {

			addBlobToExistingBlobs(currentFrameBlobs[j], existingBlobs, intIndexOfLeastDistance);

		}

		else {

			if (dblLeastDistance < currentFrameBlobs[j].dblCurrentDiagonalSize * 0.5
				&& existingBlobs[intIndexOfLeastDistance].mergeid != 0) {



				int intLeastDistance = 0;
				double LeastDistance = 100000.0;



				for (int w = 0; w < tempBlob.size(); w++) {

					double dblDistances = distanceBetweenPoints(tempBlob[w].centerPositions.back(), existingBlobs[intIndexOfLeastDistance].predictedNextPosition);

					if (dblDistances < LeastDistance && w != j) {
						LeastDistance = dblDistances;
						intLeastDistance = w;

					}
				}

				if (LeastDistance < tempBlob[intLeastDistance].dblCurrentDiagonalSize * 1) {
					std::cout << "split jorrrrrrrrrrrrrrrrrrrrrrrrrrr\n";
					splitBlob(currentFrameBlobs[j], currentFrameBlobs[intLeastDistance], existingBlobs, intIndexOfLeastDistance);
				}
				else {
					addBlobToExistingBlobs(currentFrameBlobs[j], existingBlobs, intIndexOfLeastDistance);
				}







				//addBlobToExistingBlobs(currentFrameBlobs[j], existingBlobs, intIndexOfLeastDistance);


			}

			else if (dblLeastDistance < currentFrameBlobs[j].dblCurrentDiagonalSize * 0.5
				&& dblLeastDistance2 < currentFrameBlobs[j].dblCurrentDiagonalSize * 0.5
				&& existingBlobs[intIndexOfLeastDistance].unitID != 0
				&& existingBlobs[intIndexOfLeastDistance2].unitID != 0
				&& existingBlobs[intIndexOfLeastDistance].park == false
				&& existingBlobs[intIndexOfLeastDistance2].park == false) {


				addBlobToGroupState(currentFrameBlobs[j], existingBlobs, intIndexOfLeastDistance, intIndexOfLeastDistance2);
				std::cout << "merge jor " << existingBlobs[intIndexOfLeastDistance].unitID << " + " << existingBlobs[intIndexOfLeastDistance2].unitID << "\n";
			}


			else if (dblLeastDistance < currentFrameBlobs[j].dblCurrentDiagonalSize * 0.5 && existingBlobs[intIndexOfLeastDistance].park == false) {
				//std::cout << "match : " << existingBlobs[intIndexOfLeastDistance].unitID << "\n";
				addBlobToExistingBlobs(currentFrameBlobs[j], existingBlobs, intIndexOfLeastDistance);

			}

			else {

				int intIndexOfLeastColor = -1;
				double dblColor = 100000.0;


				int leastDistancesss = -1;
				double dblDistancesss = 100000.0;


				std::vector<std::vector<cv::Point> > contourVec;
				contourVec.push_back(currentFrameBlobs[j].currentContour);
				cv::Mat ctr(entrance1.size(), CV_8UC3, SCALAR_BLACK);
				cv::drawContours(ctr, contourVec, -1, SCALAR_WHITE, -1);
				contourVec.clear();


				int counter;
				cv::Mat bitwise;
				cv::Mat bwInt;

				cv::bitwise_and(carParkZone, ctr, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counter = cv::countNonZero(bwInt);

				bitwise.release();
				bwInt.release();
				ctr.release();


				if (counter > 0) {
					//std::cout << "i m matching\n";
					for (int m = 0; m < existingBlobs.size(); m++) {
						if (existingBlobs[m].blnCurrentMatchFoundOrNewBlob == false && existingBlobs[m].blnStillBeingTracked == true
							&& existingBlobs[m].exit == false && existingBlobs[m].park == false) {

							cv::Scalar tempSca = currentFrameBlobs[j].getAverageColorOnce();

							double tempcurrentFrameBlobColor = tempSca.val[0] + tempSca.val[1] + tempSca.val[2];
							double tempexistingBlobColor = 0.0;
							int tempIndex = existingBlobs[m].AvgColor.size();

							if (tempIndex != 0) {
								tempSca = existingBlobs[m].AvgColor[tempIndex - 1];
								tempexistingBlobColor = tempSca.val[0] + tempSca.val[1] + tempSca.val[2];
							}



							double diff = tempcurrentFrameBlobColor - tempexistingBlobColor;
							if (diff < 0) {
								diff = diff * -1;
							}

							if (dblColor > diff) {
								dblColor = diff;
								intIndexOfLeastColor = m;

							}
							//	double distancediff = 1000000000;
							double distancediff = sqrt(((currentFrameBlobs[j].centerPositions[currentFrameBlobs[j].centerPositions.size() - 1].x - existingBlobs[m].centerPositions[existingBlobs[m].centerPositions.size() - 1].x) *
								(currentFrameBlobs[j].centerPositions[currentFrameBlobs[j].centerPositions.size() - 1].x - existingBlobs[m].centerPositions[existingBlobs[m].centerPositions.size() - 1].x)) +

								((currentFrameBlobs[j].centerPositions[currentFrameBlobs[j].centerPositions.size() - 1].y - existingBlobs[m].centerPositions[existingBlobs[m].centerPositions.size() - 1].y) *
								(currentFrameBlobs[j].centerPositions[currentFrameBlobs[j].centerPositions.size() - 1].y - existingBlobs[m].centerPositions[existingBlobs[m].centerPositions.size() - 1].y)));

							if (dblDistancesss > distancediff) {
								dblDistancesss = distancediff;
								leastDistancesss = m;
							}

						}
					}


					if (intIndexOfLeastColor >= 0 && leastDistancesss >= 0 && (intIndexOfLeastColor == leastDistancesss) && dblDistancesss < 200 && existingBlobs[leastDistancesss].park == false) {
						std::cout << existingBlobs[intIndexOfLeastColor].unitID << " : Distances : " << dblDistancesss << "\n";
						addBlobToExistingBlobs(currentFrameBlobs[j], existingBlobs, leastDistancesss);
					}
					else if (leastDistancesss >= 0 && dblDistancesss < 300 && existingBlobs[leastDistancesss].park == false) {
						std::cout << existingBlobs[intIndexOfLeastColor].unitID << " : Distances 300 : " << dblDistancesss << "\n";
						addBlobToExistingBlobs(currentFrameBlobs[j], existingBlobs, leastDistancesss);
					}

					else {
						addNewBlob(currentFrameBlobs[j], existingBlobs);
					}



				}



				else {







					addNewBlob(currentFrameBlobs[j], existingBlobs);
				}
			}
		}

	}



	for (int i = 0; i < existingBlobs.size(); i++) {

		if (existingBlobs[i].blnCurrentMatchFoundOrNewBlob == false) {

			existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch++;
		}

		if ((existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch >= 10 && existingBlobs[i].enter == false) ||
			(existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch >= 3 && existingBlobs[i].exit == true)) {

			existingBlobs[i].blnStillBeingTracked = false;

			//std::cout << existingBlobs[i].unitID << " : " << "false\n";
		}

		if (existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch >= 40
			&& existingBlobs[i].enter == true
			&& existingBlobs[i].exit == false
			&& existingBlobs[i].park == false
			&& existingBlobs[i].unitID != 0) {

			existingBlobs[i].matchBack = false;
			missMatchBlob.push_back(existingBlobs[i]);
			existingBlobs[i].blnStillBeingTracked = false;
			std::cout << "Add to Miss Match State: " << existingBlobs[i].unitID << "\n";
		}

		if (existingBlobs[i].nonTrackParkingZone == true && existingBlobs[i].nonTrackZoneDelay == 0) {
			if (existingBlobs[i].nonTrackParkingZoneLocation == 1) {
				nTrackzone1.push_back(existingBlobs[i]);
			}
			else if (existingBlobs[i].nonTrackParkingZoneLocation == 2) {
				nTrackzone2.push_back(existingBlobs[i]);
			}
			//existingBlobs[i].blnStillBeingTracked = false;
			existingBlobs[i].nonTrackZoneDelay++;


		}

		if (existingBlobs[i].nonTrackParkingZone == true && existingBlobs[i].nonTrackZoneDelay > 0) {
			existingBlobs[i].nonTrackZoneDelay++;
			if (existingBlobs[i].nonTrackZoneDelay > 18) {
				existingBlobs[i].blnStillBeingTracked = false;

			}
		}





	}


	for (int f = 0; f < existingBlobs.size(); f++) {
		if (existingBlobs[f].blnStillBeingTracked == false) {
			std::cout << "Deleting : " << existingBlobs[f].unitID << "\n";
			existingBlobs.erase(existingBlobs.begin() + f);
			if (f > 0) {
				f--;
			}

		}

	}


}


///////////////////////////////////////////////////////////////////////////////////////////////////

void addBlobToGroupState(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex, int &intIndex2) {
	currentFrameBlob.addornot = true;

	existingBlobs[intIndex].getAverageColorLast();
	existingBlobs[intIndex2].getAverageColorLast();



	// no 1
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


	// no 1

	// no 2
	existingBlobs[intIndex2].currentContour = currentFrameBlob.currentContour;
	existingBlobs[intIndex2].currentBoundingRect = currentFrameBlob.currentBoundingRect;

	existingBlobs[intIndex2].centerPositions.push_back(currentFrameBlob.centerPositions.back());

	existingBlobs[intIndex2].dblCurrentDiagonalSize = currentFrameBlob.dblCurrentDiagonalSize;
	existingBlobs[intIndex2].dblCurrentAspectRatio = currentFrameBlob.dblCurrentAspectRatio;

	existingBlobs[intIndex2].blnStillBeingTracked = true;
	existingBlobs[intIndex2].blnCurrentMatchFoundOrNewBlob = true;

	existingBlobs[intIndex2].rawImage = currentFrameBlob.rawImage;
	existingBlobs[intIndex2].maskImage = currentFrameBlob.maskImage;
	existingBlobs[intIndex2].points = currentFrameBlob.points;
	existingBlobs[intIndex2].keypoints_new = currentFrameBlob.keypoints_new;
	existingBlobs[intIndex2].des = currentFrameBlob.des;
	existingBlobs[intIndex2].desNoGpu = currentFrameBlob.desNoGpu;

	existingBlobs[intIndex2].existInSceen++;

	// no2

	existingBlobs[intIndex].mergeid = existingBlobs[intIndex2].unitID;
	existingBlobs[intIndex2].mergeid = existingBlobs[intIndex].unitID;

	if (existingBlobs[intIndex].enter == true) {
		existingBlobs[intIndex].getAverageColor();
	}

	if (existingBlobs[intIndex2].enter == true) {
		existingBlobs[intIndex2].getAverageColor();
	}
	std::cout << "merged!\n";

}

void splitBlob(Blob &currentFrameBlob1, Blob &currentFrameBlob2, std::vector<Blob> &existingBlobs, int &intIndex) {


	int intIndexOfLeastColor = -1;
	double dblColor = 100000.0;

	cv::Scalar tempSca1 = currentFrameBlob1.getAverageColorOnce();
	//double tempcurrentFrameBlobColor1 = (tempSca1.val[0] + tempSca1.val[1] + tempSca1.val[2]) / 3;

	cv::Scalar tempSca2 = currentFrameBlob2.getAverageColorOnce();
	//double tempcurrentFrameBlobColor2 = (tempSca2.val[0] + tempSca2.val[1] + tempSca2.val[2]) / 3;

	double diff1 = sqrt(((tempSca1.val[0] - existingBlobs[intIndex].avgColorBeforeMerge.val[0]) * (tempSca1.val[0] - existingBlobs[intIndex].avgColorBeforeMerge.val[0])) +
		((tempSca1.val[1] - existingBlobs[intIndex].avgColorBeforeMerge.val[1]) * (tempSca1.val[1] - existingBlobs[intIndex].avgColorBeforeMerge.val[1])) +
		((tempSca1.val[2] - existingBlobs[intIndex].avgColorBeforeMerge.val[2]) * (tempSca1.val[2] - existingBlobs[intIndex].avgColorBeforeMerge.val[2])));

	double diff3 = sqrt(((tempSca2.val[0] - existingBlobs[intIndex].avgColorBeforeMerge.val[0]) * (tempSca2.val[0] - existingBlobs[intIndex].avgColorBeforeMerge.val[0])) +
		((tempSca2.val[1] - existingBlobs[intIndex].avgColorBeforeMerge.val[1]) * (tempSca2.val[1] - existingBlobs[intIndex].avgColorBeforeMerge.val[1])) +
		((tempSca2.val[2] - existingBlobs[intIndex].avgColorBeforeMerge.val[2]) * (tempSca2.val[2] - existingBlobs[intIndex].avgColorBeforeMerge.val[2])));


	double diff4 = 0.0;
	double diff2 = 0.0;
	for (int y = 0; y < existingBlobs.size(); y++) {
		if (existingBlobs[intIndex].mergeid == existingBlobs[y].unitID) {

			diff2 = sqrt(((tempSca1.val[0] - existingBlobs[y].avgColorBeforeMerge.val[0]) * (tempSca1.val[0] - existingBlobs[y].avgColorBeforeMerge.val[0])) +
				((tempSca1.val[1] - existingBlobs[y].avgColorBeforeMerge.val[1]) * (tempSca1.val[1] - existingBlobs[y].avgColorBeforeMerge.val[1])) +
				((tempSca1.val[2] - existingBlobs[y].avgColorBeforeMerge.val[2]) * (tempSca1.val[2] - existingBlobs[y].avgColorBeforeMerge.val[2])));

			diff4 = sqrt(((tempSca2.val[0] - existingBlobs[y].avgColorBeforeMerge.val[0]) * (tempSca2.val[0] - existingBlobs[y].avgColorBeforeMerge.val[0])) +
				((tempSca2.val[1] - existingBlobs[y].avgColorBeforeMerge.val[1]) * (tempSca2.val[1] - existingBlobs[y].avgColorBeforeMerge.val[1])) +
				((tempSca2.val[2] - existingBlobs[y].avgColorBeforeMerge.val[2]) * (tempSca2.val[2] - existingBlobs[y].avgColorBeforeMerge.val[2])));
			break;
		}
	}


	std::cout << "diff 1 : " << diff1 << " diff 2 : " << diff2 << "\n";
	std::cout << "diff 3 : " << diff3 << " diff 4 : " << diff4 << "\n";

	if ((diff1 < diff2 && diff1 < diff3 && diff1 < diff4)
		|| (diff4 < diff1 && diff4 < diff2 && diff4 < diff3)) {
		existingBlobs[intIndex].currentContour = currentFrameBlob1.currentContour;
		existingBlobs[intIndex].currentBoundingRect = currentFrameBlob1.currentBoundingRect;

		existingBlobs[intIndex].centerPositions.push_back(currentFrameBlob1.centerPositions.back());

		existingBlobs[intIndex].dblCurrentDiagonalSize = currentFrameBlob1.dblCurrentDiagonalSize;
		existingBlobs[intIndex].dblCurrentAspectRatio = currentFrameBlob1.dblCurrentAspectRatio;

		existingBlobs[intIndex].blnStillBeingTracked = true;
		existingBlobs[intIndex].blnCurrentMatchFoundOrNewBlob = true;

		existingBlobs[intIndex].rawImage = currentFrameBlob1.rawImage;
		existingBlobs[intIndex].maskImage = currentFrameBlob1.maskImage;
		existingBlobs[intIndex].points = currentFrameBlob1.points;
		existingBlobs[intIndex].keypoints_new = currentFrameBlob1.keypoints_new;
		existingBlobs[intIndex].des = currentFrameBlob1.des;
		existingBlobs[intIndex].desNoGpu = currentFrameBlob1.desNoGpu;

		existingBlobs[intIndex].existInSceen++;

		for (int j = 0; j < existingBlobs.size(); j++) {
			if (existingBlobs[intIndex].mergeid == existingBlobs[j].unitID) {

				existingBlobs[j].currentContour = currentFrameBlob2.currentContour;
				existingBlobs[j].currentBoundingRect = currentFrameBlob2.currentBoundingRect;

				existingBlobs[j].centerPositions.push_back(currentFrameBlob2.centerPositions.back());

				existingBlobs[j].dblCurrentDiagonalSize = currentFrameBlob2.dblCurrentDiagonalSize;
				existingBlobs[j].dblCurrentAspectRatio = currentFrameBlob2.dblCurrentAspectRatio;

				existingBlobs[j].blnStillBeingTracked = true;
				existingBlobs[j].blnCurrentMatchFoundOrNewBlob = true;

				existingBlobs[j].rawImage = currentFrameBlob2.rawImage;
				existingBlobs[j].maskImage = currentFrameBlob2.maskImage;
				existingBlobs[j].points = currentFrameBlob2.points;
				existingBlobs[j].keypoints_new = currentFrameBlob2.keypoints_new;
				existingBlobs[j].des = currentFrameBlob2.des;
				existingBlobs[j].desNoGpu = currentFrameBlob2.desNoGpu;

				existingBlobs[j].existInSceen++;

				existingBlobs[j].mergeid = 0;
			}
		}

		existingBlobs[intIndex].mergeid = 0;
	}

	else if ((diff3 < diff1 && diff3 < diff2 && diff3 < diff4)
		|| (diff2 < diff1 && diff2 < diff3 && diff2 < diff4)) {
		std::cout << "this else\n";
		existingBlobs[intIndex].currentContour = currentFrameBlob2.currentContour;
		existingBlobs[intIndex].currentBoundingRect = currentFrameBlob2.currentBoundingRect;

		existingBlobs[intIndex].centerPositions.push_back(currentFrameBlob2.centerPositions.back());

		existingBlobs[intIndex].dblCurrentDiagonalSize = currentFrameBlob2.dblCurrentDiagonalSize;
		existingBlobs[intIndex].dblCurrentAspectRatio = currentFrameBlob2.dblCurrentAspectRatio;

		existingBlobs[intIndex].blnStillBeingTracked = true;
		existingBlobs[intIndex].blnCurrentMatchFoundOrNewBlob = true;

		existingBlobs[intIndex].rawImage = currentFrameBlob2.rawImage;
		existingBlobs[intIndex].maskImage = currentFrameBlob2.maskImage;
		existingBlobs[intIndex].points = currentFrameBlob2.points;
		existingBlobs[intIndex].keypoints_new = currentFrameBlob2.keypoints_new;
		existingBlobs[intIndex].des = currentFrameBlob2.des;
		existingBlobs[intIndex].desNoGpu = currentFrameBlob2.desNoGpu;

		existingBlobs[intIndex].existInSceen++;

		for (int j = 0; j < existingBlobs.size(); j++) {
			if (existingBlobs[intIndex].mergeid == existingBlobs[j].unitID) {

				existingBlobs[j].currentContour = currentFrameBlob1.currentContour;
				existingBlobs[j].currentBoundingRect = currentFrameBlob1.currentBoundingRect;

				existingBlobs[j].centerPositions.push_back(currentFrameBlob1.centerPositions.back());

				existingBlobs[j].dblCurrentDiagonalSize = currentFrameBlob1.dblCurrentDiagonalSize;
				existingBlobs[j].dblCurrentAspectRatio = currentFrameBlob1.dblCurrentAspectRatio;

				existingBlobs[j].blnStillBeingTracked = true;
				existingBlobs[j].blnCurrentMatchFoundOrNewBlob = true;

				existingBlobs[j].rawImage = currentFrameBlob1.rawImage;
				existingBlobs[j].maskImage = currentFrameBlob1.maskImage;
				existingBlobs[j].points = currentFrameBlob1.points;
				existingBlobs[j].keypoints_new = currentFrameBlob1.keypoints_new;
				existingBlobs[j].des = currentFrameBlob1.des;
				existingBlobs[j].desNoGpu = currentFrameBlob1.desNoGpu;

				existingBlobs[j].existInSceen++;

				existingBlobs[j].mergeid = 0;
			}
		}

		existingBlobs[intIndex].mergeid = 0;
	}













}

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
	existingBlobs[intIndex].intNumOfConsecutiveFramesWithoutAMatch = 0;

	existingBlobs[intIndex].existInSceen++;

	if (existingBlobs[intIndex].enter == true) {
		existingBlobs[intIndex].getAverageColor();
	}

	if (existingBlobs[intIndex].enter == true && existingBlobs[intIndex].unitID == 0) {
		existingBlobs[intIndex].unitID = unitObjCounter;
		//std::cout << "Number : " << existingBlobs[intIndex].unitID << "\n";
		unitObjCounter++;
	}

	if (existingBlobs[intIndex].mergeid != 0) {

		for (int k = 0; k < existingBlobs.size(); k++) {

			if (existingBlobs[k].unitID == existingBlobs[intIndex].mergeid) {

				existingBlobs[k].currentContour = currentFrameBlob.currentContour;
				existingBlobs[k].currentBoundingRect = currentFrameBlob.currentBoundingRect;

				existingBlobs[k].centerPositions.push_back(currentFrameBlob.centerPositions.back());

				existingBlobs[k].dblCurrentDiagonalSize = currentFrameBlob.dblCurrentDiagonalSize;
				existingBlobs[k].dblCurrentAspectRatio = currentFrameBlob.dblCurrentAspectRatio;

				existingBlobs[k].blnStillBeingTracked = true;
				existingBlobs[k].blnCurrentMatchFoundOrNewBlob = true;

				existingBlobs[k].rawImage = currentFrameBlob.rawImage;
				existingBlobs[k].maskImage = currentFrameBlob.maskImage;
				existingBlobs[k].points = currentFrameBlob.points;
				existingBlobs[k].keypoints_new = currentFrameBlob.keypoints_new;
				existingBlobs[k].des = currentFrameBlob.des;
				existingBlobs[k].desNoGpu = currentFrameBlob.desNoGpu;

				existingBlobs[k].existInSceen++;

				if (existingBlobs[k].enter == true) {
					existingBlobs[k].getAverageColor();
				}
				break;
			}
		}
	}







}

void addNewBlob(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs) {

	currentFrameBlob.blnCurrentMatchFoundOrNewBlob = true;
	carDensity = carDensity + currentFrameBlob.currentBoundingRect.area();
	existingBlobs.push_back(currentFrameBlob);

}

double distanceBetweenPoints(cv::Point point1, cv::Point point2) {

	int intX = abs(point1.x - point2.x);
	int intY = abs(point1.y - point2.y);

	return(sqrt(pow(intX, 2) + pow(intY, 2)));
}

void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName) {
	cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

	cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);

	cv::imshow(strImageName, image);
}

cv::Mat drawAndShowContoursProccess(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName) {
	cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

	cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);

	cv::erode(image, image, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(4, 4)));

	//cv::imshow(strImageName, image);
	cv::Mat outputimg;
	cv::transform(image, outputimg, cv::Matx13f(1, 1, 1));

	return outputimg;
}

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

//bool checkIfBlobsCrossedTheLine(std::vector<Blob> &blobs, int &intHorizontalLinePosition, int &carCount) {
bool checkIfBlobsCrossedTheLine(std::vector<Blob> &blobs, int &intHorizontalLinePosition, int &carCount, CarParkTrackExporter &openDB, int &frameCount, int &vidLength) {


	std::vector<Blob> tempBb;
	for (int r = 0; r < blobs.size(); r++) {
		tempBb.push_back(blobs[r]);
	}

	bool blnAtLeastOneBlobCrossedTheLine = false;

	//for (auto blob : blobs) {
	for (unsigned int i = 0; i < blobs.size(); i++) {

		std::vector<std::vector<cv::Point> > contourVec;
		contourVec.push_back(blobs[i].currentContour);
		cv::Mat ctr(entrance1.size(), CV_8UC3, SCALAR_BLACK);
		cv::drawContours(ctr, contourVec, -1, SCALAR_WHITE, -1);


		int counter;
		cv::Mat bitwise;
		cv::Mat bwInt;

		if (blobs[i].blnStillBeingTracked == true && blobs[i].centerPositions.size() >= 5) {

			if (blobs[i].park == false) {
				int prevprevprevprevFrameIndex = (int)blobs[i].centerPositions.size() - 5;
				int prevprevprevFrameIndex = (int)blobs[i].centerPositions.size() - 4;
				int prevprevFrameIndex = (int)blobs[i].centerPositions.size() - 3;
				int prevFrameIndex = (int)blobs[i].centerPositions.size() - 2;
				int currFrameIndex = (int)blobs[i].centerPositions.size() - 1;

				cv::bitwise_and(entrance1, ctr, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counter = cv::countNonZero(bwInt);

				if (counter > 0) {

					if (blobs[i].centerPositions[prevFrameIndex].y < blobs[i].centerPositions[currFrameIndex].y
						&& blobs[i].enter == false
						&& blobs[i].centerPositions[prevprevFrameIndex].y < blobs[i].centerPositions[prevFrameIndex].y
						&& blobs[i].centerPositions[prevprevprevFrameIndex].y < blobs[i].centerPositions[prevprevFrameIndex].y

						) {

						printNumberofCar(1, true);
						blobs[i].enter = true;
						blobs[i].entExt = 1;
						blobs[i].changed = true;
						blobs[i].IO_indicator = true;


					}
					else if (blobs[i].centerPositions[prevFrameIndex].y > blobs[i].centerPositions[currFrameIndex].y
						&& blobs[i].enter == true
						&& blobs[i].exit == false
						&& blobs[i].centerPositions[prevprevFrameIndex].y > blobs[i].centerPositions[prevFrameIndex].y
						&& blobs[i].centerPositions[prevprevprevFrameIndex].y > blobs[i].centerPositions[prevprevFrameIndex].y

						) {


						printNumberofCar(1, false);
						blobs[i].exit = true;
						blobs[i].entExt = 1;
						blobs[i].changed = true;
						blobs[i].IO_indicator = false;



					}
				}
				else {

					cv::bitwise_and(entrance5, ctr, bitwise);
					cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
					counter = cv::countNonZero(bwInt);

					if (counter > 0) {
						if (blobs[i].centerPositions[prevFrameIndex].y < blobs[i].centerPositions[currFrameIndex].y
							&& blobs[i].enter == false
							&& blobs[i].centerPositions[prevprevFrameIndex].y < blobs[i].centerPositions[prevFrameIndex].y
							&& blobs[i].centerPositions[prevprevprevFrameIndex].y < blobs[i].centerPositions[prevprevFrameIndex].y
							&& blobs[i].centerPositions[prevprevprevprevFrameIndex].y < blobs[i].centerPositions[prevprevprevFrameIndex].y) {

							printNumberofCar(2, true);
							blobs[i].enter = true;
							blobs[i].entExt = 2;
							blobs[i].changed = true;
							blobs[i].IO_indicator = true;

						}
						else if (blobs[i].centerPositions[prevFrameIndex].y > blobs[i].centerPositions[currFrameIndex].y
							&& blobs[i].enter == true
							&& blobs[i].exit == false
							&& blobs[i].centerPositions[prevprevFrameIndex].y > blobs[i].centerPositions[prevFrameIndex].y
							&& blobs[i].centerPositions[prevprevprevFrameIndex].y > blobs[i].centerPositions[prevprevFrameIndex].y
							&& blobs[i].centerPositions[prevprevprevprevFrameIndex].y > blobs[i].centerPositions[prevprevprevFrameIndex].y) {


							printNumberofCar(2, false);
							blobs[i].exit = true;
							blobs[i].entExt = 2;
							blobs[i].changed = true;
							blobs[i].IO_indicator = false;

						}
					}
					else {

						cv::bitwise_and(entrance6, ctr, bitwise);
						cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
						counter = cv::countNonZero(bwInt);

						if (counter > 0) {
							if (blobs[i].centerPositions[prevFrameIndex].x < blobs[i].centerPositions[currFrameIndex].x
								&& blobs[i].enter == false
								&& blobs[i].centerPositions[prevprevFrameIndex].x < blobs[i].centerPositions[prevFrameIndex].x
								&& blobs[i].centerPositions[prevprevprevFrameIndex].x < blobs[i].centerPositions[prevprevFrameIndex].x
								&& blobs[i].centerPositions[prevprevprevprevFrameIndex].x < blobs[i].centerPositions[prevprevprevFrameIndex].x) {

								printNumberofCar(3, true);
								blobs[i].enter = true;
								blobs[i].entExt = 3;
								blobs[i].changed = true;
								blobs[i].IO_indicator = true;

							}
							else if (blobs[i].centerPositions[prevFrameIndex].x > blobs[i].centerPositions[currFrameIndex].x
								&& blobs[i].enter == true
								&& blobs[i].exit == false
								&& blobs[i].centerPositions[prevprevFrameIndex].x > blobs[i].centerPositions[prevFrameIndex].x
								&& blobs[i].centerPositions[prevprevprevFrameIndex].x > blobs[i].centerPositions[prevprevFrameIndex].x
								&& blobs[i].centerPositions[prevprevprevprevFrameIndex].x > blobs[i].centerPositions[prevprevprevFrameIndex].x) {

								printNumberofCar(3, false);
								blobs[i].exit = true;
								blobs[i].entExt = 3;
								blobs[i].changed = true;
								blobs[i].IO_indicator = false;

							}
						}
						else {

							cv::bitwise_and(entrance2, ctr, bitwise);
							cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
							counter = cv::countNonZero(bwInt);

							if (counter > 0) {

								if (blobs[i].centerPositions[prevFrameIndex].y > blobs[i].centerPositions[currFrameIndex].y
									&& blobs[i].enter == false
									&& blobs[i].centerPositions[prevprevFrameIndex].y > blobs[i].centerPositions[prevFrameIndex].y
									&& blobs[i].centerPositions[prevprevprevFrameIndex].y > blobs[i].centerPositions[prevprevFrameIndex].y
									&& blobs[i].centerPositions[prevprevprevprevFrameIndex].y > blobs[i].centerPositions[prevprevprevFrameIndex].y) {

									printNumberofCar(4, true);
									blobs[i].enter = true;
									blobs[i].entExt = 4;
									blobs[i].changed = true;
									blobs[i].IO_indicator = true;
								}
								else if (blobs[i].centerPositions[prevFrameIndex].y < blobs[i].centerPositions[currFrameIndex].y
									&& blobs[i].enter == true
									&& blobs[i].exit == false
									&& blobs[i].centerPositions[prevprevFrameIndex].y < blobs[i].centerPositions[prevFrameIndex].y
									&& blobs[i].centerPositions[prevprevprevFrameIndex].y < blobs[i].centerPositions[prevprevFrameIndex].y
									&& blobs[i].centerPositions[prevprevprevprevFrameIndex].y < blobs[i].centerPositions[prevprevprevFrameIndex].y) {


									printNumberofCar(4, false);
									blobs[i].exit = true;
									blobs[i].enter = false;
									blobs[i].entExt = 4;
									blobs[i].changed = true;
									blobs[i].IO_indicator = false;
								}

							}
							else {

								cv::bitwise_and(entrance3, ctr, bitwise);
								cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
								counter = cv::countNonZero(bwInt);

								if (counter > 0) {
									if (blobs[i].centerPositions[prevFrameIndex].x > blobs[i].centerPositions[currFrameIndex].x
										&& blobs[i].enter == false
										&& blobs[i].centerPositions[prevprevFrameIndex].x > blobs[i].centerPositions[prevFrameIndex].x
										&& blobs[i].centerPositions[prevprevprevFrameIndex].x > blobs[i].centerPositions[prevprevFrameIndex].x
										&& blobs[i].centerPositions[prevprevprevprevFrameIndex].x > blobs[i].centerPositions[prevprevprevFrameIndex].x) {

										printNumberofCar(5, true);
										blobs[i].enter = true;
										blobs[i].exit = false;
										blobs[i].entExt = 5;
										blobs[i].changed = true;
										blobs[i].IO_indicator = true;
									}
									else if (blobs[i].centerPositions[prevFrameIndex].x < blobs[i].centerPositions[currFrameIndex].x
										&& blobs[i].enter == true
										&& blobs[i].exit == false
										&& blobs[i].centerPositions[prevprevFrameIndex].x < blobs[i].centerPositions[prevFrameIndex].x
										&& blobs[i].centerPositions[prevprevprevFrameIndex].x < blobs[i].centerPositions[prevprevFrameIndex].x
										&& blobs[i].centerPositions[prevprevprevprevFrameIndex].x < blobs[i].centerPositions[prevprevprevFrameIndex].x) {


										printNumberofCar(5, false);

										blobs[i].exit = true;
										blobs[i].entExt = 5;
										blobs[i].changed = true;
										blobs[i].IO_indicator = false;
									}
								}

								else {

									cv::bitwise_and(entrance4, ctr, bitwise);
									cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
									counter = cv::countNonZero(bwInt);

									if (counter > 0) {
										if (blobs[i].centerPositions[prevFrameIndex].x < blobs[i].centerPositions[currFrameIndex].x
											&& blobs[i].enter == false
											&& blobs[i].centerPositions[prevprevFrameIndex].x < blobs[i].centerPositions[prevFrameIndex].x
											&& blobs[i].centerPositions[prevprevprevFrameIndex].x < blobs[i].centerPositions[prevprevFrameIndex].x
											&& blobs[i].centerPositions[prevprevprevprevFrameIndex].x < blobs[i].centerPositions[prevprevprevFrameIndex].x) {

											printNumberofCar(6, true);
											blobs[i].enter = true;
											blobs[i].entExt = 6;
											blobs[i].changed = true;
											blobs[i].IO_indicator = true;

										}
										else if (blobs[i].centerPositions[prevFrameIndex].x > blobs[i].centerPositions[currFrameIndex].x
											&& blobs[i].enter == true
											&& blobs[i].exit == false
											&& blobs[i].centerPositions[prevprevFrameIndex].x > blobs[i].centerPositions[prevFrameIndex].x
											&& blobs[i].centerPositions[prevprevprevFrameIndex].x > blobs[i].centerPositions[prevprevFrameIndex].x
											&& blobs[i].centerPositions[prevprevprevprevFrameIndex].x > blobs[i].centerPositions[prevprevprevFrameIndex].x) {


											printNumberofCar(6, false);

											blobs[i].exit = true;
											blobs[i].entExt = 6;
											blobs[i].changed = true;
											blobs[i].IO_indicator = false;

										}
									}
									else {
										cv::bitwise_and(nonTrackZone2, ctr, bitwise);
										cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
										counter = cv::countNonZero(bwInt);


										if (counter > 0 && blobs[i].centerPositions.size() >= 9) {


											int prev5 = (int)blobs[i].centerPositions.size() - 5;
											int prev6 = (int)blobs[i].centerPositions.size() - 6;
											int prev7 = (int)blobs[i].centerPositions.size() - 7;
											int prev8 = (int)blobs[i].centerPositions.size() - 8;
											int prev9 = (int)blobs[i].centerPositions.size() - 9;


											if (blobs[i].centerPositions[prevFrameIndex].y < blobs[i].centerPositions[currFrameIndex].y
												&& blobs[i].centerPositions[prevprevFrameIndex].y < blobs[i].centerPositions[prevFrameIndex].y
												&& blobs[i].centerPositions[prevprevprevFrameIndex].y < blobs[i].centerPositions[prevprevFrameIndex].y
												&& blobs[i].centerPositions[prev5].y < blobs[i].centerPositions[prevprevprevFrameIndex].y
												&& blobs[i].centerPositions[prev6].y < blobs[i].centerPositions[prev5].y
												&& blobs[i].centerPositions[prev7].y < blobs[i].centerPositions[prev6].y
												&& blobs[i].centerPositions[prev8].y < blobs[i].centerPositions[prev7].y
												&& blobs[i].centerPositions[prev9].y < blobs[i].centerPositions[prev8].y
												&& blobs[i].exit == false && blobs[i].nonTrackParkingZone == false && blobs[i].unitID != 0) {

												blobs[i].nonTrackParkingZone = true;
												blobs[i].nonTrackParkingZoneLocation = 2;
												blobs[i].enter = true;
												blobs[i].leavingNonTrackzone = false;

												std::cout << "vehicle " << blobs[i].unitID << " : non tracking zone 2\n";

												//temporary set park to true to write into database
												blobs[i].park = true;
												openDB.writeToDB_park(blobs, i, frameCount, vidLength, "NTZ 2");
												blobs[i].park = false;


											}
											else if (blobs[i].centerPositions[prevFrameIndex].y > blobs[i].centerPositions[currFrameIndex].y
												&& blobs[i].centerPositions[prevprevFrameIndex].y > blobs[i].centerPositions[prevFrameIndex].y
												&& blobs[i].centerPositions[prevprevprevFrameIndex].y > blobs[i].centerPositions[prevprevFrameIndex].y

												&& blobs[i].centerPositions[prev5].y > blobs[i].centerPositions[prevprevprevFrameIndex].y
												&& blobs[i].centerPositions[prev6].y > blobs[i].centerPositions[prev5].y
												&& blobs[i].centerPositions[prev7].y > blobs[i].centerPositions[prev6].y
												&& blobs[i].centerPositions[prev8].y > blobs[i].centerPositions[prev7].y
												&& blobs[i].centerPositions[prev9].y > blobs[i].centerPositions[prev8].y

												&& blobs[i].leavingNonTrackzone == false) {

												if (nTrackzone2.size() != 0) {
													blobs[i].nonTrackParkingZone = false;
													blobs[i].leavingNonTrackzone = true;
													blobs[i].nonTrackParkingZoneLocation = -1;
													blobs[i].unitID = nTrackzone2[0].unitID;
													blobs[i].enter = true;
													nTrackzone2.erase(nTrackzone2.begin() - 0);

												}
												else {
													blobs[i].nonTrackParkingZone = false;
													blobs[i].nonTrackParkingZoneLocation = -1;
													blobs[i].enter = true;
													blobs[i].leavingNonTrackzone = true;
													blobs[i].unitID = unitObjCounter;
													unitObjCounter++;
												}

												std::cout << "vehicle " << blobs[i].unitID << " : leaving non tracking zone 2\n";
												//printNumberofCar(1, false);
												//blobs[i].exit = true;

												//temporary set park to true to write into database
												openDB.writeToDB_park(blobs, i, frameCount, vidLength, "NTZ 2");


											}



										}
										else {
											cv::bitwise_and(nonTrackZone, ctr, bitwise);
											cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
											counter = cv::countNonZero(bwInt);
											if (counter > 0) {

												/*int prev5 = (int)blobs[i].centerPositions.size() - 5;
												int prev6 = (int)blobs[i].centerPositions.size() - 6;
												int prev7 = (int)blobs[i].centerPositions.size() - 7;
												int prev8 = (int)blobs[i].centerPositions.size() - 8;
												int prev9 = (int)blobs[i].centerPositions.size() - 9;*/


												if (blobs[i].centerPositions[prevFrameIndex].x < blobs[i].centerPositions[currFrameIndex].x
													&& blobs[i].centerPositions[prevprevFrameIndex].x < blobs[i].centerPositions[prevFrameIndex].x
													&& blobs[i].centerPositions[prevprevprevFrameIndex].x < blobs[i].centerPositions[prevFrameIndex].x
													//&& blobs[i].centerPositions[prev5].x < blobs[i].centerPositions[prevprevprevFrameIndex].x
													//&& blobs[i].centerPositions[prev6].x < blobs[i].centerPositions[prev5].x
													//&& blobs[i].centerPositions[prev7].x < blobs[i].centerPositions[prev6].x
													//&& blobs[i].centerPositions[prev8].x < blobs[i].centerPositions[prev7].x
													//&& blobs[i].centerPositions[prev9].x < blobs[i].centerPositions[prev8].x


													&& blobs[i].exit == false && blobs[i].nonTrackParkingZone == false && blobs[i].unitID != 0) {

													blobs[i].nonTrackParkingZone = true;
													blobs[i].nonTrackParkingZoneLocation = 1;
													blobs[i].enter = true;
													blobs[i].leavingNonTrackzone = false;
													std::cout << "vehicle " << blobs[i].unitID << " : non tracking zone 1\n";

													//blobs[i].entExt = 999;
													//blobs[i].changed = true;
													//blobs[i].IO_indicator = true;
													//temporary set park to true to write into database
													blobs[i].park = true;
													openDB.writeToDB_park(blobs, i, frameCount, vidLength, "NTZ 1");
													blobs[i].park = false;

												}
												else if (blobs[i].centerPositions[prevFrameIndex].x > blobs[i].centerPositions[currFrameIndex].x
													&& blobs[i].centerPositions[prevprevFrameIndex].x > blobs[i].centerPositions[prevFrameIndex].x
													&& blobs[i].centerPositions[prevprevprevFrameIndex].x > blobs[i].centerPositions[prevFrameIndex].x

													//&& blobs[i].centerPositions[prev5].x > blobs[i].centerPositions[prevprevprevFrameIndex].x
													//&& blobs[i].centerPositions[prev6].x > blobs[i].centerPositions[prev5].x
													//	&& blobs[i].centerPositions[prev7].x > blobs[i].centerPositions[prev6].x
													//&& blobs[i].centerPositions[prev8].x > blobs[i].centerPositions[prev7].x
													//&& blobs[i].centerPositions[prev9].x > blobs[i].centerPositions[prev8].x

													&& blobs[i].leavingNonTrackzone == false) {

													if (nTrackzone1.size() != 0) {
														blobs[i].nonTrackParkingZone = false;
														blobs[i].nonTrackParkingZoneLocation = -1;
														blobs[i].unitID = nTrackzone1[0].unitID;
														blobs[i].enter = true;
														blobs[i].leavingNonTrackzone = true;
														nTrackzone1.erase(nTrackzone1.begin() - 0);
													}
													else {
														blobs[i].nonTrackParkingZone = false;
														blobs[i].nonTrackParkingZoneLocation = -1;
														blobs[i].enter = true;
														blobs[i].unitID = unitObjCounter;
														blobs[i].leavingNonTrackzone = true;
														unitObjCounter++;

													}
													std::cout << "vehicle " << blobs[i].unitID << " : leaving non tracking zone 1\n";

													//temporary set park to true to write into database
													openDB.writeToDB_park(blobs, i, frameCount, vidLength, "NTZ 1");


												}
											}
										}
									}
								}
							}
						}

					}

				}
			}




			if (blobs[i].exit == false) {
				cv::bitwise_and(zoneA, ctr, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counter = cv::countNonZero(bwInt);
				if (counter > 0 && blobs[i].park == false) {
					if (blobs[i].parkLocation != 1) {
						blobs[i].parkLocation = 1;
						blobs[i].parkframe = 1;
						if (blobs[i].unitID == 0) {
							blobs[i].enter = false;
						}
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
							if (blobs[i].unitID == 0) {
								blobs[i].enter = false;
							}
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
								if (blobs[i].unitID == 0) {
									blobs[i].enter = false;
								}
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
									if (blobs[i].unitID == 0) {
										blobs[i].enter = false;
									}
								}
								else if (blobs[i].parkLocation == 4) {
									blobs[i].parkframe++;
								}



							}
							else {
								cv::bitwise_and(dangerZone, ctr, bitwise);
								cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
								counter = cv::countNonZero(bwInt);

								if (counter > 0 && blobs[i].park == false) {
									if (blobs[i].parkLocation != 6) {
										blobs[i].parkLocation = 6;
										blobs[i].parkframe = 1;
										if (blobs[i].unitID == 0) {
											blobs[i].enter = false;
										}
									}
									else if (blobs[i].parkLocation == 6) {
										blobs[i].parkframe++;
									}
								}
								else {
									cv::bitwise_and(zoneE, ctr, bitwise);
									cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
									counter = cv::countNonZero(bwInt);

									if (counter > 0 && blobs[i].park == false) {
										if (blobs[i].parkLocation != 5) {
											blobs[i].parkLocation = 5;
											blobs[i].parkframe = 1;
											if (blobs[i].unitID == 0) {
												blobs[i].enter = false;
											}
										}
										else if (blobs[i].parkLocation == 5) {
											blobs[i].parkframe++;
										}
									}
									else {
										//// nothing
									}
								}

							}

						}
					}
				}
			}

			if (blobs[i].parkframe > 20 && blobs[i].unitID == 0) {
				blobs[i].intNumOfConsecutiveFramesWithoutAMatch = 0;

				//	blobs[i].enter == true;
			}

			if (blobs[i].parkframe > 50 && blobs[i].park == false) {

				int a = blobs[i].parkLocation;
				if (blobs[i].unitID == 0 && blobs[i].parkLocation != 6) {
					//std::cout << "hoho\n";
					int highIndexx = -1;
					double nerestDis = 1000000;
					int lenght = missMatchBlob.size();
					if (lenght == 0) {
						blobs[i].unitID = unitObjCounter;
						unitObjCounter++;
						blobs[i].enter = true;
						std::cout << "hehe\n";
					}
					else {
						for (int k = 0; k < missMatchBlob.size(); k++) {
							double distancediff = sqrt(((blobs[i].centerPositions[blobs[i].centerPositions.size() - 1].x - missMatchBlob[k].centerPositions[missMatchBlob[k].centerPositions.size() - 1].x) *
								(blobs[i].centerPositions[blobs[i].centerPositions.size() - 1].x - missMatchBlob[k].centerPositions[missMatchBlob[k].centerPositions.size() - 1].x)) +
								((blobs[i].centerPositions[blobs[i].centerPositions.size() - 1].y - missMatchBlob[k].centerPositions[missMatchBlob[k].centerPositions.size() - 1].y) *
								(blobs[i].centerPositions[blobs[i].centerPositions.size() - 1].y - missMatchBlob[k].centerPositions[missMatchBlob[k].centerPositions.size() - 1].y)));

							if (distancediff < nerestDis) {
								nerestDis = distancediff;
								highIndexx = k;
							}
						}
						if (highIndexx != -1) {
							missMatchBlob[highIndexx].parkLocation = blobs[i].parkLocation;
							missMatchBlob[highIndexx].parkinglot = blobs[i].parkinglot;
							missMatchBlob[highIndexx].parkframe = 50;
							addBlobToExistingBlobsMissMatch(blobs[i], missMatchBlob, highIndexx);
							missMatchBlob[highIndexx].matchBack = true;
							missMatchBlob[highIndexx].matchbackid = i;
						}
						else {
							blobs[i].unitID = unitObjCounter;
							unitObjCounter++;
							blobs[i].enter = true;
							// to be continue
						}
					}
				}
				else if (blobs[i].unitID == 0 && blobs[i].parkLocation == 6) {
					blobs[i].parkframe = 0;
				}
				else {
					if (a == 1 && blobs[i].park == false) {


						int indexOflot = -1;
						int tempCounter = 0;
						for (int p = 0; p < zoneAlot.size(); p++) {
							cv::bitwise_and(zoneAlot[p].image, ctr, bitwise);
							cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
							counter = cv::countNonZero(bwInt);
							if (counter > tempCounter) {
								tempCounter = counter;
								indexOflot = p;
							}
						}

						if (zoneAlot[indexOflot].parked == false) {
							blobs[i].parkinglot = zoneAlot[indexOflot].lot;
							zoneAlot[indexOflot].parked = true;

							std::cout << "Park - Vehicle " << blobs[i].unitID << "Lot A" << blobs[i].parkinglot << "\n";

							blobs[i].park = true;
							openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot A");

						}
						else {
							blobs[i].parkframe = 0;
						}



					}
					else if (a == 2 && blobs[i].park == false) {

						int indexOflot = -1;
						int tempCounter = 0;
						for (int p = 0; p < zoneBlot.size(); p++) {
							cv::bitwise_and(zoneBlot[p].image, ctr, bitwise);
							cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
							counter = cv::countNonZero(bwInt);
							if (counter > tempCounter) {
								tempCounter = counter;
								indexOflot = p;
							}
						}

						if (zoneBlot[indexOflot].parked == false) {
							blobs[i].parkinglot = zoneBlot[indexOflot].lot;
							zoneBlot[indexOflot].parked = true;

							std::cout << "Park - Vehicle " << blobs[i].unitID << "Lot B" << blobs[i].parkinglot << "\n";

							blobs[i].park = true;
							openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot B");
						}
						else {
							blobs[i].parkframe = 0;
						}
					}
					else if (a == 3 && blobs[i].park == false) {
						int indexOflot = -1;
						int tempCounter = 0;
						for (int p = 0; p < zoneClot.size(); p++) {
							cv::bitwise_and(zoneClot[p].image, ctr, bitwise);
							cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
							counter = cv::countNonZero(bwInt);
							if (counter > tempCounter) {
								tempCounter = counter;
								indexOflot = p;
							}
						}

						if (zoneClot[indexOflot].parked == false) {
							blobs[i].parkinglot = zoneClot[indexOflot].lot;
							zoneClot[indexOflot].parked = true;

							std::cout << "Park - Vehicle " << blobs[i].unitID << "Lot C" << blobs[i].parkinglot << "\n";

							blobs[i].park = true;
							openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot C");
						}
						else {
							blobs[i].parkframe = 0;
						}
					}
					else if (a == 4 && blobs[i].park == false) {
						int indexOflot = -1;
						int tempCounter = 0;
						for (int p = 0; p < zoneDlot.size(); p++) {
							cv::bitwise_and(zoneDlot[p].image, ctr, bitwise);
							cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
							counter = cv::countNonZero(bwInt);
							if (counter > tempCounter) {
								tempCounter = counter;
								indexOflot = p;
							}
						}

						if (zoneDlot[indexOflot].parked == false) {
							blobs[i].parkinglot = zoneDlot[indexOflot].lot;

							zoneDlot[indexOflot].parked = true;



							std::cout << "Park - Vehicle " << blobs[i].unitID << "Lot D" << blobs[i].parkinglot << "\n";

							blobs[i].park = true;
							openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot D");
						}
						else {
							blobs[i].parkframe = 0;
						}
					}
					else if (a == 5 && blobs[i].park == false) {
						int indexOflot = -1;
						int tempCounter = 0;
						for (int p = 0; p < zoneElot.size(); p++) {
							cv::bitwise_and(zoneElot[p].image, ctr, bitwise);
							cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
							counter = cv::countNonZero(bwInt);
							if (counter > tempCounter) {
								tempCounter = counter;
								indexOflot = p;
							}
						}

						if (zoneElot[indexOflot].parked == false) {
							blobs[i].parkinglot = zoneElot[indexOflot].lot;
							zoneElot[indexOflot].parked = true;




							std::cout << "Park - Vehicle " << blobs[i].unitID << "Lot E" << blobs[i].parkinglot << "\n";

							blobs[i].park = true;
							openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot E");
						}
						else {
							blobs[i].parkframe = 0;
						}
					}

					else if (a == 6 && blobs[i].park == false) {
						blobs[i].parkframe++;
						if (blobs[i].parkframe > 70) {
							std::cout << "Park - Vehicle " << blobs[i].unitID << "DANGER ZONE!\n";
							blobs[i].park = true;
							openDB.writeToDB_park(blobs, i, frameCount, vidLength, "DANGER ZONE!");
						}
					}


				}




				if (blobs[i].park == true) {
					if (blobs[i].mergeid != 0) {
						for (int b = 0; b < tempBb.size(); b++) {
							if (blobs[i].mergeid == tempBb[b].unitID) {
								blobs[b].mergeid = 0;
								blobs[b].parkframe = 0;
								break;
							}
						}
						blobs[i].mergeid = 0;
					}
				}


			}

			if (blobs[i].park == true) {

				if (blobs[i].parkLocation == 1) {
					cv::bitwise_and(zoneA, ctr, bitwise);
					cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
					counter = cv::countNonZero(bwInt);

					if (counter > 0) {
						blobs[i].parkframe++;
						blobs[i].leavingcarpark = 0;
					}
					else {
						blobs[i].leavingcarpark++;

					}


				}
				else if (blobs[i].parkLocation == 2) {
					cv::bitwise_and(zoneB, ctr, bitwise);
					cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
					counter = cv::countNonZero(bwInt);

					if (counter > 0) {
						blobs[i].parkframe++;
						blobs[i].leavingcarpark = 0;
					}
					else {
						blobs[i].leavingcarpark++;

					}

				}
				else if (blobs[i].parkLocation == 3) {
					cv::bitwise_and(zoneC, ctr, bitwise);
					cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
					counter = cv::countNonZero(bwInt);

					if (counter > 0) {
						blobs[i].parkframe++;
						blobs[i].leavingcarpark = 0;
					}
					else {
						blobs[i].leavingcarpark++;

					}

				}
				else if (blobs[i].parkLocation == 4) {
					cv::bitwise_and(zoneD, ctr, bitwise);
					cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
					counter = cv::countNonZero(bwInt);

					if (counter > 0) {
						blobs[i].parkframe++;
						blobs[i].leavingcarpark = 0;
					}
					else {
						blobs[i].leavingcarpark++;

					}

				}
				else if (blobs[i].parkLocation == 5) {
					cv::bitwise_and(zoneE, ctr, bitwise);
					cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
					counter = cv::countNonZero(bwInt);

					if (counter > 0) {
						blobs[i].parkframe++;
						blobs[i].leavingcarpark = 0;
					}
					else {
						blobs[i].leavingcarpark++;

					}

				}

				else if (blobs[i].parkLocation == 6) {
					cv::bitwise_and(dangerZone, ctr, bitwise);
					cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
					counter = cv::countNonZero(bwInt);

					if (counter > 0) {
						blobs[i].parkframe++;
						blobs[i].leavingcarpark = 0;
					}
					else {
						blobs[i].leavingcarpark++;

					}

				}










			}



			if (blobs[i].leavingcarpark > 5) {
				if (blobs[i].parkLocation == 1) {
					zoneAlot[blobs[i].parkinglot - 1].parked = false;
					std::cout << "Leaving car park - Vehicle " << blobs[i].unitID << "Lot A" << blobs[i].parkinglot << "\n";
					openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot A");
				}
				else if (blobs[i].parkLocation == 2) {
					zoneBlot[blobs[i].parkinglot - 1].parked = false;
					std::cout << "Leaving car park - Vehicle " << blobs[i].unitID << "Lot B" << blobs[i].parkinglot << "\n";
					openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot B");
				}
				else if (blobs[i].parkLocation == 3) {
					zoneClot[blobs[i].parkinglot - 1].parked = false;
					std::cout << "Leaving car park - Vehicle " << blobs[i].unitID << "Lot C" << blobs[i].parkinglot << "\n";
					openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot C");
				}
				else if (blobs[i].parkLocation == 4) {
					zoneDlot[blobs[i].parkinglot - 1].parked = false;
					std::cout << "Leaving car park - Vehicle " << blobs[i].unitID << "Lot D" << blobs[i].parkinglot << "\n";
					openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot D");
				}
				else if (blobs[i].parkLocation == 5) {
					zoneElot[blobs[i].parkinglot - 1].parked = false;
					std::cout << "Leaving car park - Vehicle " << blobs[i].unitID << "Lot E" << blobs[i].parkinglot << "\n";
					openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot E");
				}
				else if (blobs[i].parkLocation == 6) {

					std::cout << "Leaving car park - Vehicle " << blobs[i].unitID << "DANGER ZONE" << blobs[i].parkinglot << "\n";
					openDB.writeToDB_park(blobs, i, frameCount, vidLength, "DANGER ZONE");
				}


				blobs[i].park = false;
				blobs[i].parkframe = 0;
				blobs[i].parkLocation = 0;
				blobs[i].parkinglot = -1;
				blobs[i].leavingcarpark = 0;

			}




			///////////////////////////////////////////////////////////////////////////////////////////////////////////



			blnAtLeastOneBlobCrossedTheLine = true;



		}

		contourVec.clear();
		ctr.release();
		bitwise.release();
		bwInt.release();







	}

	return blnAtLeastOneBlobCrossedTheLine;

}

void addBack(std::vector<Blob> &blobs) {


	for (int i = 0; i < missMatchBlob.size(); i++) {
		if (missMatchBlob[i].matchBack == true) {
			missMatchBlob[i].matchBack = false;
			blobs.push_back(missMatchBlob[i]);
			blobs.erase(blobs.begin() + missMatchBlob[i].matchbackid);
			missMatchBlob.erase(missMatchBlob.begin() + i);
			if (i > 0)
				i--;
		}
	}
}


//void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy) {
void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy, CarParkTrackExporter &openDB, int &frameCount, int &vidLength) {

	for (unsigned int i = 0; i < blobs.size(); i++) {

		if (blobs[i].blnStillBeingTracked == true) {


			int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
			double dblFontScale = 1.0;
			int intFontThickness = (int)std::round(dblFontScale * 1.0);

			if (blobs[i].mergeid != 0) {
				if (blobs[i].mergeid < blobs[i].unitID) {
					cv::rectangle(imgFrame2Copy, blobs[i].currentBoundingRect, SCALAR_RED, 2);
					cv::putText(imgFrame2Copy, std::to_string(blobs[i].mergeid) + ", " + std::to_string(blobs[i].unitID), blobs[i].centerPositions.back(), intFontFace, dblFontScale, SCALAR_GREEN, 2);
				}
			}
			else if (blobs[i].unitID == 0) {
				cv::rectangle(imgFrame2Copy, blobs[i].currentBoundingRect, SCALAR_YELLOW, 2);
				cv::putText(imgFrame2Copy, std::to_string(blobs[i].unitID), blobs[i].centerPositions.back(), intFontFace, dblFontScale, SCALAR_GREEN, 2);
			}
			else {
				cv::rectangle(imgFrame2Copy, blobs[i].currentBoundingRect, SCALAR_RED, 2);
				cv::putText(imgFrame2Copy, std::to_string(blobs[i].unitID), blobs[i].centerPositions.back(), intFontFace, dblFontScale, SCALAR_GREEN, 2);
				//write to db
				//if (blobs[i].enter == true && blobs[i].exit == false && blobs[i].changed == true) 
				if (blobs[i].enter == true && blobs[i].changed == true)
				{
					openDB.writeToDB_entExt(blobs, i, frameCount, vidLength, blobs[i].entExt, true);
					blobs[i].changed = false;
				}
				//else if (blobs[i].enter == false && blobs[i].exit == true && blobs[i].changed == true)
				else if (blobs[i].exit == true && blobs[i].changed == true)
				{
					openDB.writeToDB_entExt(blobs, i, frameCount, vidLength, blobs[i].entExt, false);
					blobs[i].changed = false;
				}
				else
				{
					openDB.writeToDB(blobs, i, frameCount, vidLength);
				}

			}
		}
	}
}

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

void checkLeaveWithNoEnter() {




}

void addBlobToExistingBlobsMissMatch(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex) {
	carDensity = carDensity - existingBlobs[intIndex].currentBoundingRect.area();
	carDensity = carDensity + currentFrameBlob.currentBoundingRect.area();

	currentFrameBlob.addornot = true;
	existingBlobs[intIndex].enter = true;
	existingBlobs[intIndex].exit = false;
	existingBlobs[intIndex].park = false;



	existingBlobs[intIndex].currentContour = currentFrameBlob.currentContour;
	existingBlobs[intIndex].currentBoundingRect = currentFrameBlob.currentBoundingRect;
	for (int i = 0; i < currentFrameBlob.centerPositions.size(); i++) {
		existingBlobs[intIndex].centerPositions.push_back(currentFrameBlob.centerPositions[i]);
	}
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
	existingBlobs[intIndex].intNumOfConsecutiveFramesWithoutAMatch = 0;

	existingBlobs[intIndex].existInSceen++;

	if (existingBlobs[intIndex].enter == true) {
		existingBlobs[intIndex].getAverageColor();
	}

	if (existingBlobs[intIndex].enter == true && existingBlobs[intIndex].unitID == 0) {
		existingBlobs[intIndex].unitID = unitObjCounter;
		//std::cout << "Number : " << existingBlobs[intIndex].unitID << "\n";
		unitObjCounter++;
	}

	if (existingBlobs[intIndex].mergeid != 0) {

		for (int k = 0; k < existingBlobs.size(); k++) {

			if (existingBlobs[k].unitID == existingBlobs[intIndex].mergeid) {

				existingBlobs[k].currentContour = currentFrameBlob.currentContour;
				existingBlobs[k].currentBoundingRect = currentFrameBlob.currentBoundingRect;

				existingBlobs[k].centerPositions.push_back(currentFrameBlob.centerPositions.back());

				existingBlobs[k].dblCurrentDiagonalSize = currentFrameBlob.dblCurrentDiagonalSize;
				existingBlobs[k].dblCurrentAspectRatio = currentFrameBlob.dblCurrentAspectRatio;

				existingBlobs[k].blnStillBeingTracked = true;
				existingBlobs[k].blnCurrentMatchFoundOrNewBlob = true;

				existingBlobs[k].rawImage = currentFrameBlob.rawImage;
				existingBlobs[k].maskImage = currentFrameBlob.maskImage;
				existingBlobs[k].points = currentFrameBlob.points;
				existingBlobs[k].keypoints_new = currentFrameBlob.keypoints_new;
				existingBlobs[k].des = currentFrameBlob.des;
				existingBlobs[k].desNoGpu = currentFrameBlob.desNoGpu;

				existingBlobs[k].existInSceen++;

				if (existingBlobs[k].enter == true) {
					existingBlobs[k].getAverageColor();
				}
				break;
			}
		}
	}
}