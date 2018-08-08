#pragma once
#include <string>
#include <vector> 
#include <mutex>
#include <chrono>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/log/trivial.hpp>
#include "xyz_robot.h"

namespace fs = ::boost::filesystem;// Define all the thread locks
static std::mutex mt_job_file_lock, mt_job_queue_lock;

class MoveJob
{
public:
	enum MoveJobStatus 
	{
		NO_STATUS = -2,
		FAIL = -1,
		SUCCESS = 1,
		TIMEOUT = 10
	};
};

class GripperJob
{ 
public: 
	enum GripperJobStatus
	{
		NO_STATUS = -2,
		FAIL = -1,
		SUCCESS = 1,
		TIMEOUT = 10
	};
};

// Each step task consisits of one move(if not moving then use the same previous xyz and an gripper job 
class StepTask
{
public:
	enum StepStatus
	{
		NO_STATUS = -2,
		SUCCESS = 1,
		MOVE_FAIL = -1,
		GRIPPER_FAIL = 0
	};
};


class Task
{
public:
	enum Status
	{
		NO_STATUS = -2,
		FAIL = 0,
		SUCCESS = 1
	};
};



// Get all job description files from folder  
// If job folder does not exists, then retur false
bool _GetFilesOfExtenstion(
	const fs::path& root,
	const std::string& ext,
	std::vector<fs::path>&ret
);


// Monitor files of job request 
void _MonitorJobRequest(
	std::string &sJobFolder,
	std::string &sExt,
	std::vector<fs::path> &vFiles,
	bool &bMonitorJobRequest
);

// Read Job Steps from file 
bool _ReadJobFromeFile(
	fs::path& jbFile,
	boost::property_tree::ptree &jbDescription
);

// Read Configure From File
bool _ReadConfigJson(
	std::string &sConfigPath, 
	std::string & sSever, 
	int &nPort, 
	std::string &sPath
);

// Fech job list for execution 
int _ExecuteJobQueue(
	bool &bExecute,
	std::vector<fs::path> &vJobFiles,
	xyz_robot & robotArm 
);

Task::Status _ExecuteJobStepByStep(
	boost::property_tree::ptree &jbDescription,
	xyz_robot & robotArm
);


MoveJob::MoveJobStatus _ExecuteMoveJob(xyz_robot & robotArm, int16_t x, int16_t y, int16_t z);

GripperJob::GripperJobStatus _ExecuteGripperJob(xyz_robot & robotArm, std::string sGripperCmd);


StepTask::StepStatus _ExecutStepJob(
	xyz_robot & robotArm, 
	boost::property_tree::ptree &ptJob
);

bool _ReplaceFileExtension(fs::path &file, std::string &sExt);