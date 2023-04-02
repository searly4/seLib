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


#include <nrf.h>
#include <app_util_platform.h>
//#include "nrf_delay.h"
//#include "nrf_gpio.h"
//#include "app_gpiote.h"

#ifdef __cplusplus
extern "C" {
#endif

    int get_clock_ms(unsigned long *count);

	__attribute__((always_inline)) inline void BKPT(void) {
	#ifdef DEBUG
		__ASM volatile("nop");
	#endif
	}


#ifdef __cplusplus
}

namespace seLib {
namespace Platform {

class CriticalSection_t {
protected:
	bool Released = 0;
	uint8_t __CR_NESTED = 0;

public:
	inline CriticalSection_t() {
		//CRITICAL_REGION_ENTER();
		app_util_critical_region_enter(&__CR_NESTED);
	}
	
	inline void Release() {
		if (Released)
			return;
		Released = true;
		//CRITICAL_REGION_EXIT();
		app_util_critical_region_exit(__CR_NESTED);
	}
	
	inline ~CriticalSection_t() {
		Release();
	}
};


inline void SetWakeFlag() {
    __SEV();
}


inline void Sleep() {
#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT
    ret_code_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
#else
    __WFE();
    __SEV();
    __WFE();
#endif
}

}
}

#endif
