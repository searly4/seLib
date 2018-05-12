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

class ServiceState_t {
public:
    bool Status_Running : 1;
    bool Status_Suspended : 1;
    bool Action_Start : 1;
    bool Action_Stop : 1;
    bool Action_Suspend : 1;
    bool Action_Resume : 1;
    
    virtual void Init() {}
};

// Base class for service definition classes
class ServiceDefBase {
public:
	bool AutoStart : 1;

    constexpr ServiceDefBase(bool _autostart) :
        AutoStart(_autostart)
    {}
	//virtual ~ServiceDef() = 0;

    virtual ServiceMessages_t Init() const = 0;
    virtual ServiceMessages_t Start() const = 0;
	virtual ServiceMessages_t Stop() const = 0;
	virtual ServiceMessages_t Update() const = 0;
	/*virtual ServiceMessages_t Suspend() const = 0;
	virtual ServiceMessages_t Resume() const = 0;*/
};

// Template class to add a service-specific runtime state pointer
template <typename ServiceState_T>
class ServiceDef : public ServiceDefBase {
public:
    ServiceState_T* State;

    constexpr ServiceDef(ServiceState_T* _state, bool _autostart) :
        State(_state),
        ServiceDefBase(_autostart)
    {}

    virtual ServiceMessages_t Init() const override {
        memset(State, 0, sizeof(ServiceState_T));
        return SvcMsg_OK;
    }
    virtual ServiceMessages_t Start() const override {
        State->Action_Start = true;
        State->Action_Resume = true;
        return SvcMsg_OK;
    }
    virtual ServiceMessages_t Stop() const override {
        State->Action_Suspend = true;
        State->Action_Stop = true;
        return SvcMsg_OK;
    }
    /*virtual ServiceMessages_t Suspend() const override {
        State->Action_Resume = true;
        return SvcMsg_OK;
    }
    virtual ServiceMessages_t Resume() const override {
        State->Action_Suspend = true;
        return SvcMsg_OK;
    }*/
};

}
}

#endif
