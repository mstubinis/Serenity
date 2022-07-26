#pragma once
#ifndef ENGINE_SYSTEM_TIME_H
#define ENGINE_SYSTEM_TIME_H


#include <chrono>

namespace Engine::time {


	[[nodiscard]] inline auto now() noexcept {
		return std::chrono::steady_clock::now();
	}
	[[nodiscard]] inline auto difference(const auto startTime) noexcept {
		return Engine::time::now() - startTime;
	}



	[[nodiscard]] inline auto difference_as_nanoseconds(const auto startTime) noexcept {
		return std::chrono::duration_cast<std::chrono::nanoseconds>(Engine::time::difference(startTime));
	}
	[[nodiscard]] inline auto difference_as_milliseconds(const auto startTime) noexcept {
		return std::chrono::duration_cast<std::chrono::milliseconds>(Engine::time::difference(startTime));
	}
	[[nodiscard]] inline auto difference_as_seconds(const auto startTime) noexcept {
		return std::chrono::duration_cast<std::chrono::seconds>(Engine::time::difference(startTime));
	}
}

#endif