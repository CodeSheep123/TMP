#ifndef MVG_META_FUNCS_HPP_
#define MVG_META_FUNCS_HPP_

#include <type_traits>

namespace mvg
{

namespace detail
{

template<typename T>
struct is_same_wrapper
{
	template<typename U>
	struct is_same : public std::is_same<T, U>
	{

	};
};

} //namespace detail

template<template<typename> typename F>
struct metafunc
{
	//Only if F is a boolean metafunction (has a ::value bool member)
	template<typename T>
	struct negate
	{
		static constexpr bool value = !(F<T>::value);
	};

	template<template<typename> typename Other>
	struct binary_op
	{
		template<typename T>
		struct logic_or
		{
			static constexpr bool value = F<T>::value || Other<T>::value;
		};

		template<typename T>
		struct logic_and
		{
			static constexpr bool value = F<T>::value && Other<T>::value;
		};

		template<typename T>
		struct after
		{
			using type = typename F<typename Other<T>::type>::type;
		};

		template<typename T>
		using after_t = typename after<T>::type;

		template<typename T>
		struct then
		{
			using type = typename Other<typename F<T>::type>::type;
		};

		template<typename T>
		using then_t = typename then<T>::type;
	};
};

}

#endif