#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "config.h"
#include "XYZRobot/xyz_robot.h"
#include "XYZRobot/modbus.h"
//#include <boost/lambda/lambda.hpp>
#include <iostream>
#include <iterator>
#include <algorithm>
//#include "XYZRobot/JobManager.h"

int main(int argc, char *argv[])
{	
	/*
	
	std::string sServer, sPath;
	int  nPort = 0; 
	bool bRet = 0; 
	
	while(true)
	{
		std::string sConfigPath("config.json");
		bRet = _ReadConfigJson(
			sConfigPath, 
			sServer, 
			nPort, 
			sPath);

		if (bRet) break; 

		// Failed to read configure file 
		BOOST_LOG_TRIVIAL(error) << "Failed to read config file" << std::endl;
		BOOST_LOG_TRIVIAL(info) << "The config file path should be: " << sConfigPath << std::endl;
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	JobManager jbm; 
	while (true)
	{
		bRet = jbm.StartRobot(sServer, nPort, 1);
		if (!bRet)
		{
			BOOST_LOG_TRIVIAL(error) << "Failed to connect to robot" << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			BOOST_LOG_TRIVIAL(info) << "Retry to connect robot" << std::endl;
			continue; 
		}
		jbm.StartFetchJob(sPath);
	}

		
		*/
	return 0;
}