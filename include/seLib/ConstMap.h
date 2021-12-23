#pragma once
///-------------------------------------------------------------------------------------------------
/// @file	ConstMap.h
/// @brief	Defines a constexpr map class.

/// \cond
#include <stdint.h>
#include <array>
#include <type_traits>
/// \endcond

#include "Sort.h"
#include "Iterators.h"

namespace seLib {

template <typename Key_T, typename Value_T, size_t COUNT> class ConstMap_t;

class ConstMapHelper_t {
public:
	template <typename Key_T, typename... Value_T, size_t COUNT = sizeof...(Value_T)>
	static constexpr auto Create(std::pair<Key_T, Value_T>...args) {
		return ConstMap_t<Key_T, std::common_type_t<Value_T...>, COUNT>(args...);
	}
};

template <typename Key_T, typename... Value_T>
ConstMap_t(std::pair<Key_T, Value_T>...) -> ConstMap_t<Key_T, std::common_type_t<Value_T...>, sizeof...(Value_T)>;

template <typename Key_T, typename Value_T, size_t COUNT=0>
class ConstMap_t : public ConstMapHelper_t {
public:
	using Self_T = ConstMap_t;//<Key_T, Value_T, COUNT>;
	using key_type = Key_T;
	using value_type = Value_T;
	using pair_type = std::pair<key_type, value_type>;

	template <typename Instance_T, typename IterValue_T>
	class iterator_t {
	protected:
		Instance_T & mInstance;
		IterValue_T * mValue_p;

	public:
		constexpr iterator_t(Instance_T & instance, IterValue_T * value) : mInstance(instance), mValue_p(value) {}
		constexpr iterator_t(iterator_t<Instance_T, IterValue_T> const &) = default;
		//constexpr iterator_t(iterator_t<Instance_T> &&) = delete;

		constexpr bool operator==(iterator_t<Instance_T, IterValue_T> const b) const {
			return (&mInstance == &b.mInstance) && (mValue_p == b.mValue_p);
		}
		
		constexpr bool operator!=(iterator_t<Instance_T, IterValue_T> const b) const {
			return !(*this == b);
		}

		iterator_t& operator++() {
			if (*this == mInstance.end())
				#if defined(__cpp_exceptions) && __cpp_exceptions==199711
					throw std::exception();
				#else
					abort();//return Identity<Return_T>();
				#endif

			mValue_p++;
			return *this;
		}

		iterator_t operator++(int) {
			if (*this == mInstance.end())
				#if defined(__cpp_exceptions) && __cpp_exceptions==199711
					throw std::exception();
				#else
					abort();//return Identity<Return_T>();
				#endif

			iterator_t retval = *this;
			mValue_p++;
			return retval;
		}

		iterator_t& operator--() {
			if (*this == mInstance.begin())
				#if defined(__cpp_exceptions) && __cpp_exceptions==199711
					throw std::exception();
				#else
					abort();//return Identity<Return_T>();
				#endif

			mValue_p--;
			return *this;
		}

		iterator_t operator--(int) {
			if (*this == mInstance.end())
				#if defined(__cpp_exceptions) && __cpp_exceptions==199711
					throw std::exception();
				#else
					abort();//return Identity<Return_T>();
				#endif

			iterator_t retval = *this;
			mValue_p--;
			return retval;
		}

		typename Instance_T::key_type const & key() const noexcept {
			return mInstance.mKeys[mValue_p - mInstance.mValues.begin()];
		}
		
		IterValue_T& value() const noexcept {
			return *mValue_p;
		}
		
		//constexpr bool IsConst(typename std::enable_if<std::is_const<Instance_T>::value>::type* = 0) const noexcept { return true; }
		constexpr bool IsConst() const noexcept { return std::is_const<Instance_T>::value; }
	};

	using iterator = iterator_t<Self_T, value_type>;
	using const_iterator = iterator_t<Self_T const, value_type const>;

protected:
	std::array<Key_T, COUNT> mKeys = { };
	std::array<Value_T, COUNT> mValues = { };

public:
	constexpr ConstMap_t(std::array<pair_type, COUNT> const & pairs) : mKeys({(Key_T)0}), mValues({(Value_T)0}) {
		auto remap = seLib::Sort::SortIndex(pairs);

		for(size_t i = 0; i < COUNT; i++) {
			auto from_i = remap[i];
			mKeys[i] = pairs[from_i].first;
			mValues[i] = pairs[from_i].second;
		}
	}

	template <typename Key_T2, typename Value_T2>
	constexpr ConstMap_t(std::pair<Key_T2, Value_T2> arg) :
		mKeys({ arg.first }), mValues({ arg.second })
	{}

	template <typename Key_T2, typename... Value_T2>
	constexpr ConstMap_t(std::pair<Key_T2, Value_T2>...args) :
		//mKeys({(Key_T)0}), mValues({(Value_T)0})
		mKeys(std::array<key_type, COUNT> {(args.first)...}),
		mValues(std::array<value_type, COUNT> {(args.second)...})
	{}

	constexpr ConstMap_t(key_type key, value_type value) : mKeys({key}), mValues({value}) {}

	constexpr ConstMap_t(ConstMap_t<key_type, value_type, COUNT-1> const & b, key_type key, value_type value) {
		for (size_t i = 0; i < (COUNT - 1); i++) {
			mKeys[i] = b.keys[i];
			mValues[i] = b.values[i];
		}
		mKeys[COUNT-1] = key;
		mValues[COUNT-1] = value;
	}

	/*template <typename Key_T2, typename... Value_T2>
	constexpr ConstMap_t(std::pair<Key_T2, Value_T2>...args) :
		//mKeys({(Key_T)0}), mValues({(Value_T)0})
		mKeys(seLib::Sort::MapIndex(seLib::Sort::SortIndex(std::array {(args.first)...}), std::array {(args.first)...})),
		mValues(seLib::Sort::MapIndex(seLib::Sort::SortIndex(std::array {(args.first)...}), std::array {(args.second)...}))
	{
		std::array keys {(args.first)...};
		std::array values {(args.second)...};
		auto remap = seLib::Sort::SortIndex(keys);
		for(size_t i = 0; i < COUNT; i++) {
			auto from_i = remap[i];
			mKeys[i] = keys[from_i];
			mValues[i] = values[from_i];
		}
	}// */

	constexpr ConstMap_t(std::array<Key_T, COUNT> const & keys, std::array<Value_T, COUNT> const & values) :
		mKeys({(Key_T)0}), mValues({(Value_T)0})
	{
		auto remap = seLib::Sort::SortIndex(mKeys);

		for(size_t i = 0; i < COUNT; i++) {
			auto from_i = remap[i];
			mKeys[i] = keys[from_i];
			mValues[i] = values[from_i];
		}
	}

	constexpr ConstMap_t(Self_T const &) = default;
	constexpr ConstMap_t(Self_T &&) = default;
	Self_T & operator=(Self_T const &) = default;
	Self_T & operator=(Self_T &&) = default;

	constexpr size_t size() const noexcept {
		return COUNT;
	}
	
	iterator begin() noexcept {
		return iterator(*this, mValues.data());
	}

	constexpr const_iterator begin() const noexcept {
		return const_iterator(*this, mValues.data());
	}

	iterator end() noexcept {
		static_assert(!std::is_const<decltype(*this)>::value);
		return iterator(*this, mValues.data() + COUNT);
	}// */

	constexpr const_iterator end() const noexcept {
		return const_iterator(*this, mValues.data() + COUNT);
	}
	
	iterator find(key_type const & key) noexcept {
		for (auto iter = mKeys.begin(); iter != mKeys.end(); iter++) {
			if (*iter >= key)
				return iterator(*this, mValues.data() + (iter - mKeys.begin()));
		}
		return end();
	}// */
	
	constexpr const_iterator find(key_type const & key) const noexcept {
		for (auto iter = mKeys.begin(); iter != mKeys.end(); iter++) {
			if (*iter >= key)
				return const_iterator(*this, mValues.data() + (iter - mKeys.begin()));
		}
		return end();
	}

	iterator find_value(value_type const & value) noexcept {
		for (auto iter = mValues.begin(); iter != mValues.end(); iter++) {
			if (*iter == value)
				return iterator(*this, mValues.data() + (iter - mValues.begin()));
		}
		return end();
	}// */

	constexpr const_iterator find_value(value_type const & value) const noexcept {
		for (auto iter = mValues.begin(); iter != mValues.end(); iter++) {
			if (*iter == value)
				return const_iterator(*this, mValues.data() + (iter - mValues.begin()));
		}
		return end();
	}

	value_type & operator[](key_type const & key) {
		return find(key).value();
	}

	constexpr value_type const & operator[](key_type const & key) const {
		return find(key).value();
	}

	constexpr bool IsConst() noexcept { return false; }
	constexpr bool IsConst() const noexcept { return true; }
};

template <typename Key_T, typename Value_T, size_t COUNT>
ConstMap_t(std::array<std::pair<Key_T, Value_T>, COUNT> const &) -> ConstMap_t<Key_T, Value_T, COUNT>;

//template <typename Key_T, typename Value_T, size_t COUNT>
//ConstMap_t(std::array<std::pair<Key_T, Value_T>, COUNT> const &) -> ConstMap_t <Key_T, Value_T, COUNT>;

//template <typename Key_T, typename Value_T, typename... ARGS_T>
//ConstMap_t(std::pair<Key_T, Value_T>, ARGS_T...) -> ConstMap_t<std::enable_if_t<(std::is_same_v<std::pair<Key_T, Value_T>, ARGS_T> && ...), Key_T>, Value_T, 1 + sizeof...(ARGS_T)>;


template <typename Key_T, typename Value_T>
ConstMap_t(std::pair<Key_T, Value_T>) -> ConstMap_t<Key_T, Value_T, 1>;

template <typename Key_T, typename Value_T, typename... Value_T2>
ConstMap_t(std::pair<Key_T, Value_T>, std::pair<Key_T, Value_T2>...) -> ConstMap_t<Key_T, std::common_type_t<Value_T, Value_T2...>, sizeof...(Value_T2) + 1>;

//template <typename Key_T, typename... Value_T>
//ConstMap_t<Key_T, std::common_type_t<Value_T...>, sizeof...(Value_T)> To_ConstMap(std::pair<Key_T, Value_T>...args) {
//	return ConstMap_t<Key_T, std::common_type_t<Value_T...>, sizeof...(Value_T)> {
//		std::array<Key_T, sizeof...(Value_T)>((args.first)...),
//		std::array<std::common_type_t<Value_T...>, sizeof...(Value_T)>((args.second)...)
//	};
//}
// 
template <typename Key_T, typename Value_T>
ConstMap_t(Key_T, Value_T) -> ConstMap_t<Key_T, Value_T, 1>;

template <typename Key_T, typename Value_T, size_t N>
ConstMap_t(ConstMap_t<Key_T, Value_T, N> const &, Key_T, Value_T) -> ConstMap_t<Key_T, Value_T, N + 1>;


template <typename Enum_T, typename Value_T, size_t COUNT>
class ConstEnumMap_t {
public:
	typedef ConstEnumMap_t<Enum_T, Value_T, COUNT> Self_T;
	typedef Enum_T key_type;
	typedef Value_T value_type;
	typedef std::pair<key_type, value_type> pair_type;

	template <typename Instance_T, typename IterValue_T>
	class iterator_t {
	protected:
		Instance_T & mInstance;
		IterValue_T * mValue_p;

	public:
		constexpr iterator_t(Instance_T & instance, IterValue_T * value) : mInstance(instance), mValue_p(value) {}
		constexpr iterator_t(iterator_t<Instance_T, IterValue_T> const &) = default;
		//constexpr iterator_t(iterator_t<Instance_T> &&) = delete;

		constexpr bool operator==(iterator_t<Instance_T, IterValue_T> const b) const {
			return (&mInstance == &b.mInstance) && (mValue_p == b.mValue_p);
		}
		
		constexpr bool operator!=(iterator_t<Instance_T, IterValue_T> const b) const {
			return !(*this == b);
		}

		typename Instance_T::key_type const & key() const noexcept {
			return mInstance.mKeys[mValue_p - mInstance.mValues.begin()];
		}
		
		IterValue_T& value() const noexcept {
			return *mValue_p;
		}
		
		//constexpr bool IsConst(typename std::enable_if<std::is_const<Instance_T>::value>::type* = 0) const noexcept { return true; }
		constexpr bool IsConst() const noexcept { return std::is_const<Instance_T>::value; }
	};

	using iterator = iterator_t<Self_T, value_type>;
	using const_iterator = iterator_t<Self_T const, value_type const>;

protected:
	std::array<Value_T, COUNT> mValues = { (Value_T)0 };

public:
	constexpr ConstEnumMap_t(std::array<pair_type, COUNT> const & pairs) : mValues({(Value_T)0}) {
		for(size_t i = 0; i < COUNT; i++) {
			if constexpr(true)
				static_assert((size_t)pairs[i].first < mValues.size());
			else
				assert((size_t)pairs[i].first < mValues.size());
			mValues[(size_t)pairs[i].first] = pairs[i].second;
		}
	}

	constexpr ConstEnumMap_t(Self_T const &) = default;
	constexpr ConstEnumMap_t(Self_T &&) = default;
	Self_T & operator=(Self_T const &) = default;
	Self_T & operator=(Self_T &&) = default;

	constexpr size_t size() const noexcept {
		return COUNT;
	}
	
	iterator begin() noexcept {
		return iterator(*this, mValues.data());
	}

	constexpr const_iterator begin() const noexcept {
		return const_iterator(*this, mValues.data());
	}

	iterator end() noexcept {
		static_assert(!std::is_const<decltype(*this)>::value);
		return iterator(*this, mValues.data() + COUNT);
	}// */

	constexpr const_iterator end() const noexcept {
		return const_iterator(*this, mValues.data() + COUNT);
	}
	
	iterator find(key_type const & key) noexcept {
		for (auto iter = mValues.begin(); iter != mValues.end(); iter++) {
			if (*iter == key)
				return iterator(*this, mValues.data() + (iter - mValues.begin()));
		}
		return end();
	}// */

	constexpr const_iterator find(key_type const & key) const noexcept {
		for (auto iter = mValues.begin(); iter != mValues.end(); iter++) {
			if (*iter == key)
				return const_iterator(*this, mValues.data() + (iter - mValues.begin()));
		}
		return end();
	}

	iterator find_value(value_type const & value) noexcept {
		for (auto iter = mValues.begin(); iter != mValues.end(); iter++) {
			if (*iter == value)
				return iterator(*this, mValues.data() + (iter - mValues.begin()));
		}
		return end();
	}// */

	constexpr const_iterator find_value(value_type const & value) const noexcept {
		for (auto iter = mValues.begin(); iter != mValues.end(); iter++) {
			if (*iter == value)
				return const_iterator(*this, mValues.data() + (iter - mValues.begin()));
		}
		return end();
	}

	value_type & operator[](key_type const & key) {
		return find(key).value();
	}

	constexpr value_type const & operator[](key_type const & key) const {
		return find(key).value();
	}

	constexpr bool IsConst() noexcept { return false; }
	constexpr bool IsConst() const noexcept { return true; }
};

template <typename Key_T, typename Value_T, size_t COUNT>
ConstEnumMap_t(std::array<std::pair<Key_T, Value_T>, COUNT> const &) -> ConstEnumMap_t<Key_T, Value_T, COUNT>;

//template <typename Key_T, typename Value_T, size_t COUNT>
//ConstMap_t(std::array<std::pair<Key_T, Value_T>, COUNT> const &) -> ConstMap_t <Key_T, Value_T, COUNT>;

template <typename Key_T, typename Value_T, typename... ARGS_T>
ConstEnumMap_t(std::pair<Key_T, Value_T>, ARGS_T...) -> ConstEnumMap_t<std::enable_if_t<(std::is_same_v<std::pair<Key_T, Value_T>, ARGS_T> && ...), Key_T>, Value_T, 1 + sizeof...(ARGS_T)>;


}
