#pragma once

#include "RefHolder.h"

namespace seLib {

template <typename T>
class RefCounter_t : public RefHolder_t<T> {
public:
	size_t Count { 0 };

protected:
	virtual void Last() = 0;

	bool RefTake(RefAccessorBase_t const &) {
		Count++;
		return true;
	}

	void RefRelease(RefAccessorBase_t const &) {
		Count--;
		if (!Count)
			Last();
	}

	bool RefTake(RefAccessorBase_t const & accessor) const override final {
		return const_cast<RefCounter_t*>(this)->RefTake(accessor);
	}

	void RefRelease(RefAccessorBase_t const & accessor) const override final {
		const_cast<RefCounter_t*>(this)->RefRelease(accessor);
	}
};


template <typename T>
class OptionalHeapRefCounter_t final : public RefCounter_t<T> {
public:
	struct NotInitializedException_t : std::exception {};

	T* Instance { nullptr };

protected:
	void Last() override {
		if (!Instance)
			return;
		delete Instance;
		Instance = nullptr;
	}

	//[[nodiscard]] T* data() {
	//	return Instance;
	//}
	//
	//[[nodiscard]] T* data() const override {
	//	return const_cast<OptionalHeapRefCounter_t*>(this)->data();
	//}

public:
	template <typename...Args_T>
	[[nodiscard]] auto First(Args_T...args) {
		if (!this->Count) {
			Instance = new T(args...);
		}
		//return RefAccessor_t<T>(*this, Instance);
		return RefHolderBase_t::GetAccessor<T>(Instance);
	}

	[[nodiscard]] auto First() {
		if (!this->Count) {
			Instance = new T();
		}
		return RefHolderBase_t::GetAccessor<T>(Instance);
	}

	template <typename T2 = T const>
	[[nodiscard]] RefAccessor_t<T2> GetAccessor() const {
		static_assert(std::is_const<T2>, "Requested type must be const.");
		if (Instance == nullptr)
			throw NotInitializedException_t();
		return RefHolderBase_t::GetAccessor<T>(Instance);
	}

	template <typename T2 = T>
	[[nodiscard]] RefAccessor_t<T2> GetAccessor() {
		if (Instance == nullptr)
			return First();
		return RefHolderBase_t::GetAccessor<T>(Instance);
	}
};


template <typename T>
class SharedRefCounter_t final : public RefCounter_t<T> {
public:
	T Instance;

protected:
	void Last() override {
		delete this;
	}

	//[[nodiscard]] T* data() {
	//	return Instance;
	//}
	//
	//[[nodiscard]] T* data() const override {
	//	return const_cast<SharedRefCounter_t*>(this)->data();
	//}

public:
	template <typename...Args_T>
	SharedRefCounter_t(Args_T...args) : Instance(args...) {}

	template <typename...Args_T>
	[[nodiscard]] static auto Create(Args_T...args) {
		auto instance = new SharedRefCounter_t<T>(args...);
		return instance->GetAccessor();
	}

	template <typename T2 = T const>
	[[nodiscard]] RefAccessor_t<T2> GetAccessor() const {
		static_assert(std::is_const<T2>, "Requested type must be const.");
		return RefHolderBase_t::GetAccessor<T2>(&Instance);
		//return RefAccessor_t<T2>(*this, &Instance);
	}

	template <typename T2 = T>
	[[nodiscard]] RefAccessor_t<T2> GetAccessor() {
		return RefHolderBase_t::GetAccessor<T2>(&Instance);
	}
};


template <typename T>
class WrapRefCounter_t final : public RefCounter_t<T> {
public:
	T& Instance;

protected:
	void Last() override {
		delete this;
	}

	//[[nodiscard]] T* data() {
	//	return Instance;
	//}
	//
	//[[nodiscard]] T* data() const override {
	//	return const_cast<WrapRefCounter_t*>(this)->data();
	//}

public:
	WrapRefCounter_t(T& arg) : Instance(arg) {}

	static auto Create(T& arg) {
		auto instance = new SharedRefCounter_t<T>(arg);
		return instance->GetAccessor();
	}

	template <typename T2 = T const>
	[[nodiscard]] RefAccessor_t<T2> GetAccessor() const {
		static_assert(std::is_const<T2>, "Requested type must be const.");
		return RefHolderBase_t::GetAccessor<T2>(&Instance);
	}

	template <typename T2 = T>
	[[nodiscard]] RefAccessor_t<T2> GetAccessor() {
		return RefHolderBase_t::GetAccessor<T2>(&Instance);
	}
};

}
