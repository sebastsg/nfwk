#pragma once

// If enabled, redundant texture and shader binds, among others, will be reported. 
// Disable for release builds.
#define TRACK_REDUNDANT_BINDS  1

// Windows, cameras, textures, etc...
#define ENABLE_GRAPHICS  1

// TODO: Remove. Should be more decoupled than it currently is.
#define ENABLE_ASSIMP  0

// Audio
#define ENABLE_AUDIO  1

// Network
#define ENABLE_NETWORK  1

// Logging macros (MESSAGE, WARNING, etc...)
// Important: Also sets the ASSERT and BUG macros.
#define ENABLE_DEBUG_LOG  1

// Logs information that can be useful while debugging. Usually disabled otherwise.
#define ENABLE_VERBOSE_DEBUG_LOG  0

// Automatically add the log writer when a new log is created.
// When disabled, you have to manually call no::debug::log::add_writer<Writer>()
#define AUTO_ADD_STDOUT_LOG_WRITER  0
#define AUTO_ADD_HTML_LOG_WRITER    1
