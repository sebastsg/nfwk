#include "winsock_socket.hpp"
#include "winsock_connection_manager.hpp"

#include "log.hpp"
#include "windows_platform.hpp"

#define WS_PRINT_ERROR(ERR)   print_winsock_error(ERR, __FUNCSIG__, __LINE__)
#define WS_PRINT_LAST_ERROR() print_winsock_error(WSAGetLastError(), __FUNCSIG__, __LINE__)

namespace nfwk {

static void print_winsock_error(int error_code, std::string_view function_signature, int line) {
	const auto message = platform::windows::get_error_message(error_code);
	error(network::log, "WSA Error {} on line {} in {}\n{}", error_code, line, function_signature, message);
}

winsock_socket::winsock_socket(connection_manager& manager_, std::size_t managed_index_, HANDLE completion_port_handle) : network_socket{ manager_, managed_index_ } {
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;
	address.sin_family = hints.ai_family;
	if (handle != INVALID_SOCKET) {
		return;
	}
	handle = WSASocketW(hints.ai_family, hints.ai_socktype, hints.ai_protocol, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (handle == INVALID_SOCKET) {
		WS_PRINT_LAST_ERROR();
		return;
	}
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(handle), completion_port_handle, static_cast<ULONG_PTR>(managed_index), 0);
}

winsock_socket::~winsock_socket() {
	if (handle == INVALID_SOCKET) {
		return;
	}
	if (const auto status = closesocket(handle); status == SOCKET_ERROR) {
		WS_PRINT_LAST_ERROR();
		return;
	}
	handle = INVALID_SOCKET;
	for (auto send : io.send) {
		delete send;
	}
	for (auto receive : io.receive) {
		delete receive;
	}
	for (auto accept : io.accept) {
		delete accept;
	}
}

bool winsock_socket::connect_socket() {
	if (const auto success = ::connect(handle, reinterpret_cast<SOCKADDR*>(&address), address_size); success != 0) {
		WS_PRINT_LAST_ERROR();
		return false;
	} else {
		connected = true;
		return true;
	}
}

void winsock_socket::load_address(std::string_view address_name, int port) {
	info("network", "Loading address %yellow{}%cyan:%yellow{}", address_name, port);
	addrinfo* result{ nullptr };
	if (const auto status = getaddrinfo(reinterpret_cast<const char*>(address_name.data()), std::to_string(port).c_str(), &hints, &result); status == 0) {
		while (result) {
			address = *reinterpret_cast<SOCKADDR_IN*>(result->ai_addr);
			hints.ai_family = result->ai_family;
			freeaddrinfo(result);
			break; // result = result->ai_next;
		}
	} else {
		warning(network::log, "Failed to get address info for {}:{}\nStatus: {}", address_name, port, status);
	}
}

void winsock_socket::bind_and_listen() {
	info("network", "Binding socket");
	if (::bind(handle, reinterpret_cast<SOCKADDR*>(&address), address_size)) {
		WS_PRINT_LAST_ERROR();
		return;
	}
	if (::listen(handle, SOMAXCONN)) {
		WS_PRINT_LAST_ERROR();
		return;
	}
	listening = true;
	auto& winsock_manager = static_cast<winsock_connection_manager&>(manager);
	winsock_manager.load_extensions(handle);
	synchronized_accept();
}

void winsock_socket::on_send_completed(iocp_send_data& data) {
	std::lock_guard lock{ mutex };
	if (data.transferred == 0) {
		on_disconnect_queue.emplace(socket_close_status::disconnected_gracefully);
	}
	io.send.erase(&data);
}

void winsock_socket::on_receive_completed(iocp_receive_data& data) {
	std::lock_guard lock{ mutex };
	if (data.transferred != 0) {
		parse(data.buffer.buf, data.transferred);
		synchronized_receive();
	} else {
		on_disconnect_queue.emplace(socket_close_status::disconnected_gracefully);
	}
	io.receive.erase(&data);
}

void winsock_socket::on_accept_completed(iocp_accept_data& data) {
	std::lock_guard lock{ mutex };
	auto& winsock_manager = static_cast<winsock_connection_manager&>(manager);
	winsock_manager.get_accept_sockaddrs(data);
	if (const auto status = setsockopt(data.accepted_socket->handle, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char*>(&handle), sizeof(handle)); status != NO_ERROR) {
		warning(network::log, "Failed to update context for accepted socket.");
		WS_PRINT_LAST_ERROR();
		// todo: should the socket be closed here?
	}
	data.accepted_socket->connected = true;
	on_accept_queue.emplace(data.accepted_socket);
	io.accept.erase(&data);
	synchronized_accept();
}

void winsock_socket::synchronized_send(io_stream& stream) {
	auto data = new iocp_send_data{};
	io.send.emplace(data);
	data->buffer = { static_cast<ULONG>(stream.size_left_to_read()), stream.at_read() };
	// unlike regular non-blocking send(), WSASend() will complete the operation asynchronously,
	// and this might happen before it returns. the data can immediately be discarded on return.
	const auto result = WSASend(handle, &data->buffer, 1, nullptr, 0, &data->overlapped, nullptr);
	verify_socket_state(result);
}

void winsock_socket::synchronized_receive() {
	auto data = new iocp_receive_data{};
	io.receive.emplace(data);
	// unlike regular non-blocking recv(), WSARecv() will complete asynchronously. this can happen before it returns
	DWORD flags{ 0 };
	const auto result = WSARecv(handle, &data->buffer, 1, nullptr, &flags, &data->overlapped, nullptr);
	verify_socket_state(result);
}

bool winsock_socket::synchronized_accept() {
	// use completion ports with AcceptEx extension if loaded
	auto& winsock_manager = static_cast<winsock_connection_manager&>(manager);
	if (winsock_manager.accept_ex(*this)) {
		return true;
	}
	// use regular accept instead
	const SOCKET accepted_handle{ ::accept(handle, reinterpret_cast<SOCKADDR*>(&address), &address_size) };
	if (accepted_handle == SOCKET_ERROR) {
		// if the socket is non-blocking, there probably weren't any connections to be accepted
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			WS_PRINT_LAST_ERROR();
		}
		return false;
	}

	// todo: fix when vs 16.10 is out.
	auto accepted_socket = manager.create_socket();
	//const auto accepted_socket = static_cast<winsock_socket*>(manager.create_socket());
	const auto accepted_socket_ = static_cast<winsock_socket*>(manager.create_socket());
	accepted_socket_->handle = accepted_handle;
	on_accept.emit(accepted_socket);
	return true;
}

bool winsock_socket::verify_socket_state(int result) {
	if (result == SOCKET_ERROR) {
		int error = WSAGetLastError();
		switch (error) {
		case WSAECONNRESET:
			on_disconnect_queue.emplace(socket_close_status::connection_reset);
			return false;
		case WSAENOTSOCK:
			WS_PRINT_ERROR(error);
			return false;
		case WSA_IO_PENDING:
			return true; // normal error message if the data wasn't received immediately
		default:
			WS_PRINT_ERROR(error);
			return false;
		}
	}
	return true;
}

}

std::ostream& operator<<(std::ostream& out, nfwk::iocp_operation operation) {
	switch (operation) {
	case nfwk::iocp_operation::invalid: return out << "Invalid";
	case nfwk::iocp_operation::send: return out << "Send";
	case nfwk::iocp_operation::receive: return out << "Receive";
	case nfwk::iocp_operation::accept: return out << "Accept";
	case nfwk::iocp_operation::close: return out << "Close";
	}
}
