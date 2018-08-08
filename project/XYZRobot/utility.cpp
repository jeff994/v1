#include "utility.h"

bool _GetFilesOfExtenstion(
	const fs::path& root,
	const std::string& ext,
	std::vector<fs::path>& ret
)
{
	if (!fs::exists(root) || !fs::is_directory(root)) return false;
	fs::recursive_directory_iterator it(root);
	fs::recursive_directory_iterator endit;
	while (it != endit)
	{
		boost::system::error_code ec;
		std::string sFileExt = it->path().extension().generic_string();
		if (fs::is_regular_file(it->path()) && sFileExt == ext)
		{
			ret.push_back(it->path());
		}
		++it;
	}
	// If has any job, then return true 
	return true;
}

void _MonitorJobRequest(
	std::string &sJobFolder,
	std::string &sExt,
	std::vector<fs::path> &vFiles,
	bool &bMonitorJobRequest
)
{
	while (bMonitorJobRequest)
	{
		fs::path jobFolder(sJobFolder);
		std::vector<fs::path> vFilesNotProcessed; 
		bool bRet = _GetFilesOfExtenstion(jobFolder, sExt, vFilesNotProcessed);
		if (!bRet)
		{
			BOOST_LOG_TRIVIAL(error) << "The job folder does not exist in: " << jobFolder << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		for (auto it = vFilesNotProcessed.begin(); it != vFilesNotProcessed.end(); ++it)
		{
			std::string sExt("qrj"); 
			_ReplaceFileExtension(*it, sExt);
			mt_job_file_lock.lock();
			vFiles.push_back(*it);
			mt_job_file_lock.unlock();
		}
		// Log the job request 
		// check whether there's a job files 
	}
}

bool _ReadJobFromeFile(
	fs::path& jbFile,
	boost::property_tree::ptree &jbDescription
)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
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


int _ExecuteJobQueue(
	bool &bExecute,
	std::vector<fs::path> &vJobFiles,
	xyz_robot & robotArm
)
{
	while (bExecute)
	{
		mt_job_file_lock.lock();
		if (vJobFiles.size() == 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			mt_job_file_lock.unlock();
			continue; 
		}
		auto it = vJobFiles.begin();
		fs::path jbFile = *it; 
		vJobFiles.erase(it);
		mt_job_file_lock.unlock();
		BOOST_LOG_TRIVIAL(error) << "Number of jobs pending " << vJobFiles.size() << std::endl;
		boost::property_tree::ptree jobDescription; 
		if (!_ReadJobFromeFile(jbFile, jobDescription))
		{
			BOOST_LOG_TRIVIAL(error) << "Not able read job description from " << jbFile << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue; 
		}
		std::string sExt("inp");
		_ReplaceFileExtension(jbFile, sExt);

		Task::Status tStatus = _ExecuteJobStepByStep(jobDescription, robotArm);
		if (tStatus != Task::Status::SUCCESS)
		{
			std::string sExt("err");
			_ReplaceFileExtension(jbFile, sExt);
			BOOST_LOG_TRIVIAL(error) << "Task executing get error " << std::endl;
		}
		else
		{
			std::string sExt("don");
			_ReplaceFileExtension(jbFile, sExt);
			BOOST_LOG_TRIVIAL(info) << "Task executing successful " << std::endl;
		}
	}
	return true; 
}

MoveJob::MoveJobStatus _ExecuteMoveJob(xyz_robot & robotArm, int16_t x, int16_t y, int16_t z)
{
	robotArm.GoToXYZ(x, y, z);
	int nCount = 0; 
	MoveJob::MoveJobStatus nStatus = MoveJob::MoveJobStatus::FAIL;
	while (true)
	{
		bool bRet = robotArm.UpdateRobotStatus();
		if (!bRet)
		{
			nStatus = MoveJob::MoveJobStatus::NO_STATUS;
			break; 
		}
		if (robotArm.armStatus == xyz_robot::ArmStatus::moving)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			nCount = 0; 
		}
		// Steps completed with no error 
		if (robotArm.armStatus == xyz_robot::ArmStatus::idle)
		{
			nStatus = MoveJob::MoveJobStatus::SUCCESS; //Normal 
			break;
		}
		if (robotArm.armStatus == xyz_robot::ArmStatus::error)
		{
			nStatus = MoveJob::MoveJobStatus::FAIL;  // Error 
		}

		if (nCount > 10) {
			nStatus = MoveJob::MoveJobStatus::TIMEOUT;  // Time out 
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		nCount++; 
	}
	return nStatus;
}

GripperJob::GripperJobStatus _ExecuteGripperJob(xyz_robot & robotArm, std::string sGripperCmd)
{
	bool bGripperClose = true;
	GripperJob::GripperJobStatus status = GripperJob::GripperJobStatus::FAIL;
	// Hanlde open command 
	if (sGripperCmd == "open")
	{
		robotArm.OpenOrCloseGripper(!bGripperClose);

		int nCount = 0;
		while (true)
		{
			bool bRet = robotArm.UpdateRobotStatus();
			if (!bRet)
			{
				status = GripperJob::GripperJobStatus::NO_STATUS;
				break;
			}
			if (robotArm.gripperStatus == xyz_robot::GripperStatus::open)
			{
				status = GripperJob::GripperJobStatus::SUCCESS;
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (nCount > 10)
			{
				status = GripperJob::GripperJobStatus::TIMEOUT;
				break;
			}
			nCount++;
		}
	}
	// Handle close command 
	else if (sGripperCmd == "close")
	{
		robotArm.OpenOrCloseGripper(bGripperClose);

		int nCount = 0;
		while (true)
		{
			bool bRet = robotArm.UpdateRobotStatus();
			if (!bRet)
			{
				status = GripperJob::GripperJobStatus::NO_STATUS;
				break;
			}
			if (robotArm.gripperStatus == xyz_robot::GripperStatus::closed ||
				robotArm.gripperStatus == xyz_robot::GripperStatus::holding)
			{
				status = GripperJob::GripperJobStatus::SUCCESS;
				break;
			}

			if (robotArm.gripperStatus == xyz_robot::GripperStatus::closed)
			{
				status = GripperJob::GripperJobStatus::SUCCESS;
				break;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (nCount > 10)
			{
				status = GripperJob::GripperJobStatus::TIMEOUT;
				break;
			}
			nCount++;
		}
	}
	else if (sGripperCmd == "NA")
	{
		status = GripperJob::GripperJobStatus::SUCCESS;
	}
	else
	{
		BOOST_LOG_TRIVIAL(error) << "Invalid gripper status detected: " << sGripperCmd <<std::endl;
	}
	return status; 

}

StepTask::StepStatus _ExecutStepJob(
	xyz_robot & robotArm, 
	boost::property_tree::ptree &ptJob
)
{
	std::string desc					= ptJob.get<std::string>("desc");
	boost::property_tree::ptree ptMove	= ptJob.get_child("move"); 
	int16_t x							= ptMove.get<int16_t>("x");
	int16_t y							= ptMove.get<int16_t>("y");
	int16_t z							= ptMove.get<int16_t>("z");
	int nWatiting						= ptJob.get<int>("wait_time");
	int nStatus							= ptJob.get<int>("status");
	std::string sGripper				= ptJob.get<std::string>("gripper");


	MoveJob::MoveJobStatus move_status = _ExecuteMoveJob(robotArm, x, y, z);
	if (move_status == MoveJob::MoveJobStatus::NO_STATUS)
	{
		BOOST_LOG_TRIVIAL(error) << "The robot to get any status feedback from robot"<< std::endl;
		return StepTask::StepStatus::NO_STATUS;
	}

	if (move_status != MoveJob::MoveJobStatus::SUCCESS)
	{
		BOOST_LOG_TRIVIAL(error) << "The robot failed at the step while moving to  (" << x << ", " << y << ", " << z << ")" << std::endl;
		return StepTask::StepStatus::MOVE_FAIL;
	}
	GripperJob::GripperJobStatus gripper_status = _ExecuteGripperJob(robotArm, sGripper);

	if (gripper_status != GripperJob::GripperJobStatus::SUCCESS)
	{
		BOOST_LOG_TRIVIAL(error) << "The robot failed at the step while operating gripper" << std::endl;
		return StepTask::StepStatus::GRIPPER_FAIL;
	}

	 //_GetStepJobResult(robotArm, bGripperClose);
	return  StepTask::StepStatus::SUCCESS;
}

Task::Status _ExecuteJobStepByStep(
	boost::property_tree::ptree &jbDescription,
	xyz_robot & robotArm
)
{

	Task::Status tStatus = Task::Status::SUCCESS;
	
	int nCurrentStep = 0;  // Logging current steps 
	int nCount = 0; 
	boost::property_tree::ptree pt = jbDescription.get_child("JobSteps");
	for (auto it = pt.begin(); it != pt.end(); ++it)
	{
		StepTask::StepStatus nStatus = StepTask::StepStatus::SUCCESS;
		boost::property_tree::ptree ptJob = it->second;

		std::ostringstream oss;
		boost::property_tree::write_json(oss, ptJob);
		std::string inifile_text = oss.str();
		BOOST_LOG_TRIVIAL(info) << "Executing: \n"<< inifile_text <<std::endl;

		
		nStatus  = _ExecutStepJob(robotArm, ptJob);
		if (nStatus != StepTask::StepStatus::SUCCESS)
		{
			BOOST_LOG_TRIVIAL(error) << "The robot failed while executing job step " << nCurrentStep << std::endl;
			tStatus = Task::Status::FAIL;
			break; 
		}
		
		// Proceed to next job step 
		nCurrentStep++; 
	}
	return tStatus;
}

bool _ReadConfigJson(
	std::string &sConfigPath, 
	std::string & sServer, 
	int &nPort, 
	std::string &sPath
)
{
	boost::filesystem::path spath = boost::filesystem::current_path();
	spath.append(sConfigPath);
	boost::property_tree::ptree cfg;  
	bool bRet = _ReadJobFromeFile(spath, cfg);
	if (!bRet) return bRet; 

	sServer = cfg.get<std::string>("robot_ip");
	nPort 	= cfg.get<int>("port"); 
	sPath	= cfg.get<std::string>("job_folder");
	return true; 
}

bool _ReplaceFileExtension(fs::path &file, std::string &sExt)
{
	fs::path oldFile = file;
	fs::path newpath = file.replace_extension(sExt);
	fs::rename(oldFile, newpath);
	int nCount = 0; 
	bool bStatus = false; 
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (fs::exists(newpath) && fs::is_regular_file(newpath))
		{
			bStatus = true; 
			break;
		}
		nCount++; 
		if (nCount > 10)
		{
			bStatus = false; 
			break;
		}
		BOOST_LOG_TRIVIAL(info) << "Waiting ranme of file " << newpath << std::endl;
	}
	return bStatus;
}