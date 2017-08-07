//HSV
// CarCounting.cpp : Defines the entry point for the console application.
//include CVUI for user interface
#include "cvui.h"

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
#include<cuda.h>


#include<stdio.h>
#include<iostream>
#include<conio.h> // it may be necessary to change or remove this line if not using Windows


#include "Blob.h"
#include "AdaptiveBackgroundLearning.h"
#include "FramedifferenceBGS.h"
#include "ParkingLot.h"


//clarence added the following:
#include <string>
#include <cstring>
#include <atlstr.h>
#include "sqlite3.h" 
#include "CarParkTrackExporter.h"
#include "Logger.h"
#include "SQLiteManager.h"
#include "StringHelpers.h"
#include <sstream>
#include "GlobalClass.h"
#include <regex>
#define NOMINMAX
#include "dirent.h"
#include <vector>
#include <ctime>
#include "Switches.h"
#include "GetSetLog.hxx"
#include "ColorTerms.h"


#include <thread>
#include "alphanum.h"


extern "C" {
#include "darknet.c"
#include <cuda.h>
	void Init_YOLO(int argc, char **argv);
	int predict_image_c(IplImage* input);

}




#define SHOW_STEPS            // un-comment or comment this line to show steps or not

// global variables ///////////////////////////////////////////////////////////////////////////////
const cv::Scalar SCALAR_BLACK = cv::Scalar(0.0, 0.0, 0.0);
const cv::Scalar SCALAR_WHITE = cv::Scalar(255.0, 255.0, 255.0);
const cv::Scalar SCALAR_YELLOW = cv::Scalar(0.0, 255.0, 255.0);
const cv::Scalar SCALAR_GREEN = cv::Scalar(0.0, 200.0, 0.0);
const cv::Scalar SCALAR_RED = cv::Scalar(0.0, 0.0, 255.0);

// function prototypes ////////////////////////////////////////////////////////////////////////////
void matchCurrentFrameBlobsToExistingBlobs(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs, std::vector<ColorTerm> &colorsList);
void addBlobToExistingBlobs(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex, std::vector<ColorTerm> &colorsList);
void addNewBlob(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs);
void addNewBlobLeavingParking(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs);
bool check_matching(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs);
double distanceBetweenPoints(cv::Point point1, cv::Point point2);
void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName);
cv::Mat drawAndShowContoursProccess(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName);
void drawAndShowContours(cv::Size imageSize, std::vector<Blob> blobs, std::string strImageName, cv::Mat colourImage);

//clarence changed the following function to include write to db:
//bool checkIfBlobsCrossedTheLine(std::vector<Blob> &blobs, int &intHorizontalLinePosition, int &carCount);
bool checkIfBlobsCrossedTheLine(std::vector<Blob> &blobs, int &intHorizontalLinePosition, int &carCount, CarParkTrackExporter &openDB, int &frameCount, int &vidLength, std::vector<ColorTerm> &colorsList);

//clarence changed the following function to include write to db:
//void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy);
void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy, CarParkTrackExporter &openDB, int &frameCount, int &vidLength);
//void vehicleIO(std::vector<Blob> &blobs, CarParkTrackExporter &openDB);

void getIOU(std::vector<Blob> &identifiedBlob, std::vector<Blob> &newBlob0, int &intIndex);
void drawCarCountOnImage(int &carCount, cv::Mat &imgFrame2Copy);
void drawCarDensityOnImage(double &carCount, cv::Mat &imgFrame2Copy);
void drawRegion(cv::Size imageSize, cv::vector<cv::Point2f> points, cv::Mat imageCopy);
void matchCurrentFrameBlobsToExistingBlobs2(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs, std::vector<ColorTerm> &colorsList);
void CallBackFunc(int event, int x, int y, int flags, void* userdata);
void printNumberofCar(int entrance, bool entExt);
void addBlobToGroupState(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex, int &intIndex2, std::vector<ColorTerm> &colorsList);
void splitBlob(Blob &currentFrameBlob1, Blob &currentFrameBlob2, std::vector<Blob> &existingBlobs, int &intIndex);
void checkLeaveWithNoEnter();
void addBack(std::vector<Blob> &blobs);
void addBlobToExistingBlobsMissMatch(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex, std::vector<ColorTerm> &colorsList);
bool checkIfPedestrain(cv::Mat tempCropImage);
void removeBlobMemory(std::vector<Blob> &blobs);
void check_vehicles();
IplImage* mat_to_iplimage(cv::Mat input);
cv::Rect enlargeROI(cv::Mat frm, cv::Rect boundingBox, int padding);
void getBlobMotion(std::vector<Blob> &blobs);


void QueryModule(CarParkTrackExporter &openDB);

//void getNumOfTrajs(CarParkTrackExporter &openDB);
int getOption();

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

cv::Mat ff;
cv::Mat entrance1;
cv::Mat entrance2;
cv::Mat entrance3;
cv::Mat entrance4;
cv::Mat entrance5;
cv::Mat entrance6;

cv::Mat carParkZone;
cv::Mat just_to_test;

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

cv::gpu::HOGDescriptor d_hog;

int updateFrameCounter = 0;

cv::Mat global_img;


bool keep_update = false;
int keep_update_counter = 0;
bool skip_frame = false;


////////////////////////////////////////////// Parameter

// Blob Filter Parameter
int param_G_BoundingRectArea = 650;
double param_G_AspectRatio = 0.2;
double param_L_AspectRatio = 4.0;
int param_G_BoundingRectWidth = 25;
int param_G_BoundingRectHeight = 25;
double param_G_DiagonalSize = 40.0;
double param_L_DiagonalSize = 200.0;
// End of Blob Filter Parameter

// Update BGS 
int param_L_ConeUpdateFrame = 50;
int param_G_UpdateFrame = 5;
// End Update BGS

// Match Blob

// Parking
int param_G_parkframe = 50;
int param_G_DangerParkFrame = 70;
int param_G_LeavingParking = 5;
// End Parking

int skip_frame_num = 3;
int skip_frame_buffer_time = 50;

// End of Match Blob

////////////////////////////////////////////// End of Parameter



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
	//clear result after each directory/day run
	results.clear();
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


static void onMouse(int event, int x, int y, int, void*)
{
	if (event != CV_EVENT_LBUTTONDOWN)
	{
		return;
	}

	cv::Point mouseLoc = cv::Point(x-300, y);
	std::vector<cv::Point> drawQuery;

	drawQuery.push_back(mouseLoc);
	const cv::Point *pts = (const cv::Point*) cv::Mat(drawQuery).data;
	int npts = cv::Mat(drawQuery).rows;

	std::cout << "Mouse location: " << mouseLoc.x << "," << mouseLoc.y << std::endl;
	return;
	//polylines(img, &pts, &npts, 1, false, Scalar(0, 255, 0));

}


#define WINDOW_NAME "Retrieval / Query GUI"

int main(void) {



	//write to log
	GetSetLog log("myfile.log");


	//print current time before running program:
	time_t t = time(0);   // get time now
	struct tm * now = localtime(&t);

	std::cout << "Start of job: " << (now->tm_hour) << ':'
		<< (now->tm_min) << ':'
		<< now->tm_sec
		<< std::endl;

	//define exporter
	CarParkTrackExporter openDB;

	//std::thread threadObj1(check_vehicles);
	//	threadObj1.join();

	





	//run Get_options function
	int getOptionReturn = getOption();
	bool invalid = true;

	//Program options:
	//std::cout << "|  1) Recovery Mode                        |\n";
	//std::cout << "|  2) State Extraction (Batch)             |\n";
	//std::cout << "|  3) Retrieval Module (Batch)             |\n";
	//std::cout << "|  4) Debugging Mode (Batch)               |\n";


	while (invalid == true)
	{
		if (getOptionReturn == 1)
		{
			invalid = false;
			resumeFromError = 1;
		}
		else if (getOptionReturn == 2)
		{
			invalid = false;
			Init_YOLO(0, 0);
			//do nothing
		}
		else if (getOptionReturn == 3)
		{
			invalid = false;
			//run the retrieval module
			QueryModule(openDB);

		}
		else if (getOptionReturn == 4)
		{
			invalid = false;
			debug_on = 1;
		}
		else
		{
			system("cls");
			std::cout << "You have selected an invalid Option, kindly try again." << std::endl;
			getOptionReturn = getOption();
		}
	}






	//initialize DB

	openDB.run();

	//load color term start
	ColorTerm allColors("NULL", "NULL");
	std::vector<ColorTerm> colorsList;
	colorsList = allColors.loadHSVtoTerms(colorsList);


	//load color term end


	for (int i = 0; i < 25; i++) {
		countingfeatures.push_back(0);
	}


	IBGS *bgs;
	bgs = new AdaptiveBackgroundLearning;

	IBGS *bgs2;
	bgs2 = new FrameDifferenceBGS;

	//std::cout << "haha1\n";
	//d_hog = cv::gpu::HOGDescriptor(cv::Size(48, 96));
	//d_hog.setSVMDetector(d_hog.getPeopleDetector48x96());
	//std::cout << "haha2\n";

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

	cv::Mat structuringElement3x3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::Mat structuringElement5x5 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
	cv::Mat structuringElement7x7 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
	cv::Mat structuringElement15x15 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15));
	//std::cout << "haha3\n";


	cv::Mat mask;
	//std::cout << "haha5\n";
	mask = cv::imread("mask.jpg");

	//std::cout << "haha4\n";


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

	just_to_test = cv::imread("Capture2.jpg");


	/*minusMask = cv::imread("minusmask.png");
	std::cout << "test3\n";
	cv::transform(minusMask, minusMask, cv::Matx13f(1, 1, 1));
	std::cout << "test4\n";*/






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
	int numDates, temp_numDates;

	std::string myRoot = "D:\\Videos Database\\Carpark Data\\";
	std::string myRoot2;
	std::vector<std::string> listOfDates;				// holds search results
	std::string temp_root;

	
	if (user == "Clarence") {
		std::cout << "Please enter processing date (CCYYMMDD): ";
		std::cin >> cinDate;

		bool bad = false;
		do {
			std::cout << "Please enter number of processing dates: ";
			std::cin >> numDates;

			bad = std::cin.fail();
			if (bad)
			{
				std::cout << "You have entered an invalid char, please try again : " << std::endl;
			}
			std::cin.clear();
			std::cin.ignore(10, '\n');

		} while (bad);

		temp_numDates = 0;



		std::cout << "< !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! >" << std::endl;;

		do {
			temp_root = myRoot + std::to_string(stoi(cinDate) + temp_numDates) + "\\c2\\";

			//std::cout << temp_root << std::endl;

			pd = opendir(temp_root.c_str());
			if (pd == NULL) {
				//return error code 
				std::cout << "< !!  Error(" << errno << ") opening " << temp_root << "  !! >" << std::endl;
				//std::cout << "ERROR IN LOCATING THE ROOT DIRECTORY FOR VIDEOS!\n";
				//exit(1);
				//add the particular date into skip list

			}
			else
			{
				//write into run list
				listOfDates.push_back(std::to_string(stoi(cinDate) + temp_numDates));

			}

			temp_numDates = temp_numDates + 1;
		} while (temp_numDates != numDates);

		if (listOfDates.size()) {
			std::cout << "\n@ Processing # of dates: " << listOfDates.size() << std::endl;
			for (int i = 0; i < listOfDates.size(); i++)	// used unsigned to appease compiler warnings
			{
				std::cout << i + 1 << ": " << listOfDates[i] << std::endl;;
			}
			std::cout << "< !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! >" << std::endl;;


		}

	}
	else if (user == "Ryan")
	{
		//cinDate = "20161018";
		cinDate = "20161018";
		temp_numDates = 0;
		temp_root = myRoot + std::to_string(stoi(cinDate) + temp_numDates) + "\\c2\\";
		listOfDates.push_back(std::to_string(stoi(cinDate) + temp_numDates));

	}
	//check if root directory is correct
	//std::cout << "Kindly confirm your video database root directory: \nie: " << myRoot << "CCYYMMDD\\c2 (Y/n)";
	//bbcc!!!!: else update 
	//std::cout << "TO BE IMPLEMENTED!\n";


	int resume_from;
	if (resumeFromError)
	{

		resume_from = 0;
		unitObjCounter = 390;

		std::cout << "**************************** ATTENTION ****************************\n";
		std::cout << "*******************************************************************\n\n";

		std::cout << "Video resume_from: " << resume_from;
		std::cout << " with unitObjCounter: " << unitObjCounter << std::endl;
		std::cout << "\n*******************************************************************\n";

		//std::cout << "test2\n";
		//std::string myRoot2 = myRoot + cinDate + "\\c2\\";



	}
	else
	{
		resume_from = 0;
		unitObjCounter = 1;
	}





	for (unsigned int i = 0; i < listOfDates.size(); ++i)	// used unsigned to appease compiler warnings
	{


		//std::cout << i + 1 << ": " << listOfDates[i] << std::endl;;


		myRoot2 = myRoot + listOfDates[i] + "\\c2\\";
		std::cout << myRoot2 << std::endl;
		pd = opendir(myRoot2.c_str());
		if (pd == NULL) {
			//return error code 
			std::cout << "Error(" << errno << ") opening " << myRoot << std::endl;
			std::cout << "ERROR IN LOCATING THE ROOT DIRECTORY FOR VIDEOS!\n";
			exit(1);

		}

		std::string extension = ".mp4";
		search(myRoot2, extension);

		//
		//  SET THE INITIAL RUNNING STATE - ie: resume after error
		//  resume _from = video number -1
		//  unitObjCounter = last deleted obj_id + 1
		//
		char cin_temp = NULL;


		bool first_video = true;
		bool firstTime = true;





		// output results
		if (results.size()) {
			std::cout << results.size() << " files were found:" << std::endl;
			for (; resume_from < results.size(); ++resume_from)	// used unsigned to appease compiler warnings
			{
				if (!first_video) {
					removeBlobMemory(blobs);
				}

				if (!first_video) {
					std::cout << "clear blob memory \n";
					removeBlobMemory(blobs);

				}

				//UPDATE i to change video, eg:
				//i = 2;

				//performing loop over all 100 videos to keep the obj_ID
				//obtain the time difference between 2 videos as well.
				std::cout << resume_from + 1 << ": " << results[resume_from] << std::endl;

				if (first_video) {
					std::cout << "Press Enter to continue: ";
					//std::cin >> cin_temp;
				}

				int vidLength;
				if (user == "Clarence") {

					//if last file name ~= 182400, automatically set time = 6
					if ((resume_from + 1) == results.size()) {

						if (results[resume_from].substr(53, 6) == "182400")
						{
							vidLength = 6;
						}
					}
					else
					{


						vidLength = stoi(results[resume_from + 1].substr(53, 4)) - stoi(results[resume_from].substr(53, 4));

						//to cater for change of hour
						if (vidLength == 46)
						{
							vidLength = 6;
						}
						//temp testing only for double skip method
						//vidLength = vidLength * 2;
					}

				}
				else if (user == "Ryan")
				{
					vidLength = 6;
				}

				// 	 __                     ___     ___ 
				//	|__) |  | |\ |    |\ | |__  \_/  |  
				//	|  \ \__/ | \|    | \| |___ / \  |  
				//                                        



				GlobalClass::instance()->set_InputFileName(results[resume_from].c_str());
				std::string InputFile = GlobalClass::instance()->get_InputFileName();

				capVideo.open(InputFile.c_str());
				std::cout << "Processing file: " << InputFile.c_str() << std::endl;



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



				int counter_skip_frame = 1;
				bool skip_frame_switch = false;


				// start of main while loop							   	   
				while (capVideo.isOpened() && chCheckForEscKey != 27) {

					double start = CLOCK();
					if (counter_skip_frame % skip_frame_num != 0 && skip_frame_switch == true) {
						counter_skip_frame++;
					}
					else {

						counter_skip_frame = 1;
						counter_skip_frame++;


						std::vector<Blob> currentFrameBlobs;



						cv::Mat ROImask1 = cv::Mat::zeros(ROImask.size(), ROImask.type());
						cv::Mat ROImask2 = cv::Mat::zeros(ROImask.size(), ROImask.type());


						cv::Mat imgFrame1Copy = imgFrame1.clone();
						cv::Mat imgFrame2Copy = imgFrame2.clone();


						imgFrame1Copy.copyTo(ROImask1, ROImask);
						imgFrame2Copy.copyTo(ROImask2, ROImask);

						imgFrame1Copy = ROImask1;
						imgFrame2Copy = ROImask2;

						ROImask1.release();
						ROImask2.release();





						//BGS
						cv::Mat img_mask;
						cv::Mat img_bkgmodel;
						bgs->process(imgFrame1Copy, img_mask, img_bkgmodel);
						//BGS

						//BGS2
						cv::Mat img_mask2;
						cv::Mat img_bkgmodel2;
						bgs2->process(imgFrame1Copy, img_mask2, img_bkgmodel2);

	


						if (imshow_display)
							cv::imshow("original mask", img_mask);





						//BGS2
						cv::Mat fusion;
						//
						if (img_mask2.cols > 0) {
							cv::bitwise_and(img_mask, img_mask2, fusion);
							if (imshow_display)
								cv::imshow("fusion", fusion);
							img_mask = fusion;
						}





						cv::Mat imgThresh;

						cv::Mat colorForeground = cv::Mat::zeros(img_mask.size(), img_mask.type());
						imgFrame1Copy.copyTo(colorForeground, img_mask);








						//Threshold (Grey Scale Image)
						cv::threshold(img_mask, imgThresh, 30, 255.0, CV_THRESH_BINARY);
						if (imshow_display)
							cv::imshow("imgThresh", imgThresh);



						//Threshold (Grey Scale Image)

						//release unuse MAT
						img_mask.release();
						img_bkgmodel.release();
						img_mask2.release();
						img_bkgmodel2.release();



						//

						// dilate + find contours



						cv::erode(imgThresh, imgThresh, structuringElement3x3);
						cv::dilate(imgThresh, imgThresh, structuringElement7x7);
						cv::erode(imgThresh, imgThresh, structuringElement3x3);
						cv::dilate(imgThresh, imgThresh, structuringElement7x7);
						cv::erode(imgThresh, imgThresh, structuringElement5x5);




						if (imshow_display)
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
						//	std::cout << "Start: Store Possible Blob!\n";
						//filter convexHull
						for (auto &convexHull : convexHulls) {
							Blob possibleBlob(convexHull);

							if (possibleBlob.currentBoundingRect.area() > param_G_BoundingRectArea &&
								possibleBlob.dblCurrentAspectRatio > param_G_AspectRatio &&
								possibleBlob.dblCurrentAspectRatio < param_L_AspectRatio &&
								possibleBlob.currentBoundingRect.width > param_G_BoundingRectWidth &&
								possibleBlob.currentBoundingRect.height > param_G_BoundingRectHeight &&
								possibleBlob.dblCurrentDiagonalSize > param_G_DiagonalSize &&
								possibleBlob.dblCurrentDiagonalSize < param_L_DiagonalSize &&
								possibleBlob.currentBoundingRect.width * 2 > possibleBlob.currentBoundingRect.height &&

								(cv::contourArea(possibleBlob.currentContour) / (double)possibleBlob.currentBoundingRect.area()) > 0.50) {

								cv::Mat cropImage = imgFrame1Copy(possibleBlob.currentBoundingRect);
								cv::resize(cropImage, cropImage, cv::Size(48, 96));

								if (imshow_display)
									cv::imshow("cropImgae", cropImage);
								if (!checkIfPedestrain(cropImage)) {
									possibleBlob.storeImage(imgFrame1Copy);
									currentFrameBlobs.push_back(possibleBlob);
								}

							}
						}

						global_img = imgFrame1Copy;



						drawAndShowContours(imgThresh.size(), currentFrameBlobs, "imgCurrentFrameBlobs", imgFrame1Copy);

						//std::cout << "OLALALALALALA: " << keep_update << "\n";
						//std::cout << "OLELELELELELE: " << keep_update_counter << "\n";



						if (keep_update) {
							if (keep_update_counter < param_L_ConeUpdateFrame) {

								//std::cout << "Here here: " << keep_update_counter << "\n";
								bgs2->updatemask();
								keep_update_counter++;
							}
							else {
								keep_update = false;
								keep_update_counter = 0;
							}
						}
						else {

							if (currentFrameBlobs.size() == 0) {
								skip_frame_switch = true;
								counter_skip_frame = 1;



								updateFrameCounter++;
								//std::cout << "counter: " << updateFrameCounter << "\n";
								if (updateFrameCounter > param_G_UpdateFrame) {
									bgs2->updatemask();
									updateFrameCounter = 0;
								}

							}
							else {
								skip_frame_switch = false;
							}
						}



						//bgs2->updatemask();




						//match blob
						if (blnFirstFrame == true && first_video == true) {
							first_video = false;
							//std::cout << "Start: First Frame!\n";
							for (auto &currentFrameBlob : currentFrameBlobs) {
								addNewBlobLeavingParking(currentFrameBlob, blobs);
							}
							//std::cout << "End: First Frame!\n";

						}
						else {
							if (blobs.size() == 0) {
								//std::cout << "Start: If Existing Blobs empty!\n";
								for (auto &currentFrameBlob : currentFrameBlobs) {

									addNewBlobLeavingParking(currentFrameBlob, blobs);
								}
								//std::cout << "End: If Existing Blobs empty!\n";
							}
							else {
								updateFrameCounter = 0;
								//	std::cout << "Start: Blobs Matching!\n";

								matchCurrentFrameBlobsToExistingBlobs2(blobs, currentFrameBlobs, colorsList);
								//std::cout << "End: Blobs Matching!\n";

							}
						}






						drawAndShowContours(imgThresh.size(), blobs, "imgBlobs", imgFrame1Copy);


						//match blob

						cv::Mat imgFrame2Copy2 = imgFrame2.clone();


						// Vehicle counting

						imgFrame2Copy = imgFrame2.clone();          // get another copy of frame 2 since we changed the previous frame 2 copy in the processing above


																	//clarence - get the motion direction of the vehicle
						getBlobMotion(blobs);


						//write blob info into DB as well when tracked.
						//drawBlobInfoOnImage(blobs, imgFrame2Copy);
						drawBlobInfoOnImage(blobs, imgFrame2Copy, openDB, frameCount, vidLength);



						//std::cout << "Start: Check vehicle status!\n";
						//bool blnAtLeastOneBlobCrossedTheLine = checkIfBlobsCrossedTheLine(blobs, intHorizontalLinePosition2, carCount);
						bool blnAtLeastOneBlobCrossedTheLine = checkIfBlobsCrossedTheLine(blobs, intHorizontalLinePosition2, carCount, openDB, frameCount, vidLength, colorsList);
						//std::cout << "End: Check vehicle status!\n";
						//std::cout << "Start: Match Miss Match Blobs!\n";
						addBack(blobs);
						//std::cout << "End: Match Miss Match Blobs!\n";

						cv::line(imgFrame2Copy, crossingLine[0], crossingLine[1], SCALAR_RED, 2);
						cv::line(imgFrame2Copy, crossingLine2[0], crossingLine2[1], SCALAR_RED, 2);
						cv::line(imgFrame2Copy, crossingLine3[0], crossingLine3[1], SCALAR_RED, 2);
						cv::line(imgFrame2Copy, crossingLine4[0], crossingLine4[1], SCALAR_RED, 2);
						cv::line(imgFrame2Copy, crossingLine5[0], crossingLine5[1], SCALAR_RED, 2);
						cv::line(imgFrame2Copy, crossingLine6[0], crossingLine6[1], SCALAR_RED, 2);






						drawCarCountOnImage(carCount, imgFrame2Copy);




						cv::cvtColor(colorForeground, colorForeground, CV_BGR2GRAY);




						//double dur = CLOCK() - start;

						//double fps = avgfps();
						//drawCarDensityOnImage(fps, imgFrame2Copy);



						//draw position of video
						cv::Point Bar1, Bar2, VidPos;
						Bar1.x = 56;
						Bar1.y = 10;
						Bar2.x = 164;
						Bar2.y = 10;
						//skip_frame = false;	   

						cv::line(imgFrame2Copy, Bar1, Bar2, SCALAR_BLACK, 5);

						VidPos.y = 10;
						VidPos.x = 60 + ((capVideo.get(CV_CAP_PROP_POS_FRAMES) / capVideo.get(CV_CAP_PROP_FRAME_COUNT)) * 100);

						cv::line(imgFrame2Copy, Bar1, VidPos, SCALAR_GREEN, 2);

						//end





						cv::imshow("imgFrame2Copy", imgFrame2Copy);


						//cv::waitKey(0);                 // uncomment this line to go frame by frame for debugging

						// now we prepare for the next iteration

						currentFrameBlobs.clear();


						imgFrame1 = imgFrame2.clone();

						// move frame 1 up to where frame 2 is

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


						//The above method uses 2x frames 
					}




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



					double dur = CLOCK() - start;

					double fps = avgfps();
					drawCarDensityOnImage(fps, imgFrame2);

				} // end of main while loop







				if (chCheckForEscKey != 27) {               // if the user did not press esc (i.e. we reached the end of the video)
					cv::waitKey(0);                         // hold the windows open to allow the "end of video" message to show
				}
				// note that if the user did press esc, we don't need to hold the windows open, we can simply let the program end which will close the windows
			}


			//end of job, print time

			t = time(0);   // get time now
			now = localtime(&t);
			std::cout << "End of current file: " << (now->tm_hour) << ':'
				<< (now->tm_min) << ':'
				<< now->tm_sec
				<< std::endl;



			//std::cout << "Press Enter to continue: ";
			//std::cin >> cin_temp;
			resume_from = 0;
		}

		else
		{
			std::cout << "No files ending in '" << extension << "' were found." << std::endl;
			std::cin.get();
		}
	}
	//_CrtDumpMemoryLeaks();
	return(0);
}

void check_vehicles() {
	int result = std::system("cd yolo & darknet.exe detector test ../Yolo/data/coco.data ../Yolo/yolo.cfg ../Yolo/yolo.weights -i 0 -thresh 0.1");
	std::cout << "result: " << result << "\n";
}

//CString ExecuteExternalFile(CString csExecute) {
//	SECURITY_ATTRIBUTES secattr;
//	ZeroMemory(&secattr, sizeof(secattr));
//	secattr.nLength = sizeof(secattr);
//	secattr.bInheritHandle = TRUE;
//
//	HANDLE rPipe, wPipe;
//
//	//Create pipes to write and read data
//	CreatePipe(&rPipe, &wPipe, &secattr, 0);
//	//
//	STARTUPINFO sInfo;
//	ZeroMemory(&sInfo, sizeof(sInfo));
//
//	PROCESS_INFORMATION pInfo;
//	ZeroMemory(&pInfo, sizeof(pInfo));
//
//	sInfo.cb = sizeof(sInfo);
//	sInfo.dwFlags = STARTF_USESTDHANDLES;
//	sInfo.hStdInput = NULL;
//	sInfo.hStdOutput = wPipe;
//	sInfo.hStdError = wPipe;
//
//	
//
//	char command[1024];
//	strcpy(command, csExecute.GetBuffer(csExecute.GetLength()));
//
//	//Create the process here.
//	CreateProcess(0, command, 0, 0, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, 0, 0, &sInfo, &pInfo);
//	CloseHandle(wPipe);
//
//	//now read the output pipe here.
//
//	char buf[100];
//	DWORD reDword;
//	CString m_csOutput, csTemp;
//	BOOL res;
//	do
//	{
//		res = ::ReadFile(rPipe, buf, 100, &reDword, 0);
//		csTemp = buf;
//		m_csOutput += csTemp.Left(reDword);
//	} while (res);
//
//
//	CloseHandle(pInfo.hProcess);
//	CloseHandle(pInfo.hThread);
//
//	return  m_csOutput;
//}


///////////////////////////////////////////////////////////////////////////////////////////////////

void getIOU(std::vector<Blob> &identifiedBlob, std::vector<Blob> &newBlob0, int &intIndex)
{

	//idea: if blob center position is very close .. give a parameter 
	//then compare the IOU of the blobs, if the above a certain threshold, assume that it's the same item
	//do not let the blob be added

	/*def bb_intersection_over_union(boxA, boxB) :
	# determine the(x, y) - coordinates of the intersection rectangle
	xA = max(boxA[0], boxB[0])
	yA = max(boxA[1], boxB[1])
	xB = min(boxA[2], boxB[2])
	yB = min(boxA[3], boxB[3])
	# compute the area of intersection rectangle
	interArea = (xB - xA + 1) * (yB - yA + 1)
	# compute the area of both the prediction and ground - truth
	# rectangles
	boxAArea = (boxA[2] - boxA[0] + 1) * (boxA[3] - boxA[1] + 1)
	boxBArea = (boxB[2] - boxB[0] + 1) * (boxB[3] - boxB[1] + 1)
	# compute the intersection over union by taking the intersection
	# area and dividing it by the sum of prediction + ground - truth
	# areas - the interesection area
	iou = interArea / float(boxAArea + boxBArea - interArea)
	# return the intersection over union value
	return iou*/




}




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






	if (imshow_display)
		cv::imshow("sdsd", countingRegion);
}

void matchCurrentFrameBlobsToExistingBlobs2(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs, std::vector<ColorTerm> &colorsList) {

	//std::cout << "Start: Matching temp blobs!\n";
	std::vector<Blob> tempBlob;
	for (int g = 0; g < currentFrameBlobs.size(); g++) {
		tempBlob.push_back(currentFrameBlobs[g]);
	}
	//std::cout << "End: Matching temp blobs!\n";

	//std::cout << "Start: Predict Blobs Position!\n";
	for (int i = 0; i < existingBlobs.size(); i++) {
		existingBlobs[i].blnCurrentMatchFoundOrNewBlob = false;
		if (existingBlobs[i].park == true)
			existingBlobs[i].predictNextPositionPark();
		else
			existingBlobs[i].predictNextPosition();
	}
	//std::cout << "End: Predict Blobs Position!\n";


	//std::cout << "Start: Matching!\n";
	for (int j = 0; j < currentFrameBlobs.size(); j++) {

		int intIndexOfLeastDistance = 0;
		double dblLeastDistance = 100000.0;

		int intIndexOfLeastDistance2 = 0;
		double dblLeastDistance2 = 100000.0;
		//std::cout << "(Matching) Start: finding nearest blobs!\n";
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
		//std::cout << "(Matching) End: finding nearest blobs!\n";

		//if (existingBlobs[intIndexOfLeastDistance].park == true && dblLeastDistance < currentFrameBlobs[j].dblCurrentDiagonalSize * 0.1) {
		//	//	std::cout << existingBlobs[intIndexOfLeastDistance].unitID << " : Testing\n";
		//	cv::Mat bitwise;
		//	cv::Mat bwInt;
		//	int counterww = 0;
		//	int defaultCount = 0;
		//	//int counterww2 = 0;
		//	//int counterww3 = 0;

		//	std::vector<std::vector<cv::Point> > contourVec4;
		//	contourVec4.push_back(currentFrameBlobs[j].currentContour);
		//	cv::Mat ctr4(entrance1.size(), CV_8UC3, SCALAR_BLACK);
		//	cv::drawContours(ctr4, contourVec4, -1, SCALAR_WHITE, -1);
		//	contourVec4.clear();

		//	if (existingBlobs[intIndexOfLeastDistance].parkLocation == 1) {
		//		cv::bitwise_and(ctr4, zoneAlot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].image, bitwise);
		//		cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
		//		counterww = cv::countNonZero(bwInt);
		//		defaultCount = zoneAlot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].countNZero;


		//	}
		//	else if (existingBlobs[intIndexOfLeastDistance].parkLocation == 2) {
		//		cv::bitwise_and(ctr4, zoneBlot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].image, bitwise);
		//		cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
		//		counterww = cv::countNonZero(bwInt);
		//		defaultCount = zoneBlot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].countNZero;


		//	}
		//	else if (existingBlobs[intIndexOfLeastDistance].parkLocation == 3) {
		//		cv::bitwise_and(ctr4, zoneClot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].image, bitwise);
		//		cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
		//		counterww = cv::countNonZero(bwInt);
		//		defaultCount = zoneClot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].countNZero;

		//	}
		//	else if (existingBlobs[intIndexOfLeastDistance].parkLocation == 4) {
		//		cv::bitwise_and(ctr4, zoneDlot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].image, bitwise);
		//		cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
		//		counterww = cv::countNonZero(bwInt);
		//		defaultCount = zoneDlot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].countNZero;


		//	}
		//	else if (existingBlobs[intIndexOfLeastDistance].parkLocation == 5) {
		//		cv::bitwise_and(ctr4, zoneElot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].image, bitwise);
		//		cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
		//		counterww = cv::countNonZero(bwInt);
		//		defaultCount = zoneElot[existingBlobs[intIndexOfLeastDistance].parkinglot - 1].countNZero;

		//	}

		//	else if (existingBlobs[intIndexOfLeastDistance].parkLocation == 6) {
		//		cv::bitwise_and(ctr4, dangerZone, bitwise);
		//		cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
		//		counterww = cv::countNonZero(bwInt);
		//	}

		//	else if (existingBlobs[intIndexOfLeastDistance].parkLocation == 7) {
		//		cv::bitwise_and(ctr4, nonTrackZone, bitwise);
		//		cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
		//		counterww = cv::countNonZero(bwInt);
		//	}

		//	ctr4.release();
		//	bitwise.release();
		//	bwInt.release();


		//	if (counterww >= defaultCount * 0.4) {

		//		matchParked = true;

		//	}

		//}


		if (existingBlobs[intIndexOfLeastDistance].park == true && dblLeastDistance < currentFrameBlobs[j].dblCurrentDiagonalSize * 0.2) {
			//std::cout << "(Matching) Start: Match parked blobs!\n";
			addBlobToExistingBlobs(currentFrameBlobs[j], existingBlobs, intIndexOfLeastDistance, colorsList);
			//std::cout << "(Matching) End: Match parked blobs!\n";
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
					std::cout << "(Matching) start: split!\n";

					splitBlob(currentFrameBlobs[j], currentFrameBlobs[intLeastDistance], existingBlobs, intIndexOfLeastDistance);
					std::cout << "split jorrrrrrrrrrrrrrrrrrrrrrrrrrr\n";
				}
				else {
					addBlobToExistingBlobs(currentFrameBlobs[j], existingBlobs, intIndexOfLeastDistance, colorsList);
				}







				//addBlobToExistingBlobs(currentFrameBlobs[j], existingBlobs, intIndexOfLeastDistance);


			}

			else if (dblLeastDistance < currentFrameBlobs[j].dblCurrentDiagonalSize * 0.5
				&& dblLeastDistance2 < currentFrameBlobs[j].dblCurrentDiagonalSize * 0.5
				&& existingBlobs[intIndexOfLeastDistance].unitID != 0
				&& existingBlobs[intIndexOfLeastDistance2].unitID != 0
				&& existingBlobs[intIndexOfLeastDistance].park == false
				&& existingBlobs[intIndexOfLeastDistance2].park == false) {

				std::cout << "(Matching) start: merge!\n";
				addBlobToGroupState(currentFrameBlobs[j], existingBlobs, intIndexOfLeastDistance, intIndexOfLeastDistance2, colorsList);
				std::cout << "merge jor " << existingBlobs[intIndexOfLeastDistance].unitID << " + " << existingBlobs[intIndexOfLeastDistance2].unitID << "\n";
			}


			else if (dblLeastDistance < currentFrameBlobs[j].dblCurrentDiagonalSize * 0.5 && existingBlobs[intIndexOfLeastDistance].park == false) {
				//std::cout << "match : " << existingBlobs[intIndexOfLeastDistance].unitID << "\n";
				addBlobToExistingBlobs(currentFrameBlobs[j], existingBlobs, intIndexOfLeastDistance, colorsList);

			}

			else {
				//std::cout << "(Matching) Start: Match blobs with color and distance!\n";

				int intIndexOfLeastColor = -1;
				double dblColor = 100000.0;


				int leastDistancesss = -1;
				double dblDistancesss = 100000.0;


				std::vector<std::vector<cv::Point> > contourVec;
				contourVec.push_back(currentFrameBlobs[j].currentContour);
				cv::Mat ctr(entrance1.size(), CV_8UC3, SCALAR_BLACK);
				cv::drawContours(ctr, contourVec, -1, SCALAR_WHITE, -1);
				contourVec.clear();
				// testing



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
						//	std::cout << existingBlobs[intIndexOfLeastColor].unitID << " : Distances : " << dblDistancesss << "\n";
						//if(existingBlobs[leastDistancesss].unitID != 0)
						bool temp_check = check_matching(currentFrameBlobs[j], existingBlobs);
						if (temp_check == false) {
							addBlobToExistingBlobs(currentFrameBlobs[j], existingBlobs, leastDistancesss, colorsList);
						}
					}
					else if (leastDistancesss >= 0 && dblDistancesss < 300 && existingBlobs[leastDistancesss].park == false) {
						//	std::cout << existingBlobs[intIndexOfLeastColor].unitID << " : Distances 300 : " << dblDistancesss << "\n";
						//if (existingBlobs[leastDistancesss].unitID != 0)
						bool temp_check = check_matching(currentFrameBlobs[j], existingBlobs);
						if (temp_check == false) {
							addBlobToExistingBlobs(currentFrameBlobs[j], existingBlobs, leastDistancesss, colorsList);
						}
					}

					else {
						//std::cout << "(Matching with color) Start: add new blobs!\n";
						addNewBlobLeavingParking(currentFrameBlobs[j], existingBlobs);
						//std::cout << "(Matching with color) End: add new blobs!\n";
						// 

					}
					//std::cout << "(Matching) End: Match blobs with color and distance!\n";



				}



				else {

					addNewBlob(currentFrameBlobs[j], existingBlobs);
				}
			}
		}

	}


	//std::cout << "End: Matching!\n";
	//std::cout << "Start: check to delete!\n";
	for (int i = 0; i < existingBlobs.size(); i++) {
		//std::cout << "check to delete 1!\n";
		if (existingBlobs[i].blnCurrentMatchFoundOrNewBlob == false) {

			existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch++;
		}
		//std::cout << "check to delete 2!\n";
		if ((existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch >= 10 && existingBlobs[i].enter == false) ||
			(existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch >= 3 && existingBlobs[i].exit == true) ||
			(existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch >= 18 && existingBlobs[i].park == true)) {

			existingBlobs[i].blnStillBeingTracked = false;

			//std::cout << existingBlobs[i].unitID << " : " << "false\n";
		}
		//std::cout << "check to delete 3!\n";
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
		//std::cout << "check to delete 4!\n";
		if (existingBlobs[i].nonTrackParkingZone == true && existingBlobs[i].nonTrackZoneDelay == 0) {
			if (existingBlobs[i].nonTrackParkingZoneLocation == 1) {

				nTrackzone1.push_back(existingBlobs[i]);
			}
			else if (existingBlobs[i].nonTrackParkingZoneLocation == 2) {
				std::cout << existingBlobs[i].unitID << "\n";
				nTrackzone2.push_back(existingBlobs[i]);
			}
			//existingBlobs[i].blnStillBeingTracked = false;
			existingBlobs[i].nonTrackZoneDelay++;


		}
		//std::cout << "check to delete 5!\n";
		if (existingBlobs[i].nonTrackParkingZone == true && existingBlobs[i].nonTrackZoneDelay > 0) {
			existingBlobs[i].nonTrackZoneDelay++;
			if (existingBlobs[i].nonTrackZoneDelay > 18) {
				existingBlobs[i].blnStillBeingTracked = false;

			}
		}





	}

	//std::cout << "End: check to delete!\n";
	//std::cout << "Start:Delete Blobs!\n";
	for (int f = 0; f < existingBlobs.size(); f++) {
		if (existingBlobs[f].blnStillBeingTracked == false) {
			std::cout << "Deleting : " << existingBlobs[f].unitID << "\n";

			existingBlobs.erase(existingBlobs.begin() + f);
			if (f > 0) {
				f--;
			}

		}

	}
	//std::cout << "End:Delete Blobs!\n";

}

///////////////////////////////////////////////////////////////////////////////////////////////////

void addBlobToGroupState(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex, int &intIndex2, std::vector<ColorTerm> &colorsList) {
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
		existingBlobs[intIndex].getAverageColor(colorsList);
	}

	if (existingBlobs[intIndex2].enter == true) {
		existingBlobs[intIndex2].getAverageColor(colorsList);
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

void addBlobToExistingBlobs(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex, std::vector<ColorTerm> &colorsList) {

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
		existingBlobs[intIndex].getAverageColor(colorsList);
	}

	if (existingBlobs[intIndex].enter == true && existingBlobs[intIndex].unitID == 0 && existingBlobs[intIndex].park == false) {
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
					existingBlobs[k].getAverageColor(colorsList);
				}
				break;
			}
		}
	}







}

void addNewBlob(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs) {

	currentFrameBlob.blnCurrentMatchFoundOrNewBlob = true;

	existingBlobs.push_back(currentFrameBlob);

}

void addNewBlobLeavingParking(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs) {


	cv::Rect tempBoundingRect;
	cv::Mat cropImage;
	IplImage* to_crop;
	int crop_result;

	currentFrameBlob.blnCurrentMatchFoundOrNewBlob = true;

	std::vector<std::vector<cv::Point> > contourVec;
	contourVec.push_back(currentFrameBlob.currentContour);
	cv::Mat ctr(entrance1.size(), CV_8UC3, SCALAR_BLACK);
	cv::drawContours(ctr, contourVec, -1, SCALAR_WHITE, -1);

	int counter;
	cv::Mat bitwise;
	cv::Mat bwInt;

	cv::bitwise_and(zoneA, ctr, bitwise);
	cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
	counter = cv::countNonZero(bwInt);

	int highMatch = -1;
	int highMatchCounter = -1;


	tempBoundingRect = enlargeROI(global_img, currentFrameBlob.currentBoundingRect, 10);
	cropImage = global_img(tempBoundingRect);
	to_crop = mat_to_iplimage(cropImage);
	crop_result = predict_image_c(to_crop);
	std::cout << "prediction: " << crop_result << "\n";
	//system("pause");

	//if (crop_result == 1) {



	if (counter > 0 && crop_result == 1) {
		//std::cout << "Start: new blob * A\n";
		for (int i = 0; i < zoneAlot.size(); i++) {
			cv::bitwise_and(zoneAlot[i].image, ctr, bitwise);
			cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
			counter = cv::countNonZero(bwInt);

			if (counter > highMatchCounter) {
				highMatchCounter = counter;
				highMatch = i;
			}
		}
		//std::cout << "End: new blob * A\n";
		std::cout << "A: " << "highMatchCounter : " << highMatchCounter << ", " << "highMatch : " << highMatch << "zoneAlot[highMatch].countNZero * 0.6 : " << zoneAlot[highMatch].countNZero * 0.4 << "\n";
		if (highMatch >= 0 && highMatchCounter >= zoneAlot[highMatch].countNZero * 0.4) {





			currentFrameBlob.park = true;
			currentFrameBlob.enter = true;
			currentFrameBlob.parkLocation = 1;
			currentFrameBlob.parkinglot = zoneAlot[highMatch].lot;
			std::cout << "matching parked car : Lot A\n";

			keep_update = true;
			keep_update_counter = 0;

		}
	}
	else {
		//std::cout << "ggggggg\n";
		cv::bitwise_and(zoneB, ctr, bitwise);
		cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
		counter = cv::countNonZero(bwInt);

		if (counter > 0) {
			std::cout << "Start: new blob * B\n";
			for (int i = 0; i < zoneBlot.size(); i++) {
				cv::bitwise_and(zoneBlot[i].image, ctr, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counter = cv::countNonZero(bwInt);

				if (counter > highMatchCounter) {
					highMatchCounter = counter;
					highMatch = i;
				}
			}
			std::cout << "B: " << "highMatchCounter : " << highMatchCounter << ", " << "highMatch : " << highMatch << "zoneAlot[highMatch].countNZero * 0.6 : " << zoneBlot[highMatch].countNZero * 0.5 << "\n";
			//std::cout << "End: new blob * B\n";
			if (highMatch >= 0 && highMatchCounter >= zoneBlot[highMatch].countNZero * 0.4) {
				currentFrameBlob.park = true;
				currentFrameBlob.enter = true;
				currentFrameBlob.parkLocation = 2;
				currentFrameBlob.parkinglot = zoneBlot[highMatch].lot;
				std::cout << "matching parked car : Lot B\n";
				keep_update = true;
				keep_update_counter = 0;
			}
		}
		else {
			cv::bitwise_and(zoneC, ctr, bitwise);
			cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
			counter = cv::countNonZero(bwInt);

			if (counter > 0 && crop_result == 1) {
				//std::cout << "Start: new blob * C\n";
				for (int i = 0; i < zoneClot.size(); i++) {
					cv::bitwise_and(zoneClot[i].image, ctr, bitwise);
					cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
					counter = cv::countNonZero(bwInt);

					if (counter > highMatchCounter) {
						highMatchCounter = counter;
						highMatch = i;
					}
				}
				std::cout << "C: " << "highMatchCounter : " << highMatchCounter << ", " << "highMatch : " << highMatch << "zoneAlot[highMatch].countNZero * 0.6 : " << zoneClot[highMatch].countNZero * 0.5 << "\n";
				//	std::cout << "End: new blob * C\n";
				if (highMatch >= 0 && highMatchCounter >= zoneClot[highMatch].countNZero * 0.4) {
					currentFrameBlob.park = true;
					currentFrameBlob.enter = true;
					currentFrameBlob.parkLocation = 3;
					currentFrameBlob.parkinglot = zoneClot[highMatch].lot;
					std::cout << "matching parked car : Lot C\n";
					keep_update = true;
					keep_update_counter = 0;
				}
			}
			else {
				cv::bitwise_and(zoneD, ctr, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counter = cv::countNonZero(bwInt);

				if (counter > 0 && crop_result == 1) {
					//	std::cout << "Start: new blob * D\n";
					for (int i = 0; i < zoneDlot.size(); i++) {
						cv::bitwise_and(zoneDlot[i].image, ctr, bitwise);
						cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
						counter = cv::countNonZero(bwInt);

						if (counter > highMatchCounter) {
							highMatchCounter = counter;
							highMatch = i;
						}
					}
					//	std::cout << "End: new blob * D\n";
					std::cout << "D: " << "highMatchCounter : " << highMatchCounter << ", " << "highMatch : " << highMatch << "zoneAlot[highMatch].countNZero * 0.6 : " << zoneDlot[highMatch].countNZero * 0.5 << "\n";
					if (highMatch >= 0 && highMatchCounter >= zoneDlot[highMatch].countNZero * 0.4) {
						currentFrameBlob.park = true;
						currentFrameBlob.enter = true;
						currentFrameBlob.parkLocation = 4;
						currentFrameBlob.parkinglot = zoneDlot[highMatch].lot;
						std::cout << "matching parked car : Lot D\n";
						keep_update = true;
						keep_update_counter = 0;
					}
				}
				else {
					cv::bitwise_and(zoneE, ctr, bitwise);
					cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
					counter = cv::countNonZero(bwInt);

					if (counter > 0) {
						//	std::cout << "Start: new blob * E\n";
						for (int i = 0; i < zoneElot.size(); i++) {
							cv::bitwise_and(zoneElot[i].image, ctr, bitwise);
							cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
							counter = cv::countNonZero(bwInt);

							if (counter > highMatchCounter) {
								highMatchCounter = counter;
								highMatch = i;
							}
						}
						std::cout << "E: " << "highMatchCounter : " << highMatchCounter << ", " << "highMatch : " << highMatch << "zoneAlot[highMatch].countNZero * 0.6 : " << zoneElot[highMatch].countNZero * 0.5 << "\n";
						//std::cout << "End: new blob * E\n";
						if (highMatch >= 0 && highMatchCounter >= zoneElot[highMatch].countNZero * 0.4) {
							currentFrameBlob.park = true;
							currentFrameBlob.enter = true;
							currentFrameBlob.parkLocation = 5;
							currentFrameBlob.parkinglot = zoneElot[highMatch].lot;
							std::cout << "matching parked car : Lot E\n";
							keep_update = true;
							keep_update_counter = 0;
						}
					}
					else {
						cv::bitwise_and(dangerZone, ctr, bitwise);
						cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
						counter = cv::countNonZero(bwInt);

						if (counter > 0 && crop_result == 1) {
							//	std::cout << "Start: new blob * F\n";
							currentFrameBlob.park = true;
							currentFrameBlob.enter = true;
							currentFrameBlob.parkLocation = 6;
							//	std::cout << "End: new blob * F\n";
						}
						else {
							//do nothing, just add
						}
					}
				}
			}
		}
	}
	//}


	contourVec.clear();
	ctr.release();
	bitwise.release();
	bwInt.release();


	existingBlobs.push_back(currentFrameBlob);


}



bool check_matching(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs) {

	bool check = false;

	std::vector<std::vector<cv::Point> > contourVec;
	contourVec.push_back(currentFrameBlob.currentContour);
	cv::Mat ctr(entrance1.size(), CV_8UC3, SCALAR_BLACK);
	cv::drawContours(ctr, contourVec, -1, SCALAR_WHITE, -1);

	int counter;
	cv::Mat bitwise;
	cv::Mat bwInt;

	cv::bitwise_and(zoneA, ctr, bitwise);
	cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
	counter = cv::countNonZero(bwInt);

	int highMatch = -1;
	int highMatchCounter = -1;

	if (counter > 0) {
		//std::cout << "Start: new blob * A\n";
		for (int i = 0; i < zoneAlot.size(); i++) {
			cv::bitwise_and(zoneAlot[i].image, ctr, bitwise);
			cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
			counter = cv::countNonZero(bwInt);

			if (counter > highMatchCounter) {
				highMatchCounter = counter;
				highMatch = i;
			}
		}
		//std::cout << "End: new blob * A\n";
		std::cout << "A: " << "highMatchCounter : " << highMatchCounter << ", " << "highMatch : " << highMatch << "zoneAlot[highMatch].countNZero * 0.6 : " << zoneAlot[highMatch].countNZero * 0.4 << "\n";
		if (highMatch >= 0 && highMatchCounter >= zoneAlot[highMatch].countNZero * 0.4) {
			currentFrameBlob.park = true;
			currentFrameBlob.enter = true;
			currentFrameBlob.parkLocation = 1;
			currentFrameBlob.parkinglot = zoneAlot[highMatch].lot;
			std::cout << "matching parked car : Lot A\n";
			check = true;
			keep_update = true;
			keep_update_counter = 0;
		}
	}
	else {
		//std::cout << "ggggggg\n";
		cv::bitwise_and(zoneB, ctr, bitwise);
		cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
		counter = cv::countNonZero(bwInt);

		if (counter > 0) {
			std::cout << "Start: new blob * B\n";
			for (int i = 0; i < zoneBlot.size(); i++) {
				cv::bitwise_and(zoneBlot[i].image, ctr, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counter = cv::countNonZero(bwInt);

				if (counter > highMatchCounter) {
					highMatchCounter = counter;
					highMatch = i;
				}
			}
			std::cout << "B: " << "highMatchCounter : " << highMatchCounter << ", " << "highMatch : " << highMatch << "zoneAlot[highMatch].countNZero * 0.6 : " << zoneBlot[highMatch].countNZero * 0.4 << "\n";
			//std::cout << "End: new blob * B\n";
			if (highMatch >= 0 && highMatchCounter >= zoneBlot[highMatch].countNZero * 0.4) {
				currentFrameBlob.park = true;
				currentFrameBlob.enter = true;
				currentFrameBlob.parkLocation = 2;
				currentFrameBlob.parkinglot = zoneBlot[highMatch].lot;
				std::cout << "matching parked car : Lot B\n";
				check = true;
				keep_update = true;
				keep_update_counter = 0;
			}
		}
		else {
			cv::bitwise_and(zoneC, ctr, bitwise);
			cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
			counter = cv::countNonZero(bwInt);

			if (counter > 0) {
				//std::cout << "Start: new blob * C\n";
				for (int i = 0; i < zoneClot.size(); i++) {
					cv::bitwise_and(zoneClot[i].image, ctr, bitwise);
					cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
					counter = cv::countNonZero(bwInt);

					if (counter > highMatchCounter) {
						highMatchCounter = counter;
						highMatch = i;
					}
				}
				std::cout << "C: " << "highMatchCounter : " << highMatchCounter << ", " << "highMatch : " << highMatch << "zoneAlot[highMatch].countNZero * 0.6 : " << zoneClot[highMatch].countNZero * 0.5 << "\n";
				//	std::cout << "End: new blob * C\n";
				if (highMatch >= 0 && highMatchCounter >= zoneClot[highMatch].countNZero * 0.4) {
					currentFrameBlob.park = true;
					currentFrameBlob.enter = true;
					currentFrameBlob.parkLocation = 3;
					currentFrameBlob.parkinglot = zoneClot[highMatch].lot;
					std::cout << "matching parked car : Lot C\n";
					check = true;
					keep_update = true;
					keep_update_counter = 0;
				}
			}
			else {
				cv::bitwise_and(zoneD, ctr, bitwise);
				cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
				counter = cv::countNonZero(bwInt);

				if (counter > 0) {
					//	std::cout << "Start: new blob * D\n";
					for (int i = 0; i < zoneDlot.size(); i++) {
						cv::bitwise_and(zoneDlot[i].image, ctr, bitwise);
						cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
						counter = cv::countNonZero(bwInt);

						if (counter > highMatchCounter) {
							highMatchCounter = counter;
							highMatch = i;
						}
					}
					//	std::cout << "End: new blob * D\n";
					std::cout << "D: " << "highMatchCounter : " << highMatchCounter << ", " << "highMatch : " << highMatch << "zoneAlot[highMatch].countNZero * 0.6 : " << zoneDlot[highMatch].countNZero * 0.5 << "\n";
					if (highMatch >= 0 && highMatchCounter >= zoneDlot[highMatch].countNZero * 0.4) {
						currentFrameBlob.park = true;
						currentFrameBlob.enter = true;
						currentFrameBlob.parkLocation = 4;
						currentFrameBlob.parkinglot = zoneDlot[highMatch].lot;
						std::cout << "matching parked car : Lot D\n";
						check = true;
						keep_update = true;
						keep_update_counter = 0;
					}
				}
				else {
					cv::bitwise_and(zoneE, ctr, bitwise);
					cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
					counter = cv::countNonZero(bwInt);

					if (counter > 0) {
						//	std::cout << "Start: new blob * E\n";
						for (int i = 0; i < zoneElot.size(); i++) {
							cv::bitwise_and(zoneElot[i].image, ctr, bitwise);
							cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
							counter = cv::countNonZero(bwInt);

							if (counter > highMatchCounter) {
								highMatchCounter = counter;
								highMatch = i;
							}
						}
						std::cout << "E: " << "highMatchCounter : " << highMatchCounter << ", " << "highMatch : " << highMatch << "zoneAlot[highMatch].countNZero * 0.6 : " << zoneElot[highMatch].countNZero * 0.5 << "\n";
						//std::cout << "End: new blob * E\n";
						if (highMatch >= 0 && highMatchCounter >= zoneElot[highMatch].countNZero * 0.4) {
							currentFrameBlob.park = true;
							currentFrameBlob.enter = true;
							currentFrameBlob.parkLocation = 5;
							currentFrameBlob.parkinglot = zoneElot[highMatch].lot;
							std::cout << "matching parked car : Lot E\n";
							check = true;
							keep_update = true;
							keep_update_counter = 0;
						}
					}
					else {
						cv::bitwise_and(dangerZone, ctr, bitwise);
						cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
						counter = cv::countNonZero(bwInt);

						if (counter > 0) {
							//	std::cout << "Start: new blob * F\n";
							currentFrameBlob.park = true;
							currentFrameBlob.enter = true;
							currentFrameBlob.parkLocation = 6;
							check = true;
							//	std::cout << "End: new blob * F\n";
						}
						else {
							//do nothing, just add
						}
					}
				}
			}
		}
	}


	contourVec.clear();
	ctr.release();
	bitwise.release();
	bwInt.release();

	if (check == true) {
		existingBlobs.push_back(currentFrameBlob);
	}
	return check;
}



double distanceBetweenPoints(cv::Point point1, cv::Point point2) {

	int intX = abs(point1.x - point2.x);
	int intY = abs(point1.y - point2.y);

	return(sqrt(pow(intX, 2) + pow(intY, 2)));
}

void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName) {
	cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

	cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);

	if (imshow_display)
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

	if (imshow_display)
		cv::imshow(strImageName, image);
}

bool checkIfBlobsCrossedTheLine(std::vector<Blob> &blobs, int &intHorizontalLinePosition, int &carCount, CarParkTrackExporter &openDB, int &frameCount, int &vidLength, std::vector<ColorTerm> &colorsList) {


	std::vector<Blob> tempBb;
	for (int r = 0; r < blobs.size(); r++) {
		tempBb.push_back(blobs[r]);
	}

	bool blnAtLeastOneBlobCrossedTheLine = false;

	cv::Rect tempBoundingRect;
	cv::Mat cropImage;
	IplImage* to_crop;
	int crop_result;


	//for (auto blob : blobs) {
	for (unsigned int i = 0; i < blobs.size(); i++) {

		std::vector<std::vector<cv::Point> > contourVec;
		contourVec.push_back(blobs[i].currentContour);
		cv::Mat ctr(entrance1.size(), CV_8UC3, SCALAR_BLACK);
		cv::drawContours(ctr, contourVec, -1, SCALAR_WHITE, -1);


		int counter;
		cv::Mat bitwise;
		cv::Mat bwInt;





		if (blobs[i].blnStillBeingTracked == true) {
			//std::cout << "(vehicle Status) start: 1\n";
			if (blobs[i].park == false && blobs[i].centerPositions.size() >= 4) {
				//int prevprevprevprevFrameIndex = (int)blobs[i].centerPositions.size() - 5;
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



						tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
						cropImage = global_img(tempBoundingRect);
						to_crop = mat_to_iplimage(cropImage);
						crop_result = predict_image_c(to_crop);
						std::cout << "prediction: " << crop_result << "\n";
						//system("pause");

						if (crop_result == 1) {


							printNumberofCar(1, true);
							blobs[i].enter = true;
							blobs[i].entExt = 1;
							blobs[i].changed = true;
							blobs[i].IO_indicator = true;
						}


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
						/*if (debug_on)
						{
							std::cout << "Blob stats: " << blobs[i].unitID << " color: " << blobs[i].ColorInTerms << std::endl;
							std::cout << "  " << " [Diff min/max]: " << blobs[i].RGBGRAYDiff_min << "/" << blobs[i].RGBGRAYDiff_max << " | " << blobs[i].RGBGRAYDiff_max - blobs[i].RGBGRAYDiff_min << std::endl;
							std::cout << "  " << " [Mean min/max]: " << blobs[i].meanValue_min << "/" << blobs[i].meanValue_max << " | " << blobs[i].meanValue_max - blobs[i].meanValue_min << std::endl;
							std::cout << "  " << " [ L2  min/max]: " << blobs[i].L2error_min << "/" << blobs[i].L2error_max <<  " | " << blobs[i].L2error_max  - blobs[i].L2error_min << std::endl;
						}*/




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
							) {


							tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
							cropImage = global_img(tempBoundingRect);
							to_crop = mat_to_iplimage(cropImage);
							crop_result = predict_image_c(to_crop);
							std::cout << "prediction: " << crop_result << "\n";
							//system("pause");

							if (crop_result == 1) {
								printNumberofCar(2, true);
								blobs[i].enter = true;
								blobs[i].entExt = 2;
								blobs[i].changed = true;
								blobs[i].IO_indicator = true;
							}

						}
						else if (blobs[i].centerPositions[prevFrameIndex].y > blobs[i].centerPositions[currFrameIndex].y
							&& blobs[i].enter == true
							&& blobs[i].exit == false
							&& blobs[i].centerPositions[prevprevFrameIndex].y > blobs[i].centerPositions[prevFrameIndex].y
							&& blobs[i].centerPositions[prevprevprevFrameIndex].y > blobs[i].centerPositions[prevprevFrameIndex].y
							) {



							printNumberofCar(2, false);
							blobs[i].exit = true;
							blobs[i].entExt = 2;
							blobs[i].changed = true;
							blobs[i].IO_indicator = false;
							/*if (debug_on)
							{
								std::cout << "Blob stats: " << blobs[i].unitID << " color: " << blobs[i].ColorInTerms << std::endl;
								std::cout << "  " << " [Diff min/max]: " << blobs[i].RGBGRAYDiff_min << "/" << blobs[i].RGBGRAYDiff_max << " | " << blobs[i].RGBGRAYDiff_max - blobs[i].RGBGRAYDiff_min << std::endl;
								std::cout << "  " << " [Mean min/max]: " << blobs[i].meanValue_min << "/" << blobs[i].meanValue_max << " | " << blobs[i].meanValue_max - blobs[i].meanValue_min << std::endl;
								std::cout << "  " << " [ L2  min/max]: " << blobs[i].L2error_min << "/" << blobs[i].L2error_max << " | " << blobs[i].L2error_max - blobs[i].L2error_min << std::endl;
							}*/

						}
					}
					else {

						cv::bitwise_and(entrance6, ctr, bitwise);
						cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
						counter = cv::countNonZero(bwInt);

						if (counter > 0) {
							//std::cout << "Entres 33333333333\n";
							//std::cout << blobs[i].centerPositions.back().x << ", " << blobs[i].centerPositions.back().y << "\n";
							if (blobs[i].centerPositions[prevFrameIndex].x < blobs[i].centerPositions[currFrameIndex].x
								&& blobs[i].enter == false
								&& blobs[i].centerPositions[prevprevFrameIndex].x < blobs[i].centerPositions[prevFrameIndex].x
								&& blobs[i].centerPositions[prevprevprevFrameIndex].x < blobs[i].centerPositions[prevprevFrameIndex].x
								) {

								/*tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
								cropImage = global_img(tempBoundingRect);
								to_crop = mat_to_iplimage(cropImage);
								crop_result = predict_image_c(to_crop);
								std::cout << "prediction: " << crop_result << "\n";*/
								//system("pause");


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
								) {



								printNumberofCar(3, false);
								blobs[i].exit = true;
								blobs[i].entExt = 3;
								blobs[i].changed = true;
								blobs[i].IO_indicator = false;
								/*if (debug_on)
								{
									std::cout << "Blob stats: " << blobs[i].unitID << " color: " << blobs[i].ColorInTerms << std::endl;
									std::cout << "  " << " [Diff min/max]: " << blobs[i].RGBGRAYDiff_min << "/" << blobs[i].RGBGRAYDiff_max << " | " << blobs[i].RGBGRAYDiff_max - blobs[i].RGBGRAYDiff_min << std::endl;
									std::cout << "  " << " [Mean min/max]: " << blobs[i].meanValue_min << "/" << blobs[i].meanValue_max << " | " << blobs[i].meanValue_max - blobs[i].meanValue_min << std::endl;
									std::cout << "  " << " [ L2  min/max]: " << blobs[i].L2error_min << "/" << blobs[i].L2error_max << " | " << blobs[i].L2error_max - blobs[i].L2error_min << std::endl;
								}*/

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
									) {

									tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
									cropImage = global_img(tempBoundingRect);
									to_crop = mat_to_iplimage(cropImage);
									crop_result = predict_image_c(to_crop);
									std::cout << "prediction: " << crop_result << "\n";
									//system("pause");

									if (crop_result == 1) {
										printNumberofCar(4, true);
										blobs[i].enter = true;
										blobs[i].entExt = 4;
										blobs[i].changed = true;
										blobs[i].IO_indicator = true;
									}
								}
								else if (blobs[i].centerPositions[prevFrameIndex].y < blobs[i].centerPositions[currFrameIndex].y
									&& blobs[i].enter == true
									&& blobs[i].exit == false
									&& blobs[i].centerPositions[prevprevFrameIndex].y < blobs[i].centerPositions[prevFrameIndex].y
									&& blobs[i].centerPositions[prevprevprevFrameIndex].y < blobs[i].centerPositions[prevprevFrameIndex].y
									) {



									printNumberofCar(4, false);
									blobs[i].exit = true;
									blobs[i].enter = false;
									blobs[i].entExt = 4;
									blobs[i].changed = true;
									blobs[i].IO_indicator = false;
									/*if (debug_on)
									{
										std::cout << "Blob stats: " << blobs[i].unitID << " color: " << blobs[i].ColorInTerms << std::endl;
										std::cout << "  " << " [Diff min/max]: " << blobs[i].RGBGRAYDiff_min << "/" << blobs[i].RGBGRAYDiff_max << " | " << blobs[i].RGBGRAYDiff_max - blobs[i].RGBGRAYDiff_min << std::endl;
										std::cout << "  " << " [Mean min/max]: " << blobs[i].meanValue_min << "/" << blobs[i].meanValue_max << " | " << blobs[i].meanValue_max - blobs[i].meanValue_min << std::endl;
										std::cout << "  " << " [ L2  min/max]: " << blobs[i].L2error_min << "/" << blobs[i].L2error_max << " | " << blobs[i].L2error_max - blobs[i].L2error_min << std::endl;
									}*/

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
										) {

										tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
										cropImage = global_img(tempBoundingRect);
										to_crop = mat_to_iplimage(cropImage);
										crop_result = predict_image_c(to_crop);
										std::cout << "prediction: " << crop_result << "\n";
										//system("pause");

										if (crop_result == 1) {

											printNumberofCar(5, true);
											blobs[i].enter = true;
											blobs[i].exit = false;
											blobs[i].entExt = 5;
											blobs[i].changed = true;
											blobs[i].IO_indicator = true;
										}
									}
									else if (blobs[i].centerPositions[prevFrameIndex].x < blobs[i].centerPositions[currFrameIndex].x
										&& blobs[i].enter == true
										&& blobs[i].exit == false
										&& blobs[i].centerPositions[prevprevFrameIndex].x < blobs[i].centerPositions[prevFrameIndex].x
										&& blobs[i].centerPositions[prevprevprevFrameIndex].x < blobs[i].centerPositions[prevprevFrameIndex].x
										) {



										printNumberofCar(5, false);

										blobs[i].exit = true;
										blobs[i].entExt = 5;
										blobs[i].changed = true;
										blobs[i].IO_indicator = false;
										/*if (debug_on)
										{
											std::cout << "Blob stats: " << blobs[i].unitID << " color: " << blobs[i].ColorInTerms << std::endl;
											std::cout << "  " << " [Diff min/max]: " << blobs[i].RGBGRAYDiff_min << "/" << blobs[i].RGBGRAYDiff_max << " | " << blobs[i].RGBGRAYDiff_max - blobs[i].RGBGRAYDiff_min << std::endl;
											std::cout << "  " << " [Mean min/max]: " << blobs[i].meanValue_min << "/" << blobs[i].meanValue_max << " | " << blobs[i].meanValue_max - blobs[i].meanValue_min << std::endl;
											std::cout << "  " << " [ L2  min/max]: " << blobs[i].L2error_min << "/" << blobs[i].L2error_max << " | " << blobs[i].L2error_max - blobs[i].L2error_min << std::endl;
										}*/

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
											) {

											tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
											cropImage = global_img(tempBoundingRect);
											to_crop = mat_to_iplimage(cropImage);
											crop_result = predict_image_c(to_crop);
											std::cout << "prediction: " << crop_result << "\n";
											//system("pause");

											if (crop_result == 1) {
												printNumberofCar(6, true);
												blobs[i].enter = true;
												blobs[i].entExt = 6;
												blobs[i].changed = true;
												blobs[i].IO_indicator = true;
											}

										}
										else if (blobs[i].centerPositions[prevFrameIndex].x > blobs[i].centerPositions[currFrameIndex].x
											&& blobs[i].enter == true
											&& blobs[i].exit == false
											&& blobs[i].centerPositions[prevprevFrameIndex].x > blobs[i].centerPositions[prevFrameIndex].x
											&& blobs[i].centerPositions[prevprevprevFrameIndex].x > blobs[i].centerPositions[prevprevFrameIndex].x
											) {




											printNumberofCar(6, false);

											blobs[i].exit = true;
											blobs[i].entExt = 6;
											blobs[i].changed = true;
											blobs[i].IO_indicator = false;
											/*if (debug_on)
											{
												std::cout << "Blob stats: " << blobs[i].unitID << " color: " << blobs[i].ColorInTerms << std::endl;
												std::cout << "  " << " [Diff min/max]: " << blobs[i].RGBGRAYDiff_min << "/" << blobs[i].RGBGRAYDiff_max << " | " << blobs[i].RGBGRAYDiff_max - blobs[i].RGBGRAYDiff_min << std::endl;
												std::cout << "  " << " [Mean min/max]: " << blobs[i].meanValue_min << "/" << blobs[i].meanValue_max << " | " << blobs[i].meanValue_max - blobs[i].meanValue_min << std::endl;
												std::cout << "  " << " [ L2  min/max]: " << blobs[i].L2error_min << "/" << blobs[i].L2error_max << " | " << blobs[i].L2error_max - blobs[i].L2error_min << std::endl;
											}*/


										}
									}
									else {
										cv::bitwise_and(nonTrackZone2, ctr, bitwise);
										cv::cvtColor(bitwise, bwInt, cv::COLOR_BGR2GRAY);
										counter = cv::countNonZero(bwInt);


										//if (counter > 0 && blobs[i].centerPositions.size() >= 6) {
										if (counter > 0 && blobs[i].centerPositions.size() >= 5) {

											int prev5 = (int)blobs[i].centerPositions.size() - 5;
											//int prev6 = (int)blobs[i].centerPositions.size() - 6;
											//int prev7 = (int)blobs[i].centerPositions.size() - 7;
											//int prev8 = (int)blobs[i].centerPositions.size() - 8;
											//int prev9 = (int)blobs[i].centerPositions.size() - 9;


											if (blobs[i].centerPositions[prevFrameIndex].y < blobs[i].centerPositions[currFrameIndex].y
												&& blobs[i].centerPositions[prevprevFrameIndex].y < blobs[i].centerPositions[prevFrameIndex].y
												&& blobs[i].centerPositions[prevprevprevFrameIndex].y < blobs[i].centerPositions[prevprevFrameIndex].y
												&& blobs[i].centerPositions[prev5].y < blobs[i].centerPositions[prevprevprevFrameIndex].y
												//&& blobs[i].centerPositions[prev6].y < blobs[i].centerPositions[prev5].y
												//&& blobs[i].centerPositions[prev7].y < blobs[i].centerPositions[prev6].y
												//&& blobs[i].centerPositions[prev8].y < blobs[i].centerPositions[prev7].y
												//&& blobs[i].centerPositions[prev9].y < blobs[i].centerPositions[prev8].y
												&& blobs[i].exit == false && blobs[i].nonTrackParkingZone == false && blobs[i].unitID != 0) {


												tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
												cropImage = global_img(tempBoundingRect);
												to_crop = mat_to_iplimage(cropImage);
												crop_result = predict_image_c(to_crop);
												std::cout << "prediction: " << crop_result << "\n";
												//system("pause");

												if (crop_result == 1) {


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


											}
											else if (blobs[i].centerPositions[prevFrameIndex].y > blobs[i].centerPositions[currFrameIndex].y
												&& blobs[i].centerPositions[prevprevFrameIndex].y > blobs[i].centerPositions[prevFrameIndex].y
												&& blobs[i].centerPositions[prevprevprevFrameIndex].y > blobs[i].centerPositions[prevprevFrameIndex].y

												&& blobs[i].centerPositions[prev5].y > blobs[i].centerPositions[prevprevprevFrameIndex].y
												//&& blobs[i].centerPositions[prev6].y < blobs[i].centerPositions[prev5].y
												//&& blobs[i].centerPositions[prev7].y > blobs[i].centerPositions[prev6].y
												//&& blobs[i].centerPositions[prev8].y > blobs[i].centerPositions[prev7].y
												//&& blobs[i].centerPositions[prev9].y > blobs[i].centerPositions[prev8].y

												&& blobs[i].leavingNonTrackzone == false
												&& blobs[i].unitID == 0) {



												tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
												cropImage = global_img(tempBoundingRect);
												to_crop = mat_to_iplimage(cropImage);
												crop_result = predict_image_c(to_crop);
												std::cout << "prediction: " << crop_result << "\n";
												//system("pause");

												if (crop_result == 1) {

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


													tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
													cropImage = global_img(tempBoundingRect);
													to_crop = mat_to_iplimage(cropImage);
													crop_result = predict_image_c(to_crop);
													std::cout << "prediction: " << crop_result << "\n";
													//system("pause");




													if (crop_result == 1) {
														if (blobs[i].unitID != 0) {

															blobs[i].nonTrackParkingZone = true;
															blobs[i].nonTrackParkingZoneLocation = 1;
															blobs[i].enter = true;
															blobs[i].leavingNonTrackzone = false;

														}
														else {
															int highIndexx = -1;
															double nerestDis = 1000000;



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

																missMatchBlob[highIndexx].nonTrackParkingZone = true;
																missMatchBlob[highIndexx].nonTrackParkingZoneLocation = 1;
																missMatchBlob[highIndexx].enter = true;
																missMatchBlob[highIndexx].leavingNonTrackzone = false;
																addBlobToExistingBlobsMissMatch(blobs[i], missMatchBlob, highIndexx, colorsList);
																missMatchBlob[highIndexx].matchBack = true;
																missMatchBlob[highIndexx].matchbackid = i;

															}
														}


														std::cout << "vehicle " << blobs[i].unitID << " : non tracking zone 1\n";








														//blobs[i].entExt = 999;
														//blobs[i].changed = true;
														//blobs[i].IO_indicator = true;
														//temporary set park to true to write into database
														blobs[i].park = true;
														openDB.writeToDB_park(blobs, i, frameCount, vidLength, "NTZ 1");
														blobs[i].park = false;
													}

												}
												else if (blobs[i].centerPositions[prevFrameIndex].x > blobs[i].centerPositions[currFrameIndex].x
													&& blobs[i].centerPositions[prevprevFrameIndex].x > blobs[i].centerPositions[prevFrameIndex].x
													&& blobs[i].centerPositions[prevprevprevFrameIndex].x > blobs[i].centerPositions[prevFrameIndex].x

													//&& blobs[i].centerPositions[prev5].x > blobs[i].centerPositions[prevprevprevFrameIndex].x
													//&& blobs[i].centerPositions[prev6].x > blobs[i].centerPositions[prev5].x
													//	&& blobs[i].centerPositions[prev7].x > blobs[i].centerPositions[prev6].x
													//&& blobs[i].centerPositions[prev8].x > blobs[i].centerPositions[prev7].x
													//&& blobs[i].centerPositions[prev9].x > blobs[i].centerPositions[prev8].x

													&& blobs[i].leavingNonTrackzone == false
													&& blobs[i].unitID == 0) {


													tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
													cropImage = global_img(tempBoundingRect);
													to_crop = mat_to_iplimage(cropImage);
													crop_result = predict_image_c(to_crop);
													std::cout << "prediction: " << crop_result << "\n";
													//system("pause");

													if (crop_result == 1) {

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
			}
			//	std::cout << "(vehicle Status) start: 2\n";
			if (blobs[i].exit == false && blobs[i].park == false) {
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
			//	std::cout << "(vehicle Status) start: 3\n";
			if (blobs[i].parkframe > 20 && blobs[i].unitID == 0 && blobs[i].park == false) {

				if (blobs[i].previous_park_frame_counter > 9) {
					blobs[i].intNumOfConsecutiveFramesWithoutAMatch = 30;

				}
				else {
					if (blobs[i].previous_park_frame == blobs[i].parkframe) {
						blobs[i].previous_park_frame_counter++;
					}
					blobs[i].previous_park_frame = blobs[i].parkframe;
					blobs[i].intNumOfConsecutiveFramesWithoutAMatch = 0;
				}


				//std::cout << "I hate this : " << blobs[i].parkframe <<  "\n";
				//	blobs[i].enter == true;
			}
			//	std::cout << "(vehicle Status) start: 4\n";
			if (blobs[i].parkframe > param_G_parkframe && blobs[i].park == false) {

				//std::cout << "11\n";
				int a = blobs[i].parkLocation;
				if (blobs[i].unitID == 0 && blobs[i].parkLocation != 6) {

					tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
					cropImage = global_img(tempBoundingRect);
					to_crop = mat_to_iplimage(cropImage);
					crop_result = predict_image_c(to_crop);
					std::cout << "prediction: " << crop_result << "\n";
					//system("pause");

					if (crop_result == 1) {


						//	std::cout << "22\n";
						int highIndexx = -1;
						double nerestDis = 1000000;
						int lenght = missMatchBlob.size();
						if (lenght == 0) {
							//	std::cout << "33\n";
							//	blobs[i].park = true;
							blobs[i].enter = true;
							//std::cout << "sdsdsdsd: " << blobs[i].parkLocation << "\n";



						}
						else {
							//std::cout << "44\n";
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
							//std::cout << "55\n";
							if (highIndexx != -1) {
								//	std::cout << "66\n";
								missMatchBlob[highIndexx].parkLocation = blobs[i].parkLocation;
								missMatchBlob[highIndexx].parkinglot = blobs[i].parkinglot;
								missMatchBlob[highIndexx].parkframe = param_G_parkframe;
								addBlobToExistingBlobsMissMatch(blobs[i], missMatchBlob, highIndexx, colorsList);
								missMatchBlob[highIndexx].matchBack = true;
								missMatchBlob[highIndexx].matchbackid = i;
								//std::cout << "77\n";
							}

							else {

								//	blobs[i].park = true;
								//	std::cout << "Enter True\n";
								blobs[i].enter = true;
								//std::cout << "88\n";

							}
						}
					}
					else {
						blobs[i].parkframe = 0;
						blobs[i].blnStillBeingTracked = false;
					}

				}
				else if (blobs[i].unitID == 0 && blobs[i].parkLocation == 6) {

					blobs[i].parkframe = 0;
					//std::cout << "99\n";
				}
				else {
					if (a == 1 && blobs[i].park == false) {
						//std::cout << "park a\n";


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

						if (indexOflot >= 0) {
							//std::cout << "here a: " << blobs[i].unitID << "\n";
							if (zoneAlot[indexOflot].parked == false) {


								tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
								cropImage = global_img(tempBoundingRect);
								to_crop = mat_to_iplimage(cropImage);
								crop_result = predict_image_c(to_crop);
								std::cout << "prediction: " << crop_result << "\n";
								//system("pause");

								if (crop_result == 1) {
									blobs[i].parkinglot = zoneAlot[indexOflot].lot;
									zoneAlot[indexOflot].parked = true;
									zoneAlot[indexOflot].vehicleId = blobs[i].unitID;
									std::cout << "Park - Vehicle " << blobs[i].unitID << "Lot A" << blobs[i].parkinglot << "\n";

									blobs[i].park = true;
									if (blobs[i].unitID == 0) {
										blobs[i].unitID = unitObjCounter;
										unitObjCounter++;
									}
									openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot A");
								}
								else {
									blobs[i].blnStillBeingTracked = false;
								}







							}
							else {

								blobs[i].parkframe = 0;
							}
						}



					}
					else if (a == 2 && blobs[i].park == false) {
						//std::cout << "park b\n";
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
						if (indexOflot >= 0) {
							if (zoneBlot[indexOflot].parked == false) {

								tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
								cropImage = global_img(tempBoundingRect);
								to_crop = mat_to_iplimage(cropImage);
								crop_result = predict_image_c(to_crop);
								std::cout << "prediction: " << crop_result << "\n";
								//system("pause");

								if (crop_result == 1) {


									blobs[i].parkinglot = zoneBlot[indexOflot].lot;
									zoneBlot[indexOflot].parked = true;
									zoneBlot[indexOflot].vehicleId = blobs[i].unitID;
									std::cout << "Park - Vehicle " << blobs[i].unitID << "Lot B" << blobs[i].parkinglot << "\n";

									blobs[i].park = true;
									if (blobs[i].unitID == 0) {
										blobs[i].unitID = unitObjCounter;
										unitObjCounter++;
									}
									openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot B");
								}
								else {
									blobs[i].blnStillBeingTracked = false;
								}
							}
							else {
								blobs[i].parkframe = 0;
							}
						}
					}
					else if (a == 3 && blobs[i].park == false) {
						//std::cout << "park c\n";
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
						//std::cout << "indexOflot : " << indexOflot << "\n";
						if (indexOflot >= 0) {
							if (zoneClot[indexOflot].parked == false) {

								tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
								cropImage = global_img(tempBoundingRect);
								to_crop = mat_to_iplimage(cropImage);
								crop_result = predict_image_c(to_crop);
								std::cout << "prediction: " << crop_result << "\n";
								//system("pause");

								if (crop_result == 1) {

									blobs[i].parkinglot = zoneClot[indexOflot].lot;
									zoneClot[indexOflot].parked = true;
									zoneClot[indexOflot].vehicleId = blobs[i].unitID;
									std::cout << "Park - Vehicle " << blobs[i].unitID << "Lot C" << blobs[i].parkinglot << "\n";

									blobs[i].park = true;
									if (blobs[i].unitID == 0) {
										blobs[i].unitID = unitObjCounter;
										unitObjCounter++;
									}
									openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot C");
								}
								else {
									blobs[i].blnStillBeingTracked = false;
								}
							}
							else {
								blobs[i].parkframe = 0;
							}
						}

					}
					else if (a == 4 && blobs[i].park == false) {
						//std::cout << "park d\n";
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
						if (indexOflot >= 0) {
							if (zoneDlot[indexOflot].parked == false) {


								//tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
								//cropImage = global_img(tempBoundingRect);
								//to_crop = mat_to_iplimage(cropImage);
								//crop_result = predict_image_c(to_crop);
								//std::cout << "prediction: " << crop_result << "\n";
								////system("pause");

								//if (crop_result == 1) {

								blobs[i].parkinglot = zoneDlot[indexOflot].lot;

								zoneDlot[indexOflot].parked = true;
								zoneDlot[indexOflot].vehicleId = blobs[i].unitID;


								std::cout << "Park - Vehicle " << blobs[i].unitID << "Lot D" << blobs[i].parkinglot << "\n";

								blobs[i].park = true;
								if (blobs[i].unitID == 0) {
									blobs[i].unitID = unitObjCounter;
									unitObjCounter++;
								}
								openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot D");
								/*}
								else {*/
								blobs[i].blnStillBeingTracked = false;
								//}
							}
							else {
								blobs[i].parkframe = 0;
							}
						}
					}
					else if (a == 5 && blobs[i].park == false) {
						std::cout << "park e\n";
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
						if (indexOflot >= 0) {
							if (zoneElot[indexOflot].parked == false) {
								//tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
								//cropImage = global_img(tempBoundingRect);
								//to_crop = mat_to_iplimage(cropImage);
								//crop_result = predict_image_c(to_crop);
								//std::cout << "prediction: " << crop_result << "\n";
								////system("pause");

								//if (crop_result == 1) {
								blobs[i].parkinglot = zoneElot[indexOflot].lot;
								zoneElot[indexOflot].parked = true;
								zoneElot[indexOflot].vehicleId = blobs[i].unitID;



								std::cout << "Park - Vehicle " << blobs[i].unitID << "Lot E" << blobs[i].parkinglot << "\n";

								blobs[i].park = true;
								if (blobs[i].unitID == 0) {
									blobs[i].unitID = unitObjCounter;
									unitObjCounter++;
								}
								openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot E");
								/*}
								else {*/
								blobs[i].blnStillBeingTracked = false;
								//}
							}
							else {
								blobs[i].parkframe = 0;
							}
						}
					}
					else if (a == 6 && blobs[i].park == false) {
						blobs[i].parkframe++;
						if (blobs[i].parkframe > param_G_DangerParkFrame) {
							tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
							cropImage = global_img(tempBoundingRect);
							to_crop = mat_to_iplimage(cropImage);
							crop_result = predict_image_c(to_crop);
							std::cout << "prediction: " << crop_result << "\n";
							//system("pause");

							if (crop_result == 1) {
								std::cout << "Park - Vehicle " << blobs[i].unitID << "DANGER ZONE!\n";
								blobs[i].park = true;
								if (blobs[i].unitID == 0) {
									blobs[i].unitID = unitObjCounter;
									unitObjCounter++;
								}
								openDB.writeToDB_park(blobs, i, frameCount, vidLength, "DANGER ZONE!");
							}
							else {
								blobs[i].blnStillBeingTracked = false;
							}
						}
					}

					//std::cout << "1010\n";
				}



				//std::cout << "1111\n";
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
				//std::cout << "1212\n";


			}
			//std::cout << "(vehicle Status) start: 5\n";
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
					//std::cout << blobs[i].unitID << ": " << blobs[i].leavingcarpark << "\n";

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
			//std::cout << "(vehicle Status) start: 6\n";
			if (blobs[i].leavingcarpark > param_G_LeavingParking) {
				bool predict_true_false = false;
				if (blobs[i].parkLocation == 1) {


					tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
					cropImage = global_img(tempBoundingRect);
					to_crop = mat_to_iplimage(cropImage);
					crop_result = predict_image_c(to_crop);
					std::cout << "prediction: " << crop_result << "\n";
					//system("pause");

					if (crop_result == 1) {

						if (zoneAlot[blobs[i].parkinglot - 1].parked == true) {
							zoneAlot[blobs[i].parkinglot - 1].parked = false;
							blobs[i].unitID = zoneAlot[blobs[i].parkinglot - 1].vehicleId;
						}
						else if (zoneAlot[blobs[i].parkinglot - 1].parked == false) {
							blobs[i].unitID = unitObjCounter;
							unitObjCounter++;
						}

						std::cout << "Leaving car park - Vehicle " << blobs[i].unitID << "Lot A" << blobs[i].parkinglot << "\n";
						blobs[i].park = false;
						openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot A");
						predict_true_false = true;
					}
					else {
						blobs[i].blnStillBeingTracked = false;
					}
				}
				else if (blobs[i].parkLocation == 2) {
					tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
					cropImage = global_img(tempBoundingRect);
					to_crop = mat_to_iplimage(cropImage);
					crop_result = predict_image_c(to_crop);
					std::cout << "prediction: " << crop_result << "\n";
					//system("pause");

					if (crop_result == 1) {

						if (zoneBlot[blobs[i].parkinglot - 1].parked == true) {
							zoneBlot[blobs[i].parkinglot - 1].parked = false;
							blobs[i].unitID = zoneBlot[blobs[i].parkinglot - 1].vehicleId;
						}
						else if (zoneBlot[blobs[i].parkinglot - 1].parked == false) {
							blobs[i].unitID = unitObjCounter;
							unitObjCounter++;
						}
						std::cout << "Leaving car park - Vehicle " << blobs[i].unitID << "Lot B" << blobs[i].parkinglot << "\n";
						blobs[i].park = false;
						openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot B");
						predict_true_false = true;
					}
					else {
						blobs[i].blnStillBeingTracked = false;
					}
				}
				else if (blobs[i].parkLocation == 3) {
					tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
					cropImage = global_img(tempBoundingRect);
					to_crop = mat_to_iplimage(cropImage);
					crop_result = predict_image_c(to_crop);
					std::cout << "prediction: " << crop_result << "\n";
					//system("pause");

					if (crop_result == 1) {
						if (zoneClot[blobs[i].parkinglot - 1].parked == true) {
							zoneClot[blobs[i].parkinglot - 1].parked = false;
							blobs[i].unitID = zoneClot[blobs[i].parkinglot - 1].vehicleId;
						}
						else if (zoneClot[blobs[i].parkinglot - 1].parked == false) {
							blobs[i].unitID = unitObjCounter;
							unitObjCounter++;
						}
						std::cout << "Leaving car park - Vehicle " << blobs[i].unitID << "Lot C" << blobs[i].parkinglot << "\n";
						blobs[i].park = false;
						openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot C");
						predict_true_false = true;
					}
					else {
						blobs[i].blnStillBeingTracked = false;
					}
				}
				else if (blobs[i].parkLocation == 4) {
					tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
					cropImage = global_img(tempBoundingRect);
					to_crop = mat_to_iplimage(cropImage);
					crop_result = predict_image_c(to_crop);
					std::cout << "prediction: " << crop_result << "\n";
					//system("pause");

					if (crop_result == 1) {
						if (zoneDlot[blobs[i].parkinglot - 1].parked == true) {
							zoneDlot[blobs[i].parkinglot - 1].parked = false;
							blobs[i].unitID = zoneDlot[blobs[i].parkinglot - 1].vehicleId;
						}
						else if (zoneDlot[blobs[i].parkinglot - 1].parked == false) {
							blobs[i].unitID = unitObjCounter;
							unitObjCounter++;
						}

						std::cout << "Leaving car park - Vehicle " << blobs[i].unitID << "Lot D" << blobs[i].parkinglot << "\n";
						blobs[i].park = false;
						openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot D");
						predict_true_false = true;
					}
					else {
						blobs[i].blnStillBeingTracked = false;
					}
				}
				else if (blobs[i].parkLocation == 5) {
					tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
					cropImage = global_img(tempBoundingRect);
					to_crop = mat_to_iplimage(cropImage);
					crop_result = predict_image_c(to_crop);
					std::cout << "prediction: " << crop_result << "\n";
					//system("pause");

					if (crop_result == 1) {
						if (zoneElot[blobs[i].parkinglot - 1].parked == true) {
							zoneElot[blobs[i].parkinglot - 1].parked = false;
							blobs[i].unitID = zoneElot[blobs[i].parkinglot - 1].vehicleId;
						}
						else if (zoneElot[blobs[i].parkinglot - 1].parked == false) {
							blobs[i].unitID = unitObjCounter;
							unitObjCounter++;
						}

						std::cout << "Leaving car park - Vehicle " << blobs[i].unitID << "Lot E" << blobs[i].parkinglot << "\n";
						blobs[i].park = false;
						openDB.writeToDB_park(blobs, i, frameCount, vidLength, "Lot E");
						predict_true_false = true;
					}
					else {
						blobs[i].blnStillBeingTracked = false;
					}
				}
				else if (blobs[i].parkLocation == 6) {
					tempBoundingRect = enlargeROI(global_img, blobs[i].currentBoundingRect, 10);
					cropImage = global_img(tempBoundingRect);
					to_crop = mat_to_iplimage(cropImage);
					crop_result = predict_image_c(to_crop);
					std::cout << "prediction: " << crop_result << "\n";
					//system("pause");

					if (crop_result == 1) {

						std::cout << "Leaving car park - Vehicle " << blobs[i].unitID << "DANGER ZONE" << blobs[i].parkinglot << "\n";
						blobs[i].park = false;
						openDB.writeToDB_park(blobs, i, frameCount, vidLength, "DANGER ZONE");
						predict_true_false = true;
					}
					else {
						blobs[i].blnStillBeingTracked = false;
					}
				}

				if (predict_true_false == true) {
					blobs[i].park = false;
					blobs[i].parkframe = 0;
					blobs[i].parkLocation = 0;
					blobs[i].parkinglot = -1;
					blobs[i].leavingcarpark = 0;
				}

			}

			//std::cout << "(vehicle Status) start: 7\n";


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


void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy, CarParkTrackExporter &openDB, int &frameCount, int &vidLength) {

	for (unsigned int i = 0; i < blobs.size(); i++) {


		if (blobs[i].blnStillBeingTracked == true) {


			//clarencetest

			cv::Rect tempBoundingRect = blobs[i].currentBoundingRect;
			cv::Size inflationSize(blobs[i].currentBoundingRect.width*0.8, blobs[i].currentBoundingRect.height*0.8);

			tempBoundingRect -= inflationSize;
			tempBoundingRect.x += inflationSize.width / 2;
			tempBoundingRect.y += inflationSize.height / 2;



			cv::Point currentcenter = blobs[i].centerPositions.back();


			if (blobs[i].motion == "up")
			{
				currentcenter.x = currentcenter.x;
				currentcenter.y = abs(currentcenter.y - inflationSize.height / 2);
			}
			else if (blobs[i].motion == "down")
			{
				currentcenter.x = currentcenter.x;
				currentcenter.y = abs(currentcenter.y + inflationSize.height / 2);
			}
			else if (blobs[i].motion == "left")
			{
				currentcenter.x = abs(currentcenter.x - inflationSize.width / 2);
				currentcenter.y = currentcenter.y;
			}
			else if (blobs[i].motion == "right")
			{
				currentcenter.x = abs(currentcenter.x + inflationSize.width / 2);
				currentcenter.y = currentcenter.y;
			}
			else if (blobs[i].motion == "left-up")
			{
				currentcenter.x = abs(currentcenter.x - inflationSize.width / 2);
				currentcenter.y = abs(currentcenter.y - inflationSize.height / 2);
			}
			else if (blobs[i].motion == "right-up")
			{
				currentcenter.x = abs(currentcenter.x + inflationSize.width / 2);
				currentcenter.y = abs(currentcenter.y - inflationSize.height / 2);
			}
			else if (blobs[i].motion == "left-down")
			{
				currentcenter.x = abs(currentcenter.x - inflationSize.width / 2);
				currentcenter.y = abs(currentcenter.y + inflationSize.height / 2);
			}
			else if (blobs[i].motion == "right-down")
			{
				currentcenter.x = abs(currentcenter.x + inflationSize.width / 2);
				currentcenter.y = abs(currentcenter.y + inflationSize.height / 2);
			}
			else
			{

			}



			//clarencetest end




			int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
			double dblFontScale = 1.0;
			int intFontThickness = (int)std::round(dblFontScale * 1.0);

			if (blobs[i].mergeid != 0) {
				if (blobs[i].mergeid < blobs[i].unitID) {
					cv::rectangle(imgFrame2Copy, blobs[i].currentBoundingRect, SCALAR_RED, 2);
					//test
					//cv::rectangle(imgFrame2Copy, tempBoundingRect, blobs[i].AvgColorScalar, 2);
					cv::arrowedLine(imgFrame2Copy, blobs[i].centerPositions.back(), currentcenter, SCALAR_BLACK, 5, CV_AA, 0, 0.3);
					cv::arrowedLine(imgFrame2Copy, blobs[i].centerPositions.back(), currentcenter, blobs[i].AvgColorScalar, 1, CV_AA, 0, 0.3);
					//test
					cv::putText(imgFrame2Copy, std::to_string(blobs[i].mergeid) + ", " + std::to_string(blobs[i].unitID), blobs[i].centerPositions.back(), intFontFace, dblFontScale, SCALAR_GREEN, 2);
				}
			}
			else if (blobs[i].unitID == 0) {
				cv::rectangle(imgFrame2Copy, blobs[i].currentBoundingRect, SCALAR_YELLOW, 2);
				cv::putText(imgFrame2Copy, std::to_string(blobs[i].unitID), blobs[i].centerPositions.back(), intFontFace, dblFontScale, SCALAR_GREEN, 2);

			}
			else {
				cv::rectangle(imgFrame2Copy, blobs[i].currentBoundingRect, SCALAR_RED, 2);

				//cv::putText(imgFrame2Copy, std::to_string(blobs[i].unitID), blobs[i].centerPositions.back(), intFontFace, dblFontScale, SCALAR_GREEN, 2);

				//test
				//cv::rectangle(imgFrame2Copy, tempBoundingRect, blobs[i].AvgColorScalar, 2);
				cv::arrowedLine(imgFrame2Copy, blobs[i].centerPositions.back(), currentcenter, SCALAR_BLACK, 5, CV_AA, 0, 0.3);
				cv::arrowedLine(imgFrame2Copy, blobs[i].centerPositions.back(), currentcenter, blobs[i].AvgColorScalar, 1, CV_AA, 0, 0.3);

				cv::putText(imgFrame2Copy, std::to_string(blobs[i].unitID), blobs[i].centerPositions.back(), intFontFace, dblFontScale, SCALAR_GREEN, 2);

				//test


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
	//std::cout << "***************************************************\n";
	if (entExt == true) {
		std::cout << "Enter : " << entrance << "\n";
		carNumberCounter++;
		//std::cout << "Number of car in Car Park : " << carNumberCounter << "\n";
	}
	else {
		std::cout << "Exit : " << entrance << "\n";
		carNumberCounter--;
		//std::cout << "Number of car in Car Park : " << carNumberCounter << "\n";
	}
	//std::cout << "***************************************************\n";
}

void matchCurrentFrameBlobsToExistingBlobs(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs, std::vector<ColorTerm> &colorsList) {

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

			addBlobToExistingBlobs(currentFrameBlobs[j], existingBlobs, intIndexOfLeastDistance, colorsList);




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

void addBlobToExistingBlobsMissMatch(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex, std::vector<ColorTerm> &colorsList) {
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
		existingBlobs[intIndex].getAverageColor(colorsList);
	}

	if (existingBlobs[intIndex].enter == true && existingBlobs[intIndex].unitID == 0 && existingBlobs[intIndex].park == false) {
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
					existingBlobs[k].getAverageColor(colorsList);
				}
				break;
			}
		}
	}
}

bool checkIfPedestrain(cv::Mat tempCropImage) {

	cv::gpu::GpuMat GpuImg;

	GpuImg.upload(tempCropImage);
	cv::gpu::cvtColor(GpuImg, GpuImg, CV_BGR2GRAY);


	std::vector< cv::Point> found_locations;
	d_hog.detect(GpuImg, found_locations);

	//	std::vector< cv::Rect> found_locations_rect;
	//d_hog.detectMultiScale(GpuImg, found_locations_rect);

	if (found_locations.size() > 0) {

		return true;
	}
	else {
		return false;
	}

}

void removeBlobMemory(std::vector<Blob> &blobs) {
	for (int i = 0; i < blobs.size(); i++) {
		if (blobs[i].currentContour.size() > 10) {
			int todel = blobs[i].currentContour.size() - 10;
			blobs[i].currentContour.erase(blobs[i].currentContour.begin(), blobs[i].currentContour.begin() + todel);

			blobs[i].currentContour.shrink_to_fit();

		}

		if (blobs[i].centerPositions.size() > 10) {
			int todel = blobs[i].centerPositions.size() - 10;
			blobs[i].centerPositions.erase(blobs[i].centerPositions.begin(), blobs[i].centerPositions.begin() + todel);

			blobs[i].centerPositions.shrink_to_fit();


		}

		if (blobs[i].AvgColor.size() > 10) {
			int todel = blobs[i].AvgColor.size() - 10;
			blobs[i].AvgColor.erase(blobs[i].AvgColor.begin(), blobs[i].AvgColor.begin() + todel);

			blobs[i].AvgColor.shrink_to_fit();

		}


	}

}


//void getNumOfTrajs(CarParkTrackExporter &openDB) {
//
//	//traj grouping - should i group ALL the trajs? 
//	//how many should i group?
//
//	//create a class traj, with a vector .push_back function to add in additional points
//	// traj class also contains a "color" value for rainbow 
//
//
//	// Generate random colors
//	//vector<Vec3b> colors;
//	//for (size_t i = 0; i < contours.size(); i++)
//	//{
//	//	int b = theRNG().uniform(0, 255);
//	//	int g = theRNG().uniform(0, 255);
//	//	int r = theRNG().uniform(0, 255);
//	//	colors.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
//	
//	std::cout << "does it even run here? 1\n";
//
//	CarParkTrackExporter countTraj();
//}


IplImage* mat_to_iplimage(cv::Mat input) {

	//IplImage* output = &input.operator IplImage();


	IplImage* output = cvCloneImage(&(IplImage)input);
	//input.convertTo(input, CV_32F);
	/*std::cout << "type: " << input.type() << "  " << "channel: " << input.channels() << "\n";

	IplImage* output;
	output = cvCreateImage(cvSize(input.cols, input.rows), 8, 3);
	IplImage ipltemp = input;
	cvCopy(&ipltemp, output);
	*/
	return output;
}




void getBlobMotion(std::vector<Blob> &blobs) {

	int diff_X = 0;
	int diff_Y = 0;
	std::string direction_X = "";
	std::string direction_Y = "";


	for (int i = 0; i < blobs.size(); i++) {


		//std::cout <<  blobs[i].centerPositions << std::endl; 

		//std::cout << "blob x,y = " << blobs[i].centerPositions[blobs.size()].x << blobs[i].centerPositions[blobs.size()].y << std::endl;

		if (blobs[i].centerPositions.size() > 12)
		{


			diff_X = blobs[i].centerPositions[blobs[i].centerPositions.size() - 10].x - blobs[i].centerPositions[blobs[i].centerPositions.size() - 1].x;
			diff_Y = blobs[i].centerPositions[blobs[i].centerPositions.size() - 10].y - blobs[i].centerPositions[blobs[i].centerPositions.size() - 1].y;
			direction_X = "";
			direction_Y = "";

			if (abs(diff_X) > 5)
			{
				if (diff_X > 0)
					direction_X = "left";
				else
					direction_X = "right";
			}

			if (abs(diff_Y) > 5)
			{
				if (diff_Y > 0)
					direction_Y = "up";
				else
					direction_Y = "down";
			}

			//handle when both directions are non - empty
			if ((direction_X != "") && (direction_Y != ""))
			{
				blobs[i].motion = direction_X + "_" + direction_Y;
			}
			//otherwise, only one direction is non - empty
			else
			{
				if ((direction_X != ""))
				{
					blobs[i].motion = direction_X;
				}
				else if((direction_Y != ""))
				{
					blobs[i].motion = direction_Y;
				}
				else
				{
					blobs[i].motion = "motionless";
				}

			}

	




			//std::cout << "testX, testY = " << direction_X << "-" << direction_Y << std::endl;
		}


	}




}



int getOption() {

	int getOptionNum;
	bool bad = false;

	std::cout << "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
	std::cout << "|              S H E R L O C K            =" << std::endl;
	std::cout << "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
	std::cout << "|  Kindly select program options:         |\n";
	std::cout << "|  1) Recovery Mode                       |\n";
	std::cout << "|  2) State Extraction (Batch)            |\n";
	std::cout << "|  3) Retrieval Module (Batch)            |\n";
	std::cout << "|  4) Debugging Mode                      |\n";
	std::cout << "|                                         |\n";
	std::cout << "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;


	do {
		std::cout << "Program options: ";
		std::cin >> getOptionNum;

		bad = std::cin.fail();
		if (bad)
		{
			std::cout << "You have entered an invalid char, please try again : " << std::endl;
		}

		std::cin.clear();
		std::cin.ignore(10, '\n');
	} while (bad);

	return getOptionNum;

}


cv::Rect enlargeROI(cv::Mat frm, cv::Rect boundingBox, int padding) {
	cv::Rect returnRect = cv::Rect(boundingBox.x - padding, boundingBox.y - padding, boundingBox.width + (padding * 1.55), boundingBox.height + (padding * 1.55));
	if (returnRect.x < 0)returnRect.x = 0;
	if (returnRect.y < 0)returnRect.y = 0;
	if (returnRect.x + returnRect.width >= frm.cols)returnRect.width = frm.cols - returnRect.x;
	if (returnRect.y + returnRect.height >= frm.rows)returnRect.height = frm.rows - returnRect.y;
	return returnRect;

}


std::vector<std::string> dim_flat(std::string v, std::vector<std::string> const &)
{
	return std::vector<std::string>(1, v);
}

std::vector<std::string> dim_flat(std::vector<std::string> const & v)
{
	return v;
}

template <typename T>
std::vector<std::string> dim_flat(std::vector<std::vector<T>> const & v)
{
	std::vector<std::string>  ret;

	for (auto const & e : v)
	{
		auto s = dim_flat(e);

		ret.reserve(ret.size() + s.size());
		ret.insert(ret.end(), s.cbegin(), s.cend());
	}

	return ret;
}




void QueryModule(CarParkTrackExporter &openDB) {

	int count = 0;
	//create GUI mat
	cv::Mat RetrievalMod = cv::Mat(480, 300, CV_8UC3);
	cv::Mat queryInputMat = cv::imread("nogrid2.jpg");
	cv::Mat queryInputMat_ORI = cv::imread("nogrid2.jpg");

	//cv::Mat queryInputMatGray;


	cv::Mat showthisMat;
	//cv::Mat RetrievalMod = cv::Mat(200, 500, CV_8UC3);

	// Init a OpenCV window and tell cvui to use it.
	cv::namedWindow(WINDOW_NAME);
	cvui::init(WINDOW_NAME);

	int low_threshold = 50, high_threshold = 150;
	//bool use_canny = false;



	//colors:
	bool find_black = false;
	bool find_blue = false;
	bool find_brown = false;
	bool find_green = false;
	bool find_grey_silver = false;
	bool find_orange = false;
	bool find_pink = false;
	bool find_purple = false;
	bool find_red = false;
	bool find_white = false;
	bool find_yellow = false;



	bool init_window = true;
	bool initialize_grid = true;

	std::vector<std::vector<int>> status;
	std::vector<std::vector<bool>> clicked;
	std::vector<std::vector<std::string>> query_dir_table;


	std::vector<std::vector<int>> status_test;

	

	int previous_x = 0;
	int	previous_y = 0;

	cv::Point centroid, direction;

	bool queryArroworCircle = 1;
	int numbOfQueryinput = 0;
	int trackbarValue = 70;

	//std::vector<cv::Point> queryLine;

	//cv::setMouseCallback(WINDOW_NAME, onMouse, NULL);

	while (true) {
		int display_x = 30;
		int display_y = 80;
		int display_gap = 18;


		// Fill the frame with a nice color
		RetrievalMod = cv::Scalar(49, 52, 49);

		cvui::printf(RetrievalMod, 20, 20, 0.8, 0x64c831, "Query Module");

		//if (use_canny)
		//{
		//	//cv::cvtColor(queryInputMat, queryInputMatGray, CV_BGR2GRAY);
		//	//cv::Canny(queryInputMatGray, queryInputMatGray, low_threshold, high_threshold, 3);
		//	//cv::cvtColor(queryInputMatGray, queryInputMatGray, CV_GRAY2BGR);

		//}
		//else
		//{
		//	//
		//	//queryInputMat.copyTo(RetrievalMod);
		//}

		// Render the settings window to house the UI
		cvui::window(RetrievalMod, 20, 50, 150, 240, "Colors");

		// Checkbox to enable/disable the use of Canny edge
		cvui::checkbox(RetrievalMod, display_x, display_y, "Black", &find_black);
		display_y = display_y + display_gap;
		cvui::checkbox(RetrievalMod, display_x, display_y, "Blue", &find_blue);
		display_y = display_y + display_gap;
		cvui::checkbox(RetrievalMod, display_x, display_y, "Brown", &find_brown);
		display_y = display_y + display_gap;
		cvui::checkbox(RetrievalMod, display_x, display_y, "Green", &find_green);
		display_y = display_y + display_gap;
		cvui::checkbox(RetrievalMod, display_x, display_y, "Grey/Silver", &find_grey_silver);
		display_y = display_y + display_gap;
		cvui::checkbox(RetrievalMod, display_x, display_y, "Orange", &find_orange);
		display_y = display_y + display_gap;
		cvui::checkbox(RetrievalMod, display_x, display_y, "Pink", &find_pink);
		display_y = display_y + display_gap;
		cvui::checkbox(RetrievalMod, display_x, display_y, "Purple", &find_purple);
		display_y = display_y + display_gap;
		cvui::checkbox(RetrievalMod, display_x, display_y, "Red", &find_red);
		display_y = display_y + display_gap;
		cvui::checkbox(RetrievalMod, display_x, display_y, "White", &find_white);
		display_y = display_y + display_gap;
		cvui::checkbox(RetrievalMod, display_x, display_y, "Yellow", &find_yellow);



		//write query:
		std::string queryCondition_table = "";
		std::string queryCondition_where = "";
		



		/*if (find_black)
		{
		if (queryCondition == "")
		queryCondition += "obj_color = \"BLACK\"";
		else
		queryCondition += " or obj_color = \"BLACK\"";

		}*/




		//draw all the rect in the search interface

		int numberofWindows = 20;
		//int start_y = 0;

		int video_width = 640;
		int video_height = 480;
		int dimension_x = 20;
		int dimension_y = 20;

		int win_x = video_width / dimension_x;
		int win_y = video_height / dimension_y;

		int atom_x, atom_y;


		






		status.resize(numberofWindows);
		status_test.resize(video_width);

		if (initialize_grid)
		{
			clicked.resize(numberofWindows);
			query_dir_table.resize(numberofWindows);
		}

		//TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST 
		//TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST 
		//TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST 
		//TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST 
		//TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST 
		for (int i = 0; i < video_width; i+=5)
		{
	
			status_test[i].resize(video_height);


			for (int j = 0; j < video_height;j+=5)
			{

				//define rect area
				cv::Rect rectangle_test(i, j, 5, 5);

				atom_x = i / win_x;
				atom_y = j / win_y;

				status_test[i][j] = (cvui::iarea(RetrievalMod.size().width + rectangle_test.x, rectangle_test.y, rectangle_test.width, rectangle_test.height));

				switch (status_test[i][j]) {
				
				case cvui::CLICK:
				{
					//std::cout << "Rectangle " << i << "," << j << " was clicked!" << std::endl; 

					previous_x = i;
					previous_y = j;

					if (clicked[atom_x][atom_y] == true)
					{
						clicked[atom_x][atom_y] = false;
						queryInputMat_ORI.copyTo(queryInputMat);


					}
					else
					{
						//clicked[i][j] = true;
						//cvui::rect(queryInputMat, rectangle.x, rectangle.y, rectangle.width, rectangle.height, 0x00ff00);
					}







					break;
				}
				case cvui::DOWN:
				{


					////set previous row/colomn
					//previous_x = i;
					//previous_y = j;

					//determine the direction the mouse went:
					if (previous_x != i && previous_y == j)
					{

						if (previous_x > i)
						{
							query_dir_table[atom_x][atom_y] = "dir_left";


						}
						else if (previous_x < i)
						{
							query_dir_table[atom_x][atom_y] = "dir_right";

						}


						previous_x = i;

					}
					else
						if (previous_y != j && previous_x == i)
						{

							if (previous_y > j)
							{
								query_dir_table[atom_x][atom_y] = "dir_up";
							}
							else if (previous_y < j)
							{
								query_dir_table[atom_x][atom_y] = "dir_down";
							}


							previous_y = j;

						}
						else
							if (previous_y != j && previous_x != i)
							{

								if (previous_x > i && previous_y > j)
								{
									query_dir_table[atom_x][atom_y] = "dir_left_up";

								}
								else if (previous_x < i && previous_y > j)
								{
									query_dir_table[atom_x][atom_y] = "dir_right_up";
								}
								else if (previous_x > i && previous_y < j)
								{
									query_dir_table[atom_x][atom_y] = "dir_left_down";
								}
								else if (previous_x < i && previous_y < j)
								{
									query_dir_table[atom_x][atom_y] = "dir_right_down";

								}

								previous_x = i;
								previous_y = j;

							}


					//cvui::text(RetrievalMod, 20, 350, "Mouse is: DOWN" + std::to_string(i) + "," + std::to_string(j) + " " + query_dir_table[i][j]);


					if (clicked[atom_x][atom_y] == true)
					{
						//clicked[i][j] = false;
						//queryInputMat_ORI.copyTo(queryInputMat);


					}
					else
					{
						clicked[atom_x][atom_y] = true;
						if (debug_on)
						{
							//cvui::rect(queryInputMat, rectangle.x, rectangle.y, rectangle.width, rectangle.height, 0x00ff00);
							cv::arrowedLine(queryInputMat, centroid, direction, SCALAR_GREEN, 5, CV_AA, 0, 0.3);
						}

					}



					break;
				}
				case cvui::OVER:
				{
					cvui::text(RetrievalMod, 20, 430, "Mouse is: OVER" + std::to_string(i) + "," + std::to_string(j));
					break;
				}
				//case cvui::OUT:		cvui::text(queryInputMat, 240, 50, "Mouse is: OUT"); break;
				}


			}
		}

		//TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST 
		//TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST 
		//TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST 
		//TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST 
		//TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST 
		//TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST 


		for (int i = 0; i < video_width;)
		{
			atom_x = i / win_x;
			status[atom_x].resize(numberofWindows);

			if (initialize_grid)
			{
				clicked[atom_x].resize(numberofWindows);
				query_dir_table[atom_x].resize(numberofWindows);
			}



			for (int j = 0; j < video_height;)
			{

				//define rect area
				cv::Rect rectangle(i, j, win_x, win_y);



				atom_y = j / win_y;
				status[atom_x][atom_y] = (cvui::iarea(RetrievalMod.size().width + rectangle.x, rectangle.y, rectangle.width, rectangle.height));



				if (initialize_grid)
				{
					clicked[atom_x][atom_y] = false;

					//std::cout << query_dir_table.size() << query_dir_table[atom_x].size() << std::endl;
					query_dir_table[atom_x][atom_y] = "";

				}



				if (clicked[atom_x][atom_y] == false)
				{
					//cvui::rect(queryInputMat, rectangle.x, rectangle.y, rectangle.width, rectangle.height, 0xffffff);
					//queryInputMat = cv::imread("nogrid2.jpg");

				}
				else
				{
					if(debug_on)
					cvui::rect(queryInputMat, rectangle.x, rectangle.y, rectangle.width, rectangle.height, 0x00ff00);

					if (query_dir_table[atom_x][atom_y] == "dir_left")
					{
						centroid.x = (atom_x *  win_x) + win_x / 2;
						centroid.y = (atom_y *  win_y) + win_y / 2;

						direction.x = (atom_x * win_x);
						direction.y = (atom_y * win_y) + win_y / 2;
					}
					else if (query_dir_table[atom_x][atom_y] == "dir_right")
					{

						centroid.x = (atom_x *  win_x) + win_x / 2;
						centroid.y = (atom_y *  win_y) + win_y / 2;

						direction.x = (atom_x * win_x) + win_x;
						direction.y = (atom_y * win_y) + win_y / 2;
					}
					else if (query_dir_table[atom_x][atom_y] == "dir_up")
					{
						centroid.x = (atom_x *  win_x) + win_x / 2;
						centroid.y = (atom_y *  win_y) + win_y / 2;

						direction.x = (atom_x * win_x) + win_x / 2;
						direction.y = (atom_y * win_y);
					}
					else if (query_dir_table[atom_x][atom_y] == "dir_down")
					{
						centroid.x = (atom_x *  win_x) + win_x / 2;
						centroid.y = (atom_y *  win_y) + win_y / 2;

						direction.x = (atom_x * win_x) + win_x / 2;
						direction.y = (atom_y * win_y) + win_y;						
					}
					else if (query_dir_table[atom_x][atom_y] == "dir_left_up")
					{
						centroid.x = (atom_x *  win_x) + win_x / 2;
						centroid.y = (atom_y *  win_y) + win_y / 2;

						direction.x = (atom_x * win_x);
						direction.y = (atom_y * win_y);
					}
					else if (query_dir_table[atom_x][atom_y] == "dir_right_up")
					{
						centroid.x = (atom_x *  win_x) + win_x / 2;
						centroid.y = (atom_y *  win_y) + win_y / 2;

						direction.x = (atom_x * win_x) + win_x;
						direction.y = (atom_y * win_y);
					}
					else if (query_dir_table[atom_x][atom_y] == "dir_left_down")
					{
						centroid.x = (atom_x *  win_x) + win_x / 2;
						centroid.y = (atom_y *  win_y) + win_y / 2;

						direction.x = (atom_x * win_x);
						direction.y = (atom_y * win_y) + win_y;
					}
					else if (query_dir_table[atom_x][atom_y] == "dir_right_down")
					{
						centroid.x = (atom_x *  win_x) + win_x / 2;
						centroid.y = (atom_y *  win_y) + win_y / 2;

						direction.x = (atom_x * win_x) + win_x;
						direction.y = (atom_y * win_y) + win_y;
					}
					
					if (queryArroworCircle)
					{
						cv::arrowedLine(queryInputMat, centroid, direction, SCALAR_GREEN, 2, CV_AA, 0, 0.3);
					}
					else
					{
						cv::circle(queryInputMat, centroid, 1, SCALAR_GREEN, 2, CV_AA);
					}
				}


				if (atom_x == 0)
				{
					cv::putText(queryInputMat, std::to_string(atom_y), cv::Point(i, j + win_y), cv::FONT_HERSHEY_SIMPLEX, 0.6, SCALAR_RED);
				}

				if (atom_y == 0)
				{
					cv::putText(queryInputMat, std::to_string(atom_x), cv::Point(i, j + win_y), cv::FONT_HERSHEY_SIMPLEX, 0.6, SCALAR_RED);
				}

				j += win_y;

				/* // ## UNCOMMENT ALL THESE

				//switch (status[atom_x][atom_y]) {
				//case cvui::CLICK:
				//{
				//	//std::cout << "Rectangle " << atom_x << "," << atom_y << " was clicked!" << std::endl; 

				//	previous_x = atom_x;
				//	previous_y = atom_y;

				//	if (clicked[atom_x][atom_y] == true)
				//	{
				//		clicked[atom_x][atom_y] = false;
				//		queryInputMat_ORI.copyTo(queryInputMat);


				//	}
				//	else
				//	{
				//		//clicked[atom_x][atom_y] = true;
				//		//cvui::rect(queryInputMat, rectangle.x, rectangle.y, rectangle.width, rectangle.height, 0x00ff00);
				//	}







				//	break;
				//}
				//case cvui::DOWN:
				//{
				//	

				//	////set previous row/colomn
				//	//previous_x = atom_x;
				//	//previous_y = atom_y;

				//	
				//	//determine the direction the mouse went:
				//	if (previous_x != atom_x && previous_y == atom_y)
				//	{

				//		if (previous_x > atom_x )
				//		{
				//			query_dir_table[atom_x][atom_y] = "dir_left";
				//		

				//		}
				//		else if (previous_x < atom_x)
				//		{
				//			query_dir_table[atom_x][atom_y] = "dir_right";
				//			
				//		}


				//		previous_x = atom_x;

				//	}
				//	else
				//	if (previous_y != atom_y && previous_x == atom_x)
				//	{

				//		if (previous_y > atom_y)
				//		{
				//			query_dir_table[atom_x][atom_y] = "dir_up";
				//		} 
				//		else if (previous_y < atom_y)
				//		{
				//			query_dir_table[atom_x][atom_y] = "dir_down";
				//		}


				//		previous_y = atom_y;

				//	}
				//	else
				//	if (previous_y != atom_y && previous_x != atom_x)
				//	{

				//		if (previous_x > atom_x && previous_y > atom_y)
				//		{
				//			query_dir_table[atom_x][atom_y] = "dir_left_up";
				//			
				//		}
				//		else if (previous_x < atom_x && previous_y > atom_y)
				//		{
				//			query_dir_table[atom_x][atom_y] = "dir_right_up";
				//		}
				//		else if (previous_x > atom_x && previous_y < atom_y)
				//		{
				//			query_dir_table[atom_x][atom_y] = "dir_left_down";
				//		}
				//		else if (previous_x < atom_x && previous_y < atom_y)
				//		{
				//			query_dir_table[atom_x][atom_y] = "dir_right_down";
				//			
				//		}

				//		previous_x = atom_x;
				//		previous_y = atom_y;

				//	}


				//	//cvui::text(RetrievalMod, 20, 350, "Mouse is: DOWN" + std::to_string(atom_x) + "," + std::to_string(atom_y) + " " + query_dir_table[atom_x][atom_y]);


				//	if (clicked[atom_x][atom_y] == true)
				//	{
				//		//clicked[atom_x][atom_y] = false;
				//		//queryInputMat_ORI.copyTo(queryInputMat);


				//	}
				//	else
				//	{
				//		clicked[atom_x][atom_y] = true;
				//		if (debug_on)
				//		{
				//			cvui::rect(queryInputMat, rectangle.x, rectangle.y, rectangle.width, rectangle.height, 0x00ff00);
				//			cv::arrowedLine(queryInputMat, centroid, direction, SCALAR_GREEN, 5, CV_AA, 0, 0.3);
				//		}
				//		
				//	}

				//	

				//	break;
				//}
				//case cvui::OVER:
				//{
				//	//cvui::text(RetrievalMod, 20, 350, "Mouse is: OVER" + std::to_string(atom_x) + "," + std::to_string(atom_y));
				//	break;
				//}
				////case cvui::OUT:		cvui::text(queryInputMat, 240, 50, "Mouse is: OUT"); break;
				//}
				*/

				// UNCOMMENT ALL THESE IF YOU WANT TO USE THE PREVIOUS METHOD

				//std::cout << atom_x << "," << atom_y << ": " <<clicked[atom_x][atom_y] << std::endl;

				
				if (clicked[atom_x][atom_y] == true)
				{
					//std::cout << "Selected rects are: " << atom_x << ", " << atom_y << std::endl;

					queryCondition_where =  "where atom_x = " + std::to_string(atom_x) + " and atom_y = " + std::to_string(atom_y);

					if (find_black)
					{
						queryCondition_table += "select filename, atom_t from " + query_dir_table[atom_x][atom_y];
						queryCondition_table += " where atom_x = (select atom_x from color_black " + queryCondition_where + ") and atom_y = (select atom_y from color_black " + queryCondition_where + ");";

					}

					if (find_blue)
					{

						queryCondition_table += "select filename, atom_t from " + query_dir_table[atom_x][atom_y];
						queryCondition_table += " where atom_x = (select atom_x from color_blue " + queryCondition_where + ") and atom_y = (select atom_y from color_blue " + queryCondition_where + ");";
					}

					if (find_brown)
					{

						queryCondition_table += "select filename, atom_t from " + query_dir_table[atom_x][atom_y];
						queryCondition_table += " where atom_x = (select atom_x from color_brown " + queryCondition_where + ") and atom_y = (select atom_y from color_brown " + queryCondition_where + ");";
					}

					if (find_green)
					{

						queryCondition_table += "select filename, atom_t from " + query_dir_table[atom_x][atom_y];
						queryCondition_table += " where atom_x = (select atom_x from color_green " + queryCondition_where + ") and atom_y = (select atom_y from color_green " + queryCondition_where + ");";
					}

					if (find_grey_silver)
					{

						queryCondition_table += "select filename, atom_t from " + query_dir_table[atom_x][atom_y];
						queryCondition_table += " where atom_x = (select atom_x from color_grey_silver " + queryCondition_where + ") and atom_y = (select atom_y from color_grey_silver " + queryCondition_where + ");";
					}

					if (find_orange)
					{

						queryCondition_table += "select filename, atom_t from " + query_dir_table[atom_x][atom_y];
						queryCondition_table += " where atom_x = (select atom_x from color_orange " + queryCondition_where + ") and atom_y = (select atom_y from color_orange " + queryCondition_where + ");";
					}

					if (find_pink)
					{

						queryCondition_table += "select filename, atom_t from " + query_dir_table[atom_x][atom_y];
						queryCondition_table += " where atom_x = (select atom_x from color_pink " + queryCondition_where + ") and atom_y = (select atom_y from color_pink " + queryCondition_where + ");";
					}

					if (find_purple)
					{

						queryCondition_table += "select filename, atom_t from " + query_dir_table[atom_x][atom_y];
						queryCondition_table += " where atom_x = (select atom_x from color_purple " + queryCondition_where + ") and atom_y = (select atom_y from color_purple " + queryCondition_where + ");";
					}

					if (find_red)
					{

						queryCondition_table += "select filename, atom_t from " + query_dir_table[atom_x][atom_y];
						queryCondition_table += " where atom_x = (select atom_x from color_red " + queryCondition_where + ") and atom_y = (select atom_y from color_red " + queryCondition_where + ");";
					}

					if (find_white)
					{

						queryCondition_table += "select filename, atom_t from " + query_dir_table[atom_x][atom_y];
						queryCondition_table += " where atom_x = (select atom_x from color_white " + queryCondition_where + ") and atom_y = (select atom_y from color_white " + queryCondition_where + ");";
					}

					if (find_yellow)
					{

						queryCondition_table += "select filename, atom_t from " + query_dir_table[atom_x][atom_y];
						queryCondition_table += " where atom_x = (select atom_x from color_yellow " + queryCondition_where + ") and atom_y = (select atom_y from color_yellow " + queryCondition_where + ");";
					}


					





				}
			}
			i += win_x;



		}






		initialize_grid = false;


		if (cvui::button(RetrievalMod, 20, 450, " Reset! ")) {

		

			for (int i = 0; i < numberofWindows; i++)
			{
				for (int j = 0; j < numberofWindows; j++)
				{
					clicked[i][j] = false;
					query_dir_table[i][j] = "";

				}
			}
			queryInputMat_ORI.copyTo(queryInputMat);
			

			
		}

		
		//create a trackbar
		cvui::text(RetrievalMod, 20, 300, "Confidence Value (%):");

		trackbarValue = round(trackbarValue);
		cvui::beginColumn(RetrievalMod, 20, 315, -1, -1, 6);
		cvui::trackbar(150, &trackbarValue,30, 100,1,"%.1Lf", cvui::TRACKBAR_DISCRETE,10);
	
		//cvui::trackbar(RetrievalMod, 20, 315, 150, &trackbarValue, 30., 100.0);
		cvui::endColumn();



		// Show a button at position (50, 280)
		if (cvui::button(RetrievalMod, 20, 360, "      Query!      ")) {
			// The button was clicked, so let's increment our counter.
			count++;



			//create a MAT to display all the results




			numbOfQueryinput = 0;
			//count number of query input given
			for (int i = 0; i < numberofWindows; i++)
			{
				for (int j = 0; j < numberofWindows; j++)
				{
					if (clicked[i][j])
					{
						numbOfQueryinput++;
						
					}
				}
			}


			std::string queryInput;
			queryInput = queryCondition_table;
			
			if(debug_on)
				std::cout << queryInput << std::endl;

			openDB.searchdatabase(queryInput);

		
			//remove all the ID.. not important
			/*for (int i = 0; i < openDB.r_result.size(); i++)
			{

				for (int j = 0; j < openDB.r_result[i].size(); j++)
				{

					if (j == 0)
						openDB.r_result[i][j] = "0";
				}
			}*/

			if (openDB.r_result.size() == 0)
				std::cout << "--- NO RESULTS FOUND!" <<std::endl;
			else
			{
				std::cout << std::endl << "--------------------" << std::endl;



				/*
				// traditional sort
				struct {
				bool operator()(std::vector<std::string> a, std::vector<std::string> b) const
				{
				return a < b;
				}
				} customSort;
				std::sort(openDB.r_result.begin(), openDB.r_result.end(), customSort);
				*/


				std::vector<std::string> v;
				// vector contents are reversed sorted contents of the old set
				//std::copy(openDB.r_result.begin(), openDB.r_result.end(), std::back_inserter(v));

				for (int i = 0; i < openDB.r_result.size(); i++)
				{
					v.push_back(openDB.r_result[i][0] + "," + openDB.r_result[i][1]);
				}

				// now sort the vector with the algorithm
				std::sort(v.begin(), v.end(), doj::alphanum_less<std::string>());
				// and print the vector to cout
				if (debug_on)
				{
					std::copy(v.begin(), v.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
				}


				//create sorted vector:
				struct grouppedResults {

					std::string resultFileName = "";
					int groupNum = 0;
					std::vector<int> frameNums = {};



					int size()
					{
						return sizeof(grouppedResults);
					}


					int frameNumsize()
					{
						return sizeof(frameNums);
					}

				};

				grouppedResults singlegrouppedResults;
				std::vector<grouppedResults> resultsFromDB;

				std::cout << " \n\n TEST \n\n";

				std::string firstFilename = openDB.r_result[0][0] = v[0].substr(0, 63);
				int resultGroupNum = 0;


				for (int i = 0; i < openDB.r_result.size(); i++)
				{
					openDB.r_result[i][0] = v[i].substr(0, 63);
					openDB.r_result[i][1] = v[i].substr(64, std::string::npos);


					std::cout << openDB.r_result[i][0] << " " << openDB.r_result[i][1] << std::endl;

					//since its sorted here already, ill group them up here.
					
					if (firstFilename == openDB.r_result[i][0])
					{

						//std::cout << "grouping items ";


						singlegrouppedResults.resultFileName = openDB.r_result[i][0];
						singlegrouppedResults.groupNum = resultGroupNum;
						singlegrouppedResults.frameNums.push_back(std::stoi(openDB.r_result[i][1]));

						
						// push back last group
						if (i == (openDB.r_result.size() - 1))
						{
							resultsFromDB.push_back(singlegrouppedResults);
						}

					}
					else
					{
						resultsFromDB.push_back(singlegrouppedResults);
						resultGroupNum++;

						singlegrouppedResults.frameNums.clear();
						firstFilename = openDB.r_result[i][0];

						//set first occurance
						singlegrouppedResults.resultFileName = openDB.r_result[i][0];
						singlegrouppedResults.groupNum = resultGroupNum;
						singlegrouppedResults.frameNums.push_back(std::stoi(openDB.r_result[i][1]));

						//std::cout << "\n new group:  " << openDB.r_result[i][0] << std::endl;
					}


				}


				std::cout << "\n\n";


				//all the grouped items are here.. now find out if it fulfils the CV% value (individual small groups)


				for (int i = 0; i < resultsFromDB.size(); i++)
				{

						
						std::cout << resultsFromDB[i].resultFileName << std::endl;
						std::cout << resultsFromDB[i].groupNum << std::endl;

						int firstAtomT = 0;
						int passedCV = 0;
						int groupNum = 0;

						grouppedResults resultFromSingleVideo;
						std::vector<grouppedResults> resultFromSingleVideo_grouped;


						resultFromSingleVideo.resultFileName = resultsFromDB[i].resultFileName;
						//resultFromSingleVideo.groupNum = 0;


						for (int j = 0; j < resultsFromDB[i].frameNums.size(); j++)
						{
							std::cout << resultsFromDB[i].frameNums[j] << ",";

							if (j == 0)
							{
								firstAtomT = resultsFromDB[i].frameNums[j];
							}

							if (resultsFromDB[i].frameNums[j] <= firstAtomT + 5)
							{
								resultFromSingleVideo.frameNums.push_back(resultsFromDB[i].frameNums[j]);
								firstAtomT = resultsFromDB[i].frameNums[j];

								if (j == (resultsFromDB[i].frameNums.size() - 1))
								{
									resultFromSingleVideo_grouped.push_back(resultFromSingleVideo);
								}

							}
							else
							{
								
								resultFromSingleVideo.groupNum = groupNum;
								resultFromSingleVideo_grouped.push_back(resultFromSingleVideo);
								resultFromSingleVideo.frameNums.clear();

								groupNum++;
								

								firstAtomT = resultsFromDB[i].frameNums[j];
								resultFromSingleVideo.frameNums.push_back(resultsFromDB[i].frameNums[j]);

								/*
									resultsFromDB.push_back(singlegrouppedResults);
									resultGroupNum++;

									singlegrouppedResults.frameNums.clear();
									firstFilename = openDB.r_result[i][0];

								//set first occurance
									singlegrouppedResults.resultFileName = openDB.r_result[i][0];
									singlegrouppedResults.groupNum = resultGroupNum;
									singlegrouppedResults.frameNums.push_back(std::stoi(openDB.r_result[i][1]));
											
								
								*/

							}

							

						}

						for (int k = 0; k < resultFromSingleVideo_grouped.size(); k++)
						{
							std::cout << "\n--- resultFromSingleVideo_grouped ---\n";
							std::cout << resultFromSingleVideo_grouped[k].groupNum << std::endl;

								for (int g = 0; g < resultFromSingleVideo_grouped[k].frameNums.size(); g++)
								{
									std::cout << resultFromSingleVideo_grouped[k].frameNums[g] << " ";

								}

							
						/*std::cout << "resultFromSingleVideo_grouped[k].frameNums.size() / numbOfQueryinput : " << resultFromSingleVideo_grouped[k].frameNums.size() << "/" << numbOfQueryinput << "\n";
						if (((resultFromSingleVideo_grouped[k].frameNums.size() / numbOfQueryinput) * 100) > trackbarValue)
						{
						}*/

						}



							



						

						std::cout << "\n\n";

					

					//test to display all first

					



				}
				












			}

		}


		// Show how many times the button has been clicked.
		// Text at position (250, 90), sized 0.4, in red.
		cvui::printf(RetrievalMod, 15, 400, 0.4, 0x0000ff, "Number of queries made: %d", count);

		// Update cvui internal stuff
		cvui::update();

		// Show everything on the screen

		cv::hconcat(RetrievalMod, queryInputMat, showthisMat);
		cv::imshow(WINDOW_NAME, showthisMat);

		// Check if ESC key was pressed
		if (cv::waitKey(20) == 27) {
			break;
		}
	}





openDB.countTraj();
std::cin.get();
}


