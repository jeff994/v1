//
// Created by Fanzhe on 5/28/2017.
// Modified by Isro in April 2018
//
#pragma once

#include <string.h>
#include <iostream>

# include <winsock2.h>

//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <unistd.h>
using namespace std;


#define MAX_MSG_LENGTH 260

//Function Code
enum{
    READ_COILS       = 0x01,
    READ_INPUT_BITS  = 0x02,
    READ_REGS        = 0x03,
    READ_INPUT_REGS  = 0x04,
    WRITE_COIL       = 0x05,
    WRITE_REG        = 0x06,
    WRITE_COILS      = 0x0F,
    WRITE_REGS       = 0x10,
};


//Exception Codes
enum {
    EX_ILLEGAL_FUNCTION = 0x01, // Function Code not Supported
    EX_ILLEGAL_ADDRESS  = 0x02, // Output Address not exists
    EX_ILLEGAL_VALUE    = 0x03, // Output Value not in Range
    EX_SERVER_FAILURE   = 0x04, // Slave Deive Fails to process request
    EX_ACKNOWLEDGE      = 0x05, // Service Need Long Time to Execute
    EX_SERVER_BUSY      = 0x06, // Server Was Unable to Accept MB Request PDU
    EX_GATEWAY_PROBLEMP = 0x0A, // Gateway Path not Available
    EX_GATEWYA_PROBLEMF = 0x0B, // Target Device Failed to Response
};


class modbus {

private:
    bool _connected;
    uint16_t PORT;
	SOCKET _socket;
    int _msg_id;
    int _slaveid ;
    string HOST;
    struct sockaddr_in _server;

    void modbus_build_request(char *to_send,int address, int func);

    void modbus_read(int address, int amount, int func);
    void modbus_write(int address, int amount, int func, uint16_t *value);

    size_t modbus_send(const char *to_send, int length);
    size_t modbus_receive(char *buffer);

    int8_t modbus_error_handle(char *msg, int func);


public:
    modbus(string host, uint16_t port);
    modbus(string host);
    modbus(void);
    ~modbus();
    void set_ipaddress_port(string host, uint16_t port);

    bool modbus_connect();
    void modbus_close();

    void modbus_set_slave_id(int id);

    int8_t modbus_read_coils(int address, int amount, bool* buffer);
    int8_t modbus_read_input_bits(int address, int amount, bool* buffer);
    int8_t modbus_read_holding_registers(int address, int amount, uint16_t *buffer);
    int8_t modbus_read_input_registers(int address, int amount, uint16_t *buffer);

    int8_t modbus_write_coil(int address, bool to_write);
    int8_t modbus_write_register(int address, uint16_t value);
    int8_t modbus_write_coils(int address, int amount, bool* value );
    int8_t modbus_write_registers(int address, int amount, uint16_t *value);
};



