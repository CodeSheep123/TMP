#ifndef MVG_ARGPACK_UTILITY_HPP_
#define MVG_ARGPACK_UTILITY_HPP_

#include <cstddef> //std::size_t

namespace mvg
{

namespace detail
{

template<std::size_t, std::size_t, typename...>
struct argpack_elem_impl;

template<std::size_t I, typename Head, typename... Tail>
struct argpack_elem_impl<I, I, Head, Tail...>
{
	using type = Head;
};

template<std::size_t J, std::size_t I, typename Head, typename... Tail>
struct argpack_elem_impl<J, I, Head, Tail...>
{
	using type = typename argpack_elem_impl<J + 1, I, Tail...>::type;
};

} //namespace detail

template<typename... Ts>
struct argpack_size
{
	static constexpr std::size_t value = sizeof...(Ts);
};

template<typename... Ts>
static constexpr std::size_t argpack_size_v = argpack_size<Ts...>::value;

template<std::size_t I, typename Head, typename... Tail>
struct argpack_elem
{
	static_assert(I < argpack_size<Head, Tail...>::value, "Index out of range");
	using type = typename detail::argpack_elem_impl<0, I, Head, Tail...>::type;
};

template<std::size_t I, typename Head, typename... Tail>
using argpack_elem_t = typename argpack_elem<I, Head, Tail...>::type;

}

#endif