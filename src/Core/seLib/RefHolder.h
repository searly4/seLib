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

#if __cplusplus < 201703L
#error C++17 support is required.
// Note: Visual C++ requires the compiler option "/Zc:__cplusplus" to set the define correctly
#endif

#include <type_traits>
#include <memory>
#include <variant>

namespace seLib {

#pragma managed(push, off) // virtual class definitions must be in an unmanaged section

struct PointerOutOfRangeException_t : public std::exception {};
struct RefTakeError : public std::exception {};

class RefAccessorBase_t;
class RefHolderBase_t;
class RefConstHolderWrapper_t;
template <typename T> class RefHolder_t;
template <typename T> class RefAccessor_t;


class RefConstHolderBase_t {
protected:
	template <typename T2>
	friend class RefAccessor_t;

#if defined(__cplusplus_cli)
	template <class T>
	friend ref class RefAccessor;
#endif

	virtual void RefTake() const {}
	virtual void RefRelease() const {}

public:
	virtual ~RefConstHolderBase_t() {}

	class Ref_t {
	private:
		RefConstHolderBase_t const * mRef;
	public:
		Ref_t(RefConstHolderBase_t const & holder) : mRef(&holder) {
			holder.RefTake();
		}

		Ref_t(Ref_t const& b) : mRef(b.mRef) {
			mRef->RefTake();
		}

		Ref_t(Ref_t&& b) noexcept : mRef(b.mRef) {
			mRef = nullptr;
		}

		~Ref_t() {
			if (mRef != nullptr)
				mRef->RefRelease();
			mRef = nullptr;
		}

		[[nodiscard]] constexpr RefConstHolderBase_t const * operator->() const {
			return mRef;
		}

		[[nodiscard]] constexpr RefConstHolderBase_t const & operator*() const {
			return *mRef;
		}

		Ref_t& operator=(Ref_t const& b) {
			if (mRef != nullptr)
				mRef->RefRelease();
			mRef = nullptr;
			b->RefTake();
			return *this;
		}

		[[nodiscard]] constexpr bool operator==(Ref_t const& b) const {
			return mRef == b.mRef;
		}
};
};



class RefHolderBase_t {
protected:
	template <typename T2>
	friend class RefAccessor_t;

#if defined(__cplusplus_cli)
	template <class T>
	friend ref class RefAccessor;
#endif

	virtual void RefTake() {}
	virtual void RefRelease() {}

public:
	virtual ~RefHolderBase_t() {}

	class Ref_t {
	private:
		RefHolderBase_t* mRef;
	public:
		Ref_t(RefHolderBase_t& holder) : mRef(&holder) {
			holder.RefTake();
		}

		Ref_t(Ref_t const& b) : mRef(b.mRef) {
			if (mRef != nullptr)
				mRef->RefTake();
		}

		Ref_t(Ref_t&& b) noexcept : mRef(b.mRef) {
			mRef = nullptr;
		}

		~Ref_t() {
			if (mRef != nullptr)
				mRef->RefRelease();
			mRef = nullptr;
		}

		[[nodiscard]] constexpr RefHolderBase_t* operator->() const {
			return mRef;
		}

		[[nodiscard]] constexpr RefHolderBase_t& operator*() const {
			return *mRef;
		}

		Ref_t& operator=(Ref_t const& b) {
			if (mRef != nullptr)
				mRef->RefRelease();
			mRef = nullptr;
			b->RefTake();
			return *this;
		}

		[[nodiscard]] constexpr bool operator==(Ref_t const& b) const {
			return mRef == b.mRef;
		}
	};
};

#pragma managed(pop)



template <typename T>
class RefHolder_t : public RefHolderBase_t {
protected:
	template <typename T2>
	friend class RefAccessor_t;

#if defined(__cplusplus_cli)
	template <class T>
	friend ref class RefAccessor;
#endif

	RefHolder_t() : RefHolderBase_t() {}

	[[nodiscard]] virtual T* data() = 0;
};


template <typename T>
class RefConstHolder_t : public RefConstHolderBase_t {
protected:
	template <typename T2>
	friend class RefAccessor_t;

#if defined(__cplusplus_cli)
	template <class T>
	friend ref class RefAccessor;
#endif

	RefConstHolder_t() : RefConstHolderBase_t() {}

	[[nodiscard]] virtual T* data() const = 0;
};

namespace {
	struct shared_ptr_offset_base_t {
		virtual ~shared_ptr_offset_base_t() {}
	};

	template <typename T_base>
	struct shared_ptr_offset_t : public shared_ptr_offset_base_t {
		std::shared_ptr<T_base> ptr;
	};

}

template <typename T>
using RefAccessorVariant_t = std::variant<
	std::nullptr_t,
	std::shared_ptr<T>,
	shared_ptr_offset_base_t*,
	RefConstHolderBase_t::Ref_t,
	RefHolderBase_t::Ref_t
>;

template <typename T>
class RefAccessor_t {
public:

protected:
	template <typename T2>
	friend class RefAccessor_t;

	RefAccessorVariant_t<T> mRef { nullptr };

	T* Data { nullptr };

	void RefRelease() {
		mRef.emplace<std::nullptr_t>(nullptr);
	}

	void RefTake(std::shared_ptr<T> const& b) {
		Data = mRef.emplace<std::shared_ptr<T>>(b).get();
	}

	template <typename T2>
	void RefTake(std::shared_ptr<T2> const& b) {
		if constexpr (std::is_base_of_v<T, T2>) {
			Data = mRef.emplace<std::shared_ptr<T>>(std::static_pointer_cast<T>(b)).get();
		} else {
			Data = mRef.emplace<std::shared_ptr<T>>(std::dynamic_pointer_cast<T>(b)).get();
		}
	}

	template <typename T2>
	void RefTake(RefConstHolder_t<T2> const& new_holder) {
		mRef.emplace<RefConstHolderBase_t::Ref_t>(new_holder);
		if constexpr (std::is_base_of_v<T, T2>) {
			Data = static_cast<T*>(new_holder.data());
		} else {
			Data = dynamic_cast<T*>(new_holder.data());
		}
	}

	template <typename T2>
	void RefTake(RefHolder_t<T2> & new_holder) {
		mRef.emplace<RefHolderBase_t::Ref_t>(new_holder);
		if constexpr (std::is_base_of_v<T, T2>) {
			Data = static_cast<T*>(new_holder.data());
		} else {
			Data = dynamic_cast<T*>(new_holder.data());
		}
	}

	template <typename T2>
	void RefTake2(RefAccessorVariant_t<T2> const& b, T* data_ptr) {
		if (std::holds_alternative<std::shared_ptr<T2>>(b)) {
			RefTake(std::get<std::shared_ptr<T2>>(b));
		} else if (std::holds_alternative<RefHolderBase_t::Ref_t>(b)) {
			mRef.emplace<RefHolderBase_t::Ref_t>(std::get<RefHolderBase_t::Ref_t>(b));
		} else if (std::holds_alternative<RefConstHolderBase_t::Ref_t>(b)) {
			mRef.emplace<RefConstHolderBase_t::Ref_t>(std::get<RefConstHolderBase_t::Ref_t>(b));
		} else {
			return;
		}

		Data = data_ptr;
	}

	template <typename T2>
	void RefTake(RefAccessor_t<T2> const& b, T* data_ptr) {
		if (data_ptr < Data || data_ptr >(Data + sizeof(T) - sizeof(T2)))
			throw PointerOutOfRangeException_t;
		RefTake2(b.mRef, data_ptr);
	}

	template <typename T2>
	void RefTake(RefAccessor_t<T2> const& b) {
		RefTake2(b.mRef, b.Data);
	}

	RefAccessor_t(RefAccessor_t const& b, T* data_ptr) {
		RefTake(b.mRef, data_ptr);
	}

public:
	RefAccessor_t(T* data) : Data(data) {}

	template <typename T2>
	RefAccessor_t(std::shared_ptr<T2> const & ref) {
		RefTake(ref);
	}

	RefAccessor_t() = default;

	~RefAccessor_t() {
		RefRelease();
	}

	RefAccessor_t(RefAccessor_t const & b) {
		RefTake(b);
	}

	RefAccessor_t(RefAccessor_t && b) noexcept
		: Data(b.Data)
	{
		mRef.swap(b.mRef);
		b.Data = nullptr;
	}

	template <typename T2>
	explicit RefAccessor_t(RefAccessor_t<T2> const& b) {
		RefTake(b);
	}

	template <typename T2>
	explicit RefAccessor_t(RefConstHolder_t<T2> const & holder) {
		//static_assert(!std::is_const<T2> || std::is_const<T>, "Requested type must be const.");
		RefTake(holder);
	}

	template <typename T2>
	explicit RefAccessor_t(RefHolder_t<T2> & holder) {
		RefTake(holder);
	}

	template <typename T2>
	RefAccessor_t& operator=(RefAccessor_t<T2> const& b) {
		RefTake(b);
		return *this;
	}

	RefAccessor_t& operator=(RefAccessor_t const & b) {
		RefTake(b);
		return *this;
	}

	[[nodiscard]] constexpr T* operator->() const {
		return Data;
	}

	[[nodiscard]] constexpr T& operator*() const {
		return *Data;
	}

	[[nodiscard]] constexpr T& operator[](size_t index) const {
		return Data[index];
	}

	[[nodiscard]] constexpr T* data() const {
		return Data;
	}

	[[nodiscard]] constexpr bool operator==(RefAccessor_t const & b) const {
		return Data == b.Data;
	}

	template <typename T2>
	[[nodiscard]] constexpr RefAccessor_t<T2> Subobject(T2 * b) const {
		return RefAccessor_t<T2>(*this, b);
	}

	template <typename T2=T, typename...Args_T>
	[[nodiscard]] static RefAccessor_t<T> InstantiateShared(Args_T...args);

	constexpr bool IsValid() const noexcept { return !std::holds_alternative<nullptr_t>(mRef); }

	//template <typename T2>
	//bool IsSame(RefAccessor_t<T2> const& b) const {
	//	if (mRef.index() != b.mRef.index())
	//		return false;
	//	if (std::holds_alternative<std::shared_ptr<T>>(mRef)) {
	//	}
	//	return mRef == b.Holder;
	//}

#if defined(__cplusplus_cli)
	template <class T>
	friend ref class RefAccessor;

	//T* CLI_GetData() const {
	//	return Data;
	//}
	//RefConstHolderBase_t const* CLI_GetHolder() const {
	//	return Holder;
	//}
#endif

	//struct CLI_Accessor_t {
	//	static RefConstHolderBase_t const* GetHolder(RefAccessor_t<T> const& b) {
	//		return b.Holder;
	//	}
	//	static T* GetData(RefAccessor_t<T> const& b) {
	//		return b.Data;
	//	}
	//	static RefAccessor_t<T> Instantiate(RefConstHolderBase_t const& b, T* data_ptr);
	//};
	//friend struct CLI_Accessor_t;
};



///-------------------------------------------------------------------------------------------------
/// @class	StaticRefHolder_t
/// @brief	An object container for static or stack storage.
/// @tparam	T	Generic type parameter.
template <typename T>
class StaticRefHolder_t : public RefHolder_t<T> {
protected:
	T Instance;

	void RefTake() override {}
	void RefRelease() override {}

public:
	template <typename...Args_T>
	StaticRefHolder_t(Args_T...args) : Instance(args...) {}

	[[nodiscard]] T* data() override {
		return &Instance;
	}

	//template <typename T2 = T const>
	////typename std::enable_if_t<std::is_const<T2>, RefAccessor_t<T2>>
	//[[nodiscard]] RefAccessor_t<T2> GetAccessor() const {
	//	static_assert(std::is_const<T2>, "Requested type must be const.");
	//	return RefConstHolder_t<T>::template GetAccessor<T2>(&Instance);
	//}
	//
	//template <typename T2 = T>
	//[[nodiscard]] RefAccessor_t<T2> GetAccessor() {
	//	return RefConstHolder_t<T>::template GetAccessor<T2>(&Instance);
	//}
};


///-------------------------------------------------------------------------------------------------
/// @class	StaticRefHolder_t<T const>
/// @brief	An object container for static or stack storage. Specialized for const data.
/// @tparam	T	Generic type parameter.
template <typename T>
class StaticRefHolder_t<T const> : public RefConstHolder_t<T const> {
protected:
	T const Instance;

	bool RefTake() const override { return true; }
	void RefRelease() const override {}

public:
	template <typename...Args_T>
	constexpr StaticRefHolder_t(Args_T...args) : Instance(args...) {}

	[[nodiscard]] T const * data() const override {
		return &Instance;
	}
};


template <typename T>
class RefCounter_t : public RefHolder_t<T> {
public:
	size_t Count{ 0 };

protected:
	virtual void First() {
	}
	virtual void Last() {}

	void RefTake() override final {
		if (!Count)
			First();
		Count++;
	}

	void RefRelease() override final {
		Count--;
		if (!Count)
			Last();
	}
};


template <typename T>
class OptionalHeapRefCounter_t final : public RefCounter_t<T> {
public:
	struct NotInitializedException_t : std::exception {};

	T* Instance{ nullptr };

protected:
	void Last() override {
		if (!Instance)
			return;
		auto inst = Instance;
		Instance = nullptr;
		delete inst;
	}

public:
	[[nodiscard]] T* data() override {
		return Instance;
	}

	//template <typename...Args_T>
	//[[nodiscard]] auto First(Args_T...args) {
	//	if (!this->Count) {
	//		Instance = new T(args...);
	//	}
	//	//return RefAccessor_t<T>(*this, Instance);
	//	return RefConstHolderBase_t::GetAccessor<T>(Instance);
	//}

	void First() override {
		if (!this->Count) {
			Instance = new T();
		}
		//return RefHolderBase_t::GetAccessor<T>(Instance);
	}
};


template <typename T>
template <typename T2, typename...Args_T>
[[nodiscard]] inline static RefAccessor_t<T> RefAccessor_t<T>::InstantiateShared(Args_T...args) {
	return RefAccessor_t<T>{std::make_shared<T2>(args...)};
}

}
