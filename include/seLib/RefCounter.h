#pragma once

#include "RefHolder.h"

namespace seLib {

template <typename T>
class RefCounter_t : public RefHolder_t<T> {
public:
	size_t Count { 0 };

protected:
	virtual void Last() = 0;

	bool RefTake(RefAccessorBase_t const &) override {
		Count++;
		return true;
	}

	void RefRelease(RefAccessorBase_t const &) override {
		Count--;
		if (!Count)
			Last();
	}
};


template <typename T>
class OptionalHeapRefCounter_t final : public RefCounter_t<T> {
public:
	T* Instance { nullptr };

protected:
	void Last() override {
		if (!Instance)
			return;
		delete Instance;
		Instance = nullptr;
	}

	T* data() override {
		return Instance;
	}

public:
	template <typename...Args_T>
	auto First(Args_T...args) {
		if (!this->Count) {
			Instance = new T(args...);
		}
		return RefHolder_t<T>::GetAccessor();
	}

	auto First() {
		if (!this->Count) {
			Instance = new T();
		}
		return RefHolder_t<T>::GetAccessor();
	}

	seLib::RefAccessor_t<T> GetAccessor() {
		if (Instance == nullptr)
			return First();
		return RefHolder_t<T>::GetAccessor();
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

	T* data() override {
		return &Instance;
	}

public:
	template <typename...Args_T>
	SharedRefCounter_t(Args_T...args) : Instance(args...) {}

	template <typename...Args_T>
	static auto Create(Args_T...args) {
		auto instance = new SharedRefCounter_t<T>(args...);
		return instance->GetAccessor();
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

	T* data() override {
		return &Instance;
	}

public:
	WrapRefCounter_t(T& arg) : Instance(arg) {}

	static auto Create(T& arg) {
		auto instance = new SharedRefCounter_t<T>(arg);
		return instance->GetAccessor();
	}
};

}
