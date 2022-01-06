#pragma once

#include "platform.hpp"
#include "network/packetizer.hpp"
#include "event.hpp"

#include <mutex>

namespace nfwk {

class network_socket;
class connection_manager;

enum class socket_close_status { disconnected_gracefully, connection_reset, not_connected, unknown };

// todo: add option to enable secure clearing of buffers

class network_socket {
public:

	friend class connection_manager;

	event<io_stream> on_stream;
	event<io_stream> on_packet;
	event<socket_close_status> on_disconnect;
	event<network_socket*> on_accept;

	network_socket(connection_manager& manager, std::size_t managed_index);
	network_socket(const network_socket&) = delete;
	network_socket(network_socket&&) = delete;

	virtual ~network_socket() = default;

	network_socket& operator=(const network_socket&) = delete;
	network_socket& operator=(network_socket&&) = delete;

	// memory must be available until synchronized_send() is called.
	// an io_stream with shallow copy construction is created internally.
	void send(std::string_view view);

	// if stream does not own its buffer, the owner must keep it available until synchronized_send() is called.
	void send(io_stream stream);

	template<typename Packet>
	void send_packet(const Packet& packet) {
		send(packet_stream(packet));
	}

	virtual void synchronized_send(io_stream& packet) = 0;
	virtual void synchronized_receive() = 0;

	virtual void load_address(std::string_view address_name, int port) = 0;
	virtual void bind_and_listen() = 0;

	std::size_t get_managed_index() const {
		return managed_index;
	}

protected:

	event_queue<io_stream> on_stream_queue;
	event_queue<io_stream> on_packet_queue;
	event_queue<socket_close_status> on_disconnect_queue;
	event_queue<network_socket*> on_accept_queue;

	void synchronize();

	void parse(const char* buffer, std::size_t transferred);

	connection_manager& manager;
	
	// which index in connection manager's sockets vector this instance exists.
	std::size_t managed_index{ 0 };
	
	bool connected{ false };
	bool listening{ false };
	packetizer receive_packetizer;
	std::vector<io_stream> queued_buffers_to_send;
	std::mutex mutex;
	
};

class connection_manager {
public:

	static constexpr int max_broadcasts_per_sync{ 4192 };

	connection_manager() = default;
	connection_manager(const connection_manager&) = delete;
	connection_manager(connection_manager&&) = delete;

	virtual ~connection_manager() = default;

	connection_manager& operator=(const connection_manager&) = delete;
	connection_manager& operator=(connection_manager&&) = delete;

	virtual network_socket* create_socket() = 0;

	void schedule_for_closing(network_socket& socket);

	void synchronize();

	void broadcast(io_stream stream);
	void broadcast(io_stream stream, int except_id);

	template<typename Packet>
	void broadcast(const Packet& packet) {
		broadcast(packet_stream(packet));
	}

	template<typename Packet>
	void broadcast(const Packet& packet, int except_id) {
		broadcast(packet_stream(packet), except_id);
	}

protected:

	std::mutex mutex;
	std::vector<std::unique_ptr<network_socket>> sockets;

	// sockets to destroy in synchronise
	std::vector<std::unique_ptr<network_socket>> destroy_queue;

	// to keep broadcast buffers alive until sync (instead of 1 copy per socket)
	int broadcast_count{ 0 };
	io_stream queued_buffers_to_broadcast[max_broadcasts_per_sync]; // 20 * 4192 = 82 KiB
	
};

}

std::ostream& operator<<(std::ostream& out, nfwk::socket_close_status status);

NFWK_STDSPEC_FORMATTER(nfwk::socket_close_status);
