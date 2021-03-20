#include "scripts/nodes/message_node.hpp"
#include "graphics/ui.hpp"
#include "io.hpp"

namespace nfwk {

void message_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write(text);
}

void message_node::read(io_stream& stream) {
	script_node::read(stream);
	text = stream.read<std::string>();
}

bool message_node::update_editor() {
	return ui::input("##message", text, { 350.0f, ImGui::GetTextLineHeight() * 8.0f });
}

bool message_node::is_interactive() const {
	return true;
}

}
