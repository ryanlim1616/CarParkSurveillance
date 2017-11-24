#ifndef CAR_PARK_TRACK_EXPORTER_H
#define CAR_PARK_TRACK_EXPORTER_H

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
#include"Blob.h"
#include <tuple>

//#include "ExporterTask.h"
class SQLiteManager;

class CarParkTrackExporter
{
public:
	bool writing = false;
	//const std::string& mFilePath = "C:\\Users\\Clarence\\Documents\\GitHub\\carsurveilance - original\\CarParkSurveillance\\CarParkSurveillance\\CarParkSurveillance\\CPSurveillance.db";
	const std::string& mFilePath = "DBSurveillance.db";
	std::vector<std::vector<std::string>> r_result;


	void run();

	CarParkTrackExporter();
	~CarParkTrackExporter();

	bool writeToDB(std::vector<Blob> &blobs, unsigned int &i, int &frameCount, int &vidLength);
	bool writeToDB_entExt(std::vector<Blob> &blobs, unsigned int &i, int &frameCount, int &vidLength, int entrance, bool entExt);
	bool writeToDB_park(std::vector<Blob> &blobs, unsigned int &i, int &frameCount, int &vidLength, std::string parkzone);
	bool writeToDB_missmatch(std::vector<Blob> &blobs, unsigned int &i, int &frameCount, int &vidLength, int condition);

	bool countTraj();
	bool searchdatabase(std::string QueryInput);

	std::tuple<int, int, int> get_atomInfo(int pos_x, int pos_y, int currentFrame)
	{

		//calculate the x, y, t
		int video_width = 640;
		int video_height = 480;
		int dimension_x = 20;
		int dimension_y = 20;

		int win_x = video_width / dimension_x;
		int win_y = video_height / dimension_y;


		//initialize atom_x, atom_y, atom_t
		int atom_x, atom_y, atom_t;


		// for loop waste extra computation power
		atom_x = pos_x / win_x;
		atom_y = pos_y / win_y;


		//instead of doing a loop within a loop... split it up.. cuz you only need to find out once... meaning, only 20 each loop, instead of a total of 400
		/*
		for (int i = 0; i <= video_width; )
		{
			int win_min = i;
			int win_max = i + win_x;

			if (pos_x >= win_min && pos_x < win_max)
				atom_x = i / win_x;

			i += win_x;
		}



		for (int j = 0; j <= video_height; )
		{
			int win_min = j;
			int win_max = j + win_y;

			if (pos_y >= win_min && pos_y < win_max)
				atom_y = j / win_y;

			j += win_y;
		}
		*/
		//get atom_t info

		int dimension_t = 10;
		//int total_frames = GlobalClass::instance()->get_TotalFrames();

		atom_t = currentFrame/ dimension_t;


		if (debug_on)
			std::cout << "atom_x: " << atom_x << " ,atom_y: " << atom_y << " ,atom_t: " << atom_t << std::endl;



		return std::make_tuple(atom_x, atom_y, atom_t);
		throw std::invalid_argument("pos_x");
		throw std::invalid_argument("pos_y");
		throw std::invalid_argument("currentFrame");

		
	}

	
	
private:
	bool createTables();
	bool dropTables();
	bool saveObjectTypeTable();
	bool savePositionTable();
	bool saveSceneObjectTable();
	
	
	bool saveCurrentContext();


	SQLiteManager* mSQLManager;
};


#endif