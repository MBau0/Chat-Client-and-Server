#ifndef CLIENT_H
#define CLIENT_H

#include <WinSock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#include <thread>
#include <unordered_map>
#include <string>
#include <string_view>

class Client;
class Entity;
class TCPChatClient;

typedef void(Client::*ClientCommand)(void* buf, int size);

class Client {
public:
	Client();
	~Client();

	int c_startup();
	bool c_send(const char* data, int* len);
	void c_recieve();
	bool c_connected();
	void c_read(uint8_t* data);

	void load_client_commands();

	void set_id(void* buf, int size);
	void new_message(void* buf, int size);

	int get_id() const;
	bool get_connected() const;
	int get_exit_flag() const;

	void set_client_window(TCPChatClient* client_window);
private:
	int _id;
	bool _started;
	bool _connected;
	int _exit_flag;

	WSAData _wsa_data;
	SOCKET _connect_socket;

	std::thread _recieve_thread;

	std::unordered_map<std::string, ClientCommand> _client_commands;

	TCPChatClient* _client_window;
};

/********************************************************************************************************************************************************/

#endif