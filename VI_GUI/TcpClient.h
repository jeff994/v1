#pragma once
#include <boost/asio.hpp>
#include <string>
using boost::asio::ip::tcp;
class TcpClient
{
private:
	boost::asio::io_service m_ioService;
	tcp::socket  m_socketIO;
	tcp::resolver m_socketResolver;
	bool m_connEstablished;
	std::string m_tcpAddress;
	std::string m_tcpPort;
public:
	TcpClient(std::string address, std::string port);
	TcpClient();
	void SetPort(std::string port) {m_tcpPort = port;}
	void SetAddress(std::string address){  m_tcpAddress = address; }
	bool Connect();
	bool SendString(std::string strMessage); 
	bool ReceiveString(std::string &strFeedback);
	~TcpClient();
};

