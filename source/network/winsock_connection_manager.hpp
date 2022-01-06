#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include "network/network.hpp"

#include <thread>

namespace nfwk {

class winsock_socket;
class iocp_accept_data;

class winsock_connection_manager : public connection_manager {
public:

	winsock_connection_manager();
	winsock_connection_manager(const winsock_connection_manager&) = delete;
	winsock_connection_manager(winsock_connection_manager&&) = delete;

	~winsock_connection_manager() override;

	winsock_connection_manager& operator=(const winsock_connection_manager&) = delete;
	winsock_connection_manager& operator=(winsock_connection_manager&&) = delete;

	network_socket* create_socket() override;
	
	void io_port_thread();

	bool accept_ex(winsock_socket& socket);
	void get_accept_sockaddrs(iocp_accept_data& data);
	void load_extensions(SOCKET handle);

private:

	std::vector<std::thread> threads;

	LPFN_ACCEPTEX AcceptEx{ nullptr };
	LPFN_GETACCEPTEXSOCKADDRS GetAcceptExSockaddrs{ nullptr };

	WSADATA wsa_data{};
	HANDLE io_port{ INVALID_HANDLE_VALUE };

};

}
