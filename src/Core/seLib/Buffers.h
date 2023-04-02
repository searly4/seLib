///-------------------------------------------------------------------------------------------------
/// @file	Source/IDCLib/Util/Buffers.h
/// @brief	Declares various buffers classes
#pragma once

/// \cond
#include <stdint.h>
#include <stdlib.h>
#include <type_traits>
#include <exception>
#include <assert.h>
#include <array>
/// \endcond

#include "SimpleBuffer.h"


//==================================================================================================
namespace seLib { namespace Buffers {

class ByteArrayView_t;

struct ReadOnlyException : std::exception {};

///-------------------------------------------------------------------------------------------------
/// @class	ConstBuffer_t
/// @brief	An abstract accessor to a const buffer.
/// @tparam	T	Type of the buffer data.
template <typename T>
class ConstBuffer_t {
public:
	typedef T Data_t;

	virtual T const & const_get(size_t index) const = 0;
	inline T const * const_data() const { return & const_get(0); }
	inline T const & get(size_t index) { return & const_get(index); }
	inline T const & get(size_t index) const { return & const_get(index); }
	inline T const * data() { return & const_get(0); }
	inline T const * data() const { return & const_get(0); }
	virtual size_t size() const noexcept = 0;
	inline T const & operator[](size_t index) { return const_get(index); }
	inline T const & operator[](size_t index) const { return const_get(index); }
	constexpr inline bool is_const() noexcept { return true; }
	constexpr inline bool is_const() const noexcept { return true; }

	//template <size_t SIZE>
	//inline std::array<T, SIZE> const& get_array() const noexcept {
	//	assert(SIZE <= size());
	//	return *(std::array<T, SIZE> const *)const_data();
	//}
};

template <typename T, typename Buffer_T>
class BufferRef_t;

///-------------------------------------------------------------------------------------------------
/// @class	Buffer_t
/// @brief	An abstract accessor to a buffer.
/// @tparam	T	Type of the buffer data.
template <typename T>
class Buffer_t : public ConstBuffer_t<T> {
public:

	typedef T Data_t;
	virtual T& get(size_t index) = 0;
	virtual T& get(size_t index) const = 0;
	inline T * data() { return & get(0); }
	inline T * data() const { return & get(0); }
	T const & const_get(size_t index) const override { return get(index); }
	//size_t size() const noexcept override = 0;
	inline T & operator[](size_t index) { return get(index); }
	inline T & operator[](size_t index) const { return get(index); }
	virtual bool is_const() noexcept { return false; }
	virtual bool is_const() const noexcept { return false; }

	//operator Buffer_t<T const> const &() const { return BufferRef_t<T const, Buffer_t<T> const>(*this); }

	//operator BufferRef_t<T const, Buffer_t<T const>> const () const { return BufferRef_t<T const, Buffer_t<T> const>(*this); }

	//template <size_t SIZE>
	//inline std::array<T, SIZE>& get_array() {
	//	//assert(SIZE <= this->size());
	//	return *(std::array<T, SIZE>*)data();
	//}
};


///-------------------------------------------------------------------------------------------------
/// @class	BufferSubset_t
/// @brief	An accessor to a subset of a buffer. This class cannot be inherited.
/// @tparam	Buffer_T	Type of the buffer data.
template <typename Buffer_T>
class BufferSubset_t final : public Buffer_t<typename Buffer_T::Data_t> {
protected:
	Buffer_T& mRef;
	size_t mStart;
	size_t mSize;

public:
	constexpr BufferSubset_t(Buffer_T& source, size_t start, size_t length)
		: mRef(source), mStart(start), mSize(length)
	{
		assert((start + length) <= source.size());
	}
	
	constexpr BufferSubset_t(BufferSubset_t<Buffer_T> const &) = default;
	
	typename Buffer_T::Data_t const & const_get(size_t index) const override { 
		return mRef.const_get(index + mStart);
	}
	
	//typename Buffer_T::Data_t const * const_data() const noexcept override { 
	//	return mRef.data() + mStart;
	//}

	typename Buffer_T::Data_t& get(size_t index) override { 
		return mRef.get(index + mStart);
	}

	typename Buffer_T::Data_t& get(size_t index) const override { 
		return mRef.get(index + mStart);
	}

	//typename Buffer_T::Data_t* data() noexcept override {
	//	return mRef.data() + mStart;
	//}
	//
	//typename Buffer_T::Data_t* data() const noexcept override { 
	//	return mRef.data() + mStart;
	//}

	size_t size() const noexcept override {
		return mSize;
	}
};


template <typename T>
class ConstBufferRef_t final : public ConstBuffer_t<T> {
protected:
	ConstBuffer_t<T> const & mRef;

public:
	constexpr ConstBufferRef_t(ConstBuffer_t<T> const & source)
		: mRef(source)
	{ }

	
	constexpr ConstBufferRef_t(ConstBufferRef_t const &) = default;
	

	T const & const_get(size_t index) const override { 
		return mRef.const_get(index);
	}

	constexpr size_t size() const noexcept override {
		return mRef.size();
	}
};

template <typename T, typename Buffer_T>
class BufferRef_t final : public Buffer_T {
protected:
	Buffer_T& mRef;

public:
	constexpr BufferRef_t(Buffer_T& source)
		: mRef(source)
	{ }

	constexpr BufferRef_t(BufferRef_t const &) = default;
	
	//operator BufferRef_t<T const, Buffer_t<T const>> const () const { return BufferRef_t<T const, Buffer_t<T const> const>(mRef); }

	T& get(size_t index) override {
		return mRef.get(index);
	}

	T& get(size_t index) const override {
		return mRef.get(index);
	}

	T const & const_get(size_t index) const override { 
		return mRef.const_get(index);
	}

	constexpr size_t size() const noexcept override {
		return mRef.size();
	}

	bool is_const() noexcept override { return mRef.is_const(); }
	bool is_const() const noexcept override { return mRef.is_const(); }
};// */

template <typename T>
BufferRef_t(Buffer_t<T> &) -> BufferRef_t<T, Buffer_t<T>>;
template <typename T>
BufferRef_t(Buffer_t<T> const &) -> BufferRef_t<T, Buffer_t<T> const>;

//template <typename T>
//BufferRef_t(Buffer_t<T>&) -> BufferRef_t<T, Buffer_t<T>>;

///-------------------------------------------------------------------------------------------------
/// @class	BufferStoreRef_t
/// @brief	An accessor to an external buffer of known length. This class cannot be inherited.
/// @tparam	T	Type of the buffer data.
template <typename T>
class BufferStoreRef_t final : public Buffer_t<T> {
protected:
	T* const mData;
	size_t const mLength;

public:
	constexpr BufferStoreRef_t(Buffer_t<T> const & source)
		: mData(source.Data()), mLength(source.size())

	{ }
	
	constexpr BufferStoreRef_t(T * source, size_t length)
		: mData(source), mLength(length)
	{ }
	
	constexpr BufferStoreRef_t(BufferStoreRef_t<T> const &) = default;
	
	T const & const_get(size_t index) const override { 
		return mData[index];
	}

	T& get(size_t index) override { 
		return mData[index];
	}

	T& get(size_t index) const override { 
		return mData[index];
	}

	size_t size() const noexcept override {
		return mLength;
	}

	bool is_const() noexcept override { return std::is_const<T>::value; }
	bool is_const() const noexcept override { return std::is_const<T>::value; }
	
	/*BufferRef_t<T const, BufferStoreRef_t<T> const> const_ref() const noexcept {
		return BufferRef_t<T const, BufferStoreRef_t<T> const>(*this);
	}// */
};
// */


///-------------------------------------------------------------------------------------------------
/// @class	BufferStore_t
/// @brief	A buffer container. This class cannot be inherited.
/// @tparam	T	 	Type of the buffer data.
/// @tparam	COUNT	Number of elements in the buffer array.
template <typename T, size_t COUNT>
class BufferStore_t final : public Buffer_t<T> {
protected:
	std::array<typename std::remove_const<T>::type, COUNT> mData;
	//size_t mLength;

public:
	constexpr BufferStore_t()
		//: mLength(COUNT)
	{ }

	constexpr BufferStore_t(T const & source)
		: mData(source)//, mLength(source.size())
	{
		memcpy(mData, source.Data(), COUNT);
	}

	constexpr BufferStore_t(Buffer_t<T> const & source)
		//: mLength(source.size())
	{
		static_assert(source.size() == COUNT);
		memcpy(mData, source.Data(), COUNT);
	}

	constexpr BufferStore_t(T const (&str)[COUNT])
		: mData({})
		//, Length(Count)
	{
		for (size_t i = 0; i < COUNT; i++)
			mData[i] = str[i];
	}

	constexpr BufferStore_t(BufferStore_t<T,COUNT> const &) = default;

	T const & const_get(size_t index) const override {
		return mData[index];
	}

	T& get(size_t index) override {
		return mData[index];
	}

	T& get(size_t index) const override {
		throw ReadOnlyException {};
		//assert(false);
	}

	size_t size() const noexcept override {
		return COUNT;
	}

	bool is_const() noexcept override { return false; }
	bool is_const() const noexcept override { return true; }

	inline std::array<T, COUNT>& get_array() noexcept {
		return *(std::array<T, COUNT>*)this->data();
	}

	inline std::array<T, COUNT> const & get_array() const noexcept {
		return *(std::array<T, COUNT> const *)this->data();
	}
};

template <size_t COUNT>
BufferStore_t(char const(&)[COUNT]) -> BufferStore_t<char, COUNT>;

template <typename T, typename Const_T = const T>
class TypedArrayAccessor_t {
public:
	virtual size_t size() const = 0;
	virtual T & operator[](size_t index) = 0;
	virtual Const_T & operator[](size_t index) const = 0;
	virtual T* Data() = 0;
	virtual Const_T* Data() const = 0;
};


template <typename T, size_t _Size>
class TypedArray_t final : public TypedArrayAccessor_t<T, const T> {
protected:
	T _Buffer[_Size];

public:
	size_t size() const override {
		return _Size;
	}

	T & operator[](size_t index) override {
		return _Buffer[index];
	}

	T const & operator[](size_t index) const override {
		return _Buffer[index];
	}

	T* Data() override {
		return _Buffer;
	}

	T const * Data() const override {
		return _Buffer;
	}
};

template <class T, class... U>
TypedArray_t(T, U...) -> TypedArray_t<T, 1 + sizeof...(U)>;


class Accessor_t {
public:
	virtual void Subscribe() const {}
	virtual void Unsubscribe() const {}
};


/*template <typename T>
class ArrayAccessor {
public:
	virtual size_t size() const;

	virtual uint8_t operator[](size_t offset) const;
	
	template <bool, typename std::enable_if< std::is_same< ArrayAccessor, typename std::remove_const<T>::type >::value >::type >
	bool Writeable() const {
		return true;
	}

	template <bool, typename std::enable_if< std::is_same< ArrayAccessor const, T >::value >::type >
	bool Writeable() const {
		return false;
	}
	
	virtual uint8_t const * Data() const;
	
	virtual ArrayAccessor<T> View(size_t start, size_t size) const;
};*/


///-------------------------------------------------------------------------------------------------
/// @class	ByteArrayAccessor_t
/// @brief	Abstract class for defining an access interface for a byte array.
class ByteArrayAccessor_t : public Accessor_t {
public:
	class WriteAccessException : public std::exception {};
	
	virtual size_t size() const = 0;

	virtual bool Writeable() const {
		return false;
	}

	virtual bool Writeable() {
		return false;
	}
	
	virtual uint8_t const * Data() const = 0;
	virtual uint8_t * WriteableData() const = 0;
};


///-------------------------------------------------------------------------------------------------
/// @class	ByteArrayView_t
/// @brief	Provides a means to access all or a portion of a memory region represented by a
/// 		ByteArrayAccessor.
class ByteArrayView_t {
protected:
	ByteArrayAccessor_t const & _Ref;
	size_t const _Start;
	size_t const _Size;

public:
	virtual ~ByteArrayView_t() {
		_Ref.Unsubscribe();
	}

	constexpr ByteArrayView_t(ByteArrayAccessor_t const & ref, size_t start, size_t size) :
		_Ref(ref), _Start(start), _Size(size)
	{
		if constexpr(false)
			_Ref.Subscribe();
	}

	constexpr ByteArrayView_t(ByteArrayView_t const & view) :
		_Ref(view._Ref), _Start(view._Start), _Size(view._Size)
	{
		if constexpr(false)
			_Ref.Subscribe();
	}

	ByteArrayView_t(ByteArrayView_t&& view) :
		_Ref(view._Ref), _Start(view._Start), _Size(view._Size)
	{
		_Ref.Subscribe();
	}
	
	ByteArrayView_t& operator=(ByteArrayView_t const & view) = default;

	ByteArrayView_t& operator=(ByteArrayView_t&& view) = default;

	inline size_t size() const {return _Size;}

	inline uint8_t operator[](size_t offset) const {
		return _Ref.Data()[offset + _Start];
	}

	inline uint8_t const * Data() const {
		return _Ref.Data();
	}
	
	inline ByteArrayView_t View(size_t start, size_t size) const {
		return ByteArrayView_t(_Ref, start, size);
	}
};


///-------------------------------------------------------------------------------------------------
/// @class	WriteableByteArrayView_t
/// @brief	Provides a means to access all or a portion of a memory region represented by a
/// 		ByteArrayAccessor.
class WriteableByteArrayView_t : public ByteArrayView_t {
public:
	constexpr WriteableByteArrayView_t(ByteArrayAccessor_t const & ref, size_t start, size_t size) :
		ByteArrayView_t(ref, start, size)
	{ }

	~WriteableByteArrayView_t() { }
	
	constexpr WriteableByteArrayView_t(WriteableByteArrayView_t const & view) :
		ByteArrayView_t(view)
	{ }

	WriteableByteArrayView_t(WriteableByteArrayView_t&& view) :
		ByteArrayView_t(view)
	{ }

	inline uint8_t& operator[](size_t offset) const {
		return _Ref.WriteableData()[offset + _Start];
	}

	inline uint8_t * Data() const {
		return _Ref.WriteableData();
	}

	inline WriteableByteArrayView_t View(size_t start, size_t size) const {
		return WriteableByteArrayView_t(_Ref, start, size);
	}
};


template <typename T>
class TypedBufferView_t {
public:
	virtual size_t size() const;
	virtual T & operator[](size_t offset) const;
	virtual T* Data() const;
};


template <unsigned int _Size>
class ByteArrayBuffer_t : public ByteArrayAccessor_t {
protected:
	uint8_t _Buffer[_Size];

public:
	size_t size() const override {return _Size;}
	
	bool Writeable() override {
		return true;
	}
	
	uint8_t const * Data() const override {
		return _Buffer;
	}

	uint8_t * WriteableData() const override {
		if constexpr(true)
			throw WriteAccessException();
		return _Buffer;
	}

};


}} // namespace IDCLib::Buffers
//==================================================================================================
