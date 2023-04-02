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

namespace seLib {

struct PointerOutOfRangeException_t : public std::exception {};

template <typename T> class RefHolder_t;
class RefAccessorBase_t;
template <typename T> class RefAccessor_t;
template <typename T> class ConstWrapper_t;


class RefHolderBase_t {
protected:
	friend class RefAccessorBase_t;

	virtual bool RefTake(RefAccessorBase_t const &) const { return true; }
	virtual void RefRelease(RefAccessorBase_t const &) const {}
	
	template <typename Value_T>
	[[nodiscard]] RefAccessor_t<Value_T> GetAccessor(Value_T* value_ptr) {
		return RefAccessor_t<Value_T>(*this, value_ptr);
	}
	template <typename Value_T>
	[[nodiscard]] RefAccessor_t<Value_T> GetAccessor(Value_T* value_ptr) const {
		return RefAccessor_t<Value_T>(*this, value_ptr);
	}

public:
	virtual ~RefHolderBase_t() {}
};


template <typename T>
class RefHolder_t : public RefHolderBase_t {
protected:
	friend class RefAccessor_t<T>;

	//virtual T* data() const = 0;

	//template <typename T2 = T>
	//typename std::enable_if_t<!std::is_same_v<T, T2>, RefAccessor_t<T2>>
	//GetAccessor() const {
	//	return RefAccessor_t<T2>(*this);
	//}
	//
	//template <typename T2 = T>
	//typename std::enable_if_t<std::is_same_v<T, T2>, RefAccessor_t<T>>
	//GetAccessor() const {
	//	return RefAccessor_t<T>(*this);
	//}

	RefHolder_t() : RefHolderBase_t() {}
public:
};


class RefAccessorBase_t {
protected:
	friend class RefHolderBase_t;
	RefHolderBase_t const * Holder { nullptr };

	void RefRelease() {
		if (Holder == nullptr)
			return;
		Holder->RefRelease(*this);
		Holder = nullptr;
	}

	bool RefTake(RefHolderBase_t const * new_holder) {
		if (Holder != nullptr)
			RefRelease();
		if (new_holder == nullptr)
			return false;
		if (!new_holder->RefTake(*this))
			return false;
		Holder = new_holder;
		return true;
	}

	bool RefTake(RefAccessorBase_t const & b) {
		return RefTake(b.Holder);
	}

	RefAccessorBase_t() {}

	RefAccessorBase_t(RefHolderBase_t const & holder) {
		RefTake(&holder);
	}

	RefAccessorBase_t(RefAccessorBase_t const & b) {
		RefTake(b.Holder);
	}

	RefAccessorBase_t(RefAccessorBase_t && b) noexcept {
		Holder = b.Holder;
		b.Holder = nullptr;
	}

	~RefAccessorBase_t() {
		if (Holder == nullptr)
			return;
		Holder->RefRelease(*this);
	}

public:
	bool IsValid() const noexcept { return Holder != nullptr; }

	bool IsSame(RefAccessorBase_t const & b) const {
		return Holder == b.Holder;
	}
};


template <typename T>
class RefAccessor_t : public RefAccessorBase_t {
protected:
	friend class RefHolderBase_t;
	//friend class RefHolder_t<T>;

	T* Data { nullptr };

	RefAccessor_t(RefHolderBase_t const & b, T* data_ptr)
		: RefAccessorBase_t(b),
		Data((Holder != nullptr) ? data_ptr : nullptr)
	{}

	RefAccessor_t(RefHolder_t<T> const & b)
		: RefAccessorBase_t(b),
		Data((Holder != nullptr) ? b.data() : nullptr)
	{}

public:
	RefAccessor_t() = default;

	RefAccessor_t(RefAccessor_t const & b)
		: RefAccessorBase_t(b),
		Data((Holder != nullptr) ? b.Data : nullptr)
	{}

	RefAccessor_t(RefAccessor_t && b) noexcept
		: RefAccessorBase_t(std::move(b)),
		Data(b.Data)
	{
		b.Data = nullptr;
	}

	template <typename T2>
	explicit RefAccessor_t(RefAccessor_t<T2> const & b) {
		Data = dynamic_cast<T*>(b.data());
		if (Data == nullptr)
			return;
		if (!RefTake(b))
			Data = nullptr;
	}

	RefAccessor_t& operator=(RefAccessor_t const & b) {
		Data = RefTake(b.Holder) ? b.Data : nullptr;
		return *this;
	}

	[[nodiscard]] T* operator->() const {
		return Data;
	}

	[[nodiscard]] T& operator*() const {
		return *Data;
	}

	[[nodiscard]] auto operator[](size_t index) const {
		return (*Data)[index];
	}

	[[nodiscard]] T* data() const {
		return Data;
	}

	[[nodiscard]] bool operator==(RefAccessor_t const & b) const {
		return Holder == b.Holder;
	}

	template <typename T2>
	[[nodiscard]] RefAccessor_t<T2> Subobject(T2 * b) const {
		//if (b < Data || b > (Data + sizeof(T))
		//	throw PointerOutOfRangeException_t;
		return RefAccessor_t<T2>(*Holder, b);
	}
};


///-------------------------------------------------------------------------------------------------
/// @class	StaticRefHolder_t
/// @brief	An object container for static or stack storage.
/// @tparam	T	Generic type parameter.
template <typename T>
class StaticRefHolder_t : public RefHolder_t<T> {
protected:
	T Instance;

	bool RefTake(RefAccessorBase_t const &) const override { return true; }
	void RefRelease(RefAccessorBase_t const &) const override {}

	//[[nodiscard]] T* data() {
	//	return Instance;
	//}
	//
	//[[nodiscard]] T* data() const override {
	//	return const_cast<StaticRefHolder_t*>(this)->data();
	//}

public:
	template <typename...Args_T>
	StaticRefHolder_t(Args_T...args) : Instance(args...) {}
	using RefHolder_t<T>::GetAccessor;

	template <typename T2 = T const>
	//typename std::enable_if_t<std::is_const<T2>, RefAccessor_t<T2>>
	[[nodiscard]] RefAccessor_t<T2> GetAccessor() const {
		static_assert(std::is_const<T2>, "Requested type must be const.");
		return RefHolder_t<T>::template GetAccessor<T2>(&Instance);
	}

	template <typename T2 = T>
	[[nodiscard]] RefAccessor_t<T2> GetAccessor() {
		return RefHolder_t<T>::template GetAccessor<T2>(&Instance);
	}
};


///-------------------------------------------------------------------------------------------------
/// @class	StaticRefHolder_t<T const>
/// @brief	An object container for static or stack storage. Specialized for const data.
/// @tparam	T	Generic type parameter.
template <typename T>
class StaticRefHolder_t<T const> : public RefHolder_t<T const> {
protected:
	T const Instance;

	bool RefTake(RefAccessorBase_t const &) const override { return true; }
	void RefRelease(RefAccessorBase_t const &) const override {}

	//[[nodiscard]] T const * data() const override {
	//	return &Instance;
	//}

public:
	template <typename...Args_T>
	constexpr StaticRefHolder_t(Args_T...args) : Instance(args...) {}

	template <typename T2 = T const>
	[[nodiscard]] RefAccessor_t<T2> GetAccessor() const {
		static_assert(std::is_const<T2>, "Requested type must be const.");
		return RefAccessor_t<T2>(*this);
	}
};

}
