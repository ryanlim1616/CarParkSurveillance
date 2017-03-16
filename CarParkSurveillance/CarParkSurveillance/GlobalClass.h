#pragma once
#include <stdio.h>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <algorithm>

class GlobalClass
{
	int TotalFrames;
	std::string InputFileName, InputDate_old, InputTime_old, InputDate_new, InputTime_new;
	std::string ifn = "NULL";
	double time_diff = 0;
	time_t InputTime_time;
	struct tm *timeinfo = new struct tm();

	static GlobalClass *s_instance;
	//GlobalClass(std::string ifn="20161018_182400.mp4")
	GlobalClass()
	{
		/*InputFileName = ifn;
		InputDate_old = InputFileName.substr(0, 8);
		InputTime_old = InputFileName.substr(9, 6);*/

		InputFileName = "";
		InputDate_old = "";
		InputTime_old = "";
	}
public:

	std::istringstream exec(const char* cmd) {
		std::array<char, 512> buffer;
		std::string result;
		std::shared_ptr<FILE> pipe(_popen(cmd, "r"), _pclose);
		if (!pipe) throw std::runtime_error("popen() failed!");
		while (!feof(pipe.get())) {
			if (fgets(buffer.data(), 512, pipe.get()) != NULL)
				result += buffer.data();
		}
		std::istringstream iss (result);
		return iss;
	}

	void set_InputFileName(std::string inp_name)
	{
		InputFileName = inp_name;
		InputDate_old = InputFileName.substr(44, 8);
		InputTime_old = InputFileName.substr(53, 6);
	}

	std::string get_InputFileName()
	{
		return InputFileName;
	}
	std::string get_InputDate()
	{
		InputDate_new = InputDate_old.substr(0, 4) + "-" + InputDate_old.substr(4, 2) + "-" + InputDate_old.substr(6, 2);
		//std::cout << InputDate_new;
		return InputDate_new;
	}
	std::string get_InputTime(int vframeCount, int vidLength)
	{
		double vidDuration = vidLength * 60;
		int temp_time = vframeCount * (vidDuration / double(TotalFrames));
		time(&InputTime_time);
		timeinfo = localtime(&InputTime_time);
		timeinfo->tm_hour = std::stoi(InputTime_old.substr(0, 2));
		timeinfo->tm_min = std::stoi(InputTime_old.substr(2, 2));
		timeinfo->tm_sec = temp_time + std::stoi(InputTime_old.substr(4, 2));


		mktime(timeinfo);

		std::string temp_hour, temp_minute, temp_sec;

		if (timeinfo->tm_hour < 10)
		{
			temp_hour = "0" + std::to_string(timeinfo->tm_hour);
		}
		else
		{
			temp_hour = std::to_string(timeinfo->tm_hour);
		}

		if (timeinfo->tm_min < 10)
		{
			temp_minute = "0" + std::to_string(timeinfo->tm_min);
		}
		else
		{
			temp_minute = std::to_string(timeinfo->tm_min);
		}

		if (timeinfo->tm_sec < 10)
		{
			temp_sec = "0" + std::to_string(timeinfo->tm_sec);
		}
		else
		{
			temp_sec = std::to_string(timeinfo->tm_sec);
		}

		InputTime_new = temp_hour + ":" + temp_minute + ":" + temp_sec;
		
		//std::cout << strftime(buffer, 80, "%I:%M%:%S", timeinfo) << std::endl;

		return InputTime_new;
	}
	void set_TotalFrames(int vTotalFrames)
	{
		TotalFrames = vTotalFrames;
	}
	int get_TotalFrames()
	{
		//std::cout << "total frames: " + std::to_string(TotalFrames) + "\n";
		return TotalFrames;
	}


	static GlobalClass *instance()
	{
		if (!s_instance)
			s_instance = new GlobalClass;
		return s_instance;
	}
};