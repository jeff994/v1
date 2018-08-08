#pragma once

#include <stdint.h>
#include <string>
#include "modbus.h"

using namespace std;

#define INPUT_REG_SIZE		10
#define HOLDING_REG_SIZE	10


//Scripted motion status
enum ScriptedMtnStatus
{
	notRunning	= 0,
	running 	= 1
};
//Received message status
enum ReceivedMsgStatus
{
	rejected = 0,
	accepted = 1
};

class xyz_robot {
private:
	void parseRobotStatus(void);

	string ipAddress;
	uint16_t port;
	uint16_t slaveId;

	uint16_t inputBuffer[INPUT_REG_SIZE];
	uint16_t holdingBuffer[HOLDING_REG_SIZE];
	uint16_t seq;

	bool isConnected;

	modbus modBus;

public:
	//Arm status
	enum ArmStatus
	{
		other = 0,
		initializing = 1,
		homing = 2,
		idle = 3,
		moving = 4,
		paused = 5,
		error = 15
	};

	//Gripper status
	enum GripperStatus
	{
		otherOrMoving = 0,
		open = 1,
		closed = 2,
		holding = 3
	};


	xyz_robot(string ipAddress, uint16_t port, uint16_t slaveId);
    xyz_robot(void);

    void SetIpAddressPortSlaveID(string ipAddress, uint16_t port, uint16_t slaveId);

	bool ConnectRobot(void);
	int8_t DisconnectRobot(void);

	bool UpdateRobotStatus(void);
	int8_t RunOrStopScriptedMotion(uint8_t runOrStop);//1=run, 0=stop
	int8_t GoToXYZ(int16_t x, int16_t y, int16_t z);
	int8_t OpenOrCloseGripper(uint8_t closeOrOpen);//1=close, 0=open
	int8_t StopMotion(void);
	int8_t ResetMotorDriver(void);

	int16_t posX;
	int16_t posY;
	int16_t posZ;
	int16_t velX;
	int16_t velY;
	int16_t velZ;
	GripperStatus gripperStatus;
	ArmStatus armStatus;
	ScriptedMtnStatus scriptedMtnStatus;
	ReceivedMsgStatus receivedMsgStatus;
	uint16_t lastMsgRcvdSeq;
	bool limXpos;
	bool limXneg;
	bool limYpos;
	bool limYneg;
	bool limZpos;
	bool limZneg;
};
