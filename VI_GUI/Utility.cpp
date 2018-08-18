#include <sstream>
#include <iostream>
#include "stdafx.h"
#include "Utility.h"


namespace fs = ::boost::filesystem;
bool _ReadJobFromeFile(
	fs::path& jbFile,
	boost::property_tree::ptree &jbDescription
)
{
	try
	{
		boost::property_tree::read_json(jbFile.generic_string(), jbDescription);
	}
	catch (...)
	{
		return false;
	}
	return true;
}

Utility::Utility()
{
}

bool Utility::_ReadConfigJson(
	std::string &sConfigPath,
	std::string &sServer,
	std::string &sPortLeft,
	std::string &sPorRight,
	bool & bRevesePan
	
)
{
	boost::filesystem::path spath = boost::filesystem::current_path();
	spath.append(sConfigPath);
	boost::property_tree::ptree cfg;
	bool bRet = _ReadJobFromeFile(spath, cfg);
	if (!bRet) return bRet;

	sServer = cfg.get<std::string>("robot_ip");
	sPortLeft = cfg.get<std::string>("port_left");
	sPorRight = cfg.get<std::string>("port_right");
	bool bReveredPan = cfg.get<bool>("reverse_pan");
	return true;
}
Utility::~Utility()
{
}

bool Utility::_SplitString(
	std::string sInput,
	std::vector<std::string> &vOut,
	char delimiter)
{
	std::istringstream sStringStream(sInput.c_str());
	std::string s;
	while (getline(sStringStream, s, delimiter)) {
		//std::cout << s << std::endl;
		vOut.push_back(s);
	}
	return true; 
}

// sample string "?F 2\rF=-100\r"
bool Utility::_ParseQueryFeedback(std::string sInput, int &nValue)
{
	std::vector<std::string> vOut; 
	_SplitString(sInput, vOut, '\r');
	if (vOut.size() < 2)
		return false; 
	std::string sFeedBack = vOut[1]; 
	std::vector<std::string> vOutFeedback;
	_SplitString(sFeedBack, vOutFeedback, '=');
	if (vOutFeedback.size() < 2) 
		return false; 
	std::string sValue = vOutFeedback[1]; 
	nValue = std::stoi(sValue); 
	return true; 
}