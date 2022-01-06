#include "network/winsock_connection_manager.hpp"
#include "network/winsock_socket.hpp"

#include "log.hpp"
#include "windows_platform.hpp"

#define WS_PRINT_ERROR(ERR)   print_winsock_error(ERR, __FUNCSIG__, __LINE__)
#define WS_PRINT_LAST_ERROR() print_winsock_error(WSAGetLastError(), __FUNCSIG__, __LINE__)

namespace nfwk {

static void print_winsock_error(int error_code, std::string_view function_signature, int line) {
	const auto message = platform::windows::get_error_message(error_code);
	error(network::log, "WSA Error {} on line {} in {}\n{}", error_code, line, function_signature, message);
}

winsock_connection_manager::winsock_connection_manager() {
	// it is possible to create more threads than specified below
	// but only max this amount of threads can run simultaneously
	// 0 -> number of CPUs -- not supported in the for loop below
	const int thread_count{ 1 };
	
	message(network::log, "Initializing WinSock");
	constexpr auto version = MAKEWORD(2, 2);
	if (const auto status = WSAStartup(version, &wsa_data); status != 0) {
		error(network::log, "WinSock failed to start. Error: {}", status);
		WS_PRINT_LAST_ERROR();
	} else {
		io_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, thread_count);
		if (io_port) {
			for (int i{ 0 }; i < thread_count; i++) {
				threads.emplace_back([this] {
					io_port_thread();
				});
			}
		} else {
			error(network::log, "Failed to create I/O completion port. Error: {}", GetLastError());
		}
	}
}

winsock_connection_manager::~winsock_connection_manager() {
	if (io_port == INVALID_HANDLE_VALUE) {
		return;
	}
	iocp_close_data close_io;
	for (auto& thread : threads) {
		PostQueuedCompletionStatus(io_port, 0, 0, &close_io.overlapped);
	}
	// join all threads. each should receive their own close event
	for (auto& thread : threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
	threads.clear();
	CloseHandle(io_port);
	io_port = INVALID_HANDLE_VALUE;
	sockets.clear();
	if (WSACleanup() != 0) {
		warning(network::log, "Failed to stop WinSock. Some operations may still be ongoing.");
		WS_PRINT_LAST_ERROR();
	} else {
		message(network::log, "WinSock has been stopped.");
	}
}

void winsock_connection_manager::get_accept_sockaddrs(iocp_accept_data& data) {
	if (!GetAcceptExSockaddrs) {
		return;
	}
	// todo: at the moment we aren't doing anything with the result here
	const auto address_size = static_cast<DWORD>(iocp_accept_data::padded_address_size);
	sockaddr* local{ nullptr };
	sockaddr* remote{ nullptr };
	auto local_size = static_cast<int>(sizeof(sockaddr*));
	auto remote_size = static_cast<int>(sizeof(sockaddr*));
	GetAcceptExSockaddrs(data.buffer.buf, 0, address_size, address_size, &local, &local_size, &remote, &remote_size);
}

void winsock_connection_manager::load_extensions(SOCKET handle) {
	if (AcceptEx) {
		return;
	}
	// AcceptEx
	DWORD bytes{ 0 };
	DWORD code{ SIO_GET_EXTENSION_FUNCTION_POINTER };
	GUID guid = WSAID_ACCEPTEX;
	WSAIoctl(handle, code, &guid, sizeof(guid), &AcceptEx, sizeof(AcceptEx), &bytes, nullptr, nullptr);
	// GetAcceptExSockaddrs
	bytes = 0;
	guid = WSAID_GETACCEPTEXSOCKADDRS;
	WSAIoctl(handle, code, &guid, sizeof(guid), &GetAcceptExSockaddrs, sizeof(GetAcceptExSockaddrs), &bytes, nullptr, nullptr);
}

bool winsock_connection_manager::accept_ex(winsock_socket& socket) {
	if (!AcceptEx) {
		return false;
	}
	auto data = new iocp_accept_data{};
	data->accepted_socket = static_cast<winsock_socket*>(create_socket());
	socket.io.accept.emplace(data);
	const DWORD address_size{ iocp_accept_data::padded_address_size };
	if (!AcceptEx(socket.handle, data->accepted_socket->handle, data->buffer.buf, 0, address_size, address_size, nullptr, &data->overlapped)) {
		if (const auto error = WSAGetLastError(); error != WSA_IO_PENDING) {
			WS_PRINT_ERROR(error);
			sockets[data->accepted_socket->managed_index] = nullptr;
			socket.io.accept.erase(data);
			delete data;
			return false;
		} else {
			return true; // normal error message if a client wasn't accepted immediately
		}
	}
	return true;
}

void winsock_connection_manager::io_port_thread() {
	while (true) {
		DWORD transferred{ 0 }; // bytes transferred during this operation
		ULONG_PTR completion_key{ 0 }; // pointer to winsock_socket the operation was completed on
		LPOVERLAPPED overlapped{ nullptr }; // pointer to overlapped structure inside winsock_io_data

		// associate this thread with the completion port as we get the queued completion status
		if (!GetQueuedCompletionStatus(io_port, &transferred, &completion_key, &overlapped, INFINITE)) {
			WS_PRINT_LAST_ERROR();	
			if (!overlapped) {
				continue;
			}
			// if overlapped is not a nullptr, a completion status was dequeued
			// this means transferred, completion_key, and overlapped are valid
			// the socket probably disconnected, but that will be handled below
		}
		// reference for the macro: https://msdn.microsoft.com/en-us/library/aa447688.aspx
		auto data = CONTAINING_RECORD(overlapped, iocp_data<iocp_operation::invalid>, overlapped);
		if (data->operation == iocp_operation::close) {
			info(network::log, "Leaving thread");
			break;
		}
		const auto socket_index = static_cast<std::size_t>(completion_key);
		if (socket_index >= sockets.size() || !sockets[socket_index]) {
			warning(network::log, "Cannot process completion status for invalid socket {}", socket_index);
			continue;
		}
		auto socket = dynamic_cast<winsock_socket*>(sockets[socket_index].get());
		data->transferred = transferred;
		if (data->operation == iocp_operation::send) {
			auto send_data = reinterpret_cast<iocp_send_data*>(data);
			socket->on_send_completed(*send_data);
			delete send_data;
		} else if (data->operation == iocp_operation::receive) {
			auto receive_data = reinterpret_cast<iocp_receive_data*>(data);
			socket->on_receive_completed(*receive_data);
			delete receive_data;
		} else if (data->operation == iocp_operation::accept) {
			auto accept_data = reinterpret_cast<iocp_accept_data*>(data);
			socket->on_accept_completed(*accept_data);
			delete accept_data;
		}
	}
}

network_socket* winsock_connection_manager::create_socket() {
	std::size_t index{ 0 };
	for (auto& socket : sockets) {
		if (!socket) {
			socket = std::make_unique<winsock_socket>(*this, index, io_port);
			return socket.get();
		}
		index++;
	}
	return sockets.emplace_back(std::make_unique<winsock_socket>(*this, sockets.size(), io_port)).get();
}

// todo: listener socket should be created like this
#if 0
network_socket* winsock_connection_manager::create_socket(std::string_view address_name, int port) {
	auto socket = static_cast<winsock_socket*>(create_socket());
	if (socket->connect_socket(address_name, port)) {
		socket->synchronized_receive();
	}
	return socket;
}
#endif

}
