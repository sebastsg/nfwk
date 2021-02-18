#pragma once

#include "assert.hpp"

#ifdef ENABLE_VERBOSE_LOGGING
# define LOG_VERBOSE_GL(GL_CALL) INFO(#GL_CALL);
#else
# define LOG_VERBOSE_GL(GL_CALL) 
#endif

#define CHECK_GL_ERROR(GL_CALL) \
	GL_CALL; \
	LOG_VERBOSE_GL(GL_CALL) \
	if (const auto gl_error = glGetError(); gl_error != GL_NO_ERROR) { \
		nfwk::error("graphics", "{}\n{}", #GL_CALL, gluErrorString(gl_error)); \
		ASSERT(gl_error == GL_NO_ERROR); \
	}
