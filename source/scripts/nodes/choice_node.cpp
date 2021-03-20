#include "scripts/nodes/choice_node.hpp"
#include "graphics/ui.hpp"
#include "io.hpp"

namespace nfwk {

void choice_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write(text);
}

void choice_node::read(io_stream& stream) {
	script_node::read(stream);
	text = stream.read<std::string>();
}

bool choice_node::can_be_entry_point() const {
	return false;
}

bool choice_node::is_interactive() const {
	return true;
}

bool choice_node::update_editor() {
	return ui::input("##choice", text, { 350.0f, ImGui::GetTextLineHeight() * 3.0f });
}

}
