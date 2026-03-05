#pragma once

#ifdef _WIN32
#include <ppl.h>
#include <concurrent_unordered_map.h>
#include <concurrent_vector.h>
#else
#include <tbb/task_group.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>
#include <tbb/parallel_sort.h>
#endif
#include <atomic>

// Atomic types
using xr_atomic_u32 = std::atomic_uint32_t;
using xr_atomic_s32 = std::atomic_int;
using xr_atomic_bool = std::atomic_bool;

// Tasks Redefinition
#ifdef _WIN32
using xr_task_group = concurrency::task_group;

template <typename T, typename U>
using xr_concurrent_unordered_map = concurrency::concurrent_unordered_map<T, U>;

template <typename T, typename allocator = xalloc<T>>
using xr_concurrent_vector = concurrency::concurrent_vector<T, allocator>;

template<typename BlockRangeType, typename Body>
inline void xr_parallel_for(BlockRangeType Begin, BlockRangeType End, Body Functor)
{
	concurrency::parallel_for(Begin, End, Functor);
}

template<typename Index, typename Body>
inline void xr_parallel_foreach(Index Begin, Index End, Body Functor)
{
	concurrency::parallel_for_each(Begin, End, Functor);
}

// PPL behaviour - fallback to std::sort if chunk size < 2048 and cores < 2
template<typename Data, typename Body>
inline void xr_parallel_sort(Data& data, Body functor)
{
	concurrency::parallel_sort(std::begin(data), std::end(data), functor);
}
#else
using xr_task_group = tbb::task_group;

template <typename T, typename U>
using xr_concurrent_unordered_map = tbb::concurrent_unordered_map<T, U>;

template <typename T, typename allocator = xalloc<T>>
using xr_concurrent_vector = tbb::concurrent_vector<T, allocator>;

template<typename BlockRangeType, typename Body>
inline void xr_parallel_for(BlockRangeType Begin, BlockRangeType End, Body Functor)
{
	tbb::parallel_for(Begin, End, Functor);
}

template<typename Index, typename Body>
inline void xr_parallel_foreach(Index Begin, Index End, Body Functor)
{
	tbb::parallel_for_each(Begin, End, Functor);
}

template<typename Data, typename Body>
inline void xr_parallel_sort(Data& data, Body functor)
{
	tbb::parallel_sort(std::begin(data), std::end(data), functor);
}
#endif

template<typename Data, typename Body>
inline void xr_sort(Data& data, Body functor)
{
	std::sort(std::begin(data), std::end(data), functor);
}

template<typename Data, typename Body>
inline void xr_stable_sort(Data& data, Body functor)
{
	std::stable_sort(std::begin(data), std::end(data), functor);
}
