#include "RelayServer.h"
#include <iostream>

RelayServer::RelayServer()
{
	_tcpServer.AddConnectionEstablishedListener(
		[this](TcpSocket& socket)
		{
			return OnConnectionEstablished(socket);
		}
	);

	_tcpServer.AddConnectionClosedListener(
		[this](TcpSocket& socket)
		{
			return OnConnectionClosed(socket);
		}
	);

	_tcpServer.AddDataAvailableListener(
		[this](TcpSocket& socket)
		{
			return OnDataAvailable(socket);
		}
	);

	_websocketServer.clear_access_channels(websocketpp::log::alevel::all);
	_websocketServer.set_access_channels(websocketpp::log::alevel::connect);
	_websocketServer.set_access_channels(websocketpp::log::alevel::disconnect);
	_websocketServer.set_access_channels(websocketpp::log::alevel::app);
	// websocketServer.set_message_handler()...
}

int RelayServer::Run()
{
	if(!_tcpServer.Listen(9009))
	{
		return -1;
	}

	while(true)
	{
		_tcpServer.Poll(1000);
	}
}

bool RelayServer::OnConnectionEstablished(TcpSocket &socket)
{
	std::cerr << "connection established to " << socket.GetPeer() << std::endl;
	auto con = _websocketServer.get_connection();
	con->set_write_handler(
		[&socket](websocketpp::connection_hdl, char const* data, size_t size)
		{
			std::cerr << "reply" << std::endl;
			if (socket.Write(data, size, false) != static_cast<ssize_t>(size))
			{
				return websocketpp::transport::iostream::error::make_error_code(
					websocketpp::transport::iostream::error::general
				);
			}
			return websocketpp::lib::error_code();
		}
	);

	con->set_shutdown_handler(
		[&socket](websocketpp::connection_hdl)
		{
			socket.Close();
			return websocketpp::lib::error_code();
		}
	);

	con->start();
	_websocketConnections[socket.GetFileDescriptor()] = con;
	return true;
}

bool RelayServer::OnConnectionClosed(TcpSocket &socket)
{
	std::cerr << "connection to " << socket.GetPeer() << " closed." << std::endl;

	auto it = _websocketConnections.find(socket.GetFileDescriptor());
	if (it == _websocketConnections.end())
	{
		return false;
	}

	it->second->eof();
	_websocketConnections.erase(socket.GetFileDescriptor());
	return true;
}

bool RelayServer::OnDataAvailable(TcpSocket &socket)
{
	auto it = _websocketConnections.find(socket.GetFileDescriptor());
	if (it == _websocketConnections.end())
	{
		return false;
	}

	char data[1024];
	ssize_t count = socket.Read(data, sizeof(data));
	if (count > 0)
	{
		it->second->read_some(data, static_cast<size_t>(count));
	}
	return true;
}
