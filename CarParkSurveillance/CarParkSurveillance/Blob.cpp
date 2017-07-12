// Blob.cpp

#include "Blob.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
Blob::~Blob() {
	currentContour.clear();
	centerPositions.clear();
	AvgColor.clear();
	image.release();
	rawImage.release();
	maskImage.release();
	keypoints_new.clear();

	points.clear();
	des.release();
	desNoGpu.release();
	
}

Blob::Blob(std::vector<cv::Point> _contour) {
	


	addornot = false;
	currentContour = _contour;

	currentBoundingRect = cv::boundingRect(currentContour);



	cv::Point currentCenter;

	currentCenter.x = (currentBoundingRect.x + currentBoundingRect.x + currentBoundingRect.width) / 2;
	currentCenter.y = (currentBoundingRect.y + currentBoundingRect.y + currentBoundingRect.height) / 2;

	centerPositions.push_back(currentCenter);

	dblCurrentDiagonalSize = sqrt(pow(currentBoundingRect.width, 2) + pow(currentBoundingRect.height, 2));

	dblCurrentAspectRatio = (float)currentBoundingRect.width / (float)currentBoundingRect.height;

	blnStillBeingTracked = true;
	blnCurrentMatchFoundOrNewBlob = true;

	////test
	isAdded = true;
	////

	intNumOfConsecutiveFramesWithoutAMatch = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Blob::predictNextPosition(void) {

	int numPositions = (int)centerPositions.size();

	if (numPositions == 1) {

		predictedNextPosition.x = centerPositions.back().x;
		predictedNextPosition.y = centerPositions.back().y;

	}
	
	else if (numPositions == 2) {

		int deltaX = centerPositions[1].x - centerPositions[0].x;
		int deltaY = centerPositions[1].y - centerPositions[0].y;

		predictedNextPosition.x = centerPositions.back().x + deltaX;
		predictedNextPosition.y = centerPositions.back().y + deltaY;

	}
	else if (numPositions == 3) {

		int sumOfXChanges = ((centerPositions[2].x - centerPositions[1].x) * 2) +
			((centerPositions[1].x - centerPositions[0].x) * 1);

		int deltaX = (int)std::round((float)sumOfXChanges / 3.0);

		int sumOfYChanges = ((centerPositions[2].y - centerPositions[1].y) * 2) +
			((centerPositions[1].y - centerPositions[0].y) * 1);

		int deltaY = (int)std::round((float)sumOfYChanges / 3.0);

		predictedNextPosition.x = centerPositions.back().x + deltaX;
		predictedNextPosition.y = centerPositions.back().y + deltaY;

	}
	else if (numPositions == 4) {

		int sumOfXChanges = ((centerPositions[3].x - centerPositions[2].x) * 3) +
			((centerPositions[2].x - centerPositions[1].x) * 2) +
			((centerPositions[1].x - centerPositions[0].x) * 1);

		int deltaX = (int)std::round((float)sumOfXChanges / 6.0);

		int sumOfYChanges = ((centerPositions[3].y - centerPositions[2].y) * 3) +
			((centerPositions[2].y - centerPositions[1].y) * 2) +
			((centerPositions[1].y - centerPositions[0].y) * 1);

		int deltaY = (int)std::round((float)sumOfYChanges / 6.0);

		predictedNextPosition.x = centerPositions.back().x + deltaX;
		predictedNextPosition.y = centerPositions.back().y + deltaY;

	}
	else if (numPositions == 5) {



		int sumOfXChanges = ((centerPositions[4].x - centerPositions[3].x) * 4) +
			((centerPositions[3].x - centerPositions[2].x) * 3) +
			((centerPositions[2].x - centerPositions[1].x) * 2) +
			((centerPositions[1].x - centerPositions[0].x) * 1);

		int deltaX = (int)std::round((float)sumOfXChanges / 10.0);

		int sumOfYChanges = ((centerPositions[4].y - centerPositions[3].y) * 4) +
			((centerPositions[3].y - centerPositions[2].y) * 3) +
			((centerPositions[2].y - centerPositions[1].y) * 2) +
			((centerPositions[1].y - centerPositions[0].y) * 1);

		int deltaY = (int)std::round((float)sumOfYChanges / 10.0);

		predictedNextPosition.x = centerPositions.back().x + deltaX;
		predictedNextPosition.y = centerPositions.back().y + deltaY;

	}
	else if (numPositions == 6) {
		int sumOfXChanges = ((centerPositions[5].x - centerPositions[4].x) * 5) +
			((centerPositions[4].x - centerPositions[3].x) * 4) +
			((centerPositions[3].x - centerPositions[2].x) * 3) +
			((centerPositions[2].x - centerPositions[1].x) * 2) +
			((centerPositions[1].x - centerPositions[0].x) * 1);

		int deltaX = (int)std::round((float)sumOfXChanges / 15.0);

		int sumOfYChanges = ((centerPositions[5].y - centerPositions[4].y) * 5) +
			((centerPositions[4].y - centerPositions[3].y) * 4) +
			((centerPositions[3].y - centerPositions[2].y) * 3) +
			((centerPositions[2].y - centerPositions[1].y) * 2) +
			((centerPositions[1].y - centerPositions[0].y) * 1);

		int deltaY = (int)std::round((float)sumOfYChanges / 15.0);

		predictedNextPosition.x = centerPositions.back().x + deltaX;
		predictedNextPosition.y = centerPositions.back().y + deltaY;
	}
	else if (numPositions == 7) {
		int sumOfXChanges = ((centerPositions[6].x - centerPositions[5].x) * 6) +
			((centerPositions[5].x - centerPositions[4].x) * 5) +
			((centerPositions[4].x - centerPositions[3].x) * 4) +
			((centerPositions[3].x - centerPositions[2].x) * 3) +
			((centerPositions[2].x - centerPositions[1].x) * 2) +
			((centerPositions[1].x - centerPositions[0].x) * 1);

		int deltaX = (int)std::round((float)sumOfXChanges / 21.0);

		int sumOfYChanges = ((centerPositions[6].y - centerPositions[5].y) * 6) +
			((centerPositions[5].y - centerPositions[4].y) * 5) +
			((centerPositions[4].y - centerPositions[3].y) * 4) +
			((centerPositions[3].y - centerPositions[2].y) * 3) +
			((centerPositions[2].y - centerPositions[1].y) * 2) +
			((centerPositions[1].y - centerPositions[0].y) * 1);

		int deltaY = (int)std::round((float)sumOfYChanges / 21.0);

		predictedNextPosition.x = centerPositions.back().x + deltaX;
		predictedNextPosition.y = centerPositions.back().y + deltaY;
	}

	else if (numPositions == 8) {
		int sumOfXChanges = ((centerPositions[7].x - centerPositions[6].x) * 7) +
			((centerPositions[6].x - centerPositions[5].x) * 6) +
			((centerPositions[5].x - centerPositions[4].x) * 5) +
			((centerPositions[4].x - centerPositions[3].x) * 4) +
			((centerPositions[3].x - centerPositions[2].x) * 3) +
			((centerPositions[2].x - centerPositions[1].x) * 2) +
			((centerPositions[1].x - centerPositions[0].x) * 1);

		int deltaX = (int)std::round((float)sumOfXChanges / 28.0);

		int sumOfYChanges = ((centerPositions[7].y - centerPositions[6].y) * 7) +
			((centerPositions[6].y - centerPositions[5].y) * 6) +
			((centerPositions[5].y - centerPositions[4].y) * 5) +
			((centerPositions[4].y - centerPositions[3].y) * 4) +
			((centerPositions[3].y - centerPositions[2].y) * 3) +
			((centerPositions[2].y - centerPositions[1].y) * 2) +
			((centerPositions[1].y - centerPositions[0].y) * 1);

		int deltaY = (int)std::round((float)sumOfYChanges / 28.0);

		predictedNextPosition.x = centerPositions.back().x + deltaX;
		predictedNextPosition.y = centerPositions.back().y + deltaY;
	}

	else if (numPositions == 9) {
		int sumOfXChanges = ((centerPositions[8].x - centerPositions[7].x) * 8) +
			((centerPositions[7].x - centerPositions[6].x) * 7) +
			((centerPositions[6].x - centerPositions[5].x) * 6) +
			((centerPositions[5].x - centerPositions[4].x) * 5) +
			((centerPositions[4].x - centerPositions[3].x) * 4) +
			((centerPositions[3].x - centerPositions[2].x) * 3) +
			((centerPositions[2].x - centerPositions[1].x) * 2) +
			((centerPositions[1].x - centerPositions[0].x) * 1);

		int deltaX = (int)std::round((float)sumOfXChanges / 36.0);

		int sumOfYChanges = ((centerPositions[8].y - centerPositions[7].y) * 8) +
			((centerPositions[7].y - centerPositions[6].y) * 7) +
			((centerPositions[6].y - centerPositions[5].y) * 6) +
			((centerPositions[5].y - centerPositions[4].y) * 5) +
			((centerPositions[4].y - centerPositions[3].y) * 4) +
			((centerPositions[3].y - centerPositions[2].y) * 3) +
			((centerPositions[2].y - centerPositions[1].y) * 2) +
			((centerPositions[1].y - centerPositions[0].y) * 1);

		int deltaY = (int)std::round((float)sumOfYChanges / 36.0);

		predictedNextPosition.x = centerPositions.back().x + deltaX;
		predictedNextPosition.y = centerPositions.back().y + deltaY;
	}
	else if (numPositions >= 10) {
		int sumOfXChanges = ((centerPositions[numPositions - 1].x - centerPositions[numPositions - 2].x) * 9) +
			((centerPositions[numPositions - 2].x - centerPositions[numPositions - 3].x) * 8) +
			((centerPositions[numPositions - 3].x - centerPositions[numPositions - 4].x) * 7) +
			((centerPositions[numPositions - 4].x - centerPositions[numPositions - 5].x) * 6) +
			((centerPositions[numPositions - 5].x - centerPositions[numPositions - 6].x) * 5) +
			((centerPositions[numPositions - 6].x - centerPositions[numPositions - 7].x) * 4) +
			((centerPositions[numPositions - 7].x - centerPositions[numPositions - 8].x) * 3) +
			((centerPositions[numPositions - 8].x - centerPositions[numPositions - 9].x) * 2) +
			((centerPositions[numPositions - 9].x - centerPositions[numPositions - 10].x) * 1);

		int deltaX = (int)std::round((float)sumOfXChanges / 45.0);

		int sumOfYChanges = ((centerPositions[numPositions - 1].y - centerPositions[numPositions - 2].y) * 9) +
			((centerPositions[numPositions - 2].y - centerPositions[numPositions - 3].y) * 8) +
			((centerPositions[numPositions - 3].y - centerPositions[numPositions - 4].y) * 7) +
			((centerPositions[numPositions - 4].y - centerPositions[numPositions - 5].y) * 6) +
			((centerPositions[numPositions - 5].y - centerPositions[numPositions - 6].y) * 5) +
			((centerPositions[numPositions - 6].y - centerPositions[numPositions - 7].y) * 4) +
			((centerPositions[numPositions - 7].y - centerPositions[numPositions - 8].y) * 3) +
			((centerPositions[numPositions - 8].y - centerPositions[numPositions - 9].y) * 2) +
			((centerPositions[numPositions - 9].y - centerPositions[numPositions - 10].y) * 1);

		int deltaY = (int)std::round((float)sumOfYChanges / 45.0);

		predictedNextPosition.x = centerPositions.back().x + deltaX;
		predictedNextPosition.y = centerPositions.back().y + deltaY;
	}

}

void Blob::predictNextPositionPark(void) {
	predictedNextPosition.x = centerPositions.back().x;
	predictedNextPosition.y = centerPositions.back().y;
}
void Blob::storeImage(cv::Mat rawImage) {
	this->rawImage = rawImage.clone();

	image = cv::Mat(rawImage.size(), CV_8U, cv::Scalar(0.0, 0.0, 0.0));

	std::vector<std::vector<cv::Point> > contourVec;
	contourVec.push_back(currentContour);
	cv::drawContours(image, contourVec, -1, cv::Scalar(255.0, 255.0, 255.0), -1);
	contourVec.clear();

	cv::Mat colorForegrounds = cv::Mat::zeros(image.size(), image.type());
	rawImage.copyTo(colorForegrounds, image);
	cv::cvtColor(colorForegrounds, colorForegrounds, CV_BGR2GRAY);
	maskImage = colorForegrounds.clone();

	colorForegrounds.release();

	
}




void Blob::getAverageColor(std::vector<ColorTerm> &inputColorVector) {

	////clarence
	cv::Mat hsv, hsv2;
	cv::Mat temp_mat = cv::Mat::zeros(10, 10, CV_8UC3);
	cv::Mat blurredImg, resizedBlurredImg;

	cv::Size Ksize;
	Ksize.height = 5;
	Ksize.width = 5;

	cv::Size resizeScale;
	resizeScale.height = 400;
	resizeScale.width = 400;

	cv::Rect tempBoundingRect = currentBoundingRect;
	cv::Size inflationSize(currentBoundingRect.width*0.3, currentBoundingRect.height*0.3);

	tempBoundingRect -= inflationSize;
	tempBoundingRect.x += inflationSize.width / 2;
	tempBoundingRect.y += inflationSize.height / 2;

	////clarence end


	cv::Mat cropImage = rawImage(tempBoundingRect);
	cv::Scalar average = cv::mean(cropImage);
	//clarence commented out -- START --
	//AvgColor.push_back(average);
	//AvgColorScalar = average;
	//clarence -- END -- 

	//perform gaussian blur around the img, avoid noise
	cv::GaussianBlur(cropImage, blurredImg, Ksize, 5);

	////perform minmaxloc to find the highest pixel value
	////draw a circle around the highest peak
	//cv::hconcat(cropImage, blurredImg, resizedBlurredImg);

	double maxVal = 0;
	cv::Point min_locBlur, max_locBlur;

	//cv::Mat reshapedMat;
	//reshapedMat = blurredImg.reshape(1);

	//minMaxLoc(reshapedMat, 0, &maxVal, &min_locBlur, &max_locBlur);

	//cv::circle(blurredImg, max_locBlur, Ksize.width, (0,255,0), 1);



	////if (imshow_display)
	//cv::hconcat(resizedBlurredImg, blurredImg, resizedBlurredImg);
	//cv::resize(resizedBlurredImg, resizedBlurredImg, resizeScale);
	//cv::imshow("Gaussian Blur", resizedBlurredImg);


	cvtColor(blurredImg, hsv, CV_BGR2HSV);
	//cvtColor(cropImage, hsv, CV_BGR2HSV);
	//calculate the Hue-Saturation histogram
	int hbins = 15, sbins = 8, vbins = 8;
	int histSize[] = { hbins, sbins, vbins };
	// hue varies from 0 to 179, see cvtColor
	float hranges[] = { 0, 180 };
	// saturation & value varies from 0 (black-gray-white) to
	// 255 (pure spectrum color)
	float sranges[] = { 0, 256 };
	float vranges[] = { 0, 256 };

	const float* ranges[] = { hranges, sranges, vranges };
	cv::MatND hist;
	// we compute the histogram from the 0-th and 1-st channels
	int channels[] = { 0, 1, 2 };

	calcHist(&hsv, 1, channels, cv::Mat(), // do not use mask
		hist, 3, histSize, ranges,
		true, // the histogram is uniform
		false);
	//double maxVal = 0;

	//cv::Point min_loc, max_loc;
	//minMaxLoc(hist, 0, &maxVal, &min_loc, &max_loc);  <-- if there's only 2 dimension

	int max_loc[3];
	minMaxIdx(hist, 0, 0, 0, max_loc, cv::Mat());


	// formula: Hue			= ((Current bin+next bin)/2)* ((180 hue/30 bin)*2(to change to GIMP 360)
	// formula: Saturation	= (current bin+next bin)/2) * (100/32) --< eg: bin value/32 bins * 100(gimp max sat)
	// formula: value		= ((current bin + next bin)/2) * (100/32) --< eg: bin value/32 bins * 100 (gimp max value)

	//std::cout << "Hue range, Saturation, Value : (" << int((max_loc[0])) * 12 << "), " << int(((max_loc[1] + max_loc[1] + 1)/2) * 3.125) <<  ", " << int(((max_loc[2] + max_loc[2] + 1)/2) * 3.125) << std::endl;	

	//std::cout << ">> HSV: " << max_loc[0] << "," << max_loc[1] << ","<< max_loc[2] <<std::endl;

	average[0] = int(max_loc[0] * 12);  // each bin represents 12 HUES ///// 6hues (if 30,32,32)
	average[1] = int(max_loc[1] * 32);  // each bin represents 32 SATURATION //8sat (if 30,32,32)
	average[2] = int(max_loc[2] * 32);  // each bin represents 32 VALUES// 8val (if 30,32,32)

	hsv = average;  // set MAT to the HSV color
	cvtColor(hsv, temp_mat, CV_HSV2BGR);  // convert it back to BGR
	average = cv::mean(temp_mat); // get the color in BGR format
	AvgColorScalar = average;
	
	


	AvgColorScalar[0] = int((average[0] + AvgColorScalar[0]) / 2);
	AvgColorScalar[1] = int((average[1] + AvgColorScalar[1]) / 2);
	AvgColorScalar[2] = int((average[2] + AvgColorScalar[2]) / 2);


	AvgColor.push_back(AvgColorScalar);







	temp_mat = AvgColorScalar;  // set MAT to the averaged HSV color
	//std::cout << AvgColorScalar << std::endl;

	cvtColor(temp_mat, hsv, CV_BGR2HSV);

	//recalculate the Histogram value
	calcHist(&hsv, 1, channels, cv::Mat(), // do not use mask
		hist, 3, histSize, ranges,
		true, // the histogram is uniform
		false);

	max_loc[0] = 0;
	max_loc[1] = 0;
	max_loc[2] = 0;

	//reobtain the value
	minMaxIdx(hist, 0, 0, 0, max_loc, cv::Mat());

	std::string tempString = std::to_string(max_loc[0]) + "," + std::to_string(max_loc[1]) + "," + std::to_string(max_loc[2]);

	ColorTerm allColors("Null","Null");
	ColorInTerms = allColors.getTerm(inputColorVector, tempString);


	ColorInTerms = ColorInTerms + " HSV:" + std::to_string(max_loc[0]) + ", " + std::to_string(max_loc[1]) + ", " + std::to_string(max_loc[2]);
	//test

	if(debug_on)
	std::cout << "unitID: " << unitID << ", " << ColorInTerms  << ">> HSV: " << max_loc[0] << ", " << max_loc[1] << ", "<< max_loc[2] << std::endl;

	//int scale = 10;
	//cv::Mat histImg = cv::Mat::zeros(sbins*scale, hbins*scale, CV_8UC3);


	//int h_max, s_max;

	//to display and compute histogram
	//for (int h = 0; h < hbins; h++)
	//	for (int s = 0; s < sbins; s++)
	//	{
	//		float binVal = hist.at<float>(h, s);
	//		int intensity = cvRound(binVal * 255 / maxVal);
	//		rectangle(histImg, cv::Point(h*scale, s*scale),
	//			cv::Point((h + 1)*scale - 1, (s + 1)*scale - 1),
	//			cv::Scalar::all(intensity),
	//			CV_FILLED);

	//	}

	////display histogram:
	//cv::imshow("H-S histogram", histImg);

	hsv.release();
	temp_mat.release();
	cropImage.release();

}	


//void Blob::getAverageColor() {
//
//	////clarence
//	cv::Mat hsv;
//	cv::Mat temp_mat = cv::Mat::zeros(10, 10, CV_8UC3);
//
//
//	cv::Rect tempBoundingRect = currentBoundingRect;
//	cv::Size inflationSize(currentBoundingRect.width*0.4, currentBoundingRect.height*0.4);
//
//	tempBoundingRect -= inflationSize;
//	tempBoundingRect.x += inflationSize.width / 2;
//	tempBoundingRect.y += inflationSize.height / 2;
//
//	////clarence end
//
//
//	cv::Mat cropImage = rawImage(tempBoundingRect);
//	cv::Scalar average = cv::mean(cropImage);
//	//clarence commented out -- START --
//	//AvgColor.push_back(average);
//	//AvgColorScalar = average;
//	//clarence -- END -- 
//
//
//	cvtColor(cropImage, hsv, CV_BGR2HSV);
//	//calculate the Hue-Saturation histogram
//	int hbins = 15, sbins = 8, vbins = 8;
//	int histSize[] = { hbins, sbins, vbins };
//	// hue varies from 0 to 179, see cvtColor
//	float hranges[] = { 0, 180 };
//	// saturation & value varies from 0 (black-gray-white) to
//	// 255 (pure spectrum color)
//	float sranges[] = { 0, 256 };
//	float vranges[] = { 0, 256 };
//
//	const float* ranges[] = { hranges, sranges, vranges };
//	cv::MatND hist;
//	// we compute the histogram from the 0-th and 1-st channels
//	int channels[] = { 0, 1, 2 };
//
//	calcHist(&hsv, 1, channels, cv::Mat(), // do not use mask
//		hist, 3, histSize, ranges,
//		true, // the histogram is uniform
//		false);
//	double maxVal = 0;
//
//	//cv::Point min_loc, max_loc;
//	//minMaxLoc(hist, 0, &maxVal, &min_loc, &max_loc);  <-- if there's only 2 dimension
//
//	int max_loc[3];
//	minMaxIdx(hist, 0, 0, 0, max_loc, cv::Mat());
//
//
//	// formula: Hue			= ((Current bin+next bin)/2)* ((180 hue/30 bin)*2(to change to GIMP 360)
//	// formula: Saturation	= (current bin+next bin)/2) * (100/32) --< eg: bin value/32 bins * 100(gimp max sat)
//	// formula: value		= ((current bin + next bin)/2) * (100/32) --< eg: bin value/32 bins * 100 (gimp max value)
//
//	//std::cout << "Hue range, Saturation, Value : (" << int((max_loc[0])) * 12 << "), " << int(((max_loc[1] + max_loc[1] + 1)/2) * 3.125) <<  ", " << int(((max_loc[2] + max_loc[2] + 1)/2) * 3.125) << std::endl;	
//
//	std::cout << ">> HSV: " << max_loc[0] << "," << max_loc[1] << "," << max_loc[2] << std::endl;
//
//	average[0] = int(max_loc[0] * 12);  // each bin represents 12 HUES ///// 6hues (if 30,32,32)
//	average[1] = int(max_loc[1] * 32);  // each bin represents 32 SATURATION //8sat (if 30,32,32)
//	average[2] = int(max_loc[2] * 32);  // each bin represents 32 VALUES// 8val (if 30,32,32)
//
//	hsv = average;  // set MAT to the HSV color
//	cvtColor(hsv, temp_mat, CV_HSV2BGR);  // convert it back to BGR
//	average = cv::mean(temp_mat); // get the color in BGR format
//	AvgColorScalar = average;
//
//	
//
//	AvgColorScalar[0] = (average[0] + AvgColorScalar[0]) / 2;
//	AvgColorScalar[1] = (average[1] + AvgColorScalar[1]) / 2;
//	AvgColorScalar[2] = (average[2] + AvgColorScalar[2]) / 2;
//
//
//	AvgColor.push_back(AvgColorScalar);
//
//	
//
//	//int scale = 10;
//	//cv::Mat histImg = cv::Mat::zeros(sbins*scale, hbins*scale, CV_8UC3);
//
//
//	//int h_max, s_max;
//
//	//to display and compute histogram
//	//for (int h = 0; h < hbins; h++)
//	//	for (int s = 0; s < sbins; s++)
//	//	{
//	//		float binVal = hist.at<float>(h, s);
//	//		int intensity = cvRound(binVal * 255 / maxVal);
//	//		rectangle(histImg, cv::Point(h*scale, s*scale),
//	//			cv::Point((h + 1)*scale - 1, (s + 1)*scale - 1),
//	//			cv::Scalar::all(intensity),
//	//			CV_FILLED);
//
//	//	}
//
//	////display histogram:
//	//cv::imshow("H-S histogram", histImg);
//
//	hsv.release();
//	temp_mat.release();
//	cropImage.release();
//
//}

void Blob::getAverageColorLast() {
	avgColorBeforeMerge = cv::mean(rawImage, image);
}


void Blob::drawMaskImage() {

	cv::Mat image(rawImage.size(), CV_8UC3, cv::Scalar(0.0, 0.0, 0.0));

	std::vector<std::vector<cv::Point> > contourVec;
	contourVec.push_back(currentContour);
	cv::drawContours(image, contourVec, -1, cv::Scalar(255.0, 255.0, 255.0), -1);


	cv::Mat colorForegrounds = cv::Mat::zeros(image.size(), image.type());
	rawImage.copyTo(colorForegrounds, image);


	cv::imshow("cut", colorForegrounds);
}

void Blob::getFeatures() {

	cv::Mat image(rawImage.size(), CV_8UC3, cv::Scalar(0.0, 0.0, 0.0));

	std::vector<std::vector<cv::Point> > contourVec;
	contourVec.push_back(currentContour);
	cv::drawContours(image, contourVec, -1, cv::Scalar(255.0, 255.0, 255.0), -1);


	cv::Mat colorForegrounds = cv::Mat::zeros(image.size(), image.type());
	rawImage.copyTo(colorForegrounds, image);

	cv::cvtColor(colorForegrounds, colorForegrounds, CV_BGR2GRAY);




	cv::gpu::GpuMat colourForegroundGPU(colorForegrounds);

	cv::gpu::FAST_GPU::FAST_GPU(15).operator() (colourForegroundGPU, colourForegroundGPU, keypoints_new);


	for (it = keypoints_new.begin(); it != keypoints_new.end(); it++) {
		points.push_back(it->pt);
	}




	cv::drawKeypoints(colorForegrounds, keypoints_new, colorForegrounds);
	//cv::imshow("sds", colorForegrounds);

	//std::cout << "get features done\n";






}

cv::gpu::GpuMat Blob::getGpuImage() {

	cv::Mat image(rawImage.size(), CV_8UC3, cv::Scalar(0.0, 0.0, 0.0));

	std::vector<std::vector<cv::Point> > contourVec;
	contourVec.push_back(currentContour);
	cv::drawContours(image, contourVec, -1, cv::Scalar(255.0, 255.0, 255.0), -1);


	cv::Mat colorForegrounds = cv::Mat::zeros(image.size(), image.type());
	rawImage.copyTo(colorForegrounds, image);

	cv::cvtColor(colorForegrounds, colorForegrounds, CV_BGR2GRAY);
	cv::gpu::GpuMat colourForegroundGPU(colorForegrounds);

	//std::cout << "get GPU done\n";

	return colourForegroundGPU;
}

void Blob::desFeatures() {
	cv::gpu::ORB_GPU descriptorGPU;

	descriptorGPU.operator() (getGpuImage(), getGpuImage(), keypoints_new, des);

	//std::cout << "get des done\n";

	/*cv::SurfDescriptorExtractor extractor;

	extractor.compute(rawImage, keypoints_new, desNoGpu);
	std::cout << "des " << desNoGpu.size() << " " << desNoGpu.type() << "\n";*/

}

cv::Mat Blob::getDes() {
	if (!des.empty())
		des.download(desNoGpu);

	//std::cout << desNoGpu.size() << " " << desNoGpu.type() <<"\n";
	//	std::cout << "got it\n";


	return desNoGpu;

	//std::cout << "got it\n";
}

cv::gpu::GpuMat Blob::getGpuDes() {

	return des;
}

cv::Mat Blob::getRawImage() {
	return rawImage;
}

void Blob::useORBGPU() {
	cv::gpu::ORB_GPU orb(1000);


	cv::Mat image(rawImage.size(), CV_8UC3, cv::Scalar(0.0, 0.0, 0.0));

	std::vector<std::vector<cv::Point> > contourVec;
	contourVec.push_back(currentContour);
	cv::drawContours(image, contourVec, -1, cv::Scalar(255.0, 255.0, 255.0), -1);


	cv::Mat colorForegrounds = cv::Mat::zeros(image.size(), image.type());
	rawImage.copyTo(colorForegrounds, image);

	cv::cvtColor(colorForegrounds, colorForegrounds, CV_BGR2GRAY);




	cv::gpu::GpuMat colourForegroundGPU(colorForegrounds);

	orb(colourForegroundGPU, cv::gpu::GpuMat(), keypoints_new, des);




}

cv::Scalar Blob::getAverageColorOnce() {
	cv::Scalar average = cv::mean(rawImage, image);
	return average;
}

void Blob::setEnter() {
	enter = true;
}

void Blob::setExit(){
	exit = true;
}

void Blob::setPark() {
	park = true;

}




