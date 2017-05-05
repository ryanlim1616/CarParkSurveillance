#pragma once
// Trajectory.h

#ifndef MY_TRAJECTORY
#define MY_TRAJECTORY

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
class Trajectory {
public:


	cv::Scalar trackColor;
	std::vector<cv::Point> centerPositions;
	int unitID = 0;


	// function prototypes ////////////////////////////////////////////////////////////////////////
	Trajectory(int unitID);
	~Trajectory();


};

#endif    // MY_TRAJECTORY


#pragma once
