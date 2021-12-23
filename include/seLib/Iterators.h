#pragma once


namespace seLib { namespace Iterators {

template <typename Instance_T>
class index_iterator_t {
protected:
	Instance_T & mInstance;
	size_t mIndex;

public:
	constexpr index_iterator_t(Instance_T & instance, size_t index) : mInstance(instance), mIndex(index) {}
	constexpr index_iterator_t(index_iterator_t const &) = default;
	//constexpr index_iterator_t(index_iterator_t<Instance_T> &&) = delete;

	constexpr bool operator==(index_iterator_t const b) const {
		return (&mInstance == &b.mInstance) && (mIndex == b.mIndex);
	}

	constexpr bool operator!=(index_iterator_t const b) const {
		return !(*this == b);
	}

	constexpr bool operator<(index_iterator_t const b) const {
		return (&mInstance == &b.mInstance) && (mIndex < b.mIndex);
	}

	constexpr bool operator<=(index_iterator_t const b) const {
		return (&mInstance == &b.mInstance) && (mIndex <= b.mIndex);
	}

	constexpr bool operator>(index_iterator_t const b) const {
		return (&mInstance == &b.mInstance) && (mIndex > b.mIndex);
	}

	constexpr bool operator>=(index_iterator_t const b) const {
		return (&mInstance == &b.mInstance) && (mIndex >= b.mIndex);
	}

	index_iterator_t& operator++() {
		if (*this == mInstance.end())
			#if defined(__cpp_exceptions) && __cpp_exceptions==199711
				throw std::exception();
			#else
				abort();//return Identity<Return_T>();
			#endif

		mIndex++;
		return *this;
	}

	index_iterator_t operator++(int) {
		if (*this == mInstance.end())
			#if defined(__cpp_exceptions) && __cpp_exceptions==199711
				throw std::exception();
			#else
				abort();//return Identity<Return_T>();
			#endif

		index_iterator_t retval = *this;
		mIndex++;
		return retval;
	}

	index_iterator_t& operator--() {
		if (*this == mInstance.begin())
			#if defined(__cpp_exceptions) && __cpp_exceptions==199711
				throw std::exception();
			#else
				abort();//return Identity<Return_T>();
			#endif

		mIndex--;
		return *this;
	}

	index_iterator_t operator--(int) {
		if (*this == mInstance.end())
			#if defined(__cpp_exceptions) && __cpp_exceptions==199711
				throw std::exception();
			#else
				abort();//return Identity<Return_T>();
			#endif

		index_iterator_t retval = *this;
		mIndex--;
		return retval;
	}

	//constexpr bool IsConst(typename std::enable_if<std::is_const<Instance_T>::value>::type* = 0) const noexcept { return true; }
	constexpr bool IsConst() const noexcept { return std::is_const<Instance_T>::value; }
};

template <typename Instance_T>
class pointer_iterator_t {
protected:
	Instance_T * mPtr;

public:
	constexpr pointer_iterator_t(Instance_T * instance) : mPtr(instance) {}
	constexpr pointer_iterator_t(pointer_iterator_t const &) = default;
	//constexpr pointer_iterator_t(pointer_iterator_t<Instance_T> &&) = delete;

	constexpr bool operator==(pointer_iterator_t const b) const {
		return mPtr == b.mPtr;
	}

	constexpr bool operator!=(pointer_iterator_t const b) const {
		return !(*this == b);
	}

	constexpr bool operator<(pointer_iterator_t const b) const {
		return mPtr < b.mPtr;
	}

	constexpr bool operator<=(pointer_iterator_t const b) const {
		return mPtr <= b.mPtr;
	}

	constexpr bool operator>(pointer_iterator_t const b) const {
		return mPtr > b.mPtr;
	}

	constexpr bool operator>=(pointer_iterator_t const b) const {
		return mPtr >= b.mPtr;
	}


	pointer_iterator_t& operator++() {
		mPtr++;
		return *this;
	}

	pointer_iterator_t operator++(int) {
		pointer_iterator_t retval = *this;
		mPtr++;
		return retval;
	}

	pointer_iterator_t& operator--() {
		mPtr--;
		return *this;
	}

	pointer_iterator_t operator--(int) {
		pointer_iterator_t retval = *this;
		mPtr--;
		return retval;
	}

	//constexpr bool IsConst(typename std::enable_if<std::is_const<Instance_T>::value>::type* = 0) const noexcept { return true; }
	constexpr bool IsConst() const noexcept { return std::is_const<Instance_T>::value; }
};

template <typename Instance_T>
class sparse_pointer_iterator_t {
protected:
	Instance_T * mPtr;
	size_t Stride;

public:
	constexpr sparse_pointer_iterator_t(Instance_T * instance, size_t stride) : mPtr(instance), Stride(stride) {}
	constexpr sparse_pointer_iterator_t(sparse_pointer_iterator_t const &) = default;
	//constexpr pointer_iterator_t(pointer_iterator_t<Instance_T> &&) = delete;

	Instance_T& operator*() const { return *mPtr; }
	Instance_T* operator->() const { return mPtr; }

	constexpr bool operator==(sparse_pointer_iterator_t const b) const {
		return mPtr == b.mPtr && Stride == b.Stride;
	}

	constexpr bool operator!=(sparse_pointer_iterator_t const b) const {
		return !(*this == b);
	}

	constexpr bool operator<(sparse_pointer_iterator_t const b) const {
		return mPtr < b.mPtr;
	}

	constexpr bool operator<=(sparse_pointer_iterator_t const b) const {
		return mPtr <= b.mPtr;
	}

	constexpr bool operator>(sparse_pointer_iterator_t const b) const {
		return mPtr > b.mPtr;
	}

	constexpr bool operator>=(sparse_pointer_iterator_t const b) const {
		return mPtr >= b.mPtr;
	}

	sparse_pointer_iterator_t operator+(size_t count) const {
		return sparse_pointer_iterator_t<Instance_T>((Instance_T*)((uint8_t*)mPtr + Stride * count), Stride);
	}

	sparse_pointer_iterator_t operator-(size_t count) const {
		return sparse_pointer_iterator_t<Instance_T>((Instance_T*)((uint8_t*)mPtr - Stride * count), Stride);
	}

	sparse_pointer_iterator_t& operator+=(size_t count) {
		(uint8_t*&)mPtr += Stride * count;
		return *this;
	}

	sparse_pointer_iterator_t& operator-=(size_t count) {
		(uint8_t*&)mPtr -= Stride * count;
		return *this;
	}

	sparse_pointer_iterator_t& operator++() {
		(uint8_t*&)mPtr += Stride;
		return *this;
	}

	sparse_pointer_iterator_t operator++(int) {
		sparse_pointer_iterator_t retval = *this;
		(uint8_t*&)mPtr += Stride;
		return retval;
	}

	sparse_pointer_iterator_t& operator--() {
		(uint8_t*&)mPtr -= Stride;
		return *this;
	}

	sparse_pointer_iterator_t operator--(int) {
		sparse_pointer_iterator_t retval = *this;
		(uint8_t*&)mPtr -= Stride;
		return retval;
	}

	//constexpr bool IsConst(typename std::enable_if<std::is_const<Instance_T>::value>::type* = 0) const noexcept { return true; }
	constexpr bool IsConst() const noexcept { return std::is_const<Instance_T>::value; }
};

}}
