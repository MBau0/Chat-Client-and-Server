#include "Client.h"

#include <iostream>
#include <cassert>

#include "Packet.h"

#include "Fmtout.h"

#include "TCPChatClient.h"

#define DEFAULT_PORT "23001"
#define DEFAULT_HOST "192.168.1.4"

Client client;

Client::Client() :
	_id					( -1 ),
	_started			( false ),
	_connect_socket		( INVALID_SOCKET )
{
	load_client_commands();
}

Client::~Client() {
	closesocket(_connect_socket);

	if (_started) {
		WSACleanup();
		if (_recieve_thread.joinable()) {
			//_recieve_thread.detach();
			_recieve_thread.join();
		}
	}
}

int Client::c_startup() {
	if(_started) {
		return 0;
	}

	int r_startup = WSAStartup(MAKEWORD(2, 2), &_wsa_data);
	std::cout << "WsaStartup -- " << r_startup << '\n';
	_started = true;

	addrinfo* result, * ptr, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int r_addrinfo = getaddrinfo(DEFAULT_HOST, DEFAULT_PORT, &hints, &result);
	std::cout << "getaddrinfo -- " << r_addrinfo << '\n';

	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
		_connect_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (_connect_socket == INVALID_SOCKET) {
			std::cout << "Socket Error: " << WSAGetLastError() << '\n';
			continue;
		}

		break;
	}

	assert(ptr);

	int r_connect = connect(_connect_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (r_connect == SOCKET_ERROR) {
		std::cout << "Connect Error : " << WSAGetLastError() << '\n';
		return -1;
	}
	else {
		_connected = true;
		_recieve_thread = std::thread(&Client::c_recieve, this);

		return 0;
	}
}

bool Client::c_send(const char* data, int* len) {
	assert(*len <= 1024);

	int total = 0;
	int bytes_left = *len;
	while (total < bytes_left) {
		int r_send = send(_connect_socket, data + total, bytes_left, 0);
		dbgout("Sending...", r_send);
		if (r_send == SOCKET_ERROR) {
			dbgout("Send Error --- ", WSAGetLastError());
			return false;
		}

		total += r_send;
		bytes_left -= r_send;
	}

	*len = total;

	return true;
}

void Client::c_recieve() {
	char recvbuf[2048];
	char* ptr;
	int r_recv = -1;
	int recvbuflen = 1024;
	int bytes_handled = 0;
	int remaining_bytes = 0;
	int packet_length = 0;

	do {
		bytes_handled = 0;
		r_recv = recv(_connect_socket, recvbuf + remaining_bytes, recvbuflen, 0);
		if (r_recv > 0) {
			dbgout("Receiving...", r_recv);
			ptr = recvbuf;
			remaining_bytes = r_recv + remaining_bytes;

			if (remaining_bytes >= 4) {
				memcpy(&packet_length, ptr, sizeof(int));

				while (remaining_bytes >= packet_length) {
					dbgout("Command --- ", ptr + 4); // command

					const char* key = ptr + 4; // skip int header
					int command_length = strlen(ptr + 4) + 1;
					if (_client_commands.find(key) == _client_commands.end()) {
						dbgout("Unknown Client Command --- ", key);
					}
					else {
						(this->*_client_commands.at(ptr + 4))(static_cast<void*>(ptr + command_length + 4), packet_length - command_length - 4);
					}

					bytes_handled += packet_length;
					remaining_bytes -= packet_length;

					if (remaining_bytes >= 4) {
						ptr = recvbuf + bytes_handled;
						memcpy(&packet_length, ptr, sizeof(int));
					}
				}

			}

			if (remaining_bytes > 0) {
				memcpy(recvbuf, ptr, remaining_bytes);
			}
		}
		else if (r_recv == 0) {
			fmtout("Close Connection");
			_connected = false;
			_exit_flag = 0;
		}
		else {
			fmtout("Recv Error --- ", WSAGetLastError());
			_connected = false;
			_exit_flag = -1;
			return;
		}
	} while (r_recv > 0);

}

bool Client::c_connected() {
	return _connect_socket == INVALID_SOCKET;
}

void Client::set_client_window(TCPChatClient* client_window) {
	_client_window = client_window;
}

void Client::load_client_commands() {
	_client_commands.emplace("set_id", &Client::set_id);
	_client_commands.emplace("new_message", &Client::new_message);
}

void Client::set_id(void* buf, int size) {
	assert(size == sizeof(int));

	memcpy(&_id, buf, sizeof(int));
}

// Params: const char* name, const char* message
void Client::new_message(void* buf, int size) {
	const char* name = static_cast<const char*>(buf);
	const char* message = static_cast<const char*>(buf) + strlen(name) + 1;

	if(strcmp(name, "") == 0) {
		name = "Unknown";
	}

	if(_client_window) {
		_client_window->display_new_message(name, message);
	}
}

int Client::get_id() const {
	return _id;
}

bool Client::get_connected() const {
	return _connected;
}

int Client::get_exit_flag() const {
	return _exit_flag;
}

/********************************************************************************************************************************************************/