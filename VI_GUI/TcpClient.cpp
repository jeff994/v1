#include "stdafx.h"
#include "TcpClient.h"
#include <boost/log/trivial.hpp>


TcpClient::TcpClient():
	m_socketIO(m_ioService),
	m_socketResolver(m_ioService)

{	
	m_connEstablished = false;
}

TcpClient::TcpClient(std::string address, std::string port) :
	m_socketIO(m_ioService),
	m_socketResolver(m_ioService)

{
	SetPort(port);
	SetAddress(address);
	m_connEstablished = false;
}

bool TcpClient::Connect()
{
	try {
		boost::asio::connect(m_socketIO, m_socketResolver.resolve({ m_tcpAddress, m_tcpPort }));
		m_connEstablished = true;
		BOOST_LOG_TRIVIAL(info) << " Connected to Nport" << std::endl;
	}
	catch (const boost::system::system_error&) {
		BOOST_LOG_TRIVIAL(error) << "TCP Cannot Connect to " << m_tcpAddress << ":" << m_tcpPort << std::endl;
		m_connEstablished = false;
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	return m_connEstablished;
}

bool TcpClient::ReceiveString(std::string &strFeedback)
{
	bool bRet = true; 
	if (!m_connEstablished) return false; 
	try {
		boost::system::error_code error;
		boost::asio::streambuf receive_buffer;
		const std::string delimiter = "\r";
		boost::asio::read_until(m_socketIO, receive_buffer, delimiter, error);
		if (error && error != boost::asio::error::eof) {
			bRet = false; 
			BOOST_LOG_TRIVIAL(error) << "Failed to receive" << std::endl;
		}
		else
		{
			const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
			strFeedback.append(data);
		}
	}
	catch (const boost::system::system_error&) {
		
	}
	return bRet; 
}

bool TcpClient::SendString(std::string msg2Send)
{
	bool bRet = true; 
	if (!m_connEstablished) return false; 
	
	try {
		boost::system::error_code error;
		boost::asio::write(m_socketIO, boost::asio::buffer(msg2Send.c_str(), msg2Send.size()), error);
		if (error) {
			BOOST_LOG_TRIVIAL(error) << "Failed to write"<< std::endl;
			bRet = false; 
		}
	}
	catch (const boost::system::system_error&) {
		BOOST_LOG_TRIVIAL(error) << "TCP is connected to " << m_tcpAddress.c_str() << ":" << m_tcpPort.c_str() << std::endl;
		m_connEstablished = false; 
		bRet = false; 
	}
	
	return bRet ; 
}

TcpClient::~TcpClient()
{
}
