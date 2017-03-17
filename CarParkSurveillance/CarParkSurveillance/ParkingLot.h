#ifndef MY_PARKINGLOT
#define MY_PARKINGLOT

class ParkingLot {
public:
	ParkingLot() {};
	ParkingLot(int zone, int lot, bool parked, cv::Mat image, int countNZero) {
		this->zone = zone;
		this->lot = lot;
		this->parked = parked;
		this->image = image;
		this->countNZero = countNZero;
	};
	~ParkingLot() {
		
	};

	int zone;
	int lot;
	bool parked;
	cv::Mat image;

	int countNZero = 0;
	int featurePoint;
	int vehicleId = 0;

private:


};


#endif