#pragma once
#include <string>

#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <vector>

class Utility
{
public:
	Utility();

	static bool _ReadConfigJson(
		std::string &sConfigPath,
		std::string & sServer,
		std::string & sPortLeft,
		std::string & sPorRight,
		bool & bRevesePan
	);

	static bool _SplitString(
		std::string sInput,
		std::vector<std::string> &vOut,
		char delimiter
	);

	static bool _ParseQueryFeedback(std::string sInput, int &nValue); 

	~Utility();
};

