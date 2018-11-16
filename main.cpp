#include "type_list.hpp"

#include <type_traits>

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

TEST_CASE("argpack tests", "[argpack]")
{
	REQUIRE(std::is_same_v<mvg::argpack_elem_t<1, int, float, double>, float>);
}

template<typename T>
using not_integral = mvg::metafunc<std::is_integral>::negate<T>;

template<typename T>
using integr_or_float = typename mvg::metafunc<std::is_integral>::binary_op<std::is_floating_point>::logic_or<T>;

template<typename T>
using ref_to_ptr = typename mvg::metafunc<std::remove_reference>::binary_op<std::add_pointer>::then<T>;

TEST_CASE("metafunction utility tests", "[metafunc]")
{
	REQUIRE(not_integral<float>::value);
	REQUIRE(!(not_integral<int>::value));
	REQUIRE(integr_or_float<int>::value);
	REQUIRE(integr_or_float<float>::value);
	REQUIRE(!integr_or_float<std::string>::value);
	REQUIRE(std::is_same_v<typename ref_to_ptr<int&>::type, int*>);
}

TEST_CASE("Type list can map to stuff", "[tlist_map]")
{
	using start = mvg::type_list<int, float, double>;
	using tpl = std::tuple<int, float, double>;

	REQUIRE(std::is_same_v<typename start::template map<std::tuple>, tpl>);
}

TEST_CASE("Type lists can be filtered", "[tlist_filter]")
{
	using start = mvg::type_list<int, float, double, long, long long, long double>;
	using end = mvg::filter_if_t<start, std::is_integral>;

	REQUIRE(std::is_same_v<end, mvg::type_list<int, long, long long>>);

	using empty = mvg::filter_if_t<start, std::is_function>;

	REQUIRE(std::is_same_v<empty, mvg::empty_type_list>);
}

TEST_CASE("We can remove things from type lists", "[tlist_remove]")
{
	using start = mvg::type_list<int, float, double>;
	using end = typename start::template remove_at<1>;

	REQUIRE(std::is_same_v<end, mvg::type_list<int, double>>);
	
	using rm_if = typename start::template remove_if<std::is_integral>;

	REQUIRE(std::is_same_v<rm_if, mvg::type_list<float, double>>);

	using list = mvg::type_list<int, int, float, double, int, long, double, int, float>;
	using rm_all = typename list::template remove_if_same<int>;
	REQUIRE(std::is_same_v<rm_all, mvg::type_list<float, double, long, double, float>>);
}

template<class T>
void unused();

TEST_CASE("type_list elements can be replaced")
{
	using start = mvg::type_list<int, float, double>;
	using repl_idx = start::template replace_at<1, long>;

	REQUIRE(std::is_same_v<repl_idx, mvg::type_list<int, long, double>>);

	using repl_if = start::template replace_if<std::is_floating_point, unsigned int>;
	REQUIRE(std::is_same_v<repl_if, mvg::type_list<int, unsigned int, unsigned int>>);

	using too_long = mvg::type_list<int, int, float, int, double>;
	using repl_ints = too_long::template replace_if_same<int, long>;
	REQUIRE(std::is_same_v<repl_ints, mvg::type_list<long, long, float, long, double>>);
}

#include <vector>

//Currently doesn't work because template template parameters don't work with
//templates that take one argument and then a series of default arguments. 
//See https://coliru.stacked-crooked.com/a/75ef49223eaad8e6 for an example
// EDIT: Workaround found by @Toeger (parameter pack)

TEST_CASE("type_list apply_if")
{
	using start = mvg::type_list<int, float, double, long>;
	using apply_if = typename start::template apply_if<std::is_integral, std::vector>;

	REQUIRE(std::is_same_v<apply_if, mvg::type_list<std::vector<int>, float, double, std::vector<long>>>);
}

TEST_CASE("type_list map_if")
{
	using start = mvg::type_list<int, float, double, long>;
	using map_if = typename start::template map_if<std::is_integral, std::tuple>;

	REQUIRE(std::is_same_v<map_if, std::tuple<int, long>>);
}

TEST_CASE("type list ranges")
{
	using start = mvg::type_list<int, float, double, long>;
	using end = typename start::template range<0, 2>;

	REQUIRE(std::is_same_v<end, mvg::type_list<int, float, double>>);
}

TEST_CASE("Search through type lists")
{
	using start = mvg::type_list<int, float, double, long>;
	
	REQUIRE(mvg::find<start, double>::value == 2);
	REQUIRE(mvg::find<start, unsigned int>::value == start::npos);
}