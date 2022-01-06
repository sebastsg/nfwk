#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include "network/network.hpp"

#include <unordered_set>
#include <mutex>

namespace nfwk {

class winsock_socket;

enum class iocp_operation { send, receive, accept, close, invalid };

template<iocp_operation Operation>
class iocp_data {
public:
	
	WSAOVERLAPPED overlapped{};
	const iocp_operation operation{ Operation };
	DWORD transferred{ 0 }; // this is set in port i/o thread just before calling on_send() etc
	
};

class iocp_send_data : public iocp_data<iocp_operation::send> {
public:
	
	WSABUF buffer{ 0, nullptr };

};

class iocp_receive_data : public iocp_data<iocp_operation::receive> {
public:
	
	static constexpr std::size_t buffer_size{ 262144 }; // 256 KiB

	char data[buffer_size]{};
	WSABUF buffer{ buffer_size, data };

};

class iocp_accept_data : public iocp_data<iocp_operation::accept> {
public:
	
	// the buffer size for the local and remote address must be 16 bytes more than the
	// size of the sockaddr structure because the addresses are written in an internal format.
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms737524(v=vs.85).aspx
	static constexpr std::size_t padded_address_size{ sizeof(SOCKADDR_IN) + 16 };
	static constexpr std::size_t buffer_size{ padded_address_size * 2 };

	char data[buffer_size]{};
	WSABUF buffer{ buffer_size, data };
	winsock_socket* accepted_socket{ nullptr };

};

class iocp_close_data : public iocp_data<iocp_operation::close> {};

class winsock_socket : public network_socket {
public:
	
	friend class winsock_connection_manager;

	winsock_socket(connection_manager& manager, std::size_t managed_index, HANDLE completion_port_handle);
	winsock_socket(const winsock_socket&) = delete;
	winsock_socket(winsock_socket&&) = delete;

	~winsock_socket() override;

	winsock_socket& operator=(const winsock_socket&) = delete;
	winsock_socket& operator=(winsock_socket&&) = delete;

	void synchronized_send(io_stream& stream) override;
	void synchronized_receive() override;

	bool connect_socket();

	void load_address(std::string_view address_name, int port) override;
	void bind_and_listen() override;

private:
	
	void on_send_completed(iocp_send_data& data);
	void on_receive_completed(iocp_receive_data& data);
	void on_accept_completed(iocp_accept_data& data);

	bool synchronized_accept(); // todo: should this be a virtual function?

	bool verify_socket_state(int result);

	SOCKET handle{ INVALID_SOCKET };
	WSABUF received{ 0, nullptr }; // stores received buffer until a packet is recognized
	addrinfo hints{};
	SOCKADDR_IN address{};
	int address_size{ sizeof(address) };

	// todo: sets are kinda wasteful here. should probably think of something else.
	struct {
		std::unordered_set<iocp_send_data*> send;
		std::unordered_set<iocp_receive_data*> receive;
		std::unordered_set<iocp_accept_data*> accept;
	} io;

};

}

std::ostream& operator<<(std::ostream& out, nfwk::iocp_operation operation);
