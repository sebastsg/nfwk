export module nfwk.ui:ui_style;

import std.core;
import nfwk.core;

export namespace nfwk::ui {

struct custom_style_state {
	bool hovered{ false };
	bool active{ false };
};

class element_state_style {
public:

	vector4f background;
	vector4f text;
	vector4f border;

	vector2f padding;
	float rounding{ 0.0f };

};

class element_style {
public:

	element_state_style normal;
	element_state_style hover;
	element_state_style active;
	element_state_style disabled;

	void all_to_normal() {
		hover = normal;
		active = normal;
		disabled = normal;
	}

	void default_disabled() {
		disabled = normal;
		disabled.background.w *= 0.6f;
		disabled.text.w *= 0.6f;
		disabled.border.w *= 0.6f;
	}

	const element_state_style& get(bool is_hovered, bool is_active, bool is_disabled) const {
		if (is_disabled) {
			return disabled;
		} else if (is_active) {
			return active;
		} else if (is_hovered) {
			return hover;
		} else {
			return normal;
		}
	}

};

class ui_style {
public:

	element_style button;
	element_style menu_item;
	element_style input;
	element_style window;
	element_style scrollbar;
	element_style resize_grip;

	ui_style() {
		// Buttons
		button.normal.background = { 0.03f, 0.02f, 0.03f, 1.0f };
		button.normal.text = { 0.7f, 0.7f, 0.7f, 1.0f };
		button.normal.border = { 0.0f, 0.0f, 0.0f, 0.0f };
		button.normal.padding = { 8.0f, 6.0f };
		button.all_to_normal();

		button.hover.background = { 0.85f, 0.75f, 0.25f, 1.0f };
		button.hover.text = { 0.3f, 0.3f, 0.3f, 1.0f };
		button.hover.border = { 0.85f, 0.75f, 0.25f, 1.0f };

		button.active = button.hover;

		// Menu Items
		menu_item = button;
		menu_item.normal.background = 0.0f;
		menu_item.normal.text = { 0.7f, 0.7f, 0.7f, 1.0f };

		//menu_item.hover.background = { 0.85f, 0.75f, 0.25f, 1.0f };
		menu_item.hover.text = { 0.85f, 0.75f, 0.25f, 1.0f };
		menu_item.hover.background = menu_item.normal.background;

		menu_item.active = menu_item.hover;

		// Inputs
		input.normal.background = { 0.15f, 0.15f, 0.12f, 1.0f };
		input.normal.text = { 0.7f, 0.7f, 0.7f, 1.0f };
		input.normal.padding = { 8.0f, 6.0f };
		input.all_to_normal();

		//input.hover.background = { 0.7f, 0.5f, 0.2f, 1.0f };
		input.hover.text = { 0.7f, 0.7f, 0.7f, 1.0f };

		input.active.background = { 0.3f, 0.3f, 0.3f, 1.0f };

		// Windows
		//window.normal.background = { 0.041f, 0.041f, 0.041f, 1.0f };
		window.normal.background = { 0.1f, 0.1f, 0.1f, 1.0f };
		window.normal.text = { 0.85f, 0.95f, 0.95f, 1.0f };
		window.normal.border = { 0.03f, 0.02f, 0.03f, 1.0f };
		window.normal.padding = { 8.0f, 6.0f };
		window.all_to_normal();

		// Scrollbar
		scrollbar.normal.background = { 0.16f, 0.16f, 0.16f, 1.0f };
		scrollbar.normal.text = { 0.06f, 0.06f, 0.06f, 1.0f };
		scrollbar.all_to_normal();

		scrollbar.hover.text = { 0.35f, 0.35f, 0.35f, 1.0f };
		scrollbar.active.text = { 1.0f, 0.9f, 0.5f, 1.0f };

		// Resize Grip
		resize_grip.normal.background = { 0.35f, 0.35f, 0.35f, 1.0f };
		resize_grip.hover.background = { 0.6f, 0.6f, 0.6f, 1.0f };
		resize_grip.active.background = { 1.0f, 0.9f, 0.5f, 1.0f };

		// Defaults
		button.default_disabled();
		input.default_disabled();
		window.default_disabled();
		scrollbar.default_disabled();
		resize_grip.default_disabled();
	}

};

ui_style& get_style() {
	thread_local ui_style style;
	return style;
}

}
