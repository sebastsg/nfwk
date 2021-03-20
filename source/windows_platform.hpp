#pragma once

#include <string>

#ifndef _WINDEF_
struct HINSTANCE__;
typedef HINSTANCE__ *HINSTANCE;
struct HDC__;
typedef HDC__ *HDC;
struct HWND__;
typedef HWND__ *HWND;
#endif

namespace nfwk::platform::windows {

HINSTANCE current_instance();
int show_command();
std::string get_error_message(int error_code);

}
