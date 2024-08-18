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

#if defined(__cplusplus_cli)
#include <vcclr.h>
using namespace System;

#include "RefHolder.h"

namespace seLib {

//template <typename T> ref class RefAccessor;


template <class T>
public ref class RefAccessor {
protected:
	RefAccessorVariant_t<T>* mRef{ new RefAccessorVariant_t<T>() };
	T* m_Data{ nullptr };

	void RefRelease() {
		mRef->emplace<std::nullptr_t>(nullptr);
	}

	void RefTake(std::shared_ptr<T> const& b) {
		m_Data = mRef->emplace<std::shared_ptr<T>>(b).get();
	}

	template <typename T2>
	void RefTake(std::shared_ptr<T2> const& b) {
		if constexpr (std::is_base_of_v<T, T2>) {
			m_Data = mRef->emplace<std::shared_ptr<T>>(std::static_pointer_cast<T>(b)).get();
		} else {
			m_Data = mRef->emplace<std::shared_ptr<T>>(std::dynamic_pointer_cast<T>(b)).get();
		}
	}

	template <typename T2>
	void RefTake(RefConstHolder_t<T2> const& new_holder) {
		mRef->emplace<RefConstHolderBase_t::Ref_t>(&new_holder);
		if constexpr (std::is_base_of_v<T, T2>) {
			m_Data = static_cast<T*>(new_holder.data());
		} else {
			m_Data = dynamic_cast<T*>(new_holder.data());
		}
	}

	template <typename T2>
	void RefTake(RefHolder_t<T2>& new_holder) {
		mRef->emplace<RefHolderBase_t::Ref_t>(new_holder);
		if constexpr (std::is_base_of_v<T, T2>) {
			m_Data = static_cast<T*>(new_holder.data());
		} else {
			m_Data = dynamic_cast<T*>(new_holder.data());
		}
	}

	template <typename T2>
	void RefTake2(RefAccessorVariant_t<T2> const& b, T* data_ptr) {
		if (std::holds_alternative<std::shared_ptr<T2>>(b)) {
			RefTake(std::get<std::shared_ptr<T2>>(b));
		} else if (std::holds_alternative<RefHolderBase_t::Ref_t>(b)) {
			mRef->emplace<RefHolderBase_t::Ref_t>(std::get<RefHolderBase_t::Ref_t>(b));
		} else if (std::holds_alternative<RefConstHolderBase_t::Ref_t>(b)) {
			mRef->emplace<RefConstHolderBase_t::Ref_t>(std::get<RefConstHolderBase_t::Ref_t>(b));
		} else {
			return;
		}

		m_Data = data_ptr;
	}

	template <typename T2>
	void RefTake(RefAccessor_t<T2> const& b, T* data_ptr) {
		if (data_ptr < m_Data || data_ptr > (m_Data + sizeof(T) - sizeof(T2)))
			throw PointerOutOfRangeException_t;
		RefTake2(b.mRef, data_ptr);
	}

	template <typename T2>
	void RefTake(RefAccessor_t<T2> const& b) {
		RefTake2(b.mRef, b.Data);
	}


public:
	property T* Data {
		T* get() { return m_Data; }
	};

	RefAccessor() {}

	RefAccessor(RefAccessor^ b) {
		RefTake(b->mRef, b->m_Data);
	}

	RefAccessor(RefAccessor_t<T> const& b) {
		RefTake2(b.mRef, b.Data);
	}

	template <typename T2>
	RefAccessor(RefAccessor_t<T2> const& b) {
		RefTake(b.mRef, b.Data);
	}

	!RefAccessor() {
		if (mRef != nullptr)
			delete mRef;
		mRef = nullptr;
	}

	~RefAccessor() {
		if (mRef != nullptr)
			delete mRef;
		mRef = nullptr;
		//this->!RefAccessor();
	}

	RefAccessor^ operator=(RefAccessor_t<T> const& b) {
		RefTake(b.mRef, b.Data);
		return this;
	}

	template <typename T2>
	RefAccessor^ operator=(RefAccessor_t<T2> const& b) {
		RefTake(b.mRef, b.Data);
		return this;
	}

	bool IsValid() { return !std::holds_alternative<nullptr_t>(mRef); }

	//template <typename T2>
	//bool IsSame(RefAccessor_t<T2> const& b) {
	//	return m_Holder == b.CLI_GetHolder();
	//}
};

//template <typename T>
//RefAccessor_t<T> RefAccessor_t<T>::CLI_Accessor_t::Instantiate(RefConstHolderBase_t const& b, T* data_ptr) {
//	return RefAccessor_t<T>(b, data_ptr);
//}

} // namespace seLib

#endif
