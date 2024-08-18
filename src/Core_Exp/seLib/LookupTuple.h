#pragma once

#include <tuple>
#include <array>

namespace seLib {

//================================================================================================



template <typename Lookup_T, typename ...Args_T>
class LookupTuple_t {
public:
	static constexpr size_t size() { return sizeof...(Args_T); }

	std::tuple<Args_T...> Data;
	std::array<Lookup_T*, sizeof...(Args_T)> LookupTable;

	template <typename NewArg_T>
	inline constexpr LookupTuple_t< Lookup_T, Args_T..., NewArg_T> append(NewArg_T const& arg) const noexcept {
	}

	template <typename ...Args2_T>
	static constexpr auto Build(Args2_T...args) {
		return LookupTuple_t<Lookup_T, Args2_T...>{args...};
	}

	template <size_t Index_N>
	constexpr auto MakeTupleLookup2() const noexcept {
		std::array<Lookup_T*, sizeof...(Args_T) - Index_N> arry{};
		size_t index = 0;
		arry[0] = (Lookup_T*)&std::get<Index_N>(Data);
		if constexpr ((Index_N + 1) < sizeof...(Args_T))
			for (auto ptr : MakeTupleLookup2<Index_N + 1>())
				arry[++index] = ptr;

		return arry;
	}


	template <size_t Size_N = size()>
	constexpr std::array<Lookup_T*, sizeof...(Args_T)> MakeTupleLookup() {
		if constexpr (Size_N == 0)
			return std::array<Lookup_T*, 0> {};
		else
			return MakeTupleLookup2<0>();
	}

	constexpr LookupTuple_t(Args_T...args)
		: Data(args...),
		LookupTable(MakeTupleLookup())
	{}

	constexpr LookupTuple_t(LookupTuple_t const & b)
		: Data(b.Data),
		LookupTable(MakeTupleLookup())
	{}

	constexpr Lookup_T& operator[](size_t index) {
		return *LookupTable[index];
	}

	constexpr Lookup_T const & operator[](size_t index) const {
		return *LookupTable[index];
	}
};



struct LookupTupleTest1_t {
	size_t i1{ 0 };
};
struct LookupTupleTest2_t : LookupTupleTest1_t {
	size_t i2{ 2 };
	constexpr LookupTupleTest2_t() : LookupTupleTest1_t({ 1 }) {}
};

static_assert(LookupTuple_t<LookupTupleTest1_t, LookupTupleTest1_t, LookupTupleTest2_t>{ LookupTupleTest1_t{0}, LookupTupleTest2_t{}}[0].i1 == 0, "fail");
static_assert(LookupTuple_t<LookupTupleTest1_t, LookupTupleTest1_t, LookupTupleTest2_t>{ LookupTupleTest1_t{ 0 }, LookupTupleTest2_t{}}[1].i1 == 1, "fail");

}
