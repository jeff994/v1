#pragma once

// The class is designed to handle robot ARM jobs 
#include <iostream> 
#include <string> 
#include <vector> 
#include "xyz_robot.h"
#include "utility.h"
#include <thread>

class JobManager
{
public: 
    // Constructor 
    JobManager();

    // Init robot etc, connect robot and 
    bool StartRobot(string ipAddress, uint16_t port, uint16_t slaveId);
    
    // Get Jobs from json file
    bool StartFetchJob(std::string &sJobFolder);

    // Exece a job 
    bool ExecuteJob(); 

    // Get Job Feedback 
    void GetJobStatus(int nJobID); 
   
    // Complete a job 
    //bool CompleteJob();

    // Pause Job(); 
    bool PauseJob(); 

    // Resume Job(); 
    bool ResumeJob(); 

    ~JobManager(); 
private: 
    // Thread for fetching jobs from jobs files 
    std::thread m_pThreadJobsFetching; 
    std::thread m_pThreadJosExecuting; 
    
    // Job status 
    enum JobStatus {NOTSTARTED, ROBOTREADY, ROBOTERROR};
    // The worker 
    xyz_robot m_rbWorker; 
    // Jobs
    std::vector<fs::path> m_vJobFiles;  

    bool m_bMonitorJobRequest, m_bExecuteJob, m_bStepByStep; 
    



};
