#ifndef SQLITE_MANAGER_H
#define SQLITE_MANAGER_H

#include <string>
#include <vector>
#include "sqlite3.h" 
#include "StringHelpers.h"



#undef MIN
#undef MAX
//Mostly inspired by TrafficIntelligence
class SQLiteManager
{
public:
	SQLiteManager(const std::string& fileName);
	virtual ~SQLiteManager();
	bool executeStatement(const std::string& statement);
	template<class T>
	bool executeStatementGetSingleValue(const std::string& statement, T& value);
	template<class T>
	bool executeStatementGetSingleColumn(const std::string& statement, std::vector<T>& column);
	//This is returned as string since it's the only way to support multiple function
	bool executeStatementGetArray(const std::string& statement, std::vector<std::vector<std::string>>& array);

	bool tableExist(const std::string& tableName);
	bool tableFieldExist(const std::string& tableName, const std::string& fieldName);
	bool executeStatementWithReturn(const std::string& statement, char ***result, int &nrows, int &ncols);


	bool isConnected() const {return mIsConnected;}
	bool beginTransaction();
	bool endTransaction();
protected:
	sqlite3* mDatabase;
private:
	bool connect(const std::string& fileName);
	bool disconnect();

	void showError(int status, char* err);
	bool mIsConnected;

};

template<class T>
bool SQLiteManager::executeStatementGetSingleValue(const std::string& statement, T& value)
{
	char **dbResult = 0;

	int nrows, ncols;
	bool success = executeStatementWithReturn(statement, &dbResult, nrows, ncols);
	if (success && nrows == 1 && ncols == 1 && dbResult[1] != NULL) //First item is header
	{
		success = Utils::String::StringToType<T>(*dbResult[1], value);
	}
	else
	{
		success = false;
	}

	sqlite3_free_table(dbResult);

	return success;
}

template<class T>
bool SQLiteManager::executeStatementGetSingleColumn(const std::string& statement, std::vector<T>& columns)
{
	char **dbResult = 0;

	int nrows, ncols;
	bool success = executeStatementWithReturn(statement.c_str(), &dbResult, nrows, ncols);
	if(success && ncols == 1)
	{
		columns.resize(nrows);
		for (int row = 1; row < nrows+1; ++row) //We want to skip the header
		{
			std::string str(dbResult[row]);
			success = Utils::String::StringToType<T>(str, columns[row-1]);
		}
	}

	sqlite3_free_table(dbResult);

	return success;
}

#endif
