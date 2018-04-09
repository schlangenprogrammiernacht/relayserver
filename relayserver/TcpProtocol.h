#pragma once
#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <deque>
#include <vector>

class TcpProtocol
{
	public:
		typedef std::function<void(std::vector<char>&)> MessageReceivedCallback;
		void SetMessageReceivedCallback(MessageReceivedCallback callback);
		bool Read(int socket);

	private:
		size_t _awaitedSize = 0;
		std::deque<char> _buf;
		MessageReceivedCallback _messageReceivedCallback;

		void OnMessageReceived(std::vector<char>& data);
};
