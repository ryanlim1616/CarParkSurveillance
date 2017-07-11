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
