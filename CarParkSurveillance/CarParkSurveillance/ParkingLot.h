#ifndef MY_PARKINGLOT
#define MY_PARKINGLOT

class ParkingLot {
public:
	ParkingLot() {};
	ParkingLot(int zone, int lot, bool parked, cv::Mat image) {
		this->zone = zone;
		this->lot = lot;
		this->parked = parked;
		this->image = image;
	};

	int zone;
	int lot;
	bool parked;
	cv::Mat image;
	int featurePoint;

private:


};


#endif
