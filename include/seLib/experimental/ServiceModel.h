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

/*
 * ServiceModel.h
 *
 *  Created on: Jul 1, 2016
 *      Author: scotte
 */

#include <stdint.h>
#include <string.h>
//#include <stdbool>
#include <seLib_Config.h>
#include <seLib/Platform.h>
#include <seLib/experimental/seLib_Util.h>
#include <seLib/experimental/StateTransitions.h>
#include <seLib/experimental/BitFlags.h>

//= C compatible definitions =================================================

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ServiceMessages_e {
	SvcMsg_OK = 0,
	SvcMsg_None,
	SvcMsg_FaultGeneric,
	SvcMsg_NotReady,
	SvcMsg_Busy,
	SvcMsg_NoMoreTasks,
	SvcMsg_TasksPending,
	SvcMsg_Last,
} ServiceMessages_t;

ServiceMessages_t ServiceInit(const void* serviceptr);
ServiceMessages_t ServiceStart(const void* serviceptr);
ServiceMessages_t ServiceStop(const void* serviceptr);
ServiceMessages_t ServiceUpdate(const void* serviceptr);

#ifdef __cplusplus
};

namespace seLib {
namespace ServiceModel {

//= C++ specific definitions =================================================

enum class ServiceState_e {
	NotInitialized,
	Idle,
    Running,
	Sleep
};

enum class ServiceDefFlags_e {
	AutoStart,
	AlwaysPoll
};
typedef BitFlags<ServiceDefFlags_e> ServiceDefFlags_t;


class ServiceDefBase;
class ServiceManagerDef;

class ServiceState_t {
public:
	seLib::StateTransitions<ServiceState_e> State = ServiceState_e::NotInitialized;
	bool ServiceAttention : 1;
    
    virtual void Init() {}
};

// Base class for service definition classes
class ServiceDefBase {
public:
	ServiceManagerDef const & Manager;
    ServiceState_t* State;
	ServiceDefBase const * const * Dependencies = nullptr;
	uint8_t DependencyCount;
	uint32_t AttentionMask = 1;
	bool AutoStart : 1;
	bool AlwaysPoll : 1;

    constexpr ServiceDefBase(ServiceState_t* _state, ServiceManagerDef const & manager, ServiceDefFlags_t _flags) :
        State(_state), Manager(manager), DependencyCount(0),
		AutoStart(_flags[ServiceDefFlags_t::Flags::AutoStart]),
		AlwaysPoll(_flags[ServiceDefFlags_t::Flags::AlwaysPoll])
    {
	    //AutoStart = _flags[ServiceDefFlags_t::Flags::AutoStart];
    }

	constexpr ServiceDefBase(ServiceState_t* _state, ServiceManagerDef const & manager, bool _autostart) :
        State(_state), Manager(manager), AutoStart(_autostart), DependencyCount(0), AlwaysPoll(false)
    {}

	constexpr ServiceDefBase(ServiceState_t* _state, ServiceManagerDef const & manager, ServiceDefBase const * const * dependencies,  uint8_t dependency_count, bool _autostart) :
        State(_state), Manager(manager), AutoStart(_autostart), Dependencies(dependencies), DependencyCount(dependency_count), AlwaysPoll(false)
    {}
	
	constexpr ServiceDefBase(const ServiceDefBase&) = default;
	
	//virtual ~ServiceDef() = 0;

    virtual ServiceMessages_t Init() const = 0;
	virtual ServiceMessages_t Uninit() const {
		return ServiceMessages_t::SvcMsg_OK;
	}
    virtual ServiceMessages_t Start() const;
	virtual ServiceMessages_t Stop() const;
	virtual ServiceMessages_t Update() const;
	ServiceState_e Status() const {
		return State->State.CurrentValue;
	}
	virtual ServiceMessages_t TaskStatus() const {
		return (State->ServiceAttention) ? ServiceMessages_t::SvcMsg_TasksPending : ServiceMessages_t::SvcMsg_NoMoreTasks;
	}
	void Attention() const;
};

// Template class to add a service-specific runtime state pointer
template <typename ServiceState_T>
class ServiceDef : public ServiceDefBase {
public:
    constexpr ServiceDef(ServiceState_T* _state, ServiceManagerDef const & manager, bool _autostart) :
        ServiceDefBase(_state, manager, _autostart)
    {}

    constexpr ServiceDef(ServiceState_T* _state, ServiceManagerDef const & manager, ServiceDefBase const * const * dependencies, uint8_t dependency_count, bool _autostart) :
        ServiceDefBase(_state, manager, dependencies, dependency_count, _autostart)
    {}

	inline ServiceState_T& StateRef() const {
		return (ServiceState_T&)*State;
	}

	virtual ServiceMessages_t Init() const override {
        //memset(State, 0, sizeof(ServiceState_T));
	    State->State.AssignImmediate(ServiceState_e::Idle);
        return SvcMsg_OK;
    }
};

class ServiceManagerState {
public:
	uint32_t AttentionFlags = 0;
};

class ServiceManagerDef {
public:
	ServiceManagerState* State;
	ServiceDefBase const * const * ServiceList;
	uint8_t ServiceCount;

    constexpr ServiceManagerDef(ServiceManagerState* _state, ServiceDefBase const * const * _servicelist, uint8_t service_count) :
        State(_state), ServiceList(_servicelist), ServiceCount(service_count)
    {}
	
	ServiceMessages_e Init() const;
	ServiceMessages_e Start() const;
	ServiceMessages_e Run() const;
	ServiceMessages_e Stop() const;
	
	ServiceMessages_e Attention(uint32_t mask) const {
		Platform::CriticalSection_t crit_lock;
		State->AttentionFlags |= mask;
		Platform::SetWakeFlag();
	}
};

inline void ServiceDefBase::Attention() const {
	Manager.Attention(AttentionMask);
}


}
}

#endif
