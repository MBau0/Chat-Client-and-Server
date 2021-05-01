# Chat Application
Chat Application for Windows written in C/C++ using Winsock (windows networking library) and Qt (cross-platform gui).

# Table of Contents

[Libraries](#libraries)  
[How it works](#how-it-works)
  - [Server](#server)
  - [Client](#client)
  - [Packet](#packet)
  - [Commands](#commands)

 
[Screenshots](#screenshots)  
[Status](#status)  

## Libraries
[Qt VS Tools 5.1](https://doc.qt.io/qtvstools/index.html)  
[Winsock](https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-start-page-2)

## How it works

### Server
  -Accepts and manages connections from clients  
  -Sends packets to all connected clients  
  -Contains commands to execute from clients  
  -Each connected client recieves its own thread, where the server listens to the client socket and recieves data from  
  
  Command examples:  
  "new_message" -- recieves a message from a client and sends it out all connected clients  
  "set_name"    -- recieves a message from a client and sets the client's name

### Client
  -Connects the chat application to the server  
  -Sends packets to the server  
  -Contains commands to execute from the server
  
  Command examples:  
  "set_id"      -- sets the client id to match the id created by the server  
  "new_message" -- recieves a message that was sent to the server by a client
  
### Packet
  -Constructs a c-string from multiple variables to send over socket connections

Example Constructor: 

```C++ 
  template<typename ... Args>
  PacketData(Args ... args); 
``` 
  -Constructs a binary array by casting the arguments as u_int8_t
  
An expected packet is has the following argument format:  
  -``` const char* command ``` (Commands that can interpreted by Client / Server)  
  -``` Args...             ``` (Arguments passed to command function)
  
Example: 
```C++ 
  PacketData("new_message", client_id, "Hello");
```

### Commands
  -Executes a function based on the command name of a packet    
  -Function pointers   
  -Client and Server have their own commands that they cause the other to execute  
  -Defined as ```void function(void* buf, int size) ```
  
  Example:
  
 ```C++
 
// Server Side:
    PacketData packet("new_message", "Jim", "Hello");
    int len = packet.len();
    s_send(packetData.c_str(), &len, _clients.at(0));
    
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

```

## Screenshots
![](https://github.com/willardt/Chat-Client-and-Server/blob/main/ss/ss.png?raw=true "") 
![](https://github.com/willardt/Chat-Client-and-Server/blob/main/ss/ss2.png?raw=true "")  
![](https://github.com/willardt/Chat-Client-and-Server/blob/main/ss/ss3.png?raw=true "")  

## Status

The main purpose of this project was to learn and experiement using winsock to create a client and server for networking.
I dont have any plans to continue developing the chat application part of this project, but I may redesign the Server / Client for future projects.


