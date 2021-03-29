#pragma once

#define ASSERT(EXPRESSION) \
	do { \
		if (!(EXPRESSION)) { \
			const char* expression = #EXPRESSION;\
			nfwk::error(u8"main", reinterpret_cast<const char8_t*>(expression)); \
			abort(); \
		} \
	} while (0)
