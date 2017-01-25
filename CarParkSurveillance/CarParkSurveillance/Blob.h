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
	int counter = 0;
	// member variables ///////////////////////////////////////////////////////////////////////////
	std::vector<cv::Point> currentContour;

	cv::Rect currentBoundingRect;



	std::vector<cv::Point> centerPositions;

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

};

#endif    // MY_BLOB


#pragma once
