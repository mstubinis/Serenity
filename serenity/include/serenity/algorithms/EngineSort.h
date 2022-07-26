#pragma once
#ifndef ENGINE_ALGORITHMS_ENGINE_SORT_H
#define ENGINE_ALGORITHMS_ENGINE_SORT_H

//attempts to parallelize using the engine's own thread pool
#include <algorithm>
#include <execution>
#include <serenity/threading/ThreadPool.h>
#include <serenity/system/Engine.h>

namespace Engine::priv::detail {
	constexpr auto median_of_three(auto itrLeft, auto itrRight, auto&& compare) {
		const auto distance = std::distance(itrLeft, itrRight) / 2;
	    auto mid = itrLeft;
		std::advance(mid, distance);
		if (!compare(*itrLeft, *mid)) {
			std::iter_swap(itrLeft, mid);
		}
		if (!compare(*itrLeft, *itrRight)) {
			std::iter_swap(itrLeft, itrRight);
		}
		if (!compare(*mid, *itrRight)) {
			std::iter_swap(mid, itrRight);
		}
		return mid;
	}
	constexpr auto partition_hoare(auto left, auto right, auto&& compare) {
		right = std::prev(right);
		const auto pivot = *median_of_three(left, right, compare);
		while (compare(*left, pivot)) {
			++left;
		}
		while (compare(pivot, *right)) {
			--right;
		}
		while (left < right) {
			std::iter_swap(left, right);
			++left;
			while (compare(*left, pivot)) {
				++left;
			}
			--right;
			while (compare(pivot, *right)) {
				--right;
			}
		}
		return std::make_pair(right + 1, right + 1);
	}
	constexpr void quicksort_parallel_impl(auto* threadpool, auto first, auto last, auto&& compare, size_t threshold) {
		const auto size = std::distance(first, last);
		using diff_t    = decltype(size);
		using compare_t = decltype(compare);
		if (size > diff_t(1)) {
			const auto bounds = partition_hoare(first, last, std::forward<compare_t>(compare));
			auto job1 = [threadpool, first, b = bounds.first, c = std::forward<compare_t>(compare), threshold]() {
				quicksort_parallel_impl(threadpool, first, b, c, threshold);
			};
			auto job2 = [threadpool, last, b = bounds.second, c = std::forward<compare_t>(compare), threshold]() {
				quicksort_parallel_impl(threadpool, b, last, c, threshold);
			};
			if (std::distance(first, bounds.first) > diff_t(threshold)) {
				threadpool->add_job(std::move(job1));
			} else {
				job1();
			}
			if (std::distance(bounds.second, last) > diff_t(threshold)) {
				threadpool->add_job(std::move(job2));
			} else {
				job2();
			}
		}
	}
	constexpr void quicksort_parallel(auto first, auto last, auto compare, size_t parallelThreshold) {
		using compare_t = decltype(compare);
		auto& threadpool = Engine::priv::Core::m_Engine->m_ThreadingModule.m_ThreadPool;
		quicksort_parallel_impl(&threadpool, first, last, std::forward<compare_t>(compare), parallelThreshold);
		threadpool.wait_for_all();
	}
}

void engine_sort(auto left, auto right, auto&& compare) {
	std::sort(left, right, std::forward<decltype(compare)>(compare));
}
void engine_sort(auto executionPolicy, auto left, auto right, auto&& compare, size_t parallelThreshold = 100) {
	using execution_t = decltype(executionPolicy);
	using compare_t = decltype(compare);
	if constexpr(std::is_same_v<std::execution::sequenced_policy, execution_t> || std::is_same_v<std::execution::unsequenced_policy, execution_t>) {
		engine_sort(left, right, compare);
	} else {
		Engine::priv::detail::quicksort_parallel(left, right, std::forward<compare_t>(compare), parallelThreshold);
	}
}

#endif