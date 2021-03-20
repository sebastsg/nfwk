#pragma once

#define ASSERT(EXPRESSION) \
	do { \
		if (!(EXPRESSION)) { \
			nfwk::error("main", #EXPRESSION); \
			abort(); \
		} \
	} while (0)
