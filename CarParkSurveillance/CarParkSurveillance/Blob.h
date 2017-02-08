// Blob.h

#ifndef MY_BLOB
#define MY_BLOB

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/features2d/features2d.hpp>
#include<opencv2/nonfree/features2d.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/nonfree/nonfree.hpp>
#include<opencv2/gpu/gpu.hpp>

#include<iostream>
#include<conio.h> 

///////////////////////////////////////////////////////////////////////////////////////////////////
class Blob {
public:

	bool enter = false;
	bool exit = false;
	bool park = false;
	
	int parkframe = 0;
	int parkLocation = 0;



	int unitID = 0;
	int existInSceen = 0;

	int counter = 0;
	// member variables ///////////////////////////////////////////////////////////////////////////
	std::vector<cv::Point> currentContour;

	cv::Rect currentBoundingRect;



	std::vector<cv::Point> centerPositions;
	std::vector<cv::Scalar> AvgColor;

	cv::Mat image;

	double dblCurrentDiagonalSize;
	double dblCurrentAspectRatio;

	bool blnCurrentMatchFoundOrNewBlob;

	bool blnStillBeingTracked;
	/////test
	bool isAdded;
	////

	int intNumOfConsecutiveFramesWithoutAMatch;

	cv::Point predictedNextPosition;

	cv::Mat rawImage;
	cv::Mat maskImage;

	std::vector<cv::KeyPoint> keypoints_new;
	std::vector<cv::KeyPoint>::iterator it;
	std::vector<cv::Point2f> points;

	cv::gpu::GpuMat des;

	cv::Mat desNoGpu;

	bool addornot;

	// function prototypes ////////////////////////////////////////////////////////////////////////
	Blob(std::vector<cv::Point> _contour);
	void predictNextPosition(void);
	void storeImage(cv::Mat rawImage);
	void drawMaskImage();
	void getFeatures();
	cv::gpu::GpuMat getGpuImage();
	void desFeatures();
	cv::Mat getDes();
	cv::Mat getRawImage();
	cv::gpu::GpuMat getGpuDes();
	void useORBGPU();
	void setEnter();
	void setExit();
	void setPark();
	void getAverageColor();

};

#endif    // MY_BLOB


#pragma once
