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




void Blob::getAverageColor(std::vector<std::vector<double>> &w2c) {
	//void Blob::getAverageColor(std::vector<ColorTerm> &inputColorVector) {
	// void Blob::getAverageColor(std::vector<ColorTerm> &inputColorVector, firefly) {



	bool chromaticScale = true;
	bool achromaticScale2 = true;
	////clarence
	cv::Mat hsv, hsv2;
	cv::Mat temp_mat = cv::Mat::zeros(10, 10, CV_8UC3);
	cv::Mat hconcate_mat = cv::Mat::zeros(10, 10, CV_8UC3);




	cv::Rect tempBoundingRect = currentBoundingRect;
	cv::Size inflationSize(currentBoundingRect.width*0.3, currentBoundingRect.height*0.3);

	tempBoundingRect -= inflationSize;
	tempBoundingRect.x += inflationSize.width / 2;
	tempBoundingRect.y += inflationSize.height / 2;

	////clarence end



	cv::Mat cropImage = rawImage(tempBoundingRect);

	//test - amplify the Saturation layer
	
		cv::Mat hsv_mat;

		cvtColor(cropImage, hsv_mat, CV_BGR2HSV);

		std::vector<cv::Mat> channelsx;
		split(hsv_mat, channelsx);

		cv::Mat saturated;

		double saturation = 40;
		double scale = 1;

		// what it does here is dst = (uchar) ((double)src*scale+saturation); 
		channelsx[1].convertTo(channelsx[1], CV_8UC1, scale, saturation);
	

		cv::Mat result;
		cv::merge(channelsx, hsv_mat);

		cvtColor(hsv_mat, result, CV_HSV2BGR);


		//hsv_mat = result;



	cv::Scalar average = cv::mean(cropImage);


	//cv::Mat colorBalanced, colorBalancedOut;
	

	//balance_white(colorBalanced);
	//SimplestCB(cropImage, colorBalanced, 0.5);
	//colorBalanced = equalizeIntensity(cropImage);

	//firefly.Adjust(cropImage, colorBalanced);


	//cv::hconcat(cropImage, colorBalanced, colorBalancedOut);
	//cv::resize(colorBalancedOut, colorBalancedOut, cv::Size(), 5, 5, cv::INTER_LINEAR);

	//cv::imshow("colorBalanced", colorBalancedOut);


	//test: update the cropImage to Balanced image
	//cropImage = colorBalanced;

	//sobel method
	/* 

	//test sobel filter method:
	int dx = 1, dy = 1;
	int scale = 1;
	int delta = 0;

	cv::Mat sobelImage = rawImage(currentBoundingRect);

	//perform blur
	GaussianBlur(sobelImage, sobelImage, cv::Size(dx, dy), dx);

	//invert
	//bitwise_not(sobelImage, sobelImage);
	cv::Mat sobelImageout;

	sobelImageout = sobelImage;
	
	//use sobel filter:
	
	//change image to grayscale
	cvtColor(sobelImage, sobelImage, CV_BGR2GRAY);

	
	
	cv::Mat grad_x, grad_y;
	cv::Mat abs_grad_x, abs_grad_y;
	cv::Mat grad;

	int ddepth = CV_16S;

	

	//cv::Scharr(sobelImage, sobelImageout, CV_16S, dx, dy);
	cv::Scharr(sobelImage, grad_x, ddepth, dx, 0, scale, delta, cv::BORDER_DEFAULT);
	cv::Scharr(sobelImage, grad_y, ddepth, 0, dy, scale, delta, cv::BORDER_DEFAULT);
	
	//![convert]
	convertScaleAbs(grad_x, abs_grad_x);
	convertScaleAbs(grad_y, abs_grad_y);

	//![blend]
	/// Total Gradient (approximate)
	addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);
	//![blend]

	//perform threshold
	int sobelthres_val = 65;
	cv::threshold(grad, grad, sobelthres_val, 255.0, CV_THRESH_BINARY);


	cv::Mat structuringElement3x3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::Mat structuringElement5x5 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
	cv::Mat structuringElement7x7 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));



	cv::erode(grad, grad, structuringElement3x3);
	cv::dilate(grad, grad, structuringElement7x7);
	cv::dilate(grad, grad, structuringElement3x3);

	cv::erode(grad, grad, structuringElement7x7);
	


	cv::resize(grad, sobelImage, cv::Size(), 5, 5, cv::INTER_LINEAR);

	cv::imshow("sobelImage", sobelImage);

	cv::resize(sobelImageout, sobelImageout, cv::Size(), 5, 5, cv::INTER_LINEAR);

	cv::imshow("sobelImageout", sobelImageout);
	
	
	
	//![reduce_noise]
  GaussianBlur( src, src, Size(3,3), 0, 0, BORDER_DEFAULT );
  //![reduce_noise]

  //![convert_to_gray]
  cvtColor( src, src_gray, COLOR_BGR2GRAY );
  //![convert_to_gray]

  //![sobel]
  /// Generate grad_x and grad_y
  Mat grad_x, grad_y;
  Mat abs_grad_x, abs_grad_y;

  /// Gradient X
  //Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
  Sobel( src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );

  /// Gradient Y
  //Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
  Sobel( src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
  //![sobel]

  //![convert]
  convertScaleAbs( grad_x, abs_grad_x );
  convertScaleAbs( grad_y, abs_grad_y );
  //![convert]

  //![blend]
  /// Total Gradient (approximate)
  addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );
  //![blend]

  //![display]
  imshow( window_name, grad );


	*/

	
	//graph cut method :
	// -- tested, when there's more than 1 vehicle, the algorithm slows down quite badly
	/*
	cv::Mat grabImage = rawImage(currentBoundingRect);
	cv::Mat grabMask;
	grabMask.create(grabImage.size(), CV_8UC1);
	cv::Mat bgdModel, fgdModel;

	cv::Rect grabRect = cv::Rect(0, 0, grabImage.size().width, grabImage.size().height);
	cv::Size grabinflationSize(currentBoundingRect.width*0.2, currentBoundingRect.height*0.2);
	grabRect -= grabinflationSize;
	grabRect.x += grabinflationSize.width / 2;
	grabRect.y += grabinflationSize.height / 2;


	cv::grabCut(grabImage, grabMask, grabRect, bgdModel, fgdModel, 1, cv::GC_INIT_WITH_RECT);

	//cv::imshow("Grab cut 1", grabImage);

	//std::cout << "grab image size: " << grabImage.size() << std::endl;

	for (int x = 0; x < grabMask.rows ; x++)
	{
		//std::cout << "x: " << x << std::endl;
		for (int y = 0; y < grabMask.cols ; y++)
		{
			
			//std::cout << "y: " << y << std::endl;

			if (grabMask.at<uchar>(x, y) == cv::GC_BGD || grabMask.at<uchar>(x, y) == cv::GC_PR_BGD)
			{
				

				grabImage.at<cv::Vec3b>(x, y)[0] = 0;
				grabImage.at<cv::Vec3b>(x, y)[1] = 0;
				grabImage.at<cv::Vec3b>(x, y)[2] = 0;


				//grabImage.at<uchar>(x, y) = 0;
			}

		}
	}





	cv::resize(grabImage, grabImage, cv::Size(), 5, 5, cv::INTER_LINEAR);
	cv::imshow("Grab cut 2", grabImage);
	*/


	//clarence commented out -- START --
	//AvgColor.push_back(average);
	//AvgColorScalar = average;
	//clarence -- END -- 


	

	



	//experiment: blur + brightest point
	// --- start ---
	//cv::Mat blurredImg, resizedBlurredImg;
	//int gaussianBlurVar = 3;
	//cv::Size Ksize;
	//Ksize.height = gaussianBlurVar;
	//Ksize.width = gaussianBlurVar;
	//cv::Size resizeScale;
	//resizeScale.height = 400;
	//resizeScale.width = 400;
	////perform gaussian blur around the img, avoid noise
	//cv::GaussianBlur(cropImage, blurredImg, Ksize, gaussianBlurVar);
	////perform minmaxloc to find the highest pixel value
	////draw a circle around the highest peak
	//cv::hconcat(cropImage, blurredImg, resizedBlurredImg);
	//cv::hconcat(cropImage, displayThis, displayThis);
	//cv::hconcat(cropImageGRAY2BGR, displayThis, displayThis);
	////cv::resize(displayThis, displayThis, resizeScale);
	//cv::resize(displayThis, displayThis, cv::Size(), 5, 5, cv::INTER_LINEAR);
	//double maxVal = 0;
	//cv::Point min_locBlur, max_locBlur;
	//cv::Mat reshapedMat;
	////reshapedMat = blurredImg.reshape(1);
	//cvtColor(blurredImg, reshapedMat, CV_BGR2GRAY);
	//minMaxLoc(reshapedMat, 0, &maxVal, &min_locBlur, &max_locBlur);
	////if (imshow_display)
	//cv::circle(blurredImg, max_locBlur, Ksize.width, (0, 0, 255), 1);
	//cv::hconcat(resizedBlurredImg, blurredImg, resizedBlurredImg);
	//cv::resize(resizedBlurredImg, resizedBlurredImg, cv::Size(), 5, 5, cv::INTER_LINEAR);
	//cv::vconcat(resizedBlurredImg, displayThis, resizedBlurredImg);
	//cv::imshow("Original->Gaussian Blur->Peak/Diff between Gray & RGB", resizedBlurredImg);
	//// --- experiment end ---

	//check the difference between RGB and grayscale image

	cv::Mat cropImageGRAY, cropImageGRAY2BGR, displayThis, dst, displayThisGray;
	
	cvtColor(cropImage, cropImageGRAY, CV_BGR2GRAY);
	cvtColor(cropImageGRAY, cropImageGRAY2BGR, CV_GRAY2BGR);
	absdiff(result, cropImageGRAY2BGR, displayThis);


	int thres_val = 45;

	cv::threshold(displayThis, dst, thres_val, 255.0, CV_THRESH_BINARY);
	cvtColor(dst, displayThisGray, CV_BGR2GRAY);


	

	
	
	/*
	//test 250717
	//description: instead of using grayscale, use the V channel of HSV as the B&W instead

	Using HSV -- v channel as the "grayscale" image did not help improve the algorithm, more or less the same, slight decrease in performance

	cvtColor(cropImage, cropImageGRAY, CV_BGR2HSV);

	setChannel(cropImageGRAY, 0, 1);
	setChannel(cropImageGRAY, 1, 1);


	cvtColor(cropImageGRAY, cropImageGRAY2BGR, CV_HSV2BGR);
	absdiff(cropImage, cropImageGRAY2BGR, displayThis);


	int thres_val = 55;
	cv::threshold(displayThis, dst, thres_val, 255.0, CV_THRESH_BINARY);
	cvtColor(dst, displayThisGray, CV_BGR2GRAY);
	*/

	//test feature end here
	
	float numOfnonZero = cv::countNonZero(displayThisGray);
	float totalNumofPixel = displayThisGray.rows * displayThisGray.cols;

	
	float resultOfPerct = numOfnonZero / totalNumofPixel;


	
	if(imshow_display || 1)
	{
		

		cvtColor(displayThisGray, displayThisGray, CV_GRAY2BGR);
		//cvtColor(dst, dst, CV_GRAY2BGR);
		//result

		cv::hconcat(cropImage, result, result);
		cv::hconcat(result, cropImageGRAY2BGR, cropImageGRAY2BGR);
		cv::hconcat(cropImageGRAY2BGR, displayThis, displayThis);
		cv::hconcat(displayThis, dst, displayThis);
		cv::hconcat(displayThis, displayThisGray, displayThis);
		//cv::resize(displayThis, displayThis, cv::Size(400,400));
		cv::resize(displayThis, displayThis, cv::Size(), 4, 4, cv::INTER_LINEAR);
		

		//cv::putText(displayThis, std::to_string(resultOfPerct), cvPoint(30, 30),cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200, 200, 250), 1, CV_AA);
		//cv::imshow("Achromatic Threshold", displayThis);

	}
	

	//set histogram variables:


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
	//cv::MatND hist;
	cv::Mat hist;
	int max_loc[3];

	// we compute the histogram from the 0-th and 1-st channels
	int channels[] = { 0, 1, 2 };


	//check if overall saturation is high or low || result of perct
	
	//std::vector<cv::Mat> channelsy;
	//split(cropImage, channelsy);
	/*cv::Scalar h = cv::mean(channelsy[0]);
	cv::Scalar s = cv::mean(channelsy[1]);
	cv::Scalar v = cv::mean(channelsy[2]);*/

	//cv::GaussianBlur(channelsy[1], channelsy[1], cv::Size(15, 15), 15.0, 15.0);
	
	//double Smin, Smax;
	//cv::minMaxLoc(channelsy[1], &Smin, &Smax);

	//cv::Mat hsv_crop;

	//cvtColor(cropImage, hsv_crop, CV_BGR2HSV);

	//recalculate the Histogram value
	calcHist(&hsv_mat, 1, channels, cv::Mat(), // do not use mask
		hist, 3, histSize, ranges,
		true, // the histogram is uniform
		false);

	max_loc[0] = 0;
	max_loc[1] = 0;
	max_loc[2] = 0;

	//reobtain the value
	minMaxIdx(hist, 0, 0, 0, max_loc, cv::Mat());



	//std::cout << "scalar S =" << max_loc[1] << "  ";


	


	if (resultOfPerct > 0.25 || max_loc[1] >= 2)
	{
		//std::cout << unitID << " : Threhold alert -- " << resultOfPerct << " [ >> USE CHROMATIC SCALE << ] "<< std::endl;
		chromaticScale = true;
		//std::cout << "chromaticScale ";
	}
	else
	{
		chromaticScale = false;
		//std::cout << "AchromaticScale";
	}
	/*
	////check if the image is overall grayscale
	////BGR
	//int rg = abs(average[2] - average[1]);
	//int rb = abs(average[2] - average[0]);
	//int gb = abs(average[1] - average[0]);
	//int diff = rg + rb + gb;

	//
	//// try to obtain the RMS value between 2 images, to see how similar the image is
	//double similarityPercentage = getSimilarity(cropImage, cropImageGRAY2BGR);
	//int meanValuetemp = cv::mean(displayThis)[0] + cv::mean(displayThis)[1] + cv::mean(displayThis)[2];



	////initialize during first run
	//if (L2error_min == 0.0 && L2error_max == 0.0)
	//{
	//	L2error_min = similarityPercentage;
	//	L2error_max = similarityPercentage;
	//}


	//if (meanValue_min  == 0 && meanValue_max == 0)
	//{
	//	meanValue_min = meanValuetemp;
	//	meanValue_max = meanValuetemp;
	//}

	//if (RGBGRAYDiff_min == 0 && RGBGRAYDiff_max == 0)
	//{
	//	RGBGRAYDiff_min = diff;
	//	RGBGRAYDiff_max = diff;
	//}



	//if (similarityPercentage > L2error_max)
	//{
	//	L2error_max = similarityPercentage;
	//	//std::cout << "L2error_max %: " << similarityPercentage << " ,UnitID:" << unitID << std::endl;
	//}

	//if (similarityPercentage < L2error_min)
	//{
	//	L2error_min = similarityPercentage;
	//	//std::cout <<  "L2error_min %: " << similarityPercentage << " ,UnitID:" << unitID << std::endl;
	//}


	//if (meanValuetemp > meanValue_max)
	//{
	//	meanValue_max = meanValuetemp;
	//	//std::cout << "meanValue_max: "  << meanValuetemp << unitID << std::endl;
	//}

	//if (meanValuetemp < meanValue_min)
	//{
	//	meanValue_min = meanValuetemp;
	//	//std::cout << "meanValue_min: " << meanValuetemp << unitID << std::endl;
	//}


	//if (diff > RGBGRAYDiff_max)
	//{
	//	RGBGRAYDiff_max = diff;
	//	//std::cout << "RGBGRAYDiff_max GRAYSCALE/RGB: " << diff <<  " ,UnitID:" << unitID << std::endl;
	//}

	//if (diff < RGBGRAYDiff_min)
	//{
	//	RGBGRAYDiff_min = diff;
	//	//std::cout << "RGBGRAYDiff_min GRAYSCALE/RGB: " << diff << " ,UnitID:" << unitID << std::endl;
	//}

	*/

	
	

	if (!chromaticScale)
	{
		//original
		/*
		//use achromatic scale
		//std::cout << "a ";
		cvtColor(cropImage, hsv, CV_BGR2HSV);

		//image is in HSV alrdy
		//get mean of blob

		average = cv::mean(hsv);


		//calculate the Hue-Saturation histogram
		int hbins = 1, sbins = 1, vbins = 16;
		int histSize[] = { hbins, sbins, vbins };
		// hue varies from 0 to 179, see cvtColor
		float hranges[] = { 0, 180 };
		// saturation & value varies from 0 (black-gray-white) to
		// 255 (pure spectrum color)
		float sranges[] = { 0, 256 };
		float vranges[] = { 0, 256 };

		const float* ranges[] = { hranges, sranges, vranges };
		//cv::MatND hist;
		cv::Mat hist;

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


		for (int i = 0; i < hbins; i++)
		{

		//std::cout << "Start of H: " << i << std::endl;
		for (int j = 0; j < sbins; j++)
		{
		float histMax = 0.0;
		for (int k = 0; k < vbins; k++)
		{

		//std::cout << hist.at<float>(i, j, k) <<"," ;

		//std::cout << hist.at<float>(i, j, k) << std::endl << std::endl;

		if (hist.at<float>(i, j, k) > histMax)
		{
		//std::cout << hist.at<float>(i, j, k) << std::endl;
		histMax = hist.at<float>(i, j, k);

		rankings item(int(histMax), i, j, k);
		top3vals.push_back(item);

		//top3vals.push_back(std::to_string(int(histMax)) + "- " + std::to_string(i) + ", " + std::to_string(j) + ", " + std::to_string(k));


		}

		}
		//std::cout << std::endl;
		}


		}
		//static auto abs_cmp = [](int a, int b) { return std::abs(a) < std::abs(b); };


		std::sort(top3vals.begin(), top3vals.end(), rankings::sortbyVal);
		if (top3vals.size() > 1)
		{
		if (top3vals[1].HistValue / top3vals[0].HistValue > 0.9)
		{
		std::cout << unitID << " : WARNING !!! -- above 90% threshold of similarity between first and second Hist value" << std::endl;

		int tempi = 0;
		cv::Scalar scalarTop1, scalarTop2;

		std::cout << "Top 2 histogram values: " << std::endl;
		for (rankings &n : top3vals)
		{
		if (tempi == 0)
		{

		scalarTop1[0] = n.HistH * 1;
		scalarTop1[1] = n.HistS * 1;
		scalarTop1[2] = n.HistV * 16;

		std::cout << n.HistValue << ": " << n.HistLocation << std::endl;
		tempi++;
		}
		else if (tempi == 1)
		{

		scalarTop2[0] = n.HistH * 1;
		scalarTop2[1] = n.HistS * 1;
		scalarTop2[2] = n.HistV * 16;

		std::cout << n.HistValue << ": " << n.HistLocation << std::endl;
		tempi++;
		}
		else
		{
		break;
		}

		}


		cv::Vec4d d = scalarTop1 - scalarTop2;
		double distance = cv::norm(d);

		std::cout << "Distance between first 2 Histogram results: " << distance << std::endl << std::endl;
		}


		// display distribution of colors
		cv::Scalar tempScalar;
		cv::Mat ColorDistribution = cv::Mat::zeros(50, 50, CV_8UC3);
		cv::Mat tempMatColor = cv::Mat::zeros(50, 50, CV_8UC3);

		int MatWidth = 200;
		int MatHeight = 50;

		int tempi = 0;
		int totalWidth = top3vals[0].HistValue + top3vals[1].HistValue;

		for (rankings &n : top3vals)
		{
		if (tempi < 2)
		{

		tempScalar[0] = n.HistH * 1;
		tempScalar[1] = n.HistS * 1;
		tempScalar[2] = n.HistV * 16;


		int MatWidth_temp = (float(top3vals[tempi].HistValue) / float(totalWidth))*float(MatWidth);
		tempMatColor.create(MatHeight, MatWidth_temp, CV_8UC3);
		tempMatColor = tempScalar;


		if (tempi == 0)
		ColorDistribution = tempMatColor.clone();
		else
		{
		if (!tempMatColor.empty())
		{
		cv::hconcat(ColorDistribution, tempMatColor, ColorDistribution);
		}
		else
		{
		std::cout << "empty" << std::endl;

		}
		}
		tempi++;
		}
		else
		{
		break;
		}

		}

		cvtColor(ColorDistribution, ColorDistribution, CV_HSV2BGR);
		cv::imshow("Color Distribution List", ColorDistribution);
		}

		//test

		//std::cout << (float(top3vals[1].HistValue) / float(top3vals[0].HistValue)) << std::endl;
		if(top3vals.size() > 1)
		{
		if ((float(top3vals[1].HistValue) / float(top3vals[0].HistValue)) > 0.7)
		{
		//std::cout << "[v] Top value 0: " << top3vals[0].HistV << " Top value 1: " << top3vals[1].HistV << std::endl;
		if (top3vals[1].HistV < top3vals[0].HistV)
		{
		max_loc[2] = top3vals[1].HistV;
		//std::cout << "Updated V value to obtain darker shade if above 0.70%" << std::endl;
		}
		}
		}

		average[0] = int(max_loc[0] * 1);  // each bin represents 12 HUES ///// 6hues (if 30,32,32)
		average[1] = int(max_loc[1] * 1);  // each bin represents 32 SATURATION //8sat (if 30,32,32)
		average[2] = int(max_loc[2] * 16);  // each bin represents 32 VALUES// 8val (if 30,32,32)



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


		//cv::imshow("COLOR", temp_mat);

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


		std::string tempString = "15,15," + std::to_string(max_loc[2]);

		ColorTerm allColors("Null", "Null");
		ColorInTerms = allColors.getTerm(inputColorVector, tempString);



		ColorInTerms = ColorInTerms + " HSV (GRAY): 15,15," + std::to_string(max_loc[2]);
		std::cout << unitID << ": " << ColorInTerms << std::endl;
		//end of achromatic scale
		*/

		//test method

		cv::Mat cropImagegray;
		cvtColor(cropImage, hsv, CV_BGR2HSV);

		cvtColor(cropImage, cropImagegray, CV_BGR2GRAY);


		//image is in HSV alrdy
		//get mean of blob

		average = cv::mean(hsv);

		int  white_thresh = 170, black_thresh = 75;

		cv::Mat blackMat, whiteMat, displayThis;


		cv::threshold(cropImagegray, blackMat, black_thresh, 255.0, CV_THRESH_BINARY);
		cv::threshold(cropImagegray, whiteMat, white_thresh, 255.0, CV_THRESH_BINARY);


		cv::hconcat(blackMat, whiteMat, displayThis);
		cvtColor(displayThis, displayThis, CV_GRAY2BGR);

		cv::hconcat(cropImage, displayThis, displayThis);

		cv::resize(displayThis, displayThis, cv::Size(), 5, 5, cv::INTER_LINEAR);
		//cv::imshow("Black & White Filter", displayThis);


		bitwise_not(blackMat, blackMat);


		float whitePerct, blackPerct;

		whitePerct = (float(cv::countNonZero(whiteMat)) / float((whiteMat.rows*whiteMat.cols))) * 100;
		blackPerct = (float(cv::countNonZero(blackMat)) / float((blackMat.rows*blackMat.cols))) * 100;



		//std::cout << unitID << " :white mat: " << cv::countNonZero(whiteMat) << "/" << whiteMat.rows*whiteMat.cols << "= " << whitePerct;
		//std::cout << " ,black mat: " << cv::countNonZero(blackMat) << "/" << blackMat.rows*blackMat.cols << "= " << blackPerct << std::endl;


		if (whitePerct > 25 && blackPerct < 25)
		{
			//ColorInTerms = "WHITE";
			tempColorTerm = "WHITE";
			//std::cout << ColorInTerms << std::endl;
			average[0] = 0;  // each bin represents 12 HUES ///// 6hues (if 30,32,32)
			average[1] = 0;  // each bin represents 32 SATURATION //8sat (if 30,32,32)
			average[2] = 224;  // each bin represents 32 VALUES// 8val (if 30,32,32)

		}
		else if (blackPerct > 25 && whitePerct < 25)
		{
			//ColorInTerms = "BLACK";
			tempColorTerm = "BLACK";

			//std::cout << ColorInTerms << std::endl;
			average[0] = 0;  // each bin represents 12 HUES ///// 6hues (if 30,32,32)
			average[1] = 0;  // each bin represents 32 SATURATION //8sat (if 30,32,32)
			average[2] = 0;  // each bin represents 32 VALUES// 8val (if 30,32,32)

		}
		else
		{
			//ColorInTerms = "GREY_SILVER";
			tempColorTerm = "GRAY";

			//std::cout << ColorInTerms << std::endl;
			average[0] = 0;  // each bin represents 12 HUES ///// 6hues (if 30,32,32)
			average[1] = 0;  // each bin represents 32 SATURATION //8sat (if 30,32,32)
			average[2] = 160;  // each bin represents 32 VALUES// 8val (if 30,32,32)
		}





		hsv = average;  // set MAT to the HSV color
		cvtColor(hsv, temp_mat, CV_HSV2BGR);  // convert it back to BGR
		average = cv::mean(temp_mat); // get the color in BGR format
		AvgColorScalar = average;


		//AvgColorScalar[0] = int((average[0] + AvgColorScalar[0]) / 2);
		//AvgColorScalar[1] = int((average[1] + AvgColorScalar[1]) / 2);
		//AvgColorScalar[2] = int((average[2] + AvgColorScalar[2]) / 2);

		/*
		//try this.. check if R==G==B
		int similarityThresh = 20;
		if (((AvgColorScalar[0] + similarityThresh >= AvgColorScalar[1]) || (AvgColorScalar[0] - similarityThresh <= AvgColorScalar[1])) &&
		((AvgColorScalar[1] + similarityThresh >= AvgColorScalar[2]) || (AvgColorScalar[1] - similarityThresh <= AvgColorScalar[2])) &&
		((AvgColorScalar[2] + similarityThresh >= AvgColorScalar[0]) || (AvgColorScalar[2] - similarityThresh <= AvgColorScalar[0])))
		{
		std::cout << "Achromatic Vehicle detected?? Please check!" << "R: " << AvgColorScalar[2] << "G: " << AvgColorScalar[1] << "B: " << AvgColorScalar[0] << std::endl;



		}
		*/


		AvgColor.push_back(AvgColorScalar);


	}



	struct rankings
	{
		int HistValue;

		int HistH;
		int HistS;
		int HistV;
		std::string HistLocation;

		rankings(int in1, int in2, int in3, int in4)
		{
			HistValue = in1;
			HistH = in2;
			HistS = in3;
			HistV = in4;
			HistLocation = std::to_string(in2) + ", " + std::to_string(in3) + ", " + std::to_string(in4);

		}

		static bool sortbyVal(const rankings &lhs, const rankings &rhs)
		{
			return lhs.HistValue > rhs.HistValue;
		}
		//~rankings();

	};

	std::vector<rankings> top3vals;	

	max_loc[0] = 0;
	max_loc[1] = 0;
	max_loc[2] = 0;




		//std::cout << "c ";
		//use chromatic scale
		cvtColor(cropImage, hsv, CV_BGR2HSV);

		//image is in HSV alrdy
		//get mean of blob

		average = cv::mean(hsv);


		calcHist(&hsv, 1, channels, cv::Mat(), // do not use mask
			hist, 3, histSize, ranges,
			true, // the histogram is uniform
			false);
		//double maxVal = 0;

		//cv::Point min_loc, max_loc;
		//minMaxLoc(hist, 0, &maxVal, &min_loc, &max_loc);  <-- if there's only 2 dimension

		
		minMaxIdx(hist, 0, 0, 0, max_loc, cv::Mat());


		// formula: Hue			= ((Current bin+next bin)/2)* ((180 hue/30 bin)*2(to change to GIMP 360)
		// formula: Saturation	= (current bin+next bin)/2) * (100/32) --< eg: bin value/32 bins * 100(gimp max sat)
		// formula: value		= ((current bin + next bin)/2) * (100/32) --< eg: bin value/32 bins * 100 (gimp max value)

		//std::cout << "Hue range, Saturation, Value : (" << int((max_loc[0])) * 12 << "), " << int(((max_loc[1] + max_loc[1] + 1)/2) * 3.125) <<  ", " << int(((max_loc[2] + max_loc[2] + 1)/2) * 3.125) << std::endl;	

		//std::cout << ">> HSV: " << max_loc[0] << "," << max_loc[1] << ","<< max_loc[2] <<std::endl;

		if (0)
		{
		
		for (int i = 0; i < hbins; i++)
		{

			//std::cout << "Start of H: " << i << std::endl;
			for (int j = 0; j < sbins; j++)
			{
				float histMax = 0.0;
				for (int k = 0; k < vbins; k++)
				{

					//std::cout << hist.at<float>(i, j, k) <<"," ;

					//std::cout << hist.at<float>(i, j, k) << std::endl << std::endl;

					if (hist.at<float>(i, j, k) > histMax)
					{
						//std::cout << hist.at<float>(i, j, k) << std::endl;
						histMax = hist.at<float>(i, j, k);

						rankings item(int(histMax), i, j, k);
						top3vals.push_back(item);

						//top3vals.push_back(std::to_string(int(histMax)) + "- " + std::to_string(i) + ", " + std::to_string(j) + ", " + std::to_string(k));


					}

				}
				//std::cout << std::endl;
			}


		}
		//static auto abs_cmp = [](int a, int b) { return std::abs(a) < std::abs(b); };
		}

		//if (0)
		//{
		//
		//std::sort(top3vals.begin(), top3vals.end(), rankings::sortbyVal);
		//if (top3vals.size() > 3)
		//{
		//	if (top3vals[1].HistValue / top3vals[0].HistValue > 0.9)
		//	{
		//		std::cout << unitID << " : WARNING !!! -- above 90% threshold of similarity between first and second Hist value" << std::endl;
		//		int tempi = 0;
		//		cv::Scalar scalarTop1, scalarTop2;
		//		std::cout << "Top 2 histogram values: " << std::endl;
		//		for (rankings &n : top3vals)
		//		{
		//			if (tempi == 0)
		//			{
		//				scalarTop1[0] = n.HistH * 12;
		//				scalarTop1[1] = n.HistS * 32;
		//				scalarTop1[2] = n.HistV * 32;
		//				std::cout << n.HistValue << ": " << n.HistLocation << std::endl;
		//				tempi++;
		//			}
		//			else if (tempi == 1)
		//			{
		//				scalarTop2[0] = n.HistH * 12;
		//				scalarTop2[1] = n.HistS * 32;
		//				scalarTop2[2] = n.HistV * 32;
		//				std::cout << n.HistValue << ": " << n.HistLocation << std::endl;
		//				tempi++;
		//			}
		//			else
		//			{
		//				break;
		//			}
		//		}
		//		/*cv::Mat temp_scalarMat1 = cv::Mat::zeros(10, 10, CV_8UC3);
		//		cv::Mat temp_scalarMat2 = cv::Mat::zeros(10, 10, CV_8UC3);
		//		temp_scalarMat1 = scalarTop3[0];
		//		cvtColor(temp_scalarMat1, temp_scalarMat1, CV_HSV2BGR);
		//		cvtColor(temp_scalarMat1, temp_scalarMat1, CV_BGR2Lab);
		//		temp_scalarMat2 = scalarTop3[1];
		//		cvtColor(temp_scalarMat2, temp_scalarMat2, CV_HSV2BGR);
		//		cvtColor(temp_scalarMat2, temp_scalarMat2, CV_BGR2Lab); */
		//		cv::Vec4d d = scalarTop1 - scalarTop2;
		//		double distance = cv::norm(d);
		//		//std::cout << "Distance between first 2 Histogram results: " << distance << std::endl << std::endl;
		//	}
		//	if (imshow_display || 0)
		//	{
		//	
		//	// display distribution of colors
		//	cv::Scalar tempScalar;
		//	cv::Mat ColorDistribution = cv::Mat::zeros(50, 50, CV_8UC3);
		//	cv::Mat tempMatColor = cv::Mat::zeros(50, 50, CV_8UC3);
		//	int MatWidth = 200;
		//	int MatHeight = 50;
		//
		//	int tempi = 0;
		//	int totalWidth = top3vals[0].HistValue + top3vals[1].HistValue + top3vals[2].HistValue + top3vals[3].HistValue;
		//	for (rankings &n : top3vals)
		//	{
		//		if (tempi < 4)
		//		{
		//
		//			tempScalar[0] = n.HistH * 12;
		//			tempScalar[1] = n.HistS * 32;
		//			tempScalar[2] = n.HistV * 32;
		//
		//			int MatWidth_temp = (float(top3vals[tempi].HistValue) / float(totalWidth))*float(MatWidth);
		//			tempMatColor.create(MatHeight, MatWidth_temp, CV_8UC3);
		//			tempMatColor = tempScalar;
		//
		//			if (tempi == 0)
		//				ColorDistribution = tempMatColor.clone();
		//			else
		//			{
		//				if (!tempMatColor.empty())
		//				{
		//					cv::hconcat(ColorDistribution, tempMatColor, ColorDistribution);
		//				}
		//				else
		//					std::cout << "tempmatcolor empty" << std::endl;
		//			}
		//			tempi++;
		//		}
		//		else
		//		{
		//			break;
		//		}
		//	}
		//	cvtColor(ColorDistribution, ColorDistribution, CV_HSV2BGR);
		//	cv::imshow("Color Distribution List", ColorDistribution);
		//}
		//}
		////test
		//}

		average[0] = int(max_loc[0] * 12);  // each bin represents 12 HUES ///// 6hues (if 30,32,32)
		average[1] = int(max_loc[1] * 32);  // each bin represents 32 SATURATION //8sat (if 30,32,32)
		average[2] = int(max_loc[2] * 32);  // each bin represents 32 VALUES// 8val (if 30,32,32)



		hsv = average;  // set MAT to the HSV color
		cvtColor(hsv, temp_mat, CV_HSV2BGR);  // convert it back to BGR
		average = cv::mean(temp_mat); // get the color in BGR format
		AvgColorScalar = average;
		

		//AvgColorScalar[0] = int((average[0] + AvgColorScalar[0]) / 2);
		//AvgColorScalar[1] = int((average[1] + AvgColorScalar[1]) / 2);
		//AvgColorScalar[2] = int((average[2] + AvgColorScalar[2]) / 2);

		/*
		//try this.. check if R==G==B 
		int similarityThresh = 35;
		//if (18 < age && age < 30) blah
		if (((AvgColorScalar[0] - similarityThresh) < AvgColorScalar[1] &&  AvgColorScalar[1] < (AvgColorScalar[0] + similarityThresh)) &&
			((AvgColorScalar[1] - similarityThresh) < AvgColorScalar[2] && AvgColorScalar[2] < (AvgColorScalar[1] + similarityThresh)) &&
			((AvgColorScalar[2] - similarityThresh) < AvgColorScalar[0] && AvgColorScalar[0] < (AvgColorScalar[2] + similarityThresh)))
		{
			std::cout << "-------------------------------------------------------------------\n";
			std::cout << "Achromatic Vehicle detected?? Please check!" << " R: " << AvgColorScalar[2] << " G: " << AvgColorScalar[1] << " B: " << AvgColorScalar[0] << std::endl;
			//check:
			achromaticScale2 = true;
		}
		else
		{
			achromaticScale2 = false;
		}
	
		*/
		
		AvgColor.push_back(AvgColorScalar);
		

		/*
		if (achromaticScale2)
		{
			cv::Mat cropImagegray;
			cvtColor(cropImage, hsv, CV_BGR2HSV);
			cvtColor(cropImage, cropImagegray, CV_BGR2GRAY);


			int  white_thresh = 170, black_thresh = 50;

			cv::Mat blackMat, whiteMat, displayThis;


			cv::threshold(cropImagegray, blackMat, black_thresh, 255.0, CV_THRESH_BINARY);
			cv::threshold(cropImagegray, whiteMat, white_thresh, 255.0, CV_THRESH_BINARY);
			bitwise_not(blackMat, blackMat);


			float whitePerct, blackPerct;

			whitePerct = (float(cv::countNonZero(whiteMat)) / float((whiteMat.rows*whiteMat.cols))) * 100;
			blackPerct = (float(cv::countNonZero(blackMat)) / float((blackMat.rows*blackMat.cols))) * 100;
			if (whitePerct > 25 && blackPerct < 25)
			{
				std::cout << "test: --- white" << std::endl;
				ColorInTerms = "WHITE";
			}
			else if (blackPerct > 25 && whitePerct < 25)
			{
				std::cout << "test: --- black" << std::endl;
				ColorInTerms = "BLACK";
			}
			else
			{
				std::cout << "test: --- gray" << std::endl;
				ColorInTerms = "GREY_SILVER";
			}
		}
		*/
		//end of chromatic scale


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

	cv::Mat allConcatScalarMat;
	if (AvgColor.size() > 2)
	{

		for (int i = 0; i < AvgColor.size(); i++)
		{
			if (i == 0)
			{
				hconcate_mat = AvgColor[i];
				allConcatScalarMat = hconcate_mat;
			}
			else
			{
				hconcate_mat = AvgColor[i];
				cv::hconcat(allConcatScalarMat, hconcate_mat, allConcatScalarMat);
			}
		}
		

	}
	else
	{

		hconcate_mat = AvgColor[0];
		allConcatScalarMat = hconcate_mat;

	}
	//imshow("hconcat colors", allConcatScalarMat);
	
	hsv_mat = allConcatScalarMat.clone();
	
	
	AvgColorScalar = cv::mean(allConcatScalarMat);
	
	
	hsv_mat = AvgColorScalar;
	//imshow("hconcat colors - mean", hsv_mat);

	cvtColor(allConcatScalarMat, hsv, CV_BGR2HSV);

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

	ColorTerm allColors("Null", "Null");
	
	//uncomment this for the inputcolorvector method.
	//ColorInTerms = allColors.getTerm(inputColorVector, tempString);

	// get the index number:
	//AvgColorScalar <<---- BGR, hence: index 2-1-0
	//index_im = 1+floor(RR(:)/8)+32*floor(GG(:)/8)+32*32*floor(BB(:)/8);

	// for c++ -1 because index start from 0, matlab index start at 1
	int index_im = floor(AvgColorScalar[2] / 8) + (32 * floor(AvgColorScalar[1] / 8)) + (32 * 32 * floor(AvgColorScalar[0] / 8));
	
	finalIndex_im = index_im;
	

	//if (w2c[index_im].size() > 0)
	//{
	//	w2c[index_im].clear();
	//}

	//for (int w = 0; w < w2c[index_im].size(); w++)
	//{
	//	colorValues.push_back(w2c[index_im][w]);
	//}

	colorValues = w2c[index_im];

	/*std::cout << "AvgColorScalar: R:" << AvgColorScalar[2] << " G:" << AvgColorScalar[1] << " B:" << AvgColorScalar[0] << std::endl;;
	std::cout << "\nW2C info: \n Index_im = " << index_im << std::endl;;*/

	
	

	/*std::cout << "Largest element: " << *std::max_element(w2c[index_im].begin() + 3, w2c[index_im].end());
	std::cout << " \n";*/

	
	//allColors.compareColors(AvgColorScalar);
	//std::cout << "Unit ID: " << unitID << "  Dominant-Color: " << ColorInTerms << " (" << tempString << ") ";


	//cvtColor(hsv_mat, hsv, CV_BGR2HSV);
	////recalculate the Histogram value
	//calcHist(&hsv, 1, channels, cv::Mat(), // do not use mask
	//	hist, 3, histSize, ranges,
	//	true, // the histogram is uniform
	//	false);

	//max_loc[0] = 0;
	//max_loc[1] = 0;
	//max_loc[2] = 0;

	////reobtain the value
	//minMaxIdx(hist, 0, 0, 0, max_loc, cv::Mat());


	// tempString = std::to_string(max_loc[0]) + "," + std::to_string(max_loc[1]) + "," + std::to_string(max_loc[2]);

	//
	//ColorInTerms = allColors.getTerm(inputColorVector, tempString);


	//if ((ColorInTerms == "WHITE" || ColorInTerms == "BLACK" || ColorInTerms == "GRAY") && tempColorTerm != "")
	//{
	//	ColorInTerms = tempColorTerm;
	//}
	//
	//std::cout << "mean color: " <<  ColorInTerms << " (" << tempString << ") resultOfPerct: " << resultOfPerct << std::endl;

	//allColors.compareColors(AvgColorScalar);

	allConcatScalarMat.release();
	hsv_mat.release();
	hsv.release();
	hsv2.release();
	temp_mat.release();
	cropImage.release();
	hconcate_mat.release();
	result.release();
	cropImageGRAY.release();
	cropImageGRAY2BGR.release();
	displayThis.release();
	dst.release();





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

double Blob::getSimilarity(const cv::Mat A, const cv::Mat B)
{
	// Compare two images by getting the L2 error (square-root of sum of squared error).
	if (A.rows > 0 && A.rows == B.rows && A.cols > 0 && A.cols == B.cols) 
	{
		// Calculate the L2 relative error between images.
		double errorL2 = norm(A, B, CV_L2);
		// Convert to a reasonable scale, since L2 error is summed across all pixels of the image.
		double similarity = errorL2 / (double)(A.rows * A.cols);
		return similarity;
	}
	else 
	{
		//Images have a different size
		return 100000000.0;  // Return a bad value
	}
}

void Blob::setChannel(cv::Mat & mat, unsigned int channel, unsigned char value)
{
	// make sure have enough channels
	if (mat.channels() < channel + 1)
		return;

	const int cols = mat.cols;
	const int step = mat.channels();
	const int rows = mat.rows;
	for (int y = 0; y < rows; y++) {
		// get pointer to the first byte to be changed in this row
		unsigned char *p_row = mat.ptr(y) + channel;
		unsigned char *row_end = p_row + cols*step;
		for (; p_row != row_end; p_row += step)
			*p_row = value;
	}
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


cv::Mat Blob::equalizeIntensity(const cv::Mat& inputImage)
{
	if (inputImage.channels() >= 3)
	{
		cv::Mat ycrcb;

		cvtColor(inputImage, ycrcb, CV_BGR2YCrCb);

		std::vector<cv::Mat> channels;
		split(ycrcb, channels);

		equalizeHist(channels[0], channels[0]);

		cv::Mat result;
		cv::merge(channels, ycrcb);

		cvtColor(ycrcb, result, CV_YCrCb2BGR);

		return result;
	}
	return cv::Mat();
}


///// perform the Simplest Color Balancing algorithm
//// test color balancing
//void Blob::SimplestCB(cv::Mat& in, cv::Mat& out, float percent) {
//	assert(in.channels() == 3);
//	assert(percent > 0 && percent < 100);
//
//	float half_percent = percent / 200.0f;
//
//	std::vector<cv::Mat> tmpsplit; split(in, tmpsplit);
//	for (int i = 0; i<3; i++) {
//		//find the low and high precentile values (based on the input percentile)
//		cv::Mat flat; tmpsplit[i].reshape(1, 1).copyTo(flat);
//		cv::sort(flat, flat, CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);
//		int lowval = flat.at<uchar>(cvFloor(((float)flat.cols) * half_percent));
//		int highval = flat.at<uchar>(cvCeil(((float)flat.cols) * (1.0 - half_percent)));
//		//std::cout << lowval << " " << highval << std::endl;
//
//		//saturate below the low percentile and above the high percentile
//		tmpsplit[i].setTo(lowval, tmpsplit[i] < lowval);
//		tmpsplit[i].setTo(highval, tmpsplit[i] > highval);
//
//		//scale the channel
//		cv::normalize(tmpsplit[i], tmpsplit[i], 0, 255, cv::NORM_MINMAX);
//	}
//	cv::merge(tmpsplit, out);
//}
//
//
//
//
//void Blob::balance_white(cv::Mat & mat) {
//	double discard_ratio = 0.05;
//	int hists[3][256];
//	memset(hists, 0, 3 * 256 * sizeof(int));
//
//	for (int y = 0; y < mat.rows; ++y) {
//		uchar* ptr = mat.ptr<uchar>(y);
//		for (int x = 0; x < mat.cols; ++x) {
//			for (int j = 0; j < 3; ++j) {
//				hists[j][ptr[x * 3 + j]] += 1;
//			}
//		}
//	}
//
//	// cumulative hist
//	int total = mat.cols*mat.rows;
//	int vmin[3], vmax[3];
//	for (int i = 0; i < 3; ++i) {
//		for (int j = 0; j < 255; ++j) {
//			hists[i][j + 1] += hists[i][j];
//		}
//		vmin[i] = 0;
//		vmax[i] = 255;
//		while (hists[i][vmin[i]] < discard_ratio * total)
//			vmin[i] += 1;
//		while (hists[i][vmax[i]] > (1 - discard_ratio) * total)
//			vmax[i] -= 1;
//		if (vmax[i] < 255 - 1)
//			vmax[i] += 1;
//	}
//
//
//	for (int y = 0; y < mat.rows; ++y) {
//		uchar* ptr = mat.ptr<uchar>(y);
//		for (int x = 0; x < mat.cols; ++x) {
//			for (int j = 0; j < 3; ++j) {
//				int val = ptr[x * 3 + j];
//				if (val < vmin[j])
//					val = vmin[j];
//				if (val > vmax[j])
//					val = vmax[j];
//				ptr[x * 3 + j] = static_cast<uchar>((val - vmin[j]) * 255.0 / (vmax[j] - vmin[j]));
//			}
//		}
//	}
//}