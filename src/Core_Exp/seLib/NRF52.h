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


#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_gpiote.h"

#include <seLib/Platform.h>

#include <seLib/experimental/ServiceModel.h>


//#define UI_TIMER_INTERVAL                    APP_TIMER_TICKS(2000, APP_TIMER_PRESCALER) /**< User interface timer interval (ticks). */
//#define UI_TIMER_INTERVAL                    APP_TIMER_TICKS(2000/64, APP_TIMER_PRESCALER) /**< User interface timer interval (ticks). */

#define DEAD_BEEF                            0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

//#define 	APP_TIMER_TICKS(MS, PRESCALER)   ((uint32_t)ROUNDED_DIV((MS) * (uint64_t)APP_TIMER_CLOCK_FREQ, ((PRESCALER) + 1) * 1000))
//#define 	APP_TIMER_TICKS(MS, PRESCALER)   TICKS = MS * (APP_TIMER_CLOCK_FREQ / ((PRESCALER + 1) * 1000))
//#define 	APP_TIMER_MS(TICKS, PRESCALER)   MS = TICKS / (APP_TIMER_CLOCK_FREQ / ((PRESCALER + 1) * 1000))
#define 	APP_TIMER_MS(TICKS, PRESCALER)   ((uint32_t)ROUNDED_DIV(((uint32_t)TICKS), ROUNDED_DIV((uint32_t)APP_TIMER_CLOCK_FREQ, ((PRESCALER) + 1) * 1000)))


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}

namespace seLib {
namespace nrf52 {

using namespace seLib;
using namespace seLib::ServiceModel;

//=============================================================================
// Forward declarations

class NRF52_ServiceState_t;
class NRF52_HAL_t;


//=============================================================================
// App defined variables

extern NRF52_HAL_t const & Main_HAL;
extern NRF52_ServiceState_t & Main_HAL_State;

//=============================================================================
// App defined functions

//=============================================================================
// Overrideable functions (default functions have weak linkage)

void RTC_Increment();

//=============================================================================
// Definitions

struct PinConfig_t {
    uint8_t PinNumber;
    //uint8_t Port;
    //uint8_t Subport;
    bool IsInput : 1;
    bool IsOutput : 1;
    bool IsPull : 1;
    bool PullDirection : 1;
    bool InitialOutputState : 1;
	
    //inline int PinNumber() const { return Port * P0_PIN_NUM + Subport; }
    inline void Set() const {
	    nrf_gpio_pin_set(PinNumber);
    }

    inline void Clear() const {
	    nrf_gpio_pin_clear(PinNumber);
    }

	inline void Assign(bool state) const {
		if (state)
			Set();
		else
			Clear();
    }

    void Configure() const;
};

class NRF52_ServiceState_t : public ServiceModel::ServiceState_t {
public:
	uint32_t RTC_Count;
};

class NRF52_HAL_t {
public:
    bool Enable_RTC : 1;
    bool Enable_WDT : 1;
	uint32_t WDT_Interval = 0;

	virtual void Reset() const = 0;
	virtual void Sleep() const = 0;
	
    ServiceMessages_t Init(NRF52_ServiceState_t & state) const;
    ServiceMessages_t Update(NRF52_ServiceState_t & state) const;
    ServiceMessages_t Uninit(NRF52_ServiceState_t & state) const;
};

/**
 * @brief Initialize clocks, power management, I/O, and SoftDevice.
 */
template <typename ServiceState_T = NRF52_ServiceState_t>
class NRF52_ServiceDef : public ServiceModel::ServiceDef<ServiceState_T>, public NRF52_HAL_t {
public:
    constexpr NRF52_ServiceDef(NRF52_ServiceState_t* _state, ServiceModel::ServiceManagerDef const & manager, bool _autostart)
        : ServiceDef<ServiceState_T>((ServiceState_T*)_state, manager, _autostart)
    {}

	ServiceMessages_t Init() const override {
		return NRF52_HAL_t::Init(this->StateRef());
	}
    ServiceMessages_t Update() const override {
		return NRF52_HAL_t::Update(this->StateRef());
	}
	ServiceMessages_t Uninit() const override {
		return NRF52_HAL_t::Uninit(this->StateRef());
	}
};

void TestPin(const PinConfig_t& pindef, bool tryhigh, bool trylow);


void Reboot();
void WDT_Update();


}} // namespace MM::NRF52

#endif
