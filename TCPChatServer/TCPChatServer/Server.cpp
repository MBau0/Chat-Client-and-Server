#include "Server.h"

#include <iostream>
#include <cassert>

#include <string>
#include <iterator>
#include <filesystem>

#include "Packet.h"

#include "Fmtout.h"


#define DEFAULT_PORT "23001"
#define MAX_CLIENTS 12

/********************************************************************************************************************************************************/

void print_error(int val) {
	if (val == SOCKET_ERROR) {
		std::cout << "Error --- " << WSAGetLastError() << '\n';
	}
}

/********************************************************************************************************************************************************/

Server::Server() :
	_listen_socket		( INVALID_SOCKET ),
	_started			( false ),
	_accept				( false )
{}

Server::~Server() {
	closesocket(_listen_socket);

	if (_started) {
		WSACleanup();
	}
}

void Server::s_startup() {
	int r_startup = WSAStartup(MAKEWORD(2, 2), &_wsa_data);
	std::cout << "WSAStartup --- " << r_startup << '\n';
	_started = true;

	addrinfo* result, * ptr, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	int r_addrinfo = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);
	std::cout << "getaddrinfo --- " << r_addrinfo << '\n';

	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
		_listen_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (_listen_socket == INVALID_SOCKET) {
			std::cout << "socket Error --- " << WSAGetLastError() << '\n';
			continue;
		}

		int r_bind = bind(_listen_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		std::cout << "bind --- " << r_bind << '\n';
		if (r_bind == SOCKET_ERROR) {
			print_error(r_bind);
			continue;
		}

		break;
	}

	assert(ptr);

	freeaddrinfo(result);

	int r_listen = listen(_listen_socket, SOMAXCONN);
	std::cout << "listen --- " << r_listen << '\n';
	print_error(r_listen);

	load_server_commands();
}

void Server::s_accept() {
	_accept = true;
	while (_accept) {
		SOCKET client_socket = INVALID_SOCKET;

		fmtout("Waiting for client...");
		client_socket = accept(_listen_socket, nullptr, nullptr);
		if (client_socket == INVALID_SOCKET) {
			fmtout("Client Accept Error --- ", WSAGetLastError);
		}
		assert((client_socket != INVALID_SOCKET));
		fmtout("New Connection");

		if (_clients.size() >= MAX_CLIENTS) {
			// reject
			continue;
		}

		auto client = std::make_shared<ServerClient>(client_socket, _clients.size());

		_m.lock();
		_clients.push_back(client);
		_m.unlock();

		client->_thread = std::thread(&Server::s_recieve, this, client);

		PacketData data("set_id", client->_id);
		int len = data.length();

		s_send(data.c_str(), &len, client->_id);
	}
}

void Server::s_decline() {
	_accept = false;
}

void Server::s_recieve(std::shared_ptr<ServerClient> client) {
	char recvbuf[2048];
	char* ptr;
	int r_recv = -1;
	int recvbuflen = 1024;
	int bytes_handled = 0;
	int remaining_bytes = 0;
	int packet_length = 0;

	do {
		bytes_handled = 0;
		r_recv = recv(client->_client_socket, recvbuf + remaining_bytes, recvbuflen, 0);
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
					if (_server_commands.find(key) == _server_commands.end()) {
						dbgout("Unknown Server Command --- ", key);
					}
					else {
						(this->*_server_commands.at(ptr + 4))(static_cast<void*>(ptr + command_length + 4), packet_length - command_length - 4);
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
		}
		else {
			fmtout("Recv Error --- ", WSAGetLastError());
		}
	} while (r_recv > 0);

	_m.lock();
	auto it = _clients.begin();
	while (it != _clients.end() && (*it)->_id != client->_id) {
		++it;
	}
	if (it != _clients.end()) {
		_clients.erase(it);
	}
	_m.unlock();
}

bool Server::s_send(const char* data, int* len, int client_id) {
	assert(*len <= 1024);

	std::shared_ptr<ServerClient> client = nullptr;
	for(const auto c : _clients) {
		if(c->_id == client_id) {
			client = c;
		}
	}
	if(!client) {
		// client doesnt exist / disconnected
		std::cout << "Client -- " << client_id << " Doesnt exist " << '\n';
		return false;
	}

	int total = 0;
	int bytes_left = *len;
	while (total < bytes_left) {
		int r_send = send(client->_client_socket, data + total, bytes_left, 0);
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

void Server::load_server_commands() {
	_server_commands.emplace("new_message", &Server::new_message);
	_server_commands.emplace("set_name", &Server::set_name);
}

// Params: int client_id, const char* message
void Server::new_message(void* buf, int size) {
	char* ptr = static_cast<char*>(buf);
	int client_id;
	memcpy(&client_id, ptr, sizeof(int));
	ptr += sizeof(int);

	const char* message = static_cast<const char*>(ptr);

	std::cout << message << '\n';

	for(auto& client : _clients) {
		PacketData s_message("new_message", _clients.at(client_id)->_name.c_str(), message);
		int len = s_message.length();
		s_send(s_message.c_str(), &len, client->_id);
	}
}

// Params: int client_id, const char* name
void Server::set_name(void* buf, int size) {
	char* ptr = static_cast<char*>(buf);
	int client_id;
	memcpy(&client_id, ptr, sizeof(int));
	ptr += sizeof(int);

	for(auto& client : _clients) {
		if(client->_id == client_id) {
			std::cout << client->_id << "\n";
			client->_name = ptr;
			return;
		}
	}
}


/********************************************************************************************************************************************************/

ServerClient::ServerClient(SOCKET client_socket, int id) :
	_id				( id ),
	_client_socket	( client_socket )
{}

ServerClient::~ServerClient() {
	closesocket(_client_socket);
	_thread.detach();
}

/********************************************************************************************************************************************************/