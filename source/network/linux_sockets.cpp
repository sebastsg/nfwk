#include "linux_sockets.hpp"

#if ENABLE_LINUX_SOCKETS

#include "log.hpp"

#include <csignal>

#include "netinet/ip.h"
#include "netinet/tcp.h"
#include "sys/socket.h"
#include "unistd.h"
#include "fcntl.h"

namespace nfwk {

posix_network_socket::posix_network_socket(connection_manager& manager_, std::size_t managed_index_, int handle_)
	: network_socket{ manager_, managed_index_ }, handle{ handle_ } {
	if (handle == -1) {
		handle = socket(AF_INET, SOCK_STREAM, 0);
		if (handle == -1) {
			log_errno("network", "Failed to create socket.");
			return;
		}
	}
	receive_buffer = new char[max_receive_buffer_size] {};
	if (fcntl(handle, F_SETFL, O_NONBLOCK) == -1) {
		log_errno("network", "Failed to set socket to non-blocking.");
		return;
	}
	int params[]{ 1 };
	if (setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, params, sizeof(int)) == -1) {
		log_errno("network", "Failed to enable no-delay for socket.");
	}
}

posix_network_socket::~posix_network_socket() {
	info("network", "Closing connection to socket {}", handle);
	delete[] receive_buffer;
	// todo: shutdown should be called when socket is scheduled for destruction (TCP specific)
	// todo: this destructor should then be called when we have received 0 bytes (as per usual)
#if 0	
	if (shutdown(id, SHUT_RDWR) == -1) {
		log_errno("network", "Failed to shutdown socket.");
	}
#endif
	if (close(handle) == -1) {
		log_errno("network", "Failed to close socket.");
	}
}

void posix_network_socket::synchronized_send(io_stream& stream) {
	while (stream.size_left_to_read() > 0) {
		const auto count = ::send(handle, stream.at_read(), stream.size_left_to_read(), MSG_NOSIGNAL);
		if (count < 0) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				break;
			}
			log_errno("network", "Failed to write to socket {}", handle);
			sync.disconnect.emplace(socket_close_status::disconnected_gracefully);
			manager.schedule_for_closing(*this);
			break;
		}
		stream.move_read_index(count);
	}
}

void posix_network_socket::synchronized_receive() {
	if (const auto count = read(handle, receive_buffer, max_receive_buffer_size); count >= 0) {
		parse(receive_buffer, count);
	} else if (errno != EWOULDBLOCK && errno != EAGAIN) {
		log_errno("network", "Failed to read from client.");
		sync.disconnect.emplace(socket_close_status::disconnected_gracefully);
		manager.schedule_for_closing(*this);
	}
}

std::optional<int> posix_network_socket::try_accept_client() {
	std::unique_lock lock{ mutex, std::try_to_lock };
	if (!lock.owns_lock() || handle == -1) {
		return std::nullopt;
	}
	socklen_t size_of_address{ sizeof(address) };
	const auto new_socket = accept(handle, reinterpret_cast<sockaddr*>(&address), &size_of_address);
	if (new_socket != -1) {
		return new_socket;
	} else if (errno != EWOULDBLOCK && errno != EAGAIN) {
		log_errno("network", "Failed to accept client.");
		if (errno == EINVAL) {
			// todo: an event should be used here to notify the program that we can't listen to the address.
			error("network", "Invalid listener socket. Try to restart the program in a minute.");
			raise(SIGINT);
			std::exit(0);
		}
	}
	return std::nullopt;
}

void posix_network_socket::load_address(std::string_view address_name, int port) {
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	std::memset(address.sin_zero, 0, sizeof(address.sin_zero));
}

void posix_network_socket::bind_and_listen() {
	constexpr bool allow_reuse{ true };
	if (allow_reuse) {
		allow_listener_socket_to_reuse_address();
	}
	if (bind(handle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != -1) {
		if (::listen(handle, 16) == -1) {
			log_errno("network", "Failed to listen to port {}.", port);
		}
	} else {
		log_errno("network", "Failed to bind to port %cyan{}", ntohs(address.sin_port));
	}
}

void posix_network_socket::allow_listener_socket_to_reuse_address() {
	int params[]{ 1 };
	info("network", "Allowing reuse of address and port.");
	if (setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, params, sizeof(int)) == -1) {
		log_errno("network", "Failed to allow reuse of address.");
	}
	if (setsockopt(handle, SOL_SOCKET, SO_REUSEPORT, params, sizeof(int)) == -1) {
		log_errno("network", "Failed to allow reuse of port.");
	}
}

network_socket* posix_connection_manager::create_socket() {
	std::size_t index{ 0 };
	for (auto& socket : sockets) {
		if (!socket) {
			socket = std::make_unique<posix_network_socket>(*this, index, -1);
			return socket.get();
		}
		index++;
	}
	return sockets.emplace_back(std::make_unique<posix_network_socket>(*this, sockets.size(), -1)).get();
}

}

#endif
