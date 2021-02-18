export module nfwk.core:platform;

#if defined _WIN32
export import :platform.windows;
#elif defined __linux__
//export import :platform.linux;
#error Linux not yet supported.
#else
#error Unknown platform.
#endif
