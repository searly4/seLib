#pragma once


namespace seLib { namespace Iterators {

template <typename Container_t, typename Instance_T>
class IndexIterator_t : public IndexIterator_t<Container_t, void> {
public:
	constexpr IndexIterator_t(Container_t& instance, size_t index)
		:IndexIterator_t<Container_t, void>(instance, index)
	{}

	constexpr IndexIterator_t(IndexIterator_t const&) = default;
	IndexIterator_t& operator=(IndexIterator_t const&) = default;

	[[nodiscard]] constexpr IndexIterator_t operator+(size_t count) const noexcept;

	[[nodiscard]] constexpr IndexIterator_t operator-(size_t count) const noexcept;

	[[nodiscard]] constexpr Instance_T& operator*() const noexcept;

	[[nodiscard]] constexpr Instance_T* operator->() const noexcept;

	static constexpr bool IsConst() noexcept { return std::is_const<Instance_T>::value; }
};

template <typename Container_t>
class IndexIterator_t<Container_t, void> {
protected:
	Container_t* mInstance;
	size_t mIndex;

public:
	struct DifferentObjectException_t : std::exception {};

	constexpr IndexIterator_t(Container_t& instance, size_t index) : mInstance(&instance), mIndex(index) {}
	constexpr IndexIterator_t(IndexIterator_t const &) = default;
	//constexpr IndexIterator_t(IndexIterator_t<Instance_T> &&) = delete;
	IndexIterator_t& operator=(IndexIterator_t const&) = default;

	[[nodiscard]] constexpr bool operator==(IndexIterator_t const& b) const {
		if (mInstance != b.mInstance)
			throw DifferentObjectException_t{};
		return mIndex == b.mIndex;
	}

	[[nodiscard]] constexpr bool operator!=(IndexIterator_t const& b) const { return !(*this == b); }

	[[nodiscard]] constexpr bool operator<(IndexIterator_t const & b) const {
		if (mInstance != b.mInstance)
			throw DifferentObjectException_t{};
		return mIndex < b.mIndex;
	}
	[[nodiscard]] constexpr bool operator>=(IndexIterator_t const& b) const { return !(*this < b); }

	[[nodiscard]] constexpr bool operator>(IndexIterator_t const & b) const {
		if (mInstance != b.mInstance)
			throw DifferentObjectException_t{};
		return mIndex > b.mIndex;
	}

	[[nodiscard]] constexpr bool operator<=(IndexIterator_t const& b) const { return !(*this > b); }

	IndexIterator_t& operator++() noexcept {
		//if (*this == mInstance.end())
		//	#if defined(__cpp_exceptions) && __cpp_exceptions==199711
		//		throw std::exception();
		//	#else
		//		abort();//return Identity<Return_T>();
		//	#endif

		mIndex++;
		return *this;
	}

	IndexIterator_t& operator--() noexcept {
		mIndex--;
		return *this;
	}

	IndexIterator_t operator++(int) noexcept {
		IndexIterator_t retval = *this;
		mIndex++;
		return retval;
	}

	IndexIterator_t operator--(int) noexcept {
		IndexIterator_t retval = *this;
		mIndex--;
		return retval;
	}

	IndexIterator_t& operator+=(size_t count) {
		mIndex += count;
		return *this;
	}

	IndexIterator_t& operator-=(size_t count) {
		mIndex -= count;
		return *this;
	}

	[[nodiscard]] constexpr IndexIterator_t operator+(size_t count) const noexcept {
		return IndexIterator_t<Container_t, void>(*mInstance, mIndex + count);
	}

	[[nodiscard]] constexpr IndexIterator_t operator-(size_t count) const noexcept {
		return IndexIterator_t<Container_t, void>(*mInstance, mIndex - count);
	}

	//constexpr bool IsConst(typename std::enable_if<std::is_const<Instance_T>::value>::type* = 0) const noexcept { return true; }
	static constexpr bool IsConst() noexcept { return std::is_const<Container_t>::value; }
};

template <typename Container_t, typename Instance_T>
[[nodiscard]] inline constexpr Instance_T&
IndexIterator_t<Container_t, Instance_T>::operator*() const noexcept {
	return (*this->mInstance)[this->mIndex];
}

template <typename Container_t, typename Instance_T>
[[nodiscard]] inline constexpr Instance_T*
IndexIterator_t<Container_t, Instance_T>::operator->() const noexcept
{
	return &(*this->mInstance)[this->mIndex];
}

template <typename Container_t, typename Instance_T>
[[nodiscard]] inline constexpr IndexIterator_t<Container_t, Instance_T>
IndexIterator_t<Container_t, Instance_T>::operator+(size_t count) const noexcept {
	return IndexIterator_t<Container_t, Instance_T>(*this->mInstance, this->mIndex + count);
}

template <typename Container_t, typename Instance_T>
[[nodiscard]] inline constexpr IndexIterator_t<Container_t, Instance_T>
IndexIterator_t<Container_t, Instance_T>::operator-(size_t count) const noexcept {
	return IndexIterator_t<Container_t, Instance_T>(*this->mInstance, this->mIndex - count);
}

template <typename Instance_T>
class pointer_iterator_t {
protected:
	Instance_T * mPtr;

public:
	constexpr pointer_iterator_t(Instance_T * instance) : mPtr(instance) {}
	constexpr pointer_iterator_t(pointer_iterator_t const &) = default;
	//constexpr pointer_iterator_t(pointer_iterator_t<Instance_T> &&) = delete;
	pointer_iterator_t& operator=(pointer_iterator_t const&) = default;

	[[nodiscard]] constexpr Instance_T& operator*() const noexcept { return *mPtr; }
	[[nodiscard]] constexpr Instance_T* operator->() const noexcept { return mPtr; }

	[[nodiscard]] constexpr bool operator==(pointer_iterator_t const b) const {
		return mPtr == b.mPtr;
	}

	[[nodiscard]] constexpr bool operator!=(pointer_iterator_t const& b) const { return !(*this == b); }

	[[nodiscard]] constexpr bool operator<(pointer_iterator_t const b) const {
		return mPtr < b.mPtr;
	}

	[[nodiscard]] constexpr bool operator>=(pointer_iterator_t const& b) const { return !(*this < b); }

	[[nodiscard]] constexpr bool operator>(pointer_iterator_t const b) const {
		return mPtr > b.mPtr;
	}

	[[nodiscard]] constexpr bool operator<=(pointer_iterator_t const& b) const { return !(*this > b); }


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

	pointer_iterator_t& operator+=(size_t count) {
		mPtr += count;
		return *this;
	}

	pointer_iterator_t& operator-=(size_t count) {
		mPtr -= count;
		return *this;
	}

	[[nodiscard]] constexpr pointer_iterator_t operator+(size_t count) const noexcept {
		return pointer_iterator_t<Instance_T>(mPtr + count);
	}

	[[nodiscard]] constexpr pointer_iterator_t operator-(size_t count) const noexcept {
		return pointer_iterator_t<Instance_T>(mPtr - count);
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

	[[nodiscard]] Instance_T& operator*() const { return *mPtr; }
	[[nodiscard]] Instance_T* operator->() const { return mPtr; }

	[[nodiscard]] constexpr bool operator==(sparse_pointer_iterator_t const b) const {
		return mPtr == b.mPtr && Stride == b.Stride;
	}

	[[nodiscard]] constexpr bool operator!=(sparse_pointer_iterator_t const b) const {
		return !(*this == b);
	}

	[[nodiscard]] constexpr bool operator<(sparse_pointer_iterator_t const b) const {
		return mPtr < b.mPtr;
	}

	[[nodiscard]] constexpr bool operator>=(sparse_pointer_iterator_t const b) const {
		return !(*this < b);
	}

	[[nodiscard]] constexpr bool operator>(sparse_pointer_iterator_t const b) const {
		return mPtr > b.mPtr;
	}

	[[nodiscard]] constexpr bool operator<=(sparse_pointer_iterator_t const b) const {
		return !(*this > b);
	}

	[[nodiscard]] sparse_pointer_iterator_t operator+(size_t count) const {
		return sparse_pointer_iterator_t<Instance_T>((Instance_T*)((uint8_t*)mPtr + Stride * count), Stride);
	}

	[[nodiscard]] sparse_pointer_iterator_t operator-(size_t count) const {
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
