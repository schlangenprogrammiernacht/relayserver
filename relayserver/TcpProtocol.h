#pragma once
#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <deque>
#include <vector>

class TcpProtocol
{
	public:
		typedef std::function<void(std::vector<uint8_t> data)> MessageReceivedCallback;
		void SetMessageReceivedCallback(MessageReceivedCallback callback);
		bool Read(int socket);

	private:
		size_t _awaitedSize = 0;
		std::deque<uint8_t> _buf;
		MessageReceivedCallback _messageReceivedCallback;
};
