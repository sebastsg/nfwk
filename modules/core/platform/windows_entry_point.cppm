module;

#include <Windows.h>

export module nfwk.core:platform.windows.entry_point;

import :loop;
import :log;

namespace nfwk {

HINSTANCE current_instance{ nullptr };
int show_command{ 0 };

}

export namespace nfwk::windows {

int get_show_command() {
	return show_command;
}

HINSTANCE get_current_instance() {
	return current_instance;
}

}

export int main() {
	if (const auto result = CoInitializeEx(nullptr, COINIT_MULTITHREADED); result != S_OK && result != S_FALSE) {
		nfwk::warning("platform", "Failed to initialize COM library.");
	}
	const int result{ nfwk::run_main_loop() };
	CoUninitialize();
	return result;
}

export int WINAPI WinMain(HINSTANCE current_instance, HINSTANCE previous_instance, LPSTR command_line, int show_command) {
	nfwk::current_instance = current_instance;
	nfwk::show_command = show_command;
	return main();
}
