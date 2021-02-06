#pragma once

#include <initializer_list>
#include <string>
#include <unordered_map>
#include <map>

typedef std::map<std::string, int> EnumMap;
typedef std::map<int, std::string> EnumReverseMap;

#define ACTION_ENUM(DO) \
	DO(Throw) \
	DO(Spin) \
	DO(None)
enum class TrickAction {
	#define MAKE_ENUM(VAR) VAR,
	ACTION_ENUM(MAKE_ENUM)
	#undef MAKE_ENUM
};
inline static const EnumMap ACTION_NAMES {
	#define MAKE_NAMES(VAR) {#VAR, (int)TrickAction::VAR},
	ACTION_ENUM(MAKE_NAMES)
	#undef MAKE_NAMES
};

inline static const EnumReverseMap ACTION_REVERSE_NAMES {
#define MAKE_NAMES(VAR) {(int)TrickAction::VAR, #VAR},
        ACTION_ENUM(MAKE_NAMES)
#undef MAKE_NAMES
};

#define SPINDIR_ENUM(DO) \
	DO(Forward) \
	DO(Backward)
enum class SpinDir {
	#define MAKE_ENUM(VAR) VAR,
	SPINDIR_ENUM(MAKE_ENUM)
	#undef MAKE_ENUM
};
inline static const EnumMap SPIN_DIR_NAMES {
	#define MAKE_NAMES(VAR) {#VAR, (int) SpinDir::VAR},
	SPINDIR_ENUM(MAKE_NAMES)
	#undef MAKE_NAMES
};

inline static const EnumReverseMap SPIN_DIR_REVERSE_NAMES {
#define MAKE_NAMES(VAR) {(int) SpinDir::VAR, #VAR},
        SPINDIR_ENUM(MAKE_NAMES)
#undef MAKE_NAMES
};


#define THUMBDIR_ENUM(DO) \
	DO(Horizontal) \
	DO(Vertical)
enum class ThumbstickDir {
	#define MAKE_ENUM(VAR) VAR,
	THUMBDIR_ENUM(MAKE_ENUM)
	#undef MAKE_ENUM
};
inline static const EnumMap THUMBSTICK_DIR_NAMES {
	#define MAKE_NAMES(VAR) {#VAR, (int)ThumbstickDir::VAR},
	THUMBDIR_ENUM(MAKE_NAMES)
	#undef MAKE_NAMES
};

inline static const EnumReverseMap THUMBSTICK_DIR_REVERSE_NAMES {
#define MAKE_NAMES(VAR) {(int)ThumbstickDir::VAR, #VAR},
        THUMBDIR_ENUM(MAKE_NAMES)
#undef MAKE_NAMES
};