#include "network/network.hpp"
#include "log.hpp"

namespace nfwk {

network_socket::network_socket(connection_manager& manager, std::size_t managed_index) : manager{ manager }, managed_index{ managed_index } {}

void network_socket::send(std::string_view view) {
	queued_buffers_to_send.emplace_back(io_stream{ reinterpret_cast<const char*>(view.data()), view.size(), io_stream::const_construct_by::shallow_copy });
}

void network_socket::send(io_stream stream) {
	queued_buffers_to_send.emplace_back(std::move(stream));
}

void network_socket::synchronize() {
	std::lock_guard lock{ mutex };
	if (on_disconnect_queue.size() > 0) {
		on_disconnect_queue.emit(on_disconnect);
		manager.schedule_for_closing(*this);
		return;
	}
	if (connected) {
		on_stream_queue.emit(on_stream);
		on_packet_queue.emit(on_packet);
		receive_packetizer.clean();
		for (auto& stream : queued_buffers_to_send) {
			synchronized_send(stream);
		}
	}
	queued_buffers_to_send.clear();
	if (listening) {
		on_accept_queue.all([&](network_socket* accepted_socket) {
			on_accept.emit(accepted_socket);
			accepted_socket->synchronized_receive();
		});
	}
}

void network_socket::parse(const char* buffer, std::size_t transferred) {
	const auto previous_write_index = receive_packetizer.write_index();
	receive_packetizer.write(buffer, transferred);
	// queue the stream events. use the packetizer's buffer
	char* stream_begin{ receive_packetizer.data() + previous_write_index };
	on_stream_queue.emplace(io_stream{ stream_begin, transferred, io_stream::construct_by::shallow_copy });
	// parse buffer and queue packet events
	while (true) {
		if (io_stream packet{ receive_packetizer.next() }; !packet.empty()) {
			on_packet_queue.emplace(io_stream{ packet.data(), packet.size_left_to_read(), io_stream::construct_by::shallow_copy });
		} else {
			break;
		}
	}
}

void connection_manager::synchronize() {
	std::lock_guard lock{ mutex };
	// vector may change in synchronize.
	for (std::size_t i{ 0 }; i < sockets.size(); i++) {
		if (sockets[i]) {
			sockets[i]->synchronize();
		}
	}
	broadcast_count = 0;
	destroy_queue.clear();
}

void connection_manager::schedule_for_closing(network_socket& socket) {
	destroy_queue.emplace_back(std::move(sockets[socket.managed_index]));
}

void connection_manager::broadcast(io_stream stream) {
	queued_buffers_to_broadcast[broadcast_count] = std::move(stream);
	for (auto& socket : sockets) {
		socket->queued_buffers_to_send.emplace_back(queued_buffers_to_broadcast[broadcast_count].at_read(), queued_buffers_to_broadcast[broadcast_count].size_left_to_read(), io_stream::construct_by::shallow_copy);
	}
	broadcast_count++;
}

void connection_manager::broadcast(io_stream stream, int except_id) {
	queued_buffers_to_broadcast[broadcast_count] = std::move(stream);
	for (int i{ 0 }; i < static_cast<int>(sockets.size()); i++) {
		if (i != except_id) {
			sockets[i]->queued_buffers_to_send.emplace_back(queued_buffers_to_broadcast[broadcast_count].at_read(), queued_buffers_to_broadcast[broadcast_count].size_left_to_read(), io_stream::construct_by::shallow_copy);
		}
	}
	broadcast_count++;
}

}

std::ostream& operator<<(std::ostream& out, nfwk::socket_close_status status) {
	switch (status) {
	case nfwk::socket_close_status::disconnected_gracefully: return out << "Disconnected gracefully";
	case nfwk::socket_close_status::connection_reset: return out << "Connection reset";
	case nfwk::socket_close_status::not_connected: return out << "Not connected";
	case nfwk::socket_close_status::unknown: return out << "Unknown";
	}
}
