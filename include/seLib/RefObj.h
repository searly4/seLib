#pragma once
/*
   Copyright 2018 by Scott Early

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <stdint.h>
#include <exception>

#pragma warning(disable: 4521)

using namespace std;

namespace seLib {

//============================================================================
// A reference counter for an existing memory buffer. Buffer allocation and
// de-allocation must be handled separately.
class RefBuffer {
protected:
	uint8_t* const _Data;
	size_t const _DataSize = 0;
	uint32_t _RefCount = 0;

public:
	RefBuffer(uint8_t* data, size_t datasize) : _Data(data), _DataSize(datasize) { }
	RefBuffer(const RefBuffer&) = delete;
	RefBuffer(RefBuffer&&) = delete;
	RefBuffer& operator=(const RefBuffer&) = delete;
	RefBuffer& operator=(RefBuffer&&) = delete;
	virtual ~RefBuffer() {
		_RefCount = UINT32_MAX;
	}

	void Subscribe() {
		if (_RefCount == UINT32_MAX)
			throw exception();
		_RefCount++;
	}

	void Release() {
		if (_RefCount == UINT32_MAX || _RefCount == 0)
			throw exception();
		_RefCount--;
		if (_RefCount == 0)
			delete this;
	}

	inline int RefCount() {
		return _RefCount;
	}

	inline uint8_t* operator*() {
		return _Data;
	}

	size_t size() {
		return _DataSize;
	}

	void set(const uint8_t* source, size_t len, size_t offset) {
		if (len + offset > _DataSize)
			throw exception();
		uint8_t* dest = _Data + offset;
		const uint8_t* src_end = source + len;
		while (source < src_end) {
			*dest = *source;
			dest++;
			source++;
		}
	}
};

//============================================================================
// A reference counting memory buffer. The buffer is de-allocated when the
// reference count reaches zero.
class ManagedRefBuffer : public RefBuffer {
public:
	ManagedRefBuffer(size_t datasize) : RefBuffer(new uint8_t[datasize], datasize) { }
	~ManagedRefBuffer() override {
		delete[] _Data;
		//RefBuffer::~RefBuffer();
	}

protected:
	ManagedRefBuffer(void* data, size_t datasize) : RefBuffer((uint8_t*)data, datasize) { }
};

//============================================================================
// A reference counting wrapper for an instance of the specified type.
// The instance is deleted when the reference count reaches zero.
template <typename Data_T>
class TypedManagedRefBuffer : public ManagedRefBuffer {
public:
	Data_T& Value;

	TypedManagedRefBuffer() :
		ManagedRefBuffer(new Data_T, sizeof(Data_T)),
		Value(*(Data_T*)_Data)
	{ }

	template<class... _Valty>
	TypedManagedRefBuffer(_Valty&&... _Val) :
		ManagedRefBuffer(new Data_T(std::forward<_Valty>(_Val)...), sizeof(Data_T)),
		Value(*(Data_T*)_Data)
	{ }

	~TypedManagedRefBuffer() override {
		((Data_T*)_Data)->~Data_T();
		//ManagedRefBuffer::~ManagedRefBuffer();
	}
};

//============================================================================
class BufferView {
protected:
	uint8_t* _start = nullptr;
	uint8_t* _end = nullptr;

public:
	BufferView(uint8_t* buffer, size_t len) : _start(buffer), _end(buffer + len - 1) { }

	virtual ~BufferView() { }

	BufferView(const BufferView& view) {
		*this = view;
	}

	BufferView(BufferView&& view) {
		*this = std::move(view);
	}

	virtual BufferView& operator=(const BufferView& view) {
		Release();
		_start = view._start;
		_end = view._end;
		return *this;
	}

	virtual BufferView& operator=(BufferView&& view) {
		Release();
		_start = view._start;
		_end = view._end;
		view._start = nullptr;
		view._end = nullptr;
		view.Release();
		return *this;
	}

	inline uint8_t* operator*() const {
		return _start;
	}

	inline size_t size() const {
		return _end - _start + 1;
	}

	void set(const uint8_t* source, size_t len, size_t offset) {
		if (_start + len + offset - 1 > _end)
			throw exception();
		uint8_t* dest = _start + offset;
		const uint8_t* src_end = source + len;
		while (source < src_end) {
			*dest = *source;
			dest++;
			source++;
		}
	}

	template <typename T>
	T& get() { return *(T*)_start; }

protected:
	virtual void Release() {}
};

//============================================================================
// A BufferView that attaches to a reference counting buffer (RefBuffer). It
// provides access to the specified region of the buffer.
class RefBufferView : public BufferView {
protected:
	RefBuffer* _buffer = nullptr;

public:
	RefBufferView() : BufferView(nullptr, 0), _buffer(nullptr) { }

	RefBufferView(RefBuffer* buffer) : BufferView(**buffer, buffer->size()), _buffer(buffer) {
		_buffer->Subscribe();
	}

	RefBufferView(RefBuffer* buffer, size_t offset, size_t len) : BufferView(**buffer + offset, len), _buffer(buffer) {
		if (offset + len > _buffer->size())
			throw exception();
		_buffer->Subscribe();
	}

	RefBufferView(uint8_t* data, size_t len) : BufferView(nullptr, 0) {
		if (len == 0)
			return;
		_buffer = new RefBuffer(data, len);
		_start = **_buffer;
		_end = _start + len - 1;
		_buffer->Subscribe();
	}

	RefBufferView(size_t len) : BufferView(nullptr, 0) {
		if (len == 0)
			return;
		_buffer = new ManagedRefBuffer(len);
		_start = **_buffer;
		_end = _start + len - 1;
		_buffer->Subscribe();
	}

	~RefBufferView() {
		Release();
	}

	RefBufferView(const RefBufferView& view) : BufferView(view) {
		*this = view;
	}

	RefBufferView(const RefBufferView* view) : BufferView(nullptr, 0) {
		if (view == nullptr)
			return;
		*this = *view;
	}

	RefBufferView(RefBufferView&& view) : BufferView(view) {
		*this = std::move(view);
	}

	// BufferView assignments not accepted
	BufferView& operator=(const BufferView&) override {
		throw exception();
	}
	// BufferView assignments not accepted
	BufferView& operator=(BufferView&&) override {
		throw exception();
	}

	RefBufferView& operator=(const RefBufferView& view) {
		if (view._buffer != nullptr)
			view._buffer->Subscribe();
		BufferView::operator=(view);
		_buffer = view._buffer;
		return *this;
	}

	RefBufferView& operator=(RefBufferView&& view) {
		if (view._buffer != nullptr)
			view._buffer->Subscribe();
		BufferView::operator=(std::move(view));
		_buffer = view._buffer;
		view._buffer = nullptr;
		return *this;
	}

protected:
	virtual void Release() override {
		if (_buffer == nullptr)
			return;
		_buffer->Release();
	}
};

//============================================================================
template <typename Data_T>
class TypedBufferView {
protected:
	Data_T* _start;

public:
	TypedBufferView(Data_T* buffer) : _start(buffer) {
	}

	TypedBufferView(uint8_t* buffer, size_t len) : _start((Data_T*)buffer) {
		if (len < sizeof(Data_T))
			throw exception();
	}

	virtual ~TypedBufferView() { }

	TypedBufferView(const TypedBufferView<Data_T>& view) {
		*this = view;
	}

	TypedBufferView(TypedBufferView<Data_T>&& view) {
		*this = std::move(view);
	}

	virtual TypedBufferView<Data_T>& operator=(const TypedBufferView<Data_T>& view) {
		Release();
		_start = view._start;
		return *this;
	}

	virtual TypedBufferView<Data_T>& operator=(TypedBufferView<Data_T>&& view) {
		Release();
		_start = view._start;
		view._start = nullptr;
		view.Release();
		return *this;
	}

	inline Data_T& operator*() const {
		return *_start;
	}

	inline Data_T* operator->() const {
		return _start;
	}

	inline size_t size() const {
		return sizeof(Data_T);
	}

protected:
	virtual void Release() {}
};

//============================================================================
template <typename Data_T>
class TypedRefBufferView : public TypedBufferView<Data_T> {
protected:
	RefBuffer* _buffer = nullptr;

protected:
	/*TypedRefBufferView(RefBuffer* buffer) :
		TypedBufferView<Data_T>(**buffer, buffer->size()), _buffer(buffer)
	{
		_buffer->Subscribe();
	}*/

	TypedRefBufferView(RefBuffer* buffer, size_t offset) :
		TypedBufferView<Data_T>(nullptr), _buffer(buffer)
		//TypedBufferView<Data_T>((Data_T*)(**buffer + offset)), _buffer(buffer)
	{
		// Hiding this constructor to avoid confusion with variadic constructor. Use
		// from_RefBuffer static methods instead.

		//if (offset + len > _buffer->size())
			//throw exception();
		if (_buffer != nullptr) {
			_start = (Data_T*)(**_buffer + offset);
			_buffer->Subscribe();
		}
	}

public:
	static TypedRefBufferView<Data_T> from_RefBuffer(RefBuffer* buffer) {
		return TypedRefBufferView<Data_T>(buffer, (size_t)0);
	}

	static TypedRefBufferView<Data_T> from_RefBuffer(RefBuffer* buffer, size_t offset) {
		return TypedRefBufferView<Data_T>(buffer, offset);
	}

	TypedRefBufferView(const TypedRefBufferView<Data_T>& view) : _buffer(nullptr), TypedBufferView<Data_T>(view) {
		*this = view;
	}

	TypedRefBufferView(TypedRefBufferView<Data_T>& view) : _buffer(nullptr), TypedBufferView<Data_T>(view) {
		// non-const copy constructor required to avoid variadic constructor confusion
		*this = view;
	}

	TypedRefBufferView(TypedRefBufferView<Data_T>&& view) : _buffer(nullptr), TypedBufferView<Data_T>(view) {
		*this = std::move(view);
	}

	template<class... _Valty>
	TypedRefBufferView(_Valty&&... _Val) :
		_buffer(new TypedManagedRefBuffer<Data_T>(std::forward<_Valty>(_Val)...)),
		TypedBufferView<Data_T>(nullptr)
	{
		_start = (Data_T*)_buffer->operator*();
		_buffer->Subscribe();
	}//*/

	~TypedRefBufferView() {
		if (_buffer != nullptr)
			_buffer->Release();
	}

	// BufferView assignments not accepted
	TypedBufferView<Data_T>& operator=(const TypedBufferView<Data_T>&) override {
		throw exception();
	}
	// BufferView assignments not accepted
	TypedBufferView<Data_T>& operator=(TypedBufferView<Data_T>&&) override {
		throw exception();
	}

	TypedRefBufferView<Data_T>& operator=(const TypedRefBufferView<Data_T>& view) {
		view._buffer->Subscribe(); // do first in case old and new buffer are the same
		TypedBufferView<Data_T>::operator=(view); // old buffer released here
		_buffer = view._buffer;
		return *this;
	}

	TypedRefBufferView<Data_T>& operator=(TypedRefBufferView<Data_T>&& view) {
		view._buffer->Subscribe(); // do first in case old and new buffer are the same
		TypedBufferView<Data_T>::operator=(std::move(view)); // old buffer released here
		_buffer = view._buffer;
		view._buffer = nullptr;
		return *this;
	}

	template<typename Cast_T>
	TypedRefBufferView<Cast_T> cast() {
		Cast_T* cast_ptr = (Cast_T*)_start;
		size_t offset = (uint8_t*)cast_ptr - **_buffer;
		return TypedRefBufferView<Cast_T>::from_RefBuffer(_buffer, offset);
	}

	template<class... _Valty>
	static TypedRefBufferView<Data_T> construct(_Valty&&... _Val) {
		return TypedRefBufferView<Data_T>::from_RefBuffer(new TypedManagedRefBuffer<Data_T>(std::forward<_Valty>(_Val)...));
	}

protected:
	virtual void Release() override {
		if (_buffer != nullptr)
			_buffer->Release();
	}
};

/*
template <typename _T>
class RefObj : public TypedRefBufferView<_T> {
public:
  //typedef RefObjPointer<_T*> _ContainerT;
  typedef TypedRefBufferView<_T> _ParentT;
  typedef RefObj<_T> _SelfT;

public:
  RefObj() : _ParentT(nullptr) { }

  RefObj(_SelfT& linkref) : _ParentT(linkref) { }

  RefObj(const _SelfT& linkref) : _ParentT(linkref) { }

  //RefObj(_ContainerT* container) : TypedRefBufferView<_T*>(container) { }

public:

  _T operator*() {
	return (_T)*_ParentT::operator*();
  }
  const _T operator*() const {
	return *(const _T*)_ParentT::operator*();
  }

  RefObj<const _T> ConstRef() const {
	return RefObj<const _T>(this->_buffer);
  }

  bool operator==(const _SelfT& b) const {
	return operator*() == b.operator*();
  }

  _SelfT& operator=(const _SelfT& b) {
	_ParentT::operator=(b);
	return *this;
  }
};

template <typename _T>
class RefObj<_T*> : public TypedRefBufferView<_T*> {
public:
  //typedef RefObjPointer<_T*> _ContainerT;
  typedef TypedRefBufferView<_T*> _ParentT;
  typedef RefObj<_T*> _SelfT;

public:
  RefObj() : _ParentT(nullptr) { }

  RefObj(_SelfT& linkref) : _ParentT(linkref) { }

  RefObj(const _SelfT& linkref) : _ParentT(linkref) { }

  //RefObj(_ContainerT* container) : TypedRefBufferView<_T*>(container) { }

public:

  _T* operator*() {
	return (_T*)_ParentT::operator*();
  }
  const _T* operator*() const {
	return (const _T*)_ParentT::operator*();
  }

  RefObj<const _T*> ConstRef() const {
	return RefObj<const _T*>(this->_buffer);
  }

  bool operator==(const _SelfT& b) const {
	return operator*() == b.operator*();
  }

  _SelfT& operator=(const _SelfT& b) {
	_ParentT::operator=(b);
	return *this;
  }
};
*/
}

//============================================================================

namespace SE {

class RefObjContainerBase;
template <typename _T> class RefObjContainer;
template <typename _T> class RefObjPointer;
template <typename _T> class RefObj;

//template <typename _T>
//using RefPtrObj = RefObj<_T, RefObjPointer<_T>>;

//============================================================================


class ObjContainerBase {
protected:

public:

public:
	ObjContainerBase() { }
	ObjContainerBase(const ObjContainerBase&) = delete;
	ObjContainerBase(ObjContainerBase&&) = delete;
	ObjContainerBase& operator=(const ObjContainerBase&) = delete;
	ObjContainerBase& operator=(ObjContainerBase&&) = delete;
	virtual ~ObjContainerBase() {
	}
};


//============================================================================

class RefObjContainerBase {
protected:

public:
	uint32_t _RefCount = 0;

public:
	RefObjContainerBase() { }
	RefObjContainerBase(const RefObjContainerBase&) = delete;
	RefObjContainerBase(RefObjContainerBase&&) = delete;
	RefObjContainerBase& operator=(const RefObjContainerBase&) = delete;
	RefObjContainerBase& operator=(RefObjContainerBase&&) = delete;
	virtual ~RefObjContainerBase() {
		_RefCount = UINT32_MAX;
	}

	void IncrementCount() {
		if (_RefCount == UINT32_MAX)
			throw exception();
		_RefCount++;
	}

	void DecrementCount() {
		if (_RefCount == UINT32_MAX)
			throw exception();
		_RefCount--;
	}

	int RefCount() {
		return _RefCount;
	}
};

//============================================================================

template <typename _T>
class RefObjContainer : public RefObjContainerBase {
protected:

public:
	_T value;

public:
	RefObjContainer() {}
	RefObjContainer(const RefObjContainer<_T>&) = delete;
	RefObjContainer(RefObjContainer<_T>&&) = delete;
	RefObjContainer& operator=(const RefObjContainer<_T>&) = delete;
	RefObjContainer& operator=(RefObjContainer<_T>&&) = delete;
	//RefObjContainer(const T& val) : value(val) { }
	//RefObjContainer(T&& val) : value(val) { }

	template<class... _Valty>
	RefObjContainer(_Valty&&... _Val) : value(std::forward<_Valty>(_Val)...) { }
};

//============================================================================

template <typename _T>
class RefObjPointer : public RefObjContainerBase {
protected:

public:
	_T value = nullptr;

public:
	RefObjPointer() { }
	~RefObjPointer() override {
	  //_T* ptr = value;
		value = nullptr;
		/* not safe to delete a pointer to unknown type*/
		//if (ptr != nullptr)
		  //delete ptr;
	}

	RefObjPointer(const RefObjPointer&) = delete;
	RefObjPointer(RefObjPointer&&) = delete;
	RefObjPointer& operator=(const RefObjPointer&) = delete;
	RefObjPointer& operator=(RefObjPointer&&) = delete;
	RefObjPointer(_T val) : value(val) { }

	_T release() {
		_T ptr = value;
		value = nullptr;
		return ptr;
	}
};

//============================================================================

template <typename _T>
class RefObjOwnedPointer : public RefObjPointer<_T> {
protected:

public:
	RefObjOwnedPointer() { }
	~RefObjOwnedPointer() override {
		_T ptr = this->value;
		this->value = nullptr;
		/* not safe to delete a pointer to unknown type*/
		if (ptr != nullptr)
			delete ptr;
	}
	RefObjOwnedPointer(const RefObjOwnedPointer&) = delete;
	RefObjOwnedPointer(RefObjOwnedPointer&&) = delete;
	RefObjOwnedPointer& operator=(const RefObjOwnedPointer&) = delete;
	RefObjOwnedPointer& operator=(RefObjOwnedPointer&&) = delete;
	RefObjOwnedPointer(_T val) : RefObjPointer<_T>(val) { }
	/*_T* release() override {
	  _T* ptr = value;
	  value = nullptr;
	  return ptr;
	}*/
};

//============================================================================

template <typename _ContainerT>
class RefObjBase {
protected:
	_ContainerT* _container = nullptr;

protected:
	RefObjBase() { }

public:
	RefObjBase(_ContainerT* container) : _container(container) {
		if (_container != nullptr)
			_container->IncrementCount();
	}

	~RefObjBase() {
		if (_container != nullptr) {
			_container->DecrementCount();
			if (_container->RefCount() == 0)
				delete _container;
			_container = nullptr;
		}
	}

	inline bool mapped() {
		return _container != nullptr;
	}

protected:
  // Unlink the current container and attach a new one.
	void _Map(_ContainerT* container) {
		if (_container != nullptr) {
			_container->DecrementCount();
			if (_container->RefCount() == 0)
				delete _container;
		}
		_container = container;
		if (_container != nullptr)
			_container->IncrementCount();
	}
};

//============================================================================

template <typename _T>
class RefObj : public RefObjBase<RefObjContainer<_T>> {
public:
	typedef RefObjContainer<_T> _ContainerT;
	typedef RefObj<_T> _SelfT;

public:
	RefObj() : RefObjBase<_ContainerT>(nullptr) { }

	RefObj(const _T& value) : RefObjBase<_ContainerT>(new _ContainerT(value)) { }

	RefObj(_SelfT& linkref) : RefObjBase<_ContainerT>(linkref._container) { }

	RefObj(const _SelfT& linkref) : RefObjBase<_ContainerT>(linkref._container) { }

	RefObj(_SelfT&& linkref) : RefObjBase<_ContainerT>(linkref._container) {
		linkref._Map(nullptr);
	}//*/

	template<class... _Valty>
	RefObj(_Valty&&... _Val) : RefObjBase<_ContainerT>(new _ContainerT(std::forward<_Valty>(_Val)...)) { }

	_T& operator*() {
		return this->_container->value;
	}
	const _T& operator*() const {
		return this->_container->value;
	}

	RefObj<const _T>& ConstRef() const {
		return RefObj<const _T>((RefObjContainer<const _T>*)(void*)this->_container);
	}

	bool operator==(const _SelfT& b) const {
		return this->_container == b._container;
	}

	_SelfT& operator=(const _SelfT& b) {
		_Map(b._container);
		return *this;
	}

	/*template <typename T2>
	operator RefObj<T2>& () {
	  return *this;
	}

	template <typename T2>
	operator const RefObj<T2>& () const {
	  return *this;
	}

	operator RefObj<const T>& () {
	  return *this;
	}
	operator const RefObj<const T>& () const {
	  return *this;
	}

	RefObj<_T, _ContainerT> copy() const {
	  return RefObj<_T, _ContainerT>(**_container);
	}//*/
};

//============================================================================

// Specialized RefObj for pointer types.
template <typename _T>
class RefObj<_T*> : public RefObjBase<RefObjPointer<_T*>> {
public:
	typedef RefObjPointer<_T*> _ContainerT;
	typedef RefObj<_T*> _SelfT;

public:
	RefObj() : RefObjBase<_ContainerT>(nullptr) { }

	RefObj(_T* value, bool deleteWhenDone = true) :
		RefObjBase<_ContainerT>(deleteWhenDone ? new RefObjOwnedPointer<_T*>(value) : new RefObjPointer<_T*>(value))
	{ }

	RefObj(_SelfT& linkref) : RefObjBase<_ContainerT>(linkref._container) { }

	RefObj(const _SelfT& linkref) : RefObjBase<_ContainerT>(linkref._container) { }

	/*RefObj(_SelfT&& linkref) : RefObjBase<_ContainerT>(linkref._container) {
	  linkref._container = new _ContainerT();
	}//*/

	RefObj(_ContainerT* container) : RefObjBase<_ContainerT>(container) { }

public:

	_T* operator*() {
		return (_T*)this->_container->value;
	}
	const _T* operator*() const {
		return (const _T*)this->_container->value;
	}

	RefObj<const _T*> ConstRef() const {
		return RefObj<const _T*>(this->_container);
	}

	bool operator==(const _SelfT& b) const {
		return this->_container == b._container;
	}

	_SelfT& operator=(const _SelfT& b) {
		this->_Map(b._container);
		return *this;
	}

	_T* release() {
		return (_T*)this->_container->release();
	}

  //  virtual void makeabstract() = 0;

	/*template <typename T2>
	operator RefObj<T2>& () {
	return *this;
	}

	template <typename T2>
	operator const RefObj<T2>& () const {
	return *this;
	}

	operator RefObj<const T>& () {
	return *this;
	}
	operator const RefObj<const T>& () const {
	return *this;
	}

	RefObj<_T, _ContainerT> copy() const {
	return RefObj<_T, _ContainerT>(**_container);
	}//*/
};

}
