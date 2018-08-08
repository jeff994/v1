//
// Created by Fanzhe on 5/28/2017..
// Modified by Isro in April 2018
//

#include "modbus.h"
#include <vector>
#include <winsock.h>

using namespace std;
#pragma comment(lib, "Ws2_32.lib")

/**
 * Main Constructor of Modbus Object
 * @param host IP Address of Host
 * @param port Port for Connection
 * @return     A Modbus Object
 */
modbus::modbus(string host, uint16_t port) {
    HOST = host;
    PORT = port;
    _slaveid = 1;
    _msg_id = 1;
    _connected = false;

}


/**
 * Overloading Modbus Constructor with Default Port at 502
 * @param host  IP Address of Host
 * @return      A Modbus Object
 */
modbus::modbus(string host) {
    modbus(host, 502);
}


/**
 * Destructor of Modbus Object
 */
modbus::~modbus(void) {
    //delete(&HOST);
    //delete(&PORT);
    //delete(&_socket);
    //delete(&_server);
}

modbus::modbus(void){
    _slaveid = 1;
    _msg_id = 1;
    _connected = false;
}

void modbus::set_ipaddress_port(string host, uint16_t port){
    HOST = host;
    PORT = port;
}

/**
 * Set Slave ID
 * @param id  Id of Slave in Server to Set
 */
void modbus::modbus_set_slave_id(int id) {
    _slaveid = id;
}



/**
 * Build up Connection
 * @return   If A Connection Is Successfully Built
 */
bool modbus::modbus_connect() {
    if(HOST == "" || PORT == 0) {
        cout << "Missing Host and Port" << endl;
        return false;
    } else {
        cout << "Found Proper Host " <<  HOST.c_str() << " and Port " <<PORT <<endl;
    }
	static WSADATA wd;
	if (WSAStartup(0x0101, &wd))
	{
		printf("cannot initialize WinSock\n");
		return 1;
	}
    _socket = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server;

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(HOST.c_str());
	server.sin_port = htons(PORT);
	int i = connect(_socket, (struct sockaddr*)&server, sizeof(sockaddr_in));
    if ( i < 0) {
        cout<< "Connection Error" << endl;
        return false;
    }

    cout<< "Connected" <<endl;
    _connected = true;
    return true;
}


/**
 * Close the Connection
 */
void modbus::modbus_close() {
    closesocket(_socket);
    cout <<"Socket Closed" <<endl;
}


/**
 * Modbus Request Builder
 * @param to_send   Message Buffer to Send
 * @param address   Reference Address
 * @param func      Functional Code
 */
void modbus::modbus_build_request(char *to_send, int address, int func) {
    to_send[0] = (char) _msg_id >> 8;
    to_send[1] = (char) (_msg_id & 0x00FF);
    to_send[2] = 0;
    to_send[3] = 0;
    to_send[4] = 0;
    to_send[6] = (char) _slaveid;
    to_send[7] = (char) func;
    to_send[8] = (char) (address >> 8);
    to_send[9] = (char) (address & 0x00FF);
}


/**
 * Write Request Builder and Sender
 * @param address   Reference Address
 * @param amount    Amount to Write
 * @param func      Functional Code
 * @param value     Value to Write
 */
void modbus::modbus_write(int address, int amount, int func, uint16_t *value) {
    if(func == WRITE_COIL || func == WRITE_REG) {
        char to_send[12];
        modbus_build_request(to_send, address, func);
        to_send[5] = 6;
        to_send[10] = (char) (value[0] >> 8);
        to_send[11] = (char) (value[0] & 0x00FF);
        modbus_send(to_send, 12);
    } else if(func == WRITE_REGS){
 
		std::vector<char> to_send(13 + 2 * amount);
        modbus_build_request(to_send.data(), address, func);
        to_send[5] = (char) (5 + 2 * amount);
        to_send[10] = (char) (amount >> 8);
        to_send[11] = (char) (amount & 0x00FF);
        to_send[12] = (char) (2 * amount);
        for(int i = 0; i < amount; i++) {
            to_send[13 + 2 * i] = (char) (value[i] >> 8);
            to_send[14 + 2 * i] = (char) (value[i] & 0x00FF);
        }
        modbus_send(to_send.data(), 13 + 2 * amount);
    } else if(func == WRITE_COILS) {
		std::vector<char> to_send(14 + (amount -1) / 8 );
        modbus_build_request(to_send.data(), address, func);
        to_send[5] = (char) (7 + (amount -1) / 8);
        to_send[10] = (char) (amount >> 8);
        to_send[11] = (char) (amount >> 8);
        to_send[12] = (char) ((amount + 7) / 8);
        for(int i = 0; i < amount; i++) {
            to_send[13 + (i - 1) / 8] += (char) (value[i] << (i % 8));
        }
        modbus_send(to_send.data(), 14 + (amount - 1) / 8);
    }
}


/**
 * Read Requeset Builder and Sender
 * @param address   Reference Address
 * @param amount    Amount to Read
 * @param func      Functional Code
 */
void modbus::modbus_read(int address, int amount, int func){
    char to_send[12];
    modbus_build_request(to_send, address, func);
    to_send[5] = 6;
    to_send[10] = (char) (amount >> 8);
    to_send[11] = (char) (amount & 0x00FF);
    modbus_send(to_send, 12);
}


/**
 * Read Holding Registers           MODBUS FUNCTION 0x03
 * @param address    Reference Address
 * @param amount     Amount of Registers to Read
 * @param buffer     Buffer to Store Data
 */
int8_t modbus::modbus_read_holding_registers(int address, int amount, uint16_t *buffer) {
    if(_connected) {
        if(amount > 65535 || address > 65535) {
            cout << "An error in modbus happened: "
                 << "Too many Data!" << endl;
            return -1;
        }
        modbus_read(address, amount, READ_REGS);
        char to_rec[MAX_MSG_LENGTH];
        size_t k = modbus_receive(to_rec);
        if(modbus_error_handle(to_rec, READ_REGS))
            return -1;
        if(k < (2*amount))
            return -1;
        for(int i = 0; i < amount; i++) {
            buffer[i] = ((uint16_t)to_rec[9 + 2 * i]) << 8;
            buffer[i] += (uint16_t) to_rec[10 + 2 * i];
        }
    } else {
        cout << "An error in modbus happened: "
             << "Having Modbus Connection Problem" << endl;
        return -1;
    }
    return 0;
}


/**
 * Read Input Registers             MODBUS FUNCTION 0x04
 * @param address     Reference Address
 * @param amount      Amount of Registers to Read
 * @param buffer      Buffer to Store Data
 */
int8_t modbus::modbus_read_input_registers(int address, int amount, uint16_t *buffer) {
    if(_connected){
        if(amount > 65535 || address > 65535) {
            cout << "An error in modbus happened: "
                 << "Too many Data!" << endl;
            return -1;
        }
        modbus_read(address, amount, READ_INPUT_REGS);
        char to_rec[MAX_MSG_LENGTH];
        size_t k = modbus_receive(to_rec);
        if(modbus_error_handle(to_rec, READ_INPUT_REGS))
            return -1;
        if(k < (2*amount))
            return -1;
        for(int i = 0; i < amount; i++) {
            buffer[i] = ((uint16_t)to_rec[9 + 2 * i]) << 8;
            buffer[i] += (uint16_t) to_rec[10 + 2 * i];
        }
    } else {
        cout << "An error in modbus happened: "
             << "Having Modbus Connection Problem" << endl;
        return -1;
    }
    return 0;
}


/**
 * Read Coils           MODBUS FUNCTION 0x01
 * @param address     Reference Address
 * @param amount      Amount of Coils to Read
 * @param buffer      Buffers to Store Data
 */
int8_t modbus::modbus_read_coils(int address, int amount, bool *buffer) {
    if(_connected) {
        if(amount > 2040 || address > 65535) {
            cout << "An error in modbus happened: "
                 << "Too many Data!" << endl;
            return -1;
        }
        modbus_read(address, amount, READ_COILS);
        char to_rec[MAX_MSG_LENGTH];
        size_t k = modbus_receive(to_rec);
        if(modbus_error_handle(to_rec, READ_COILS))
            return -1;
        if(k < (2*amount))
            return -1;
        for(int i = 0; i < amount; i++) {
            buffer[i] = (bool) ((to_rec[9 + i / 8] >> (i % 8)) & 1);
        }
    } else {
        cout << "An error in modbus happened: "
             << "Having Modbus Connection Problem" << endl;
        return -1;
    }
    return 0;
}


/**
 * Read Input Bits(Discrete Data)      MODBUS FUNCITON 0x02
 * @param address   Reference Address
 * @param amount    Amount of Bits to Read
 * @param buffer    Buffer to store Data
 */
int8_t modbus::modbus_read_input_bits(int address, int amount, bool* buffer) {
    if(_connected) {
        if(amount > 2040 || address > 65535) {
            cout << "An error in modbus happened: "
                 << "Too many Data!" << endl;
            return -1;
        }
        modbus_read(address, amount, READ_INPUT_BITS);
        char to_rec[MAX_MSG_LENGTH];
        size_t k = modbus_receive(to_rec);
        if(modbus_error_handle(to_rec, READ_INPUT_BITS))
            return -1;
        if(k < (2*amount))
            return -1;
        for(int i = 0; i < amount; i++) {
            buffer[i] = (bool) ((to_rec[9 + i / 8] >> (i % 8)) & 1);
        }
    } else {
        cout << "An error in modbus happened: "
             << "Having Modbus Connection Problem" << endl;
        return -1;
    }
    return 0;
}


/**
 * Write Single Coils         MODBUS FUNCTION 0x05
 * @param address    Reference Address
 * @param to_write   Value to Write to Coil
 */
int8_t modbus::modbus_write_coil(int address, bool to_write) {
    if(_connected) {
        if(address > 65535) {
            cout << "An error in modbus happened: "
                 << "Too many Data!" << endl;
            return -1;
        }
        int value = to_write * 0xFF00;
        modbus_write(address, 1, WRITE_COIL, (uint16_t *)&value);
        char to_rec[MAX_MSG_LENGTH];
        modbus_receive(to_rec);
        if(modbus_error_handle(to_rec, WRITE_COIL))
            return -1;
    } else {
        cout << "An error in modbus happened: "
             << "Having Modbus Connection Problem" << endl;
        return -1;
    }
    return 0;
}


/**
 * Write Single Register        FUCTION 0x06
 * @param address   Reference Address
 * @param value     Value to Write to Register
 */
int8_t modbus::modbus_write_register(int address, uint16_t value) {
    if(_connected) {
        if(address > 65535) {
            cout << "An error in modbus happened: "
                 << "Too many Data!" << endl;
            return -1;
        }
        modbus_write(address, 1, WRITE_REG, &value);
        char to_rec[MAX_MSG_LENGTH];
        modbus_receive(to_rec);
        if(modbus_error_handle(to_rec, WRITE_COIL))
            return -1;
    } else {
        cout << "An error in modbus happened: "
             << "Having Modbus Connection Problem" << endl;
        return -1;
    }
    return 0;
}


/**
 * Write Multiple Coils        MODBUS FUNCTION 0x0F
 * @param address  Reference Address
 * @param amount   Amount of Coils to Write
 * @param value    Values to Write
 */
int8_t modbus::modbus_write_coils(int address, int amount, bool *value) {
    if(_connected) {
        if(address > 65535 || amount > 65535) {
            cout << "An error in modbus happened: "
                 << "Too many Data!" << endl;
            return -1;
        }

		std::vector<uint16_t> temp(amount);
        for(int i = 0; i < 4; i++) {
            temp[i] = (uint16_t)value[i];
        }
        modbus_write(address, amount, WRITE_COILS,  temp.data());
        char to_rec[MAX_MSG_LENGTH];
        size_t k = modbus_receive(to_rec);
        if(modbus_error_handle(to_rec, WRITE_COILS))
            return -1;
        if(k < (2*amount))
            return -1;
    } else {
        cout << "An error in modbus happened: "
             << "Having Modbus Connection Problem" << endl;
        return -1;
    }
    return 0;
}


/**
 * Write Multiple Registers    MODBUS FUNCION 0x10
 * @param address Reference Address
 * @param amount  Amount of Value to Write
 * @param value   Values to Write
 */
int8_t modbus::modbus_write_registers(int address, int amount, uint16_t *value) {
    if(_connected) {
        if(address > 65535 || amount > 65535) {
            cout << "An error in modbus happened: "
                 << "Too many Data!" << endl;
            return -1;
        }
        modbus_write(address, amount, WRITE_REGS, value);
        char to_rec[MAX_MSG_LENGTH];
        size_t k = modbus_receive(to_rec);
        if(modbus_error_handle(to_rec, WRITE_REGS))
            return -1;
        if(k < (2*amount))
            return -1;
    } else {
        cout << "An error in modbus happened: "
             << "Having Modbus Connection Problem" << endl;
        return -1;
    }
    return 0;
}


/**
 * Data Sender
 * @param to_send Requeset to Send to Server
 * @param length  Length of the Request
 * @return        Size of the request
 */
size_t modbus::modbus_send(const char *to_send, int length) {
    _msg_id = _msg_id + 1;
    return send(_socket, to_send, (size_t)length, 0);
}


/**
 * Data Receiver
 * @param buffer Buffer to Store the Data
 * @return       Size of the Incoming Data
 */
size_t modbus::modbus_receive(char *buffer) {
    return recv(_socket, (char *) buffer, 1024, 0);
}


/**
 * Error Code Handler
 * @param msg   Message Received from Server
 * @param func  Modbus Functional Code
 */
int8_t modbus::modbus_error_handle(char *msg, int func) {
    int8_t ret = 0;
    if(msg[7] == func + 0x80) {
        switch(msg[8]){
            case EX_ILLEGAL_FUNCTION:
                cout << "An error in modbus happened: "
                << "Illegal Function" << endl;
                ret = -1;
            case EX_ILLEGAL_ADDRESS:
                cout << "An error in modbus happened: "
                << "Illegal Address" << endl;
                ret = -1;
            case EX_ILLEGAL_VALUE:
                cout << "An error in modbus happened: "
                << "Illegal Data Value" << endl;
                ret = -1;
            case EX_SERVER_FAILURE:
                cout << "An error in modbus happened: "
                << "Server Failure" << endl;
                ret = -1;
            case EX_ACKNOWLEDGE:
                cout << "An error in modbus happened: "
                << "Acknowledge" << endl;
                ret = -1;
            case EX_SERVER_BUSY:
                cout << "An error in modbus happened: "
                << "Server Busy" << endl;
                ret = -1;
            case EX_GATEWAY_PROBLEMP:
            case EX_GATEWYA_PROBLEMF:
                cout << "An error in modbus happened: "
                << "Gateway Problem" << endl;
                ret = -1;
            default:
                break;
        }
    }

    return ret;
}
