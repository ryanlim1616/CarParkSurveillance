//backup

#include "CarParkTrackExporter.h"
#include "SQLiteManager.h"
#include "Logger.h"
#include <set>
#include <map>
#include <iostream>
#include "GlobalClass.h"
#include "Blob.h"
#include "Switches.h"


CarParkTrackExporter::CarParkTrackExporter(void)
:mSQLManager(nullptr)
{
	std::cout << "CarParkTrackExporter()" << std::endl;
}

CarParkTrackExporter::~CarParkTrackExporter()
{
	std::cout << "CarParkTrackExporter()" << std::endl;
}



void CarParkTrackExporter::run()
{
	mSQLManager = new SQLiteManager(mFilePath);
	if (mSQLManager->isConnected())
	{
		std::cout << mFilePath << ": ";
		
		if(!resumeFromError)
		{ 
			//dropTables();
			createTables();
			std::cout << "DB tables dropped & created" << std::endl;
			std::cout << "Initialized DB tables" << std::endl;
		}
		else
		{
			std::cout << "-- Using existing DB --" << std::endl;
		}
		
	}
	else
	{
		//Log Error
		std::cout << "[ERROR!] Unable to initialized db tables" << std::endl;
	}

	delete mSQLManager;
}

bool CarParkTrackExporter::createTables()
{
	bool success = mSQLManager->beginTransaction();
	//success &= mSQLManager->executeStatement("create table tracks ( id INT, date DATE, time DATETIME, track_id INT, frame_num INT, pos_x INT, pos_y INT, width INT, height INT, filename TEXT, PRIMARY KEY(filename, id) )");

	success &= mSQLManager->executeStatement("create table IF NOT EXISTS tracks ( track_id INTEGER PRIMARY KEY ASC, filename VARCHAR(20), track_date DATE, track_time TIME, obj_id INT, frame_num INT, obj_state VARCHAR(35), pos_x INT, pos_y INT, width INT, height INT)");
	//success &= mSQLManager->executeStatement("create table objects ( object_id INTEGER PRIMARY KEY ASC, filename VARCHAR(20), obj_id INT, obj_type VARCHAR(10), obj_color VARCHAR(10), enter_from INT, parked_at VARCHAR(10), exit_from INT, parked_duration TIME )");
	success &= mSQLManager->executeStatement("create table IF NOT EXISTS objects ( object_id INT, filename VARCHAR(20), obj_id INT, obj_type VARCHAR(10), obj_color VARCHAR(35), enter_from INT, parked_at VARCHAR(10), exit_from INT, parked_duration TIME, PRIMARY KEY( filename, obj_id) )");
	success &= mSQLManager->executeStatement("create table IF NOT EXISTS weather ( weather_id INTEGER PRIMARY KEY ASC, weather_date DATE, weather_time TIME, weather_condition VARCHAR(10))");
	success &= mSQLManager->executeStatement("create table IF NOT EXISTS days ( day_id INTEGER PRIMARY KEY ASC, day_date DATE, avg_weather VARCHAR(10), sod_car_num INT, eod_car_num INT, total_car_num INT)");

	//success &= mSQLManager->executeStatement("create table object_type ( name TEXT, PRIMARY KEY( name) )");
	//success &= mSQLManager->executeStatement("create table sceneobject ( object_id TEXT, type object_type, description TEXT, PRIMARY KEY( object_id) )");
	//success &= mSQLManager->executeStatement("create table position ( object_id sceneobject, frame_number INTEGER, top_left_corner_x REAL, top_left_corner_y REAL, bottom_right_corner_x REAL, bottom_right_corner_y REAL,  PRIMARY KEY( object_id, frame_number ) )");
	success &= mSQLManager->endTransaction();
	return success;
}

bool CarParkTrackExporter::dropTables()
{
	bool success = mSQLManager->beginTransaction();
	success &= mSQLManager->executeStatement("drop table if exists tracks;");
	success &= mSQLManager->executeStatement("drop table if exists objects;");
	success &= mSQLManager->executeStatement("drop table if exists weather;");
	success &= mSQLManager->executeStatement("drop table if exists days;");

	/*success &= mSQLManager->executeStatement("drop table if exists position;" );
	success &= mSQLManager->executeStatement("drop table if exists sceneobject;" );
	success &= mSQLManager->executeStatement("drop table if exists object_type;");*/
	success &= mSQLManager->endTransaction();
	return success;
}


bool CarParkTrackExporter::writeToDB(std::vector<Blob> &blobs, unsigned int &i, int &frameCount, int &vidLength)
{

	mSQLManager = new SQLiteManager(mFilePath);
	if (mSQLManager->isConnected())
	{
		bool success = mSQLManager->beginTransaction();

		success &= mSQLManager->executeStatement("insert into tracks (obj_id, pos_x, pos_y, width, height, track_date, track_time, filename, frame_num) values ('"
			+ std::to_string(blobs[i].unitID) + "', '" + std::to_string(blobs[i].currentBoundingRect.x) + "', '" + std::to_string(blobs[i].currentBoundingRect.y) + "', '"
			+ std::to_string(blobs[i].currentBoundingRect.width) + "', '" + std::to_string(blobs[i].currentBoundingRect.height) + "', '"
			+ GlobalClass::instance()->get_InputDate() + "', '" + GlobalClass::instance()->get_InputTime(frameCount, vidLength) + "', '" + GlobalClass::instance()->get_InputFileName() + "', '"
			+ std::to_string(frameCount) + "');");
		if (!success)
		{
			std::cin.get();
			//LOGERROR("Error saving object " << (*it)->getId());
			std::cout << "ERROR while writing into DB [Tracking], blob ID: " + std::to_string(blobs[i].unitID) + "\n";
			success = true;
		}

		success &= mSQLManager->endTransaction();
		delete mSQLManager;
		return success;
	}
	else
	{
		std::cout << "NOT CONNECTED TO DB \n";
		delete mSQLManager;
	}

	

}

bool CarParkTrackExporter::writeToDB_entExt(std::vector<Blob> &blobs, unsigned int &i, int &frameCount, int &vidLength, int entrance, bool entExt)
{

	mSQLManager = new SQLiteManager(mFilePath);
	if (mSQLManager->isConnected())
	{
		bool success = mSQLManager->beginTransaction();

		std::string colorInfo = std::to_string(blobs[i].AvgColorScalar[0]) + ", " + std::to_string(blobs[i].AvgColorScalar[1]) + ", " + std::to_string(blobs[i].AvgColorScalar[2]);

		//		if (entExt == true) {
		if (blobs[i].IO_indicator == true) {
			success &= mSQLManager->executeStatement("insert into tracks (obj_id, pos_x, pos_y, width, height, track_date, track_time, filename, frame_num, obj_state) values ('"
				+ std::to_string(blobs[i].unitID) + "', '" + std::to_string(blobs[i].currentBoundingRect.x) + "', '" + std::to_string(blobs[i].currentBoundingRect.y) + "', '"
				+ std::to_string(blobs[i].currentBoundingRect.width) + "', '" + std::to_string(blobs[i].currentBoundingRect.height) + "', '"
				+ GlobalClass::instance()->get_InputDate() + "', '" + GlobalClass::instance()->get_InputTime(frameCount, vidLength) + "', '" + GlobalClass::instance()->get_InputFileName() + "', '"
				+ std::to_string(frameCount) + "', '" + "Enter: " + std::to_string(entrance) + "');");


			success &= mSQLManager->executeStatement("UPDATE objects SET enter_from = " + std::to_string(entrance)
				+ " where filename = " + '"' + GlobalClass::instance()->get_InputFileName() + '"' + " and obj_id = " + std::to_string(blobs[i].unitID) + ";");

			success &= mSQLManager->executeStatement("INSERT into objects (obj_id, filename, obj_type, obj_color, enter_from, parked_at,  parked_duration) SELECT '"
				+ std::to_string(blobs[i].unitID) + "', '" + GlobalClass::instance()->get_InputFileName() + "', '" + "defaulted-car" + "', '" + colorInfo + "', '"
				+ std::to_string(entrance) + "', '" + "NA" + "', '" + "NA" + "'"
				+ " where NOT EXISTS(select * from objects where filename = " + '"' + GlobalClass::instance()->get_InputFileName() + '"' + " and obj_id = " + std::to_string(blobs[i].unitID) + ");");

		}
		else {
			success &= mSQLManager->executeStatement("insert into tracks (obj_id, pos_x, pos_y, width, height, track_date, track_time, filename, frame_num, obj_state) values ('"
				+ std::to_string(blobs[i].unitID) + "', '" + std::to_string(blobs[i].currentBoundingRect.x) + "', '" + std::to_string(blobs[i].currentBoundingRect.y) + "', '"
				+ std::to_string(blobs[i].currentBoundingRect.width) + "', '" + std::to_string(blobs[i].currentBoundingRect.height) + "', '"
				+ GlobalClass::instance()->get_InputDate() + "', '" + GlobalClass::instance()->get_InputTime(frameCount, vidLength) + "', '" + GlobalClass::instance()->get_InputFileName() + "', '"
				+ std::to_string(frameCount) + "', '" + "Exit: " + std::to_string(entrance) + "');");

			success &= mSQLManager->executeStatement("UPDATE objects SET exit_from = " + std::to_string(entrance)
				+ " where filename = " + '"' + GlobalClass::instance()->get_InputFileName() + '"' + " and obj_id = " + std::to_string(blobs[i].unitID) + ";");


			success &= mSQLManager->executeStatement("INSERT into objects (obj_id, filename, obj_type, obj_color, parked_at, exit_from, parked_duration) SELECT '"
				+ std::to_string(blobs[i].unitID) + "', '" + GlobalClass::instance()->get_InputFileName() + "', '" + "defaulted-car" + "', '" + colorInfo + "', '"
				+ "NA" + "', '" + std::to_string(entrance) + "', '" + "NA" + "'"
				+ " where NOT EXISTS(select * from objects where filename = " + '"' + GlobalClass::instance()->get_InputFileName() + '"' + " and obj_id = " + std::to_string(blobs[i].unitID) + ");");

		}
		if (!success)
		{
			std::cin.get();
			//LOGERROR("Error saving object " << (*it)->getId());
			std::cout << "ERROR while writing into DB [Vehicle Enter/Exit], blob ID: " + std::to_string(blobs[i].unitID) + "\n";
			success = true;
		}


		success &= mSQLManager->endTransaction();
		delete mSQLManager;
		return success;
	}
	else
	{
		std::cout << "NOT CONNECTED TO DB \n";
		delete mSQLManager;
	}
	

}


bool CarParkTrackExporter::writeToDB_park(std::vector<Blob> &blobs, unsigned int &i, int &frameCount, int &vidLength, std::string parkzone)
{

	mSQLManager = new SQLiteManager(mFilePath);
	if (mSQLManager->isConnected())
	{
		bool success = mSQLManager->beginTransaction();

		if (blobs[i].park == true) {
			success &= mSQLManager->executeStatement("insert into tracks (obj_id, pos_x, pos_y, width, height, track_date, track_time, filename, frame_num, obj_state) values ('"
				+ std::to_string(blobs[i].unitID) + "', '" + std::to_string(blobs[i].currentBoundingRect.x) + "', '" + std::to_string(blobs[i].currentBoundingRect.y) + "', '"
				+ std::to_string(blobs[i].currentBoundingRect.width) + "', '" + std::to_string(blobs[i].currentBoundingRect.height) + "', '"
				+ GlobalClass::instance()->get_InputDate() + "', '" + GlobalClass::instance()->get_InputTime(frameCount, vidLength) + "', '" + GlobalClass::instance()->get_InputFileName() + "', '"
				+ std::to_string(frameCount) + "', '" + "Park : " + parkzone + std::to_string(blobs[i].parkinglot) + "');");


			success &= mSQLManager->executeStatement("UPDATE objects SET parked_at = \"" + parkzone + std::to_string(blobs[i].parkinglot)
				+ "\" where filename = " + '"' + GlobalClass::instance()->get_InputFileName() + '"' + " and obj_id = " + std::to_string(blobs[i].unitID) + ";");

			success &= mSQLManager->executeStatement("INSERT into objects (obj_id, filename, obj_type, obj_color, enter_from, parked_at,  parked_duration) SELECT '"
				+ std::to_string(blobs[i].unitID) + "', '" + GlobalClass::instance()->get_InputFileName() + "', '" + "defaulted-car" + "', '" + "color-kiv" + "', '"
				+ "NA" + "', '" + parkzone + std::to_string(blobs[i].parkinglot) + "', '" + "NA" + "'"
				+ " where NOT EXISTS(select * from objects where filename = " + '"' + GlobalClass::instance()->get_InputFileName() + '"' + " and obj_id = " + std::to_string(blobs[i].unitID) + ");");

		}
		else {
			success &= mSQLManager->executeStatement("insert into tracks (obj_id, pos_x, pos_y, width, height, track_date, track_time, filename, frame_num, obj_state) values ('"
				+ std::to_string(blobs[i].unitID) + "', '" + std::to_string(blobs[i].currentBoundingRect.x) + "', '" + std::to_string(blobs[i].currentBoundingRect.y) + "', '"
				+ std::to_string(blobs[i].currentBoundingRect.width) + "', '" + std::to_string(blobs[i].currentBoundingRect.height) + "', '"
				+ GlobalClass::instance()->get_InputDate() + "', '" + GlobalClass::instance()->get_InputTime(frameCount, vidLength) + "', '" + GlobalClass::instance()->get_InputFileName() + "', '"
				+ std::to_string(frameCount) + "', '" + "Leave: " + parkzone + std::to_string(blobs[i].parkinglot) + "');");


			/*success &= mSQLManager->executeStatement("UPDATE objects SET parked_at = \"" + parkzone + std::to_string(blobs[i].parkinglot)
			+ "\" where filename = " + '"' + GlobalClass::instance()->get_InputFileName() + '"' + " and obj_id = " + std::to_string(blobs[i].unitID) + ";");

			success &= mSQLManager->executeStatement("INSERT into objects (obj_id, filename, obj_type, obj_color, enter_from, parked_at,  parked_duration) SELECT '"
			+ std::to_string(blobs[i].unitID) + "', '" + GlobalClass::instance()->get_InputFileName() + "', '" + "defaulted-car" + "', '" + "color-kiv" + "', '"
			+ "NA" + "', '" + parkzone + std::to_string(blobs[i].parkinglot) + "', '" + "NA" + "'"
			+ " where NOT EXISTS(select * from objects where filename = " + '"' + GlobalClass::instance()->get_InputFileName() + '"' + " and obj_id = " + std::to_string(blobs[i].unitID) + ");");*/
		}
		if (!success)
		{
			std::cin.get();
			//LOGERROR("Error saving object " << (*it)->getId());
			std::cout << "ERROR while writing into DB [Vehicle Parking], blob ID: " + std::to_string(blobs[i].unitID) + "\n";
			success = true;
		}


		success &= mSQLManager->endTransaction();
		delete mSQLManager;
		return success;
	}
	else
	{
		std::cout << "NOT CONNECTED TO DB \n";
		delete mSQLManager;
	}
	

}


bool CarParkTrackExporter::countTraj()
{
	
	int numOfTrajs = 0;

	mSQLManager = new SQLiteManager(mFilePath);
	if (mSQLManager->isConnected())
	{
		bool success = mSQLManager->beginTransaction();
		std::vector<std::vector<std::string>> r_result;
		//std::vector<char> r_result;

		
		/*bool SQLiteManager::executeStatementGetArray(const std::string& statement, std::vector<std::vector<std::string>>& result)
		{
			char **dbResult = 0;
			int nrows, ncols;
			bool success = executeStatementWithReturn(statement.c_str(), &dbResult, nrows, ncols);

			result.resize(nrows);
			if (success)
			{
				for (int row = 0; row < nrows; ++row)
				{
					result[row].resize(ncols);
					for (int col = 0; col < ncols; ++col)
					{
						if (dbResult[ncols + row * ncols + col] != NULL)
							result[row][col] = dbResult[ncols + row * ncols + col];
						else
							result[row][col] = std::string();
					}
				}
			}

			sqlite3_free_table(dbResult);

			return success;
		}*/


		success &= mSQLManager->executeStatementGetArray("select count(distinct obj_id) from tracks;", r_result);
		
		if (!success)
		{
			
			std::cout << "WARNING: Unable to obtain number of unique obj_id from DB \n";
			success = true;
		}
		else
		{

			numOfTrajs = std::stoi(r_result[0][0]);
			std::cout << "Number of unique objects in database: " << numOfTrajs << " " << std::endl;

		}

		success &= mSQLManager->endTransaction();
		delete mSQLManager;
		return success;
	}
	else
	{
		std::cout << "NOT CONNECTED TO DB \n";
		delete mSQLManager;
	}
}


bool CarParkTrackExporter::saveCurrentContext()
{
	//Sort and rename object
	/*cleanObjectsId();
	bool success = saveObjectTypeTable();
	success &= saveSceneObjectTable();
	this->updateProgressBarPercent(10);
	success &= savePositionTable();
	return success;*/
	return 0;
}


bool CarParkTrackExporter::saveSceneObjectTable()
{
	/*bool success = mSQLManager->beginTransaction();
	std::vector<SceneObject*>& sceneObjectList = mContext->getSceneObjectList();

	for(auto it = sceneObjectList.begin(); it != sceneObjectList.end();++it)
	{
	success &= mSQLManager->executeStatement("insert into sceneobject (object_id, type, description) values ('" + (*it)->getId() + "', '" +  (*it)->getType() + "', '" + (*it)->getDescription() + "');");
	if(!success)
	{
	LOGERROR("Error saving object " << (*it)->getId());
	success = true;
	}
	}

	success &= mSQLManager->endTransaction();
	return success;*/
	return 0;
}

bool CarParkTrackExporter::saveObjectTypeTable()
{
	/*bool success = mSQLManager->beginTransaction();
	const ObjectTypeManager& manager = mContext->getObjectTypeManager();
	auto typeList = manager.getTypeList();
	std::stringstream ss;
	for(auto it = typeList.begin(); it != typeList.end();++it)
	ss << "insert into object_type (name) values ('" << (*it).second << "');";
	success &= mSQLManager->executeStatement(ss.str());
	success &= mSQLManager->endTransaction();
	return success;*/
	return 0;
}

bool CarParkTrackExporter::savePositionTable()
{
	//bool success = mSQLManager->beginTransaction();
	//std::map<unsigned int, DrawableSceneView*>& timestampToScene= mContext->getTimestampToScene();
	//int nbInc = 0;
	//for (auto it = timestampToScene.begin(); it != timestampToScene.end();++it)
	//{		
	//	updateProgressBarPercent(10+90.f*((float)nbInc/timestampToScene.size()));
	//	++nbInc;
	//	std::stringstream ss;
	//	unsigned int timestamp = (*it).first;
	//	DrawableSceneView* sceneView = (*it).second;
	//	QList<QGraphicsItem *> items = sceneView->items();
	//	for (int i = 0; i < items.size(); ++i)
	//	{
	//		ObjectObservation* obj = dynamic_cast<ObjectObservation*>(items[i]);
	//		if(obj)
	//		{
	//			QRectF rect = obj->rect();
	//			std::string objId = "unassociatedObject" + Utils::String::toString(i); //This id is reserved
	//			if(obj->getSceneObject())
	//				objId = obj->getSceneObject()->getId();

	//			int x1 = rect.topLeft().x() >= 0 ? rect.topLeft().x():0;
	//			x1 = x1 < mWidth ? x1 : mWidth-1;
	//			int y1 = rect.topLeft().y() >= 0 ? rect.topLeft().y():0;
	//			y1 = y1 < mHeight ? y1 : mHeight-1;
	//			int x2 = rect.bottomRight().x() < mWidth ? rect.bottomRight().x() : mWidth-1;
	//			x2 = x2 >=0 ? x2 : 0;
	//			int y2 = rect.bottomRight().y() < mHeight ? rect.bottomRight().y() : mHeight-1;
	//			y2 = y2 >=0 ? y2 : 0;
	//			int w = x2 - x1;
	//			int h = y2 - y1;
	//			if(w<0)
	//			{
	//				auto tmp = x2;
	//				x2 = x1;
	//				x1 = tmp;
	//			}
	//				
	//			if(h<0)
	//			{
	//				auto tmp = y2;
	//				y2 = y1;
	//				y1 = tmp;
	//			}
	//			
	//			if(h != 0 && w != 0)
	//			{
	//				ss << "insert into position (object_id, frame_number, top_left_corner_x, top_left_corner_y, bottom_right_corner_x, bottom_right_corner_y) values ('"
	//				<< objId << "', " << timestamp <<", " << x1 << ", " << y1 << ", "
	//				<< x2 << ", " << y2 << ");";
	//			}				
	//		}
	//	}	
	//	success &= mSQLManager->executeStatement(ss.str());
	//	if(!success)
	//	{
	//		LOGERROR("Error at " << timestamp);
	//		success = true;
	//	}
	//}
	//
	//success &= mSQLManager->endTransaction();
	//return success;
	return 0;
}