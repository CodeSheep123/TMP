#ifndef MVG_TYPE_LIST_HPP_
#define MVG_TYPE_LIST_HPP_

#include <tuple>
#include <type_traits>

#include "argpack_util.hpp"
#include "metafuncs.hpp"

namespace mvg
{

template<typename...>
class type_list
{
public:
	template<typename T>
	using push_back = type_list<T>;

	template<typename T>
	using push_front = type_list<T>;
	
	static constexpr std::size_t size = 0;
	static constexpr std::size_t npos = static_cast<std::size_t>(-1);
	static constexpr bool empty = true;
};

using empty_type_list = type_list<>;

/*TYPE LIST OPERATIONS*/

template<typename, typename>
struct concat;

template<typename... Ts, typename... Us>
struct concat<type_list<Ts...>, type_list<Us...>>
{
	using type = type_list<Ts..., Us...>;
};

template<typename T, typename U>
using concat_t = typename concat<T, U>::type;

namespace detail
{

template<std::size_t, std::size_t, typename, typename>
struct splice_front_impl;

template<std::size_t I, typename Head, typename... Tail, typename... Res>
struct splice_front_impl<I, I, type_list<Head, Tail...>, type_list<Res...>>
{
	using type = type_list<Res...>;
};

template<std::size_t I, std::size_t J, typename Head, typename... Tail, typename... Res>
struct splice_front_impl<I, J, type_list<Head, Tail...>, type_list<Res...>>
{
	using type = typename splice_front_impl<I, J + 1, type_list<Tail...>, type_list<Res..., Head>>::type;
};

} //namespace detail

//Splices type list, returning a type list containing all types before the index (not included)
template<std::size_t I, typename>
struct splice_front;

template<std::size_t I, typename Head, typename... Tail>
struct splice_front<I, type_list<Head, Tail...>>
{
	using type = typename detail::splice_front_impl<I, 0, type_list<Head, Tail...>, empty_type_list>::type;
};

template<std::size_t I, typename T>
using splice_front_t = typename splice_front<I, T>::type;

namespace detail
{

template<std::size_t, std::size_t, typename, typename>
struct splice_back_impl;

template<std::size_t I, typename Head, typename... Tail, typename... Res>
struct splice_back_impl<I, I, type_list<Head, Tail...>, type_list<Res...>>
{
	using type = type_list<Tail...>;
};

template<std::size_t I, std::size_t J, typename Head, typename... Tail, typename... Res>
struct splice_back_impl<I, J, type_list<Head, Tail...>, type_list<Res...>>
{
	using type = typename splice_back_impl<I, J + 1, type_list<Tail...>, type_list<Res...>>::type;
};

} //namespace detail

//Splices type list, returning a type list containing all types after the index (not included)
template<std::size_t I, typename>
struct splice_back;

template<std::size_t I, typename Head, typename... Tail>
struct splice_back<I, type_list<Head, Tail...>>
{
	using type = typename detail::splice_back_impl<I, 0, type_list<Head, Tail...>, empty_type_list>::type;
};

template<std::size_t I, typename T>
using splice_back_t = typename splice_back<I, T>::type;

namespace detail
{
template<std::size_t I, template<typename> typename Pred, typename TList, typename Res>
struct filter_if_impl;

template<template<typename> typename Pred, typename... Ts, typename... Res>
struct filter_if_impl<1, Pred, type_list<Ts...>, type_list<Res...>>
	//specialization for last element to end recursion (or fist, whatever, we are iterating backwards here)
{
	using T = typename type_list<Ts...>::template at<type_list<Ts...>::size - 1>;
	using type = std::conditional_t<
		Pred<T>::value,
		typename type_list<Res...>::template push_back<T>,
		type_list<Res...>>;
};

template<std::size_t I, template<typename> typename Pred, typename... Ts, typename... Res>
struct filter_if_impl<I, Pred, type_list<Ts...>, type_list<Res...>>
{
	using T = typename type_list<Ts...>::template at<type_list<Ts...>::size - I>;
	using type = std::conditional_t<Pred<T>::value,
		typename filter_if_impl<
		I - 1, Pred, type_list<Ts...>,
		typename type_list<Res...>::template push_back<T>>::type, //true? add type and continue recursion
		typename filter_if_impl<
		I - 1, Pred, type_list<Ts...>,
		type_list<Res...>>::type //false? don't add type and continue recursion
		>;
};

} //namespace detail

template<typename TList, template<typename> typename Pred>
struct filter_if
{
	using type = typename detail::filter_if_impl<TList::size, Pred, TList, empty_type_list>::type;
};

template<typename TList, template<typename> typename Pred>
using filter_if_t = typename filter_if<TList, Pred>::type;

//Removes duplicates from a type list
template<typename TList>
struct condense
{
	//#TODO
	static_assert(std::is_same_v<TList, int>, "NOT IMPLEMENTED");
};

namespace detail
{

	template<std::size_t I, typename TList, template<typename> typename Pred>
	struct find_if_impl
	{
		using Elem = typename TList::template at<TList::size - I>;

		static constexpr std::size_t value = Pred<Elem>::value ? TList::size - I : find_if_impl<I - 1, TList, Pred>::value;
	};

	template<typename TList, template<typename> typename Pred>
	struct find_if_impl<1, TList, Pred>
	{
		using Elem = typename TList::template at<TList::size - 1>;

		static constexpr std::size_t value = Pred<Elem>::value ? 0 : TList::npos;
	};

}

template<typename TList, typename T>
struct find
{
	static constexpr std::size_t value = detail::find_if_impl<
		TList::size, TList,
		detail::is_same_wrapper<T>::template is_same>::value;
};

template<typename TList, template<typename> typename Pred>
struct find_if
{
	static constexpr std::size_t value = detail::find_if_impl<
		TList::size, TList, Pred>::value;
};

template<typename TList, typename T>
static constexpr bool find_v = find<TList, T>::value;

template<typename TList, template<typename> typename Pred>
static constexpr bool find_if_v = find_if<TList, Pred>::value;

/*CLASS DECLARATION FOR CLASS: type_list (SPECIALIZATION WITH AT LEAST 1 ELEMENT)*/

template<typename Head, typename... Tail>
class type_list<Head, Tail...>
{
private:

	template<typename, typename, typename, std::size_t>
	struct insert_impl;

	template<std::size_t I, typename T, typename RFirst, typename... Prepend, typename... Rest>
	struct insert_impl<type_list<Prepend...>, type_list<RFirst, Rest...>, T, I>
	{
		using type = typename insert_impl<type_list<Prepend..., RFirst>, type_list<Rest...>, T, I - 1>::type;
	};

	template<typename T, typename... Prepend, typename RFirst, typename... Rest>
	struct insert_impl<type_list<Prepend...>, type_list<RFirst, Rest...>, T, 0>
	{
		using type = type_list<Prepend..., T, RFirst, Rest...>;
	};

	template<std::size_t I, std::size_t J, typename TList>
	struct remove_idx_impl
	{
		using Elem = typename TList::template at<J>;

		using type = concat_t<typename remove_idx_impl<I, J - 1, TList>::type,
			std::conditional_t<I == J, empty_type_list, type_list<Elem>>>;
	};

	template<std::size_t I, typename TList>
	struct remove_idx_impl<I, 0, TList>
	{
		using Elem = typename TList::template at<0>;
		using type = std::conditional_t<I == 0, empty_type_list, type_list<Elem>>;
	};

	template<std::size_t I, std::size_t J, typename TList, typename T>
	struct replace_impl
	{
		using Elem = typename TList::template at<J>;

		using type = concat_t<typename replace_impl<I, J - 1, TList, T>::type,
			typename std::conditional_t<I == J,
			type_list<T>,
			type_list<Elem>>>;
			
	};

	template<std::size_t I, typename TList, typename T>
	struct replace_impl<I, 0, TList, T>
	{
		using Elem = typename TList::template at<0>;
		using type = std::conditional_t<I == 0,
			type_list<T>,
			type_list<Elem>>;
	};

	template<std::size_t J, typename TList, template<typename> typename Pred, typename T>
	struct replace_if_impl
	{
		using Elem = typename TList::template at<J>;

		using type = concat_t<typename replace_if_impl<J - 1, TList, Pred, T>::type,
			typename std::conditional_t<Pred<Elem>::value,
				type_list<T>,
				type_list<Elem>>>;
	};

	template<typename TList, template<typename> typename Pred, typename T>
	struct replace_if_impl<0, TList, Pred, T>
	{
		using Elem = typename TList::template at<0>;

		using type = std::conditional_t<Pred<Elem>::value,
			type_list<T>,
			type_list<Elem>>;
	};

	template<std::size_t I, std::size_t J, typename TList>
	struct range_impl
	{
		using type = typename range_impl<I + 1, J, TList>::type::template push_front<typename TList::template at<I>>;
	};

	template<std::size_t I, typename TList>
	struct range_impl<I, I, TList>
	{
		using type = type_list<typename TList::template at<I>>;
	};

public:
	using this_type = type_list<Head, Tail...>;

	static constexpr std::size_t size = argpack_size<Head, Tail...>::value;
	static constexpr std::size_t npos = static_cast<std::size_t>(-1);

	static constexpr bool empty = false;
	
	template<typename T>
	static constexpr bool has_type = find<this_type, T>::value != npos;

	using tuple_t = std::tuple<Head, Tail...>;
	
	template<typename T>
	using push_back = type_list<Head, Tail..., T>;

	template<typename T>
	using push_front = type_list<T, Head, Tail...>;

	template<std::size_t I, typename T>
	using insert = typename insert_impl<type_list<Head>, type_list<Tail...>, T, I>::type;

	template<std::size_t I>
	using remove_at = typename remove_idx_impl<I, size - 1, this_type>::type;

	template<template<typename> typename Pred>
	using remove_if = typename filter_if<this_type, metafunc<Pred>::template negate>::type;

	template<typename T>
	using remove_if_same = remove_if<detail::is_same_wrapper<T>::template is_same>;

	template<std::size_t I, typename T>
	using replace_at = typename replace_impl<I, size - 1, this_type, T>::type;

	template<template<typename> typename Pred, typename T>
	using replace_if = typename replace_if_impl<size - 1, this_type, Pred, T>::type;

	template<typename R, typename With>
	using replace_if_same = replace_if<detail::is_same_wrapper<R>::template is_same, With>;

	//Inclusive range
	template<std::size_t I, std::size_t J>
	using range = typename range_impl<I, J, this_type>::type;

	template<std::size_t I>
	using at = argpack_elem_t<I, Head, Tail...>;

	template<std::size_t I>
	using get = at<I>;

	template<typename... Ts>
	using append = type_list<Head, Tail..., Ts...>;

	template<typename... Ts>
	using append_front = type_list<Ts..., Head, Tail...>;

	template<template<typename...> typename Func>
	using apply = type_list<Func<Head>, Func<Tail>...>;

	//Currently doesn't work because template template parameters don't work with
	//templates that take one argument and then a series of default arguments. 
	//See https://coliru.stacked-crooked.com/a/75ef49223eaad8e6 for an example
	//
	//EDIT: A workaround was found by @Toeger, just make it a parameter pack

	template<template<typename> typename Pred, template<typename...> typename Func>
	using apply_if = type_list<
		std::conditional_t<Pred<Head>::value, Func<Head>, Head>,
		std::conditional_t<Pred<Tail>::value, Func<Tail>, Tail>...>;

	template<template<typename...> typename Func>
	using map = Func<Head, Tail...>;

	template<template<typename> typename Pred, template<typename...> typename Func>
	using map_if = typename filter_if_t<this_type, Pred>::template map<Func>;
};

}

#endif