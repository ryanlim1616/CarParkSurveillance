#ifndef MY_PARKINGLOT
#define MY_PARKINGLOT

class ParkingLot {
public:
	ParkingLot() {};
	ParkingLot(int zone, int lot, bool parked, cv::Mat image, int countNZero, cv::Mat parking, std::vector<std::vector<cv::Point> > _contour) {
		this->zone = zone;
		this->lot = lot;
		this->parked = parked;
		this->image = image;
		this->countNZero = countNZero;
		this->parking = parking;

		for (auto &convexHull : _contour) {
			currentBoundingRect = cv::boundingRect(convexHull);
		}

	};
	~ParkingLot() {
		
	};

	int zone;
	int lot;
	bool parked;
	cv::Mat image;

	cv::Mat parking;
	cv::Rect currentBoundingRect;



	int countNZero = 0;
	int featurePoint;
	int vehicleId = 0;

private:


};


#endif