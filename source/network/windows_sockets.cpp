#include "windows_sockets.hpp"
#include "log.hpp"
#include "windows_platform.hpp"

#define WS_PRINT_ERROR(ERR)        print_winsock_error(ERR, __FUNCSIG__, __LINE__, "network")
#define WS_PRINT_LAST_ERROR()      print_winsock_error(WSAGetLastError(), __FUNCSIG__, __LINE__, "network")

namespace nfwk {

DWORD io_port_thread(HANDLE io_port, int thread_num);

static winsock_state winsock;

static void print_winsock_error(int error_code, const std::string& funcsig, int line, const std::string& log) {
	const auto message = platform::windows::get_error_message(error_code);
	error("network", "WSA Error {} on line {} in {}\n{}", error_code, line, funcsig, message);
}

static void create_completion_port() {
	// it is possible to create more threads than specified below
	// but only max this amount of threads can run simultaneously
	// 0 -> number of CPUs -- not supported in the for loop below
	const int thread_count{ 1 };
	winsock.io_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, thread_count);
	if (!winsock.io_port) {
		error("network", "Failed to create I/O completion port. Error: {}", GetLastError());
		return;
	}
	for (int i{ 0 }; i < thread_count; i++) {
		winsock.threads.emplace_back([i] {
			io_port_thread(winsock.io_port, i);
		});
	}
}

static void destroy_completion_port() {
	if (winsock.io_port == INVALID_HANDLE_VALUE) {
		return;
	}
	iocp_close_data close_io;
	for (auto& thread : winsock.threads) {
		PostQueuedCompletionStatus(winsock.io_port, 0, 0, &close_io.overlapped);
	}
	// join all threads. each should receive their own close event
	for (auto& thread : winsock.threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
	winsock.threads.clear();
	CloseHandle(winsock.io_port);
	winsock.io_port = INVALID_HANDLE_VALUE;
}

winsock_socket::winsock_socket() {
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;
	addr.sin_family = hints.ai_family;
}

static void load_extensions(SOCKET handle) {
	if (winsock.AcceptEx) {
		return;
	}
	// AcceptEx
	DWORD bytes{ 0 };
	DWORD code{ SIO_GET_EXTENSION_FUNCTION_POINTER };
	GUID guid = WSAID_ACCEPTEX;
	WSAIoctl(handle, code, &guid, sizeof(guid), &winsock.AcceptEx, sizeof(winsock.AcceptEx), &bytes, nullptr, nullptr);
	// GetAcceptExSockaddrs
	bytes = 0;
	guid = WSAID_GETACCEPTEXSOCKADDRS;
	WSAIoctl(handle, code, &guid, sizeof(guid), &winsock.GetAcceptExSockaddrs, sizeof(winsock.GetAcceptExSockaddrs), &bytes, nullptr, nullptr);
}

static int update_accept_context(SOCKET client, SOCKET listener) {
	return setsockopt(client, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&listener, sizeof(listener));
}

static void get_accept_sockaddrs(iocp_accept_data& data) {
	if (!winsock.GetAcceptExSockaddrs) {
		return;
	}
	// todo: at the moment we aren't doing anything with the result here
	const DWORD addr_size{ iocp_accept_data::padded_addr_size };
	sockaddr* local{ nullptr };
	sockaddr* remote{ nullptr };
	int local_size{ sizeof(local) };
	int remote_size{ sizeof(remote) };
	winsock.GetAcceptExSockaddrs(data.buffer.buf, 0, addr_size, addr_size, &local, &local_size, &remote, &remote_size);
}

static void destroy_socket(int id) {
	auto& socket{ winsock.sockets[id] };
	if (socket.handle == INVALID_SOCKET) {
		return;
	}
	if (const int status{ closesocket(socket.handle) }; status == SOCKET_ERROR) {
		WS_PRINT_LAST_ERROR();
		return;
	}
	socket.handle = INVALID_SOCKET;
	for (auto send : socket.io.send) {
		delete send;
	}
	for (auto receive : socket.io.receive) {
		delete receive;
	}
	for (auto accept : socket.io.accept) {
		delete accept;
	}
	winsock.sockets[id] = {};
}

static bool create_socket(int id) {
	auto& socket = winsock.sockets[id];
	if (socket.handle != INVALID_SOCKET) {
		return true;
	}
	socket.handle = WSASocketW(socket.hints.ai_family, socket.hints.ai_socktype, socket.hints.ai_protocol, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (socket.handle == INVALID_SOCKET) {
		WS_PRINT_LAST_ERROR();
		return false;
	}
	CreateIoCompletionPort((HANDLE)socket.handle, winsock.io_port, (ULONG_PTR)id, 0);
	return true;
}

static bool connect_socket(int id) {
	auto& socket = winsock.sockets[id];
	create_socket(id);
	if (const int success{ ::connect(socket.handle, (SOCKADDR*)& socket.addr, socket.addr_size) }; success != 0) {
		WS_PRINT_LAST_ERROR();
		return false;
	}
	socket.connected = true;
	return true;
}

static bool connect_socket(int id, const std::string& address, int port) {
	auto& socket = winsock.sockets[id];
	addrinfo* result{ nullptr };
	if (const int status{ getaddrinfo(address.c_str(), CSTRING(port), &socket.hints, &result) }; status != 0) {
		warning("network", "Failed to get address info for {}:{}\nStatus: {}", address, port, status);
		return false;
	}
	while (result) {
		socket.addr = *((SOCKADDR_IN*)result->ai_addr);
		socket.hints.ai_family = result->ai_family;
		freeaddrinfo(result);
		break; // result = result->ai_next;
	}
	return connect_socket(id);
}

static bool socket_receive(int id) {
	auto& socket = winsock.sockets[id];
	auto data = new iocp_receive_data{};
	socket.io.receive.emplace(data);
	// unlike regular non-blocking recv(), WSARecv() will complete asynchronously. this can happen before it returns
	DWORD flags{ 0 };
	const int result{ WSARecv(socket.handle, &data->buffer, 1, &data->bytes, &flags, &data->overlapped, nullptr) };
	if (result == SOCKET_ERROR) {
		int error = WSAGetLastError();
		switch (error) {
		case WSAECONNRESET:
			socket.sync.disconnect.emplace(socket_close_status::connection_reset);
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

static bool socket_send(int id, const io_stream& packet) {
	auto& socket = winsock.sockets[id];
	auto data = new iocp_send_data{};
	socket.io.send.emplace(data);
	data->buffer = { (ULONG)packet.write_index(), packet.data() };
	// unlike regular non-blocking send(), WSASend() will complete the operation asynchronously,
	// and this might happen before it returns. the data can immediately be discarded on return.
	const int result{ WSASend(socket.handle, &data->buffer, 1, &data->bytes, 0, &data->overlapped, nullptr) };
	if (result == SOCKET_ERROR) {
		const int error{ WSAGetLastError() };
		switch (error) {
		case WSAECONNRESET:
			socket.sync.disconnect.emplace(socket_close_status::connection_reset);
			return false;
		case WSA_IO_PENDING:
			return true; // normal error message if the data wasn't sent immediately
		default:
			WS_PRINT_ERROR(error);
			return false;
		}
	}
	return true;
}

static bool accept_ex(int id) {
	if (!winsock.AcceptEx) {
		return false;
	}
	auto data = new iocp_accept_data{};
	data->accepted_id = open_socket();
	create_socket(data->accepted_id);
	const auto& accepted = winsock.sockets[data->accepted_id];
	auto& socket = winsock.sockets[id];
	socket.io.accept.emplace(data);
	const DWORD addr_size{ iocp_accept_data::padded_addr_size };
	const BOOL status{ winsock.AcceptEx(socket.handle, accepted.handle, data->buffer.buf, 0, addr_size, addr_size, &data->bytes, &data->overlapped) };
	if (status == FALSE) {
		const int error{ WSAGetLastError() };
		switch (error) {
		case WSA_IO_PENDING:
			return true; // normal error message if a client wasn't accepted immediately
		default:
			WS_PRINT_ERROR(error);
			destroy_socket(data->accepted_id);
			socket.io.accept.erase(data);
			delete data;
			return false;
		}
	}
	return true;
}

DWORD io_port_thread(HANDLE io_port, int thread_num) {
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
			nfwk::info("network", "Leaving thread");
			return 0;
		}
		const int socket_id{ static_cast<int>(completion_key) };
		std::lock_guard lock{ *winsock.mutexes[socket_id] };
		auto& socket = winsock.sockets[socket_id];

		if (data->operation == iocp_operation::send) {
			if (transferred == 0) {
				socket.sync.disconnect.emplace(socket_close_status::disconnected_gracefully);
				continue;
			}
			auto send_data = reinterpret_cast<iocp_send_data*>(data);
			socket.io.send.erase(send_data);
			delete send_data;

		} else if (data->operation == iocp_operation::receive) {
			if (transferred == 0) {
				socket.sync.disconnect.emplace(socket_close_status::disconnected_gracefully);
				continue;
			}
			auto receive_data = reinterpret_cast<iocp_receive_data*>(data);
			const std::size_t previous_write{ socket.receive_packetizer.write_index() };
			socket.receive_packetizer.write(receive_data->buffer.buf, transferred);
			// queue the stream events. use the packetizer's buffer
			char* stream_begin{ socket.receive_packetizer.data() + previous_write };
			socket.sync.stream.emplace(io_stream{ stream_begin, transferred, io_stream::construct_by::shallow_copy });
			// parse buffer and queue packet events
			while (true) {
				if (io_stream packet{ socket.receive_packetizer.next() }; !packet.empty()) {
					socket.sync.packet.emplace(io_stream{ packet.data(), packet.size_left_to_read(), io_stream::construct_by::shallow_copy });
				} else {
					break;
				}
			}
			socket.io.receive.erase(receive_data);
			delete receive_data;
			socket_receive(socket_id);

		} else if (data->operation == iocp_operation::accept) {
			auto accept_data = reinterpret_cast<iocp_accept_data*>(data);
			get_accept_sockaddrs(*accept_data);
			auto& accepted = winsock.sockets[accept_data->accepted_id];
			if (const int status{ update_accept_context(accepted.handle, socket.handle) }; status != NO_ERROR) {
				warning("network", "Failed to update context for accepted socket {}", accept_data->accepted_id);
				WS_PRINT_LAST_ERROR();
				// todo: should the socket be closed here?
			}
			accepted.connected = true;
			socket.sync.accept.emplace(accept_data->accepted_id);
			socket.io.accept.erase(accept_data);
			delete accept_data;
			increment_socket_accepts(socket_id);
		}
	}
	return 0;
}

void start_network() {
	message("network", "Initializing WinSock");
	constexpr auto version = MAKEWORD(2, 2);
	if (const int status{ WSAStartup(version, &winsock.wsa_data) }; status != 0) {
		error("network", "WinSock failed to start. Error: {}", status);
		WS_PRINT_LAST_ERROR();
	} else {
		create_completion_port();
	}
}

void stop_network() {
	destroy_completion_port();
	for (int i{ 0 }; i < static_cast<int>(winsock.sockets.size()); i++) {
		destroy_socket(i);
	}
	if (WSACleanup() != 0) {
		warning("network", "Failed to stop WinSock. Some operations may still be ongoing.");
		WS_PRINT_LAST_ERROR();
	} else {
		message("network", "WinSock has been stopped.");
	}
}

int open_socket() {
	const int socket_count{ static_cast<int>(winsock.sockets.size()) };
	for (int i{ 0 }; i < socket_count; i++) {
		if (!winsock.sockets[i].alive) {
			winsock.sockets[i] = {};
			winsock.sockets[i].alive = true;
			return i;
		}
	}
	winsock.mutexes.emplace_back(std::make_unique<std::mutex>());
	winsock.sockets.emplace_back().alive = true;
	return socket_count;
}

int open_socket(const std::string& address, int port) {
	const int id{ open_socket() };
	if (connect_socket(id, address, port)) {
		socket_receive(id);
	}
	return id;
}

void close_socket(int id) {
	winsock.destroy_queue.push_back(id);
}

void synchronize_socket(int id) {
	std::lock_guard lock{ *winsock.mutexes[id] };
	auto& socket = winsock.sockets[id];
	if (socket.sync.disconnect.size() > 0) {
		socket.sync.disconnect.emit(socket.events.disconnect);
		close_socket(id);
		return;
	}
	if (socket.connected) {
		socket.sync.stream.emit(socket.events.stream);
		socket.sync.packet.emit(socket.events.packet);
		socket.receive_packetizer.clean();
		for (const auto& packet : socket.queued_packets) {
			socket_send(id, packet);
		}
	}
	socket.queued_packets.clear();
	if (socket.listening) {
		socket.sync.accept.all([&](int accepted_id) {
			socket.events.accept.emit(accepted_id);
			socket_receive(accepted_id);
		});
	}
}

void synchronize_sockets() {
	for (std::size_t i{ 0 }; i < winsock.sockets.size(); i++) {
		if (winsock.sockets[i].alive) {
			synchronize_socket(static_cast<int>(i));
		}
	}
	winsock.broadcast_count = 0;
	for (const int destroy_id : winsock.destroy_queue) {
		destroy_socket(destroy_id);
	}
	winsock.destroy_queue.clear();
}

bool bind_socket(int id, const std::string& address, int port) {
	auto& socket = winsock.sockets[id];
	addrinfo* result{ nullptr };
	if (const int status{ getaddrinfo(address.c_str(), CSTRING(port), &socket.hints, &result) }; status != 0) {
		warning("network", "Failed to get address info for {}:{}\nStatus: {}", status, address, port);
		return false;
	}
	while (result) {
		socket.addr = *((SOCKADDR_IN*)result->ai_addr);
		socket.hints.ai_family = result->ai_family;
		freeaddrinfo(result);
		break; // result = result->ai_next;
	}
	create_socket(id);
	if (::bind(socket.handle, (SOCKADDR*)&socket.addr, socket.addr_size)) {
		WS_PRINT_LAST_ERROR();
		return false;
	}
	return true;
}

bool listen_socket(int id) {
	auto& socket = winsock.sockets[id];
	if (::listen(socket.handle, SOMAXCONN)) {
		WS_PRINT_LAST_ERROR();
		return false;
	}
	socket.listening = true;
	load_extensions(socket.handle);
	return increment_socket_accepts(id);
}

bool increment_socket_accepts(int id) {
	// use completion ports with AcceptEx extension if loaded
	if (accept_ex(id)) {
		return true;
	}
	// use regular accept instead
	auto& socket = winsock.sockets[id];
	const SOCKET accepted_handle{ ::accept(socket.handle, (SOCKADDR*)& socket.addr, &socket.addr_size) };
	if (accepted_handle == SOCKET_ERROR) {
		// if the socket is non-blocking, there probably weren't any connections to be accepted
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			WS_PRINT_LAST_ERROR();
		}
		return false;
	}
	const int accept_id{ open_socket() };
	winsock.sockets[accept_id].handle = accepted_handle;
	socket.events.accept.emit(accept_id);
	return true;
}

void socket_send(int id, io_stream&& stream) {
	winsock.sockets[id].queued_packets.emplace_back(std::move(stream));
}

void broadcast(io_stream&& stream) {
	winsock.queued_packets[winsock.broadcast_count] = std::move(stream);
	for (auto& socket : winsock.sockets) {
		socket.queued_packets.emplace_back(winsock.queued_packets[winsock.broadcast_count].data(), winsock.queued_packets[winsock.broadcast_count].write_index(), io_stream::construct_by::shallow_copy);
	}
	winsock.broadcast_count++;
}

void broadcast(io_stream&& stream, int except_id) {
	winsock.queued_packets[winsock.broadcast_count] = std::move(stream);
	for (int i{ 0 }; i < static_cast<int>(winsock.sockets.size()); i++) {
		if (i != except_id) {
			winsock.sockets[i].queued_packets.emplace_back(winsock.queued_packets[winsock.broadcast_count].data(), winsock.queued_packets[winsock.broadcast_count].write_index(), io_stream::construct_by::shallow_copy);
		}
	}
	winsock.broadcast_count++;
}

socket_events& socket_event(int id) {
	return winsock.sockets[id].events;
}

}

std::ostream& operator<<(std::ostream& out, nfwk::iocp_operation operation) {
	switch (operation) {
	case nfwk::iocp_operation::invalid: return out << "Invalid";
	case nfwk::iocp_operation::send: return out << "Send";
	case nfwk::iocp_operation::receive: return out << "Receive";
	case nfwk::iocp_operation::accept: return out << "Accept";
	case nfwk::iocp_operation::close: return out << "Close";
	default: return out << "Unknown (" << static_cast<int>(operation) << ")";
	}
}
