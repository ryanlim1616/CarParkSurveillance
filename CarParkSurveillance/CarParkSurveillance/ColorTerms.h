#pragma once
// ColorTerm.h

#ifndef MY_Color
#define MY_Color

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
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include<stdio.h>
#include <iterator>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <math.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
class ColorTerm {
public:

	std::string RGB;
	std::string ColorName;




	template<typename Out>
	void split(const std::string &s, char delim, Out result) {
		std::stringstream ss;
		ss.str(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			*(result++) = item;
		}
	}


	std::vector<std::string> split(const std::string &s, char delim) {
		std::vector<std::string> elems;
		split(s, delim, std::back_inserter(elems));
		return elems;
	}


	std::string tail(std::string const& source, size_t const length) {
		if (length >= source.size()) { return source; }
		return source.substr(source.size() - length);
	} // tail




	std::string getTerm(std::vector<ColorTerm> &inputColorVector, std::string &searchInput) {


		


		for (int i = 0; i < inputColorVector.size(); i++)
		{
			
			if (searchInput == inputColorVector[i].RGB)
			{
				return inputColorVector[i].ColorName;
				break;
			}

		}

		return "Color Term Not found: " + searchInput + "\n";

	}

	std::string compareColors(cv::Scalar colorSearch)
	{
		/*
		cv::Scalar Term_Black =		cv::Scalar(0.0,		0.0,	0.0);
		cv::Scalar Term_Gray =		cv::Scalar(107.0,	107.0,	107.0);
		cv::Scalar Term_White =		cv::Scalar(255.0,	255.0,	255.0);
		cv::Scalar Term_Red =		cv::Scalar(5.0,		5.0,	250.0);
		cv::Scalar Term_Orange =	cv::Scalar(6.0,		115.0,	249.0);
		cv::Scalar Term_Yellow =	cv::Scalar(20.0,	255.0,	255.0);
		cv::Scalar Term_Purple =	cv::Scalar(156.0,	30.0,	126.0);
		cv::Scalar Term_Pink =		cv::Scalar(192.0,	129.0,	255.0);
		cv::Scalar Term_Brown =		cv::Scalar(0.0,		55.0,	101.0);
		cv::Scalar Term_Blue =		cv::Scalar(223.0,	67.0,	3.0);
		cv::Scalar Term_Green =		cv::Scalar(26.0,	176.0,	21.0);
		*/
		std::vector<std::string> colorinTerms = {"BLACK", "GRAY", "WHITE", "RED", "ORANGE", "YELLOW", "PURPLE", "PINK", "BROWN", "BLUE", "GREEN"};

		std::vector<cv::Scalar> allColorTerms;

		allColorTerms.push_back(cv::Scalar(0.0, 0.0, 0.0));				//  Term_Black 
		allColorTerms.push_back(cv::Scalar(107.0, 107.0, 107.0));		//  Term_Gray
		allColorTerms.push_back(cv::Scalar(255.0, 255.0, 255.0));		//  Term_White 
		allColorTerms.push_back(cv::Scalar(5.0, 5.0, 250.0));			//  Term_Red
		allColorTerms.push_back(cv::Scalar(6.0, 115.0, 249.0));			//  Term_Orange
		allColorTerms.push_back(cv::Scalar(20.0, 255.0, 255.0));			//  Term_Yellow
		allColorTerms.push_back(cv::Scalar(156.0, 30.0, 126.0));			//  Term_Purple
		allColorTerms.push_back(cv::Scalar(192.0, 129.0, 255.0));		//  Term_Pink
		allColorTerms.push_back(cv::Scalar(0.0, 55.0, 101.0));			//  Term_Brown 
		allColorTerms.push_back(cv::Scalar(223.0, 67.0, 3.0));			//  Term_Blue
		allColorTerms.push_back(cv::Scalar(26.0, 176.0, 21.0));			//  Term_Green 


		cv::Mat tempMat = cv::Mat::zeros(10, 10, CV_8UC3);
		cv::Mat tempMat_input = cv::Mat::zeros(10, 10, CV_8UC3);
		tempMat_input = colorSearch;

		cv::Mat a, b;

		std::string colorTerm;
		std::vector<double> refNum;

		cv::Scalar inputScalar, compareScalar;

		//std::cout << "bomb here? a ";

		for (int i = 0; i < allColorTerms.size(); i++)
		{
			tempMat = allColorTerms[i];
			cvtColor(tempMat, a, CV_BGR2HSV);
			cvtColor(tempMat_input, b, CV_BGR2HSV);

			//cv::Scalar a = cv::mean(tempMat);
			//::Scalar b = cv::mean(tempMat_input);

			//cv::Vec4d d = a - b;
			//double distance = cv::norm(a - b);
			//std::cout << colorinTerms[i] << ": {cv norm}" << distance << std::endl;


			inputScalar = cv::mean(b); //
			compareScalar = cv::mean(a); //

			float dL = inputScalar[0] - compareScalar[0];
			float dA = inputScalar[1] - compareScalar[1];
			float dB = inputScalar[2] - compareScalar[2];

			float dist = sqrt(dL*dL + dA*dA + dB*dB);
			//std::cout << colorinTerms[i] << ": {sqrt   }" << dist << std::endl;


			refNum.push_back(dist);
			
		}

		//std::cout << "bomb here? b ";

		std::vector<double>::iterator result = std::min_element(std::begin(refNum), std::end(refNum));
		int finalResult = std::distance(std::begin(refNum), result);


		std::cout << "eclidean distance test: " <<colorinTerms[finalResult] <<std::endl;
		colorTerm = colorinTerms[finalResult];


	
		return colorTerm;
	}




	std::vector<ColorTerm> loadHSVtoTerms(std::vector<ColorTerm> &colors) {

		

		std::string line;
		std::ifstream infile("HSV.txt");
		std::string colorTerm;
		int R_ori, G_ori, B_ori;

		std::vector<std::string> x;


		if (infile.is_open())
		{
			while (std::getline(infile, line))
			{


				x = split(line, ',');



				R_ori = stoi(x[0]);
				G_ori = stoi(x[1]);
				B_ori = stoi(x[2]);

				colorTerm = x[3];


				colors.push_back(ColorTerm(x[0]+","+x[1]+","+x[2], x[3]));
			}
		}

		return colors;
	}

	// function prototypes ////////////////////////////////////////////////////////////////////////
	
	

	ColorTerm(std::string const & RGBInput, std::string const & NameInput);
	~ColorTerm();

};

#endif    // MY_Color


#pragma once
