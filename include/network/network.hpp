#pragma once

#include "platform.hpp"

#include "network/packetizer.hpp"
#include "event.hpp"

namespace nfwk {

enum class socket_close_status { disconnected_gracefully, connection_reset, not_connected, unknown };

void start_network();
void stop_network();

struct socket_events {
	event<io_stream> stream;
	event<io_stream> packet;
	event<socket_close_status> disconnect;
	event<int> accept;
};

int open_socket();
int open_socket(const std::string& address, int port);
void close_socket(int id);
void synchronize_socket(int id);
void synchronize_sockets();
bool bind_socket(int id, const std::string& address, int port);
bool listen_socket(int id);
bool increment_socket_accepts(int id);
void socket_send(int id, io_stream&& stream);
void broadcast(io_stream&& stream);
void broadcast(io_stream&& stream, int except_id);
socket_events& socket_event(int id);

template<typename P>
void send_packet(int id, const P& packet) {
	socket_send(id, packet_stream(packet));
}

template<typename P>
void broadcast(const P& packet) {
	broadcast(packet_stream(packet));
}

template<typename P>
void broadcast(const P& packet, int except_id) {
	broadcast(packet_stream(packet), except_id);
}

}

std::ostream& operator<<(std::ostream& out, nfwk::socket_close_status status);
