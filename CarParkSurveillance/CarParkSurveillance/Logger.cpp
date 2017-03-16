#include "Logger.h"
#include <iostream>
#include <fstream>
#include <time.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif

#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS

////Code to add color in Windows
//inline std::ostream& blue(std::ostream &s)
//{
//#ifdef _WIN32
//	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
//	SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE
//		|FOREGROUND_GREEN|FOREGROUND_INTENSITY);
//	#endif
//	return s;
//}
//
//inline std::ostream& red(std::ostream &s)
//{
//	#ifdef _WIN32
//	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
//	SetConsoleTextAttribute(hStdout, 
//		FOREGROUND_RED|FOREGROUND_INTENSITY);
//	#endif
//	return s;
//}
//
//inline std::ostream& green(std::ostream &s)
//{
//	#ifdef _WIN32
//	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
//	SetConsoleTextAttribute(hStdout, 
//		FOREGROUND_GREEN|FOREGROUND_INTENSITY);
//	#endif
//	return s;
//}
//
//inline std::ostream& yellow(std::ostream &s)
//{
//	#ifdef _WIN32
//	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
//	SetConsoleTextAttribute(hStdout, 
//		FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
//	#endif
//	return s;
//}
//
//inline std::ostream& white(std::ostream &s)
//{
//	#ifdef _WIN32
//	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
//	SetConsoleTextAttribute(hStdout, 
//		FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
//	#endif
//	return s;
//}


Logger* Logger::m_instance = 0;


Logger::Logger()
: m_level(eINFO)
, m_noFileLog(false)
, m_errorOccured(false)
{
	//Get current time formatted to generate log name
	time_t rawtime;	
	time(&rawtime);
	struct tm * timeinfo=localtime(&rawtime);
	char timebuffer[24];
	//strftime (timebuffer,sizeof(timebuffer)/sizeof(char),"run-%Y%m%d-%H%M.html",timeinfo);
	strftime(timebuffer, sizeof(timebuffer) / sizeof(char), "run-log.html", timeinfo);
	m_logName = std::string(timebuffer);
	m_fstream.open(m_logName, std::ofstream::app | std::ofstream::out);
	m_fstream << "<html><head><title>" << m_logName << "</title></head><body bgcolor=\"#272822\"><H1><font color=\"#F8F8F2\">" << m_logName << "</font></H1><table border=\"0\">";
}

Logger::~Logger()
{
	//Close File
	m_fstream << "</table></body></html>";
	m_fstream.close();
}

Logger* Logger::getInstance()
{
	if(!m_instance)
		m_instance = new Logger();
	return m_instance;
}

void Logger::cleanUp()
{
	delete m_instance;
	m_instance = 0;
}

//TODO: Separate FileLogger, ConsoleLogger to different class when we decide to add cooler logger like SendEmailOnerror and WebViewLogger

void Logger::logMsg(ELoggingLevel level, const std::stringstream& msgStream, long line, const std::string& function)
{
	logMsg(level, msgStream.str(), line, function);
}

void Logger::logMsg(ELoggingLevel level, const std::string& msg, long line, const std::string& function)
{
	if(level == eERROR)
		m_errorOccured = true;
	if(level >= m_level)
	{
		//Log Console

		{
			switch(level)
			{
			case eDEBUG:
				std::cout << "white";
				break;
			case  eINFO:
				std::cout << "green";
				break;
			case eWARNING:
				std::cout << "yellow";
				break;
			case  eERROR:
				std::cout << "red";
				break;
			}
			std::cout << msg << std::endl;
			std::cout << "white"; 
		}

		//Log File
		if(!m_noFileLog)
		{
			switch(level)
			{
			case eDEBUG:
				m_fstream <<  "<tr><td><font color=\"#F8F8F2\">[DEBUG]</font></td>";
				break;
			case  eINFO:
				m_fstream <<  "<tr><td><font color=\"#7FB347\">[INFO]</font></td>";
				break;
			case eWARNING:
				m_fstream <<  "<tr><td><font color=\"#E6DB74\">[WARNING]</font></td>";
				break;
			case  eERROR:
				m_fstream <<  "<tr><td><font color=\"#D25252\">[ERROR]</font></td>";
				break;
			}

			time_t rawtime;	
			time(&rawtime);
			struct tm * timeinfo=localtime(&rawtime);
			char timebuffer[23];
			strftime (timebuffer,sizeof(timebuffer)/sizeof(char),"[%Y.%m.%d@%H:%M:%S] ",timeinfo);
			m_fstream << "<td><font color=\"#F8F8F2\">" << timebuffer << "&nbsp;</font></td><td><font color=\"#F8F8F2\">" << function << ":" <<  line << "&nbsp;</font></td><td><font color=\"#F8F8F2\">" <<  msg << "&nbsp;</font></td></tr>"; 			
		}
	}
}