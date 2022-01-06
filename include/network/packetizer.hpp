#pragma once

#include "io.hpp"

namespace nfwk {

class packetizer {
public:

	static void start(io_stream& stream);
	static void end(io_stream& stream);

	packetizer();

	char* data();
	std::size_t write_index() const;
	void write(const void* data, std::size_t size);
	io_stream next();
	void clean();

private:

	using magic_type = std::uint32_t;
	using body_size_type = std::uint32_t;

	static constexpr magic_type magic{ 'NFWK' };
	static constexpr auto header_size = sizeof(magic_type) + sizeof(body_size_type);

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
