#include "RelayServer.h"
#include <iostream>
#include <TcpServer/EPoll.h>

RelayServer::RelayServer()
{
}

int RelayServer::Run()
{
	uWS::Hub h;
	EPoll epoll;

	_clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(9010);
	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
	if (connect(_clientSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr))<0)
	{
		perror("connect to server failed");
		return -1;
	}

	_tcpProtocol.SetFrameCompleteCallback(
		[this, &h](uint64_t frame_id)
		{
			h.getDefaultGroup<uWS::SERVER>().forEach(
				[this, frame_id](uWS::WebSocket<uWS::SERVER>* sock)
				{
					auto con = static_cast<WebsocketConnection*>(sock->getUserData());
					con->FrameComplete(frame_id, _tcpProtocol);
				}
			);
			std::cout << "frame " << frame_id << " complete." << std::endl;
		}
	);
	epoll.AddFileDescriptor(_clientSocket, EPOLLIN|EPOLLPRI|EPOLLERR);

	h.onConnection(
		[](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req)
		{
			ws->setUserData(new WebsocketConnection(ws));
		}
	);
	h.onDisconnection(
		[](uWS::WebSocket<uWS::SERVER> *ws, int code, const char *message, size_t length)
		{
			auto *con = static_cast<WebsocketConnection*>(ws->getUserData());
			delete con;
		}
	);

	h.onMessage([](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode)
	{
		//ws->send(message, length, opCode);
	});

	std::string response = "Hello!";
	h.onHttpRequest([&](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t length, size_t remainingBytes)
	{
		res->end(response.data(), response.length());
	});

	if (!h.listen(9009))
	{
		return -1;
	}

	epoll.AddFileDescriptor(h.getLoop()->getEpollFd(), EPOLLIN|EPOLLPRI|EPOLLERR|EPOLLRDHUP|EPOLLHUP); // TODO check which events are neccessary
	while (true)
	{
		epoll.Poll(1000,
			[this, &h](const epoll_event& ev)
			{
				if (ev.data.fd == _clientSocket)
				{
					return _tcpProtocol.Read(_clientSocket);
				}
				else
				{
					h.poll();
				}
				return true;
			}
		);
	}
}
