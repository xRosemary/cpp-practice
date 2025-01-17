#pragma once
#include <tuple>

template<typename ...Types>
constexpr auto Create(const Types... Args)
{
	return std::make_tuple(Args...);
}

template<typename T, typename ...Types>
constexpr auto Push(T Elem, const std::tuple<Types...> container)
{
	return std::tuple_cat(std::make_tuple(Elem), container);
}

template <typename T, typename... Args, std::size_t... I>
constexpr auto PopImpl(const std::tuple<T, Args...>& container, std::index_sequence<I...>)
{
	return std::make_tuple(std::get<I + 1>(container)...);
}

template <typename T, typename... Args>
constexpr auto Pop(const std::tuple<T, Args...>& container)
{
	return PopImpl(container, std::make_index_sequence<sizeof...(Args)>{});
}