#include "network/packetizer.hpp"
#include "log.hpp"

namespace nfwk {

void packetizer::start(io_stream& stream) {
	stream.write(magic);
	stream.write<body_size_type>(0); // offset for the size
}

void packetizer::end(io_stream& stream) {
	if (header_size > stream.write_index()) {
		return;
	}
	// go back to the beginning and write the size
	const std::size_t size{ stream.write_index() - header_size };
	stream.set_write_index(sizeof(magic_type));
	stream.write(static_cast<body_size_type>(size));
	stream.move_write_index(size);
}

packetizer::packetizer() {
	stream.allocate(1024 * 1024 * 10); // prevent resizes per sync. todo: improve, because this is lazy
}

char* packetizer::data() {
	return stream.data();
}

std::size_t packetizer::write_index() const {
	return stream.write_index();
}

void packetizer::write(char* data, std::size_t size) {
	stream.write_raw(data, size);
}

io_stream packetizer::next() {
	if (header_size > stream.size_left_to_read()) {
		return {};
	}
	auto body_size = stream.peek<body_size_type>(sizeof(magic_type));
	if (header_size + body_size > stream.size_left_to_read()) {
		return {};
	}
	if (stream.peek<magic_type>() != magic) {
		warning(network::log, u8"Skipping magic... {} != {}", stream.peek<magic_type>(), magic);
		stream.move_read_index(1); // no point in reading the same magic again
		return {};
	}
	stream.move_read_index(header_size);
	auto body_begin = stream.at_read();
	stream.move_read_index(body_size);
	return { body_begin, body_size, io_stream::construct_by::shallow_copy };
}

void packetizer::clean() {
	stream.shift_read_to_begin();
}

}
