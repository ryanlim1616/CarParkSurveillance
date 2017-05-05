/*
This file is part of BGSLibrary.
BGSLibrary is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
BGSLibrary is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with BGSLibrary.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "FrameDifferenceBGS.h"

FrameDifferenceBGS::FrameDifferenceBGS() : firstTime(true), enableThreshold(true), threshold(15), showOutput(true)
{
	std::cout << "FrameDifferenceBGS()" << std::endl;
}

FrameDifferenceBGS::~FrameDifferenceBGS()
{
	std::cout << "~FrameDifferenceBGS()" << std::endl;

}

void FrameDifferenceBGS::process(const cv::Mat &img_input, cv::Mat &img_output, cv::Mat &img_bgmodel)
{

	if (firstTime == true) {
		img_input.convertTo(meanImage, CV_32F);
	}

	cv::Mat structuringElement1x1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
	cv::Mat structuringElement12x12 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(12, 12));


	if (img_input.empty())
		return;


	if (img_input_prev.empty())
	{
		img_input.copyTo(img_input_prev);
		return;
	}

	cv::absdiff(img_input_prev, img_input, img_foreground);

	if (img_foreground.channels() == 3)
		cv::cvtColor(img_foreground, img_foreground, CV_BGR2GRAY);


	cv::threshold(img_foreground, img_foreground, threshold, 255, cv::THRESH_BINARY);


	cv::erode(img_foreground, img_foreground, structuringElement1x1);
	cv::dilate(img_foreground, img_foreground, structuringElement12x12);


	//	cv::erode(img_foreground, img_foreground, structuringElement3x3);
	//cv::dilate(img_foreground, img_foreground, structuringElement7x7);

	//cv::erode(img_foreground, img_foreground, structuringElement3x3);
	//cv::dilate(img_foreground, img_foreground, structuringElement10x10);


	img_foreground.copyTo(img_output);

	if (imshow_display)
	cv::imshow("Frame Difference", img_foreground);

	//if (couter < 100) {
	//	cv::accumulate(img_input, meanImage);
	////	cv::imshow("uuq", meanImage);
	//	couter++;
	//}
	//else {
	//	meanImage = meanImage / 101;
	//	meanImage.convertTo(meanImage2, CV_8U);
	//	cv::imshow("uuq", meanImage2);
	//	meanImage2.copyTo(img_input_prev);
	//	img_input.convertTo(meanImage, CV_32F);
	//	couter = 0;
	//}

	//if (couter < 1) {
	//	couter++;
	//}
	//else {
	//	img_input.copyTo(img_input_prev);
	//	couter = 0;
	//}

	if (update == true) {
		img_input.copyTo(img_input_prev);
		update = false;
	}


	//	img_input.copyTo(img_input_prev);
	firstTime = false;

	structuringElement1x1.release();
	structuringElement12x12.release();
	


}

void FrameDifferenceBGS::updatemask() {
	update = true;
}

void FrameDifferenceBGS::saveConfig()
{

}

void FrameDifferenceBGS::loadConfig()
{

}