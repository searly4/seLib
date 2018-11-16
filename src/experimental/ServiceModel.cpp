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

#include <seLib_Config.h>

#if defined(EnableModule_seLib_ServiceModel) && EnableModule_seLib_ServiceModel

#include <stdint.h>
#include <stdlib.h>

#include <seLib/experimental/seLib_Util.h>
#include <seLib/experimental/ServiceModel.h>

/*template <class ServiceState_T>
ServiceMessages_t ServiceDef<ServiceState_T>::Init() const {
    memset(State);
    return SvcMsg_OK;
}*/

namespace seLib {
namespace ServiceModel {

ServiceMessages_t ServiceDefBase::Start() const {
	State->State = ServiceState_e::Running;
	State->ServiceAttention = true;
    return SvcMsg_OK;
}
ServiceMessages_t ServiceDefBase::Stop() const {
	State->State = ServiceState_e::Idle;
	State->ServiceAttention = true;
    return SvcMsg_OK;
}

ServiceMessages_t ServiceDefBase::Update() const {
	State->State.Update();
	State->ServiceAttention = false;
	return ServiceMessages_t::SvcMsg_NoMoreTasks;
}

ServiceMessages_e ServiceManagerDef::Init() const {
	bool allComplete;

	// Initialize services
	allComplete = true;
	for (int i = 0; i < ServiceCount; i++) {
		ServiceMessages_t retval = ServiceList[i]->Init();
		if (retval != ServiceMessages_t::SvcMsg_OK)
			return ServiceMessages_e::SvcMsg_FaultGeneric;
		allComplete &= (retval == ServiceMessages_t::SvcMsg_OK);
	}

	return ServiceMessages_e::SvcMsg_OK;
}

ServiceMessages_e ServiceManagerDef::Start() const {
	bool allComplete;

	// Start services
	do {
		allComplete = true;
		for (int i = 0; i < ServiceCount; i++) {
			if (!ServiceList[i]->AutoStart)
				continue;
			ServiceMessages_t retval = ServiceList[i]->Start();
			allComplete &= (retval == ServiceMessages_t::SvcMsg_OK);
			if (retval == ServiceMessages_t::SvcMsg_FaultGeneric)
				return ServiceMessages_t::SvcMsg_FaultGeneric;
		}
	} while (!allComplete);

	return ServiceMessages_e::SvcMsg_OK;
}

ServiceMessages_e ServiceManagerDef::Run() const {
	while (true) {
		ServiceMessages_e serviceRetval;
		bool keepAwake = false;
		uint32_t attention_flags;

		{
			Platform::CriticalSection_t crit_lock;
			volatile auto & flags_ref = State->AttentionFlags;
			attention_flags = flags_ref;
			flags_ref = 0;
		}

		// Run each service's Update function if requested
		for(uint8_t i = 0 ; i < ServiceCount; i++) {
			auto& service = ServiceList[i];
			 // check the attention flag to avoid unnecessary function calls
			if (!service->State->ServiceAttention && !(State->AttentionFlags & service->AttentionMask) && !service->AlwaysPoll)
				continue;
			
			ServiceMessages_t retval = service->Update();
			keepAwake |= (retval == SvcMsg_TasksPending);   //*/
		}

		for(uint8_t i = 0 ; i < ServiceCount; i++) {
			auto& service = ServiceList[i];
			keepAwake |= (service->State->ServiceAttention);   //*/
		}
		
		keepAwake |= (State->AttentionFlags != 0);

		if (!keepAwake) {
			// possible race here, but chip theoretically won't sleep
			// anyways if interrupts are pending
			seLib::Platform::Sleep();
		} else {
		}
	}

	return ServiceMessages_e::SvcMsg_OK;
}

ServiceMessages_e ServiceManagerDef::Stop() const {
	bool allComplete;

	// Stop services
	do {
		allComplete = true;
		for (int i = ServiceCount - 1; i >= 0; i--) {
			ServiceMessages_t retval = ServiceList[i]->Stop();
			allComplete &= (retval == ServiceMessages_t::SvcMsg_OK);
		}
	} while (!allComplete);
}

}
}

#endif
