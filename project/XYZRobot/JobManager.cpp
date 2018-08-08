#include <chrono>
#include <thread>
#include <mutex>
#include <boost/filesystem.hpp>
#include "JobManager.h"
#include <WS2tcpip.h>
#include "utility.h"


JobManager::JobManager()
{
}

JobManager::~JobManager()
{
    // Delete the thread 
   // if(m_pThreadJobsFetching){delete m_pThreadJobsFetching; m_pThreadJobsFetching = NULL;}
}

bool JobManager::StartRobot(string ipAddress, uint16_t port, uint16_t slaveId)
{
    // Initialize robot 
    m_rbWorker.SetIpAddressPortSlaveID(ipAddress, port, 1);
    // Connect Robot
    if(!m_rbWorker.ConnectRobot())
    {
        return false; 
    } 
    return true; 
}

void JobManager::GetJobStatus(int nJobID)
{
    // Job not in the system
    // Job picked up 
    // Job Executing 
    // Job completed sucess 
    // Job filed 
}

bool JobManager::StartFetchJob(std::string &sJobFolder)
{
    // Use a thread to with file name jrq extension
    std::string sExt(".jrq");
    // Step 1: Monitor and fetch the job description files 
	m_bMonitorJobRequest = true; 
	//_MonitorJobRequest(sJobFolder, sExt, m_vJobFiles, m_bMonitorJobRequest);

	m_pThreadJobsFetching = std::thread(
        _MonitorJobRequest,
        std::ref(sJobFolder), 
        std::ref(sExt), 
        std::ref(m_vJobFiles),
        std::ref(m_bMonitorJobRequest)
        );


    //m_pThreadJobsFetching->start();
	m_bExecuteJob = true; 
    // Step 2: Start to parse the job and pass to job executor 
	m_pThreadJosExecuting = std::thread(_ExecuteJobQueue, std::ref(m_bExecuteJob), std::ref(m_vJobFiles), std::ref(m_rbWorker));
	m_pThreadJobsFetching.join();
	m_pThreadJosExecuting.join();
    //m_pThreadJosExecuting = new std::thread(&JobManager::ExecuteJob)
    return true; 
}


bool JobManager::ExecuteJob()
{
    while(m_bExecuteJob)
    {
        mt_job_file_lock.lock();
        auto it = m_vJobFiles.begin();
        boost::property_tree::ptree jobDescription;   
        //bool bRet =  _ReadJobFromFile(*it,jobDescription);
        mt_job_file_lock.unlock(); 
        // Execute jobs 
       // _ExecuteJobStepByStep(jobDescription);
    }
    // Once any job in the queue, just start execute until it's finished 
    return true; 
}

bool JobManager::PauseJob()
{
    return true; 
}

bool JobManager::ResumeJob()
{
    return true; 
}
