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

	int zone;
	int lot;
	bool parked;
	cv::Mat image;
<<<<<<< HEAD

=======
	
>>>>>>> refs/remotes/ryanlim1616/master
	int countNZero = 0;
	int featurePoint;
	int vehicleId = 0;

private:


};


#endif