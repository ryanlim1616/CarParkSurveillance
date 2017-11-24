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

//#include "ExporterTask.h"
class SQLiteManager;

class CarParkTrackExporter
{
public:
	bool writing = false;
	//const std::string& mFilePath = "C:\\Users\\Clarence\\Documents\\GitHub\\carsurveilance - original\\CarParkSurveillance\\CarParkSurveillance\\CarParkSurveillance\\CPSurveillance.db";
	const std::string& mFilePath = "DBSurveillance.db";

	void run();

	CarParkTrackExporter();
	~CarParkTrackExporter();

	bool writeToDB(std::vector<Blob> &blobs, unsigned int &i, int &frameCount, int &vidLength);
	bool writeToDB_entExt(std::vector<Blob> &blobs, unsigned int &i, int &frameCount, int &vidLength, int entrance, bool entExt);
	bool writeToDB_park(std::vector<Blob> &blobs, unsigned int &i, int &frameCount, int &vidLength, std::string parkzone);
	bool writeToDB_missmatch();

	bool countTraj();

	
	
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