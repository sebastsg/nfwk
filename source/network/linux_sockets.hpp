#pragma once

#include "network/network.hpp"
#include "io.hpp"

#define ENABLE_LINUX_SOCKETS 0

#if ENABLE_LINUX_SOCKETS

#include "netinet/in.h"

namespace nfwk {

class posix_network_socket : public network_socket {
public:

	static constexpr std::size_t max_receive_buffer_size{ 16384 };

	friend class posix_connection_manager;

	posix_network_socket(connection_manager& manager, std::size_t managed_index, int handle);
	posix_network_socket(const posix_network_socket&) = delete;
	posix_network_socket(posix_network_socket&&) = delete;

	~posix_network_socket() override;

	posix_network_socket& operator=(const posix_network_socket&) = delete;
	posix_network_socket& operator=(posix_network_socket&&) = delete;
	
	void synchronized_send(io_stream& stream) override;
	void synchronized_receive() override;

	void load_address(std::string_view address_name, int port) override;
	void bind_and_listen() override;

private:
	
	std::optional<int> try_accept_client();
	void allow_listener_socket_to_reuse_address();

	int handle{ -1 };

	// this buffer is reset on every read call, so it's always accessed from 0 to the return value
	char* receive_buffer{ nullptr };

	// for listener
	sockaddr_in address{};

};

class posix_connection_manager : public connection_manager {
public:

	network_socket* create_socket() override;
	
};


}

#endif
