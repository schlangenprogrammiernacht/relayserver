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

	const char* gameserverHost = getEnvOrDefault(ENV_GAMESERVER_HOST, ENV_GAMESERVER_HOST_DEFAULT);
	const char* gameserverPort = getEnvOrDefault(ENV_GAMESERVER_PORT, ENV_GAMESERVER_PORT_DEFAULT);
	const char* websocketPort = getEnvOrDefault(ENV_WEBSOCKET_PORT, ENV_WEBSOCKET_PORT_DEFAULT);

	fprintf(stderr, "connecting to gameserver on %s port %s...\n", gameserverHost , gameserverPort);
	_clientSocket = connectTcpSocket(gameserverHost , gameserverPort);
	if (_clientSocket < 0)
	{
		perror("connect to server failed");
		return -1;
	}
	fprintf(stderr, "connected.\n");

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
			//std::cout << "frame " << frame_id << " complete." << std::endl;
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

	if (!h.listen(atoi(websocketPort)))
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

int RelayServer::connectTcpSocket(const char *hostname, const char *port)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Datagram socket */

	struct addrinfo *result;
	int s = getaddrinfo(hostname, port, &hints, &result);
	if (s != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return s;
	}

	struct addrinfo* rp = nullptr;
	int retval = -1;
	for (rp=result; rp!=nullptr; rp=rp->ai_next)
	{
		int fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (fd == -1) { continue; }

		if (connect(fd, rp->ai_addr, rp->ai_addrlen) == 0)
		{
			retval = fd;
			break;
		}
		close(fd);
	}

	freeaddrinfo(result);
	return retval;
}

const char *RelayServer::getEnvOrDefault(const char *envVar, const char *defaultValue)
{
	const char* value = getenv(envVar);
	if (value == nullptr)
	{
		value = defaultValue;
	}
	return value;
}
