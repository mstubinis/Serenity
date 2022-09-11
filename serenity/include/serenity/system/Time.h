#pragma once
#ifndef ENGINE_SYSTEM_TIME_H
#define ENGINE_SYSTEM_TIME_H

#include <chrono>

namespace Engine::time {
	[[nodiscard]] inline std::chrono::steady_clock::time_point now() noexcept {
		return std::chrono::steady_clock::now();
	}

	template<class REPRESENTATION = int64_t>
	[[nodiscard]] inline REPRESENTATION now_as_milliseconds() noexcept {
		return std::chrono::duration_cast<std::chrono::milliseconds, REPRESENTATION>(now().time_since_epoch()).count();
	}

	[[nodiscard]] inline auto difference(const std::chrono::steady_clock::time_point startTime) noexcept {
	    return Engine::time::now() - startTime;
	}
	[[nodiscard]] inline int64_t difference_as_milliseconds(const int64_t startTimeInMilliseconds) noexcept {
		return now_as_milliseconds() - startTimeInMilliseconds;
	}
	template<class REP = double>
	[[nodiscard]] inline REP difference_as_seconds(const int64_t startTimeInMilliseconds) noexcept {
		return static_cast<REP>(double(now_as_milliseconds() - startTimeInMilliseconds) * 0.001);
	}
}

#endif