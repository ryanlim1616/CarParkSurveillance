#include "SQLiteManager.h"
#include <iostream>
#include "Logger.h"
//#include "ApplicationContext.h"
#include "sqlite3.h" 
#include <assert.h>


SQLiteManager::SQLiteManager(const std::string& fileName)
: mDatabase(nullptr) 
, mIsConnected(false)
{
	connect(fileName);
}

SQLiteManager::~SQLiteManager()
{
	disconnect();
	LOGASSERT(mDatabase == nullptr, "Database should be close");


	//table position
	//trajectory_id, frame_number, x_coordinate, y_coordinate

	//	table velocities
	//	trajectory_id, frame_number, x_coordinate, y_coordinate
}

bool SQLiteManager::executeStatementGetArray(const std::string& statement, std::vector<std::vector<std::string>>& result)
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
}


bool SQLiteManager::executeStatementWithReturn(const std::string& statement, char ***result, int &nrows, int &ncols)
{
	if (!isConnected())
	{
		return false;
	}


	char *errorMsg = NULL;
	int errorCode = sqlite3_get_table(mDatabase, statement.c_str(), result, &nrows, &ncols, &errorMsg);
	return errorCode == SQLITE_OK;
}

bool SQLiteManager::tableExist(const std::string& tableName)
{
	
	std::vector<std::vector<std::string>> objects;
	bool success = executeStatementGetArray("SELECT name FROM sqlite_master WHERE type='table' AND name='" + tableName + "'", objects); 
	return !(objects.empty()) && success;
	
}

bool SQLiteManager::tableFieldExist(const std::string& tableName, const std::string& fieldName)
{
	bool success = tableExist(tableName);
	if(success)
	{
		std::vector<std::vector<std::string>> objects;
		success = executeStatementGetArray("SELECT " + fieldName + " FROM " + tableName, objects);
	}
	return success;

}

bool SQLiteManager::connect(const std::string& fileName)
{
	bool success = true;
	if(mIsConnected)
	{
		LOGWARNING("SQLite is already connected.");
		success = false;
	}
	else
	{

		int status = sqlite3_open(fileName.c_str(), &mDatabase);
		success = status == SQLITE_OK;
		if (success)			
		{
			mIsConnected = true;
			//synchronous = OFF permet de dire à SQL lite de ne pas atteindre que l'opération d'écriture soit terminé dans le système de fichier
			//journal_mode = OFF On ne peut pas faire de rollback ou autre. Je ne pense pas que ce soit nécessaire pour l'instant de toute façon
			mIsConnected = executeStatement("PRAGMA synchronous = OFF") && executeStatement("PRAGMA journal_mode = OFF");
			//std::cout << "Connected to DB" << std::endl;
		} 
		else
		{
			sqlite3_close(mDatabase);
			LOGERROR("Could not connect to database " << fileName);
		}
	}
	return success && mIsConnected;
}

bool SQLiteManager::beginTransaction()
{ 
	const std::string statement = "begin transaction";
	return executeStatement(statement);
}
bool SQLiteManager::endTransaction()
{ 
	const std::string statement = "end transaction";
	return executeStatement(statement);
}

bool SQLiteManager::disconnect()
{
	if(mIsConnected)
	{
		int status = sqlite3_close(mDatabase);
		if (status == SQLITE_OK)
		{
			mDatabase = nullptr;
			mIsConnected = false;
		}
	}
	return !mIsConnected;
}

bool SQLiteManager::executeStatement(const std::string& statement)
{
	if(!mIsConnected)
	{
		LOGERROR("Can't execute statement " << statement << " because sqlite is not connected.");
		return false;
	}
	
	char* errMsg = nullptr;
	int status = sqlite3_exec(mDatabase, statement.c_str(),0,0, &errMsg);
	showError(status,errMsg);

	if (status != SQLITE_OK)
	{
		//display SQL statements:
		std::cout << "\n\n****************************** S Q L  D E B U G  O N ****************************** \n";
		std::cout << "SQL query: " << statement << std::endl;
		std::cout << "*********************************************************************************** \n\n";
	}

	return status == SQLITE_OK;
}


void SQLiteManager::showError(int status, char* err)
{
	if(status != SQLITE_OK)
		LOGERROR("Error code " << status << sqlite3_errmsg(mDatabase));
}
