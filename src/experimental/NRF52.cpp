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
 * MM_NRF51.c
 *
 *  Created on: Jun 20, 2016
 *      Author: scotte
 */

#include <seLib_Config.h>

#if defined(__NRF52)

#include <nrf.h>
#include <nordic_common.h>
#include <app_uart.h>
#include <nrf_delay.h>
#include <nrf_sdh_soc.h>
#include <ble.h>
#include <nrf_sdh.h>
#include <nrf_sdh_ble.h>
#include <nrf_rtc.h>
#include <nrf_sdm.h>
#include <nrfx_wdt.h>
//#include <nrfx_wdt.h>

#include <app_gpiote.h>
//#include "softdevice_handler.h"
//#include <nrfx_clock.h>
#include <nrfx_clock.h>
#include <app_timer.h>
#include <nrf_gpio.h>
//#include "device_manager.h"
//#include "app_trace.h"
//#include "pstorage.h"
//#include <nrfx_rtc.h>
#include <drv_rtc.h>
#include <nrfx_rtc.h>
#include <SEGGER_RTT.h>
#include <nrf_log.h>
#include <nrf_log_ctrl.h>
#include <nrf_log_default_backends.h>

#include <seLib/experimental/seLib_Util.h>
#include <seLib/experimental/NRF52.h>

//=============================================================================
// Definitions

#define CALDATASIZE 124
#define APP_BLE_CONN_CFG_TAG 1 // A tag identifying the SoftDevice BLE configuration


namespace seLib {
namespace nrf52 {

//=============================================================================
#pragma region Declarations

//static void ms_timeout_handler(void * p_context);

#pragma endregion

//=============================================================================
#pragma region Variables

#if NRF_MODULE_ENABLED(NRFX_RTC2)
nrfx_rtc_t rtc_inst = NRFX_RTC_INSTANCE(2);
#endif

static nrfx_wdt_channel_id wdt_channel_id;

//nrfx_clock_handler_item_t lfclk_handler_item = { nullptr, nullptr };

#pragma endregion

//=============================================================================

static void rtc_handler(nrfx_rtc_int_type_t int_type) {
	RTC_Increment();
}

static void wdt_event_handler(void) {
    //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
    //nrf_gpio_pin_clear(PIN_PWR);      // Should initiate poweroff except in debugging fixture
    //NRF_POWER->SYSTEMOFF = 1;
	//MM_Power_Off();
}

void WDT_Update() {
    nrfx_wdt_channel_feed(wdt_channel_id);
}


void Reboot() {
	uint32_t err_code;
	err_code = sd_softdevice_disable();
	APP_ERROR_CHECK(err_code);
	err_code = app_timer_stop_all();
	APP_ERROR_CHECK(err_code);
    __disable_irq();
    NVIC_SystemReset();
}

void PinConfig_t::Configure() const {
    ASSERT(!(IsInput && IsOutput));
    nrf_gpio_pin_dir_t direction = nrf_gpio_pin_dir_t::NRF_GPIO_PIN_DIR_INPUT;

    nrf_gpio_pin_input_t input_connected = (IsInput) ?
        nrf_gpio_pin_input_t::NRF_GPIO_PIN_INPUT_CONNECT :
        nrf_gpio_pin_input_t::NRF_GPIO_PIN_INPUT_DISCONNECT;

    nrf_gpio_pin_pull_t pull_config = (!IsPull) ?
		nrf_gpio_pin_pull_t::NRF_GPIO_PIN_NOPULL :
		(
			(PullDirection) ?
			nrf_gpio_pin_pull_t::NRF_GPIO_PIN_PULLUP :
			nrf_gpio_pin_pull_t::NRF_GPIO_PIN_PULLDOWN
		);

    nrf_gpio_pin_drive_t drive_type = NRF_GPIO_PIN_S0S1;

    nrf_gpio_pin_sense_t sense_type = NRF_GPIO_PIN_NOSENSE;

	if (IsOutput) {
    	direction = nrf_gpio_pin_dir_t::NRF_GPIO_PIN_DIR_OUTPUT;
		if (InitialOutputState)
			nrf_gpio_pin_set(PinNumber);
		else
			nrf_gpio_pin_clear(PinNumber);
        if (IsPull && PullDirection)
            drive_type = NRF_GPIO_PIN_S0D1;
    }

    nrf_gpio_cfg(PinNumber, direction, input_connected, pull_config, drive_type, sense_type);
}

void TestPin(const PinConfig_t& pindef, bool tryhigh, bool trylow) {
    uint32_t pin_number = pindef.PinNumber;

    NRF_GPIO_Type * reg = nrf_gpio_pin_port_decode(&pin_number);
    auto old_pin_config = reg->PIN_CNF[pin_number];
    pin_number = pindef.PinNumber; // value was modified, reset

    bool read_hiz = nrf_gpio_pin_read(pin_number);
    bool read_down = false;
    bool read_up = false;
    if (tryhigh) {
        nrf_gpio_cfg_input(pin_number, nrf_gpio_pin_pull_t::NRF_GPIO_PIN_PULLUP);
        nrf_delay_ms(2);
        read_up = nrf_gpio_pin_read(pin_number);
    }
    reg->PIN_CNF[pin_number] = old_pin_config;
    nrf_delay_ms(2);
    if (trylow) {
        nrf_gpio_cfg_input(pin_number, nrf_gpio_pin_pull_t::NRF_GPIO_PIN_PULLDOWN);
        nrf_delay_ms(2);
        read_down = nrf_gpio_pin_read(pin_number);
    }
    reg->PIN_CNF[pin_number] = old_pin_config;

    char message[32];
    sprintf(message, "Pin %u: %u hiz, %u up, %u down", (unsigned int)pin_number, (unsigned int)read_hiz, (unsigned int)read_up, (unsigned int)read_down);
    seLib_Log(seLib_TraceLevel_e::TraceLevel_Info, message);
}

ServiceMessages_t NRF52_HAL_t::Init(NRF52_ServiceState_t & state) const {
	seLib_Trace(TraceVal_EnterFunction, (void*)__func__, NULL);

    uint32_t err_code;

	SCB->SCR |= SCB_SCR_SEVONPEND_Msk;
	
	#if defined(NRF_LOG_BACKEND_RTT_ENABLED) && (NRF_LOG_BACKEND_RTT_ENABLED)
    SEGGER_RTT_Init();
    err_code = NRF_LOG_INIT(nullptr);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();
	#endif
	
    // Start the internal LFCLK oscillator.
    // This is needed by RTC1 which is used by the application timer
    // (When SoftDevice is enabled the LFCLK is always running and this is not needed).
    err_code = nrfx_clock_init(nullptr);
    APP_ERROR_CHECK(err_code);
	//nrfx_clock_lfclk_request(nullptr);
	nrfx_clock_lfclk_start();

#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT
    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);
#endif

	// Configure WDT
    nrfx_wdt_config_t wdt_config = NRFX_WDT_DEAFULT_CONFIG;
	wdt_config.reload_value = WDT_Interval;
    err_code = nrfx_wdt_init(&wdt_config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrfx_wdt_channel_alloc(&wdt_channel_id);
    APP_ERROR_CHECK(err_code);
	if (Enable_WDT)
	    nrfx_wdt_enable();
	
    // Initialize timer module, making it use the scheduler
    //err_code = app_timer_init();
    //APP_ERROR_CHECK(err_code);

    //scheduler_init();

	//err_code = app_timer_create(&mm_ms_timer_id, APP_TIMER_MODE_REPEATED, ms_timeout_handler);
    //APP_ERROR_CHECK(err_code);

	//err_code = app_timer_start(mm_ms_timer_id, 32, nullptr);

	#if NRF_MODULE_ENABLED(RTC2)
	if (Enable_RTC) {
		//nrfx_rtc_config_t rtc_cfg = NRFX_RTC_DEFAULT_CONFIG;
		nrfx_rtc_config_t rtc_cfg = {
			.prescaler = RTC_FREQ_TO_PRESCALER(1024),
			.interrupt_priority = RTC_DEFAULT_CONFIG_IRQ_PRIORITY,
			.tick_latency = RTC_US_TO_TICKS(NRF_MAXIMUM_LATENCY_US, RTC_DEFAULT_CONFIG_FREQUENCY),
			.reliable = RTC_DEFAULT_CONFIG_RELIABLE,
		};
		//rtc_cfg.prescaler = RTC_FREQ_TO_PRESCALER(1024);
		nrfx_rtc_init(&rtc_inst, &rtc_cfg, rtc_handler);
	}
	#endif

	#if NRF_MODULE_ENABLED(APP_TIMER)
	app_timer_init();
	#endif
	
	state.State.AssignImmediate(ServiceState_e::Idle);
	state.ServiceAttention = true;
	return ServiceMessages_t::SvcMsg_OK;
}

ServiceMessages_t NRF52_HAL_t::Uninit(NRF52_ServiceState_t & state) const {
	seLib_Trace(TraceVal_EnterFunction, (void*)__func__, NULL);
    uint32_t err_code;

	if (Enable_RTC) {
		nrfx_rtc_tick_disable(&rtc_inst);
		nrfx_rtc_disable(&rtc_inst);
		nrfx_rtc_uninit(&rtc_inst);
	}

	if (Enable_WDT) {
	}

#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT
    err_code = nrf_sdh_disable_request();
    APP_ERROR_CHECK(err_code);
#endif

	//nrfx_clock_lfclk_release();
	nrfx_clock_lfclk_stop();
    nrfx_clock_uninit();
	
	return ServiceMessages_t::SvcMsg_OK;
}

ServiceMessages_t NRF52_HAL_t::Update(NRF52_ServiceState_t & state) const {
	seLib_Trace(TraceVal_EnterFunction, (void*)__func__, NULL);
    uint32_t err_code;

	auto& state_val = state.State;

	if (!state_val.IsChangePending())
		return ServiceMessages_t::SvcMsg_NoMoreTasks;

	if (state_val.IsTransition(ServiceState_e::Idle, ServiceState_e::Running)) {
		#if NRF_MODULE_ENABLED(RTC2)
		if (Enable_RTC) {
			nrfx_rtc_enable(&rtc_inst);
			nrfx_rtc_tick_enable(&rtc_inst, true);
		}
		#endif
		// Start application timers.
		//err_code = app_timer_start(m_ui_timer_id, UI_TIMER_INTERVAL, NULL);
		//APP_ERROR_CHECK(err_code);

		state_val.Update();
	}

	if (state_val.IsTransition(ServiceState_e::Running, ServiceState_e::Idle)) {
		if (Enable_RTC) {
			nrfx_rtc_tick_disable(&rtc_inst);
			nrfx_rtc_disable(&rtc_inst);
		}
		state_val.Update();
	}
	
	state.ServiceAttention = false;
	return ServiceMessages_t::SvcMsg_NoMoreTasks;
}


/*static void ms_timeout_handler(void * p_context) {
    UNUSED_PARAMETER(p_context);
	MM_MS_Counter++;
}*/

void __attribute__((weak)) RTC_Increment() {
	Main_HAL_State.RTC_Count++;
};

}

namespace MCU_HAL {

void __attribute__((weak)) Sleep() {
	seLib::Platform::Sleep();
}

}


void lowpower() {
	//if (softdevice_handler_isEnabled())
		//sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
}

uint32_t GetClock() {
	#if NRF_MODULE_ENABLED(RTC)
	return nrfx_rtc_counter_get(&nrf52::rtc_inst);
	#else
	return 0;
	#endif
}


} // namespace MM::NRF52


int get_clock_ms(unsigned long *count) {
	//*count = MM::nrf52::MM_MS_Counter;
	*count = seLib::GetClock();
	return 0;
}

#pragma region // Variable declarations

volatile uint32_t m_error_code;
volatile uint32_t m_line_num;
volatile const uint8_t *m_p_file_name;

uint8_t trace_index = 0;
const char* trace_msg_buffer[8];
uint32_t trace_val_buffer[8];
seLib_TraceVal_e trace_enum_buffer[8];
const void* trace_loc_buffer[8];
//MM_TracePacket_s trace_buffer[8];
uint32_t trace_err = 0;

#pragma endregion

seLib_TraceLevel_t seLib_LogLevel = seLib_TraceLevel_t::TraceLevel_Info;

void seLib_Trace(seLib_TraceVal_e enumval, const void* loc, const char* message) {
	seLib_Trace2(enumval, loc, message, 0);
}

void seLib_TraceErr(seLib_TraceVal_e enumval, const void* loc, const char* message, uint32_t val, uint32_t errcode) {
	trace_err = errcode;
	seLib_Trace2(enumval, loc, message, val);
}

void seLib_Trace2(seLib_TraceVal_e enumval, const void* loc, const char* message, uint32_t val) {
	trace_enum_buffer[trace_index] = enumval;
	trace_loc_buffer[trace_index] = loc;
	trace_msg_buffer[trace_index] = message;
	trace_val_buffer[trace_index] = val;
	trace_index = (trace_index + 1) & 0x7;

    if (seLib_TraceLevel_t::TraceLevel_Verbose > seLib_LogLevel)
        return;

	#if defined(NRF_LOG_BACKEND_RTT_ENABLED) && (NRF_LOG_BACKEND_RTT_ENABLED)
    char hexval[11];
    sprintf(hexval, "%08X ", (unsigned int)enumval);
    hexval[9] = 0;
    SEGGER_RTT_WriteString(0, hexval);
    sprintf(hexval, "%08X ", (unsigned int)loc);
    hexval[9] = 0;
    SEGGER_RTT_WriteString(0, hexval);
    SEGGER_RTT_WriteString(0, message);
    sprintf(hexval, " %08X\n", (unsigned int)val);
    hexval[10] = 0;
    SEGGER_RTT_WriteString(0, hexval);
	#endif
}

void seLib_Assert(seLib_TraceVal_e enumval, const void* loc, const char* message, uint32_t val, uint32_t errcode) {
	seLib_TraceErr(enumval, loc, message, val, errcode);
	BKPT();
}

void seLib_DebugString(const char* string) {
    //printf(string);
    //printf("\n");
    //NRF_LOG_ERROR(string);
    //NRF_LOG_DEBUG(string);
	#if defined(NRF_LOG_BACKEND_RTT_ENABLED) && (NRF_LOG_BACKEND_RTT_ENABLED)
    SEGGER_RTT_WriteString(0, string);
    SEGGER_RTT_WriteString(0, "\n");
	#endif
    //SEGGER_RTT_printf("%s\n", string);
    /* SWO not available
    while (*string != 0) {
        ITM_SendChar(*string);
        string++;
    }//*/
}

#endif