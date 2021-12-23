#pragma once

namespace seLib {

template <typename T> class RefHolder_t;
class RefAccessorBase_t;
template <typename T> class RefAccessor_t;
template <typename T> class ConstWrapper_t;


class RefHolderBase_t {
protected:
	friend class RefAccessorBase_t;
	friend class ConstRefHolderBase_t;

	virtual bool RefTake(RefAccessorBase_t const &) { return true; }
	virtual void RefRelease(RefAccessorBase_t const &) {}

public:
	virtual ~RefHolderBase_t() {}
};


class ConstRefHolderBase_t {
protected:
	friend class RefAccessorBase_t;

	virtual bool RefTake(RefAccessorBase_t const &) const { return true; }
	virtual void RefRelease(RefAccessorBase_t const &) const {}
	
	static bool RefTake(RefHolderBase_t & holder, RefAccessorBase_t const & accessor) {
		return holder.RefTake(accessor);
	}
	static void RefRelease(RefHolderBase_t & holder, RefAccessorBase_t const & accessor) {
		holder.RefRelease(accessor);
	}
public:
	virtual ~ConstRefHolderBase_t() {}
};


template <typename T>
class ConstRefHolder_t : public ConstRefHolderBase_t {
protected:
	friend class RefAccessor_t<T>;
	friend class RefHolder_t<T>;
	//class ConstWrapper_t;

	virtual T* data() const = 0;
	RefAccessor_t<T> GetAccessor() const {
		return RefAccessor_t<T>(*this);
	}

public:
};


template <typename T>
class ConstWrapper_t : public ConstRefHolder_t<T> {
public:
	RefHolder_t<T> & Holder;

	ConstWrapper_t(RefHolder_t<T> & holder) : Holder(holder) {}

	T* data() const override { return Holder.data(); }

	bool RefTake(RefAccessorBase_t const & accessor) const override {
		return ConstRefHolderBase_t::RefTake(Holder, accessor);
	}

	void RefRelease(RefAccessorBase_t const & accessor) const override {
		ConstRefHolderBase_t::RefRelease(Holder, accessor);
	}
};


template <typename T>
class RefHolder_t : public RefHolderBase_t {
protected:
	friend class RefAccessor_t<T>;
	friend class ConstWrapper_t<T>;

	ConstWrapper_t<T> ConstWrapper;

	virtual T* data() = 0;

	template <typename T2 = T>
	typename std::enable_if_t<!std::is_same_v<T, T2>, RefAccessor_t<T2>>
	GetAccessor() const {
		return RefAccessor_t<T2>(RefAccessor_t(ConstWrapper));
	}

	template <typename T2 = T>
	typename std::enable_if_t<std::is_same_v<T, T2>, RefAccessor_t<T>>
	GetAccessor() const {
		return RefAccessor_t(ConstWrapper);
	}

	RefHolder_t() : RefHolderBase_t(), ConstWrapper(*this) {}
public:
};


class RefAccessorBase_t {
protected:
	friend class RefHolderBase_t;
	ConstRefHolderBase_t const * Holder { nullptr };

	void RefRelease() {
		if (Holder == nullptr)
			return;
		Holder->RefRelease(*this);
		Holder = nullptr;
	}

	bool RefTake(ConstRefHolderBase_t const * new_holder) {
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

	RefAccessorBase_t(ConstRefHolderBase_t const & holder) {
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
	friend class RefHolder_t<T>;
	friend class ConstRefHolder_t<T>;

	T* Data { nullptr };

	RefAccessor_t(ConstRefHolder_t<T> const & b)
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

	T* operator->() const {
		return Data;
	}

	T& operator*() const {
		return *Data;
	}

	T* data() const {
		return Data;
	}

	bool operator==(RefAccessor_t const & b) const {
		return Holder == b.Holder;
	}
};


template <typename T>
class StaticRefHolder_t : public RefHolder_t<T> {
protected:
	T Instance;

	bool RefTake(RefAccessorBase_t const &) override { return true; }
	void RefRelease(RefAccessorBase_t const &) override {}
	T* data() override {
		return &Instance;
	}

public:
	template <typename...Args_T>
	StaticRefHolder_t(Args_T...args) : Instance(args...) {}
	using RefHolder_t<T>::GetAccessor;
};


template <typename T>
class StaticRefHolder_t<T const> : public ConstRefHolder_t<T> {
protected:
	T const Instance;

	bool RefTake(RefAccessorBase_t const &) override { return true; }
	void RefRelease(RefAccessorBase_t const &) override {}
	T const * data() override {
		return &Instance;
	}

public:
	template <typename...Args_T>
	StaticRefHolder_t(Args_T...args) : Instance(args...) {}
	using ConstRefHolder_t<T const>::GetAccessor;
};

}
