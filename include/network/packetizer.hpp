#pragma once

#include "io.hpp"

namespace no {

class packetizer {
public:

	static void start(io_stream& stream);
	static void end(io_stream& stream);

	packetizer();

	char* data();
	size_t write_index() const;
	void write(char* data, size_t size);
	io_stream next();
	void clean();

private:

	using magic_type = uint32_t;
	using body_size_type = uint32_t;

	static const magic_type magic = 'NFWK';
	static const size_t header_size = sizeof(magic_type) + sizeof(body_size_type);

	io_stream stream;

};

template<typename Packet>
io_stream packet_stream(const Packet& packet) {
	io_stream stream;
	packetizer::start(stream);
	packet.write(stream);
	packetizer::end(stream);
	return stream;
}

}
