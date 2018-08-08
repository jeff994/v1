#include "xyz_robot.h"

//Input registers
#define ACTUAL_POSITION_X	0
#define ACTUAL_POSITION_Y	1
#define ACTUAL_POSITION_Z	2
#define ACTUAL_VELOCITY_X	3
#define ACTUAL_VELOCITY_Y	4
#define ACTUAL_VELOCITY_Z	5
#define ROBOT_STATUS		6
#define LAST_RCVD_CMD_SEQ	7
#define LIMIT_SWITCHES		8

//Limit switches
#define LS_X_POS_MASK	0x0001
#define LS_X_NEG_MASK	0x0002
#define LS_Y_POS_MASK	0x0004
#define LS_Y_NEG_MASK	0x0008
#define LS_Z_POS_MASK	0x0010
#define LS_Z_NEG_MASK	0x0020

//Robot status
#define GRIPPER_STATUS_MASK		0x000F
#define ARM_STATUS_MASK			0x00F0
#define SCRIPTED_STATUS_MASK	0x0F00
#define RCVD_MSG_STATUS_MASK	0xF000
#define GRIPPER_STATUS_SHIFT	0
#define ARM_STATUS_SHIFT		4
#define SCRIPTED_STATUS_SHIFT	8
#define RCVD_MSG_STATUS_SHIFT	12

//Command ID
#define SCRIPTED_MOTION_CMD_ID	0x0001
#define GO_TO_XYZ_CMD_ID		0x0002
#define GRIPPER_CMD_ID			0x0003
#define STOP_MOTION_CMD_ID		0x0004
#define RESET_CMD_ID			0x0005

xyz_robot::xyz_robot(string ipAddress, uint16_t port, uint16_t slaveId) :
modBus(ipAddress, port)
{
	uint8_t i;

	this->ipAddress = ipAddress;
	this->port = port;
	this->slaveId = slaveId;

	isConnected = false;

	posX = 0;
	posY = 0;
	posZ = 0;
	velX = 0;
	velY = 0;
	velZ = 0;
	gripperStatus = otherOrMoving;
	armStatus = other;
	scriptedMtnStatus = notRunning;
	receivedMsgStatus = rejected;
	lastMsgRcvdSeq = 0;
	limXpos = false;
	limXneg = false;
	limYpos = false;
	limYneg = false;
	limZpos = false;
	limZneg = false;

	for(i=0; i<INPUT_REG_SIZE; i++)
		inputBuffer[i] = 0;
	for(i=0; i<HOLDING_REG_SIZE; i++)
		holdingBuffer[i] = 0;
}

xyz_robot::xyz_robot(void)
{
    uint8_t i;

    isConnected = false;

    posX = 0;
    posY = 0;
    posZ = 0;
    velX = 0;
    velY = 0;
    velZ = 0;
    gripperStatus = otherOrMoving;
    armStatus = other;
    scriptedMtnStatus = notRunning;
    receivedMsgStatus = rejected;
    lastMsgRcvdSeq = 0;
    limXpos = false;
    limXneg = false;
    limYpos = false;
    limYneg = false;
    limZpos = false;
    limZneg = false;

    for(i=0; i<INPUT_REG_SIZE; i++)
        inputBuffer[i] = 0;
    for(i=0; i<HOLDING_REG_SIZE; i++)
        holdingBuffer[i] = 0;
}

void xyz_robot::SetIpAddressPortSlaveID(string ipAddress, uint16_t port, uint16_t slaveId)
{
    this->ipAddress = ipAddress;
    this->port = port;
    this->slaveId = slaveId;

    modBus.set_ipaddress_port(ipAddress, port);
}

bool xyz_robot::ConnectRobot(void)
{
	if (modBus.modbus_connect())
	{
		modBus.modbus_set_slave_id(slaveId);
		isConnected = true;
		return true;
	}
	else
	{
		isConnected = false;
		return false;
	}
}

int8_t xyz_robot::DisconnectRobot(void)
{
	if (isConnected)
	{
		modBus.modbus_close();
		isConnected = false;
        return 0;
	}
	else
		return -1;
}

bool xyz_robot::UpdateRobotStatus(void)
{
	if (isConnected)
	{
		if (modBus.modbus_read_input_registers(0, INPUT_REG_SIZE, inputBuffer))
			return false;
		parseRobotStatus();
        return true;
	}
	else
		return false;
}


int8_t xyz_robot::RunOrStopScriptedMotion(uint8_t runOrStop)//1=run, 0=stop
{
	if ((isConnected) && ((runOrStop == 1) || (runOrStop == 0)))
	{
		seq++;
		if (seq == 0)
			seq = 1;
		holdingBuffer[0] = SCRIPTED_MOTION_CMD_ID;
		holdingBuffer[1] = seq;
		holdingBuffer[2] = runOrStop;
		if (modBus.modbus_write_registers(0, 3, holdingBuffer))
			return -1;
		return 0;
	}
	else
		return -1;
}

int8_t xyz_robot::GoToXYZ(int16_t x, int16_t y, int16_t z)
{
	if (isConnected)
	{
		seq++;
		if (seq == 0)
			seq = 1;
		holdingBuffer[0] = GO_TO_XYZ_CMD_ID;
		holdingBuffer[1] = seq;
        holdingBuffer[2] = *((uint16_t*)&x);
        holdingBuffer[3] = *((uint16_t*)&y);
        holdingBuffer[4] = *((uint16_t*)&z);
		if (modBus.modbus_write_registers(0, 5, holdingBuffer))
			return -1;
		return 0;
	}
	else
		return -1;
}

int8_t xyz_robot::OpenOrCloseGripper(uint8_t closeOrOpen)//1=close, 0=open
{
	if ((isConnected) && ((closeOrOpen == 1) || (closeOrOpen == 0)))
	{
		seq++;
		if (seq == 0)
			seq = 1;
		holdingBuffer[0] = GRIPPER_CMD_ID;
		holdingBuffer[1] = seq;
		holdingBuffer[2] = closeOrOpen;
		if (modBus.modbus_write_registers(0, 3, holdingBuffer))
			return -1;
		return 0;
	}
	else
		return -1;
}

int8_t xyz_robot::StopMotion(void)
{
	if (isConnected)
	{
		seq++;
		if (seq == 0)
			seq = 1;
		holdingBuffer[0] = STOP_MOTION_CMD_ID;
		holdingBuffer[1] = seq;
		holdingBuffer[2] = 1;
		if (modBus.modbus_write_registers(0, 3, holdingBuffer))
			return -1;
		return 0;
	}
	else
		return -1;
}

int8_t xyz_robot::ResetMotorDriver(void)
{
	if (isConnected)
	{
		seq++;
		if (seq == 0)
			seq = 1;
		holdingBuffer[0] = RESET_CMD_ID;
		holdingBuffer[1] = seq;
		holdingBuffer[2] = 1;
		if (modBus.modbus_write_registers(0, 3, holdingBuffer))
			return -1;
		return 0;
	}
	else
		return -1;
}

void xyz_robot::parseRobotStatus(void)
{
	posX = inputBuffer[ACTUAL_POSITION_X];
	posY = inputBuffer[ACTUAL_POSITION_Y];
	posZ = inputBuffer[ACTUAL_POSITION_Z];
	velX = inputBuffer[ACTUAL_VELOCITY_X];
	velY = inputBuffer[ACTUAL_VELOCITY_Y];
	velZ = inputBuffer[ACTUAL_VELOCITY_Z];
	gripperStatus 		= (GripperStatus)
						  ((inputBuffer[ROBOT_STATUS] & GRIPPER_STATUS_MASK) >> GRIPPER_STATUS_SHIFT);
	armStatus 			= (ArmStatus)
						  ((inputBuffer[ROBOT_STATUS] &  ARM_STATUS_MASK) >> ARM_STATUS_SHIFT);
	scriptedMtnStatus 	= (ScriptedMtnStatus)
						  ((inputBuffer[ROBOT_STATUS] & SCRIPTED_STATUS_MASK) >> SCRIPTED_STATUS_SHIFT);
	receivedMsgStatus 	= (ReceivedMsgStatus)
						  ((inputBuffer[ROBOT_STATUS] & RCVD_MSG_STATUS_MASK) >> RCVD_MSG_STATUS_SHIFT);
	lastMsgRcvdSeq = inputBuffer[LAST_RCVD_CMD_SEQ];
	limXpos = inputBuffer[LIMIT_SWITCHES] & LS_X_POS_MASK;
	limXneg = inputBuffer[LIMIT_SWITCHES] & LS_X_NEG_MASK;
	limYpos = inputBuffer[LIMIT_SWITCHES] & LS_Y_POS_MASK;
	limYneg = inputBuffer[LIMIT_SWITCHES] & LS_Y_NEG_MASK;
	limZpos = inputBuffer[LIMIT_SWITCHES] & LS_Z_POS_MASK;
	limZneg = inputBuffer[LIMIT_SWITCHES] & LS_Z_NEG_MASK;
}
