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
#if defined(EnableModule_BLE) && defined(__NRF52)

#include "seLib/experimental/NRF52_BLE_Module.h"
//#include "IDC_NRF52.h"

#include <stdint.h>
#include <string.h>
#include <assert.h>

extern "C" {
#include <nrf_pwr_mgmt.h> // header lacks extern "C" wrapper
}

#include <nordic_common.h>
#include <nrf.h>
#include <app_timer.h>
#include <app_uart.h>
#include <app_util_platform.h>
#include <ble_hci.h>
#include <ble_advdata.h>
#include <ble_advertising.h>
#include <ble_conn_params.h>
#include <nrf_sdh.h>
#include <nrf_sdh_soc.h>
#include <nrf_sdh_ble.h>
#include <nrf_ble_gatt.h>
#include <ble_gap.h>

#include <ble_dis.h>
#if NRF_MODULE_ENABLED(BLE_DFU)
#include <ble_dfu.h>
#endif
//#include <nrf_ble_dfu.h>
//#include <ble_nus.h>
#include <nrf_power.h>
#include <nrf_bootloader_info.h>

#include <nrf_log.h>
#include <nrf_log_ctrl.h>
#include <nrf_log_default_backends.h>

#include <seLib/experimental/seLib_Util.h>
//#include "Series301_BLEService.h"


namespace seLib {
namespace BLE_Module {

namespace BLE_Peripheral_Handlers {
static void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt);
static void on_adv_evt(ble_adv_evt_t ble_adv_evt);
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt);
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context);
static void conn_params_error_handler(uint32_t nrf_error);
}

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */

#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#ifdef BLE_DFU_APP_SUPPORT
static ble_dfu_t                            m_dfus;                                    /**< Structure used to identify the DFU service. */
#endif // BLE_DFU_APP_SUPPORT

//BLE_NUS_DEF(m_nus);                                                                 /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */

//static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
//static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */

static bool m_memory_access_in_progress = false;       /**< Flag to keep track of ongoing operations on persistent memory. */

BLE_Peripheral_Def_Base_t const * BLE_Active_Peripheral = nullptr;

namespace BLE_Peripheral_Handlers {

/**
 @fn	void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)

 @brief	Function for handling events from the GATT library.

 @author	Searly
 @date	5/11/2018

 @param [in,out]	p_gatt	If non-null, the gatt.
 @param 			p_evt 	The event.
 */
static void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt) {
	if ((BLE_Active_Peripheral->GetState()->conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED)) {
		#if BLE_NUS_ENABLED==1
		m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
		NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
		#endif
	}

	NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x", p_gatt->att_mtu_desired_central, p_gatt->att_mtu_desired_periph);
}


/**
 @fn	static void on_adv_evt(ble_adv_evt_t ble_adv_evt)

 @brief	This function will be called for advertising events which are passed to the application.

 @author	Searly
 @date	5/11/2018

 @param	ble_adv_evt	The ble advance event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt) {
	uint32_t err_code;

	switch (ble_adv_evt) {
		case BLE_ADV_EVT_FAST:
			//err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
			//APP_ERROR_CHECK(err_code);
			break;
			
		case BLE_ADV_EVT_IDLE:
			seLib_Log(seLib_TraceLevel_t::TraceLevel_Verbose, "BLE_ADV_EVT_IDLE");
			BLE_Active_Peripheral->on_adv_timeout();
		    //sleep_mode_enter(); // todo: any action needed?
			break;
			
		default:
			break;
	}
}


/**
 @fn	static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)

 @brief	Function for handling BLE events.

 @author	Searly
 @date	5/11/2018

 @param 			p_ble_evt	The ble event.
 @param [in,out]	p_context	If non-null, the context.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context) {
	BLE_Active_Peripheral->on_ble_evt(p_ble_evt);
}


/**
 @fn	static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)

 @brief	This function will be called for all events in the Connection Parameters Module which are passed to the application.

 @note	All this function does is to disconnect. This could have been
 		done by simply setting the disconnect_on_fail config parameter, but instead we use the
 		event handler mechanism to demonstrate its use.

 @author	Searly
 @date	5/11/2018

 @param	p_evt	A variable-length parameters list containing event.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt) {
	uint32_t err_code;

	if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
		err_code = sd_ble_gap_disconnect(BLE_Active_Peripheral->GetState()->conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
		APP_ERROR_CHECK(err_code);
	}
}


/**
 @fn	static void conn_params_error_handler(uint32_t nrf_error)

 @brief	Function for handling errors from the Connection Parameters module.

 @author	Searly
 @date	5/11/2018

 @param	nrf_error	The nrf error.
 */
static void conn_params_error_handler(uint32_t nrf_error) {
	APP_ERROR_HANDLER(nrf_error);
}

// This function will process the data received from the Nordic UART BLE Service and send it to the UART module.
#if NRF_MODULE_ENABLED(BLE_NUS)
static void nus_data_handler(ble_nus_evt_t * p_evt) {

	if (p_evt->type == BLE_NUS_EVT_RX_DATA) {
		uint32_t err_code;

		NRF_LOG_DEBUG("Received data from BLE NUS. Writing data on UART.");
		NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);

		for (uint32_t i = 0; i < p_evt->params.rx_data.length; i++) {
			do {
				err_code = app_uart_put(p_evt->params.rx_data.p_data[i]);
				if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY)) {
					NRF_LOG_ERROR("Failed receiving NUS message. Error 0x%x. ", err_code);
					APP_ERROR_CHECK(err_code);
				}
			} while (err_code == NRF_ERROR_BUSY);
		}

		if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\r') {
			while (app_uart_put('\n') == NRF_ERROR_BUSY);
		}
	}

}
#endif



static void buttonless_dfu_sdh_state_observer(nrf_sdh_state_evt_t state, void * p_context) {
    if (state == NRF_SDH_EVT_STATE_DISABLED) {
        // Softdevice was disabled before going into reset. Inform bootloader to skip CRC on next boot.
        //nrf_power_gpregret2_set(BOOTLOADER_DFU_SKIP_CRC);

        //Go to system off.
        nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
    }
}

/* nrf_sdh state observer. */
NRF_SDH_STATE_OBSERVER(m_buttonless_dfu_state_obs, 0) = {
    .handler = buttonless_dfu_sdh_state_observer,
};

// YOUR_JOB: Update this code if you want to do anything given a DFU event (optional).
/**@brief Function for handling dfu events from the Buttonless Secure DFU service
 *
 * @param[in]   event   Event from the Buttonless Secure DFU service.
 */
#if NRF_MODULE_ENABLED(BLE_DFU)
static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event) {
    switch (event) {
        case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE:
            NRF_LOG_INFO("Device is preparing to enter bootloader mode.");
            // YOUR_JOB: Disconnect all bonded devices that currently are connected.
            //           This is required to receive a service changed indication
            //           on bootup after a successful (or aborted) Device Firmware Update.
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER:
            // YOUR_JOB: Write app-specific unwritten data to FLASH, control finalization of this
            //           by delaying reset by reporting false in app_shutdown_handler
            NRF_LOG_INFO("Device will enter bootloader mode.");
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
            NRF_LOG_ERROR("Request to enter bootloader mode failed asynchroneously.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            break;

        case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
            NRF_LOG_ERROR("Request to send a response to client failed.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            APP_ERROR_CHECK(false);
            break;

        default:
            NRF_LOG_ERROR("Unknown event from ble_dfu_buttonless.");
            break;
    }
}
#endif

}

// ** BLE_Peripheral_Def_t ***************************************************************
#pragma region

//BLE_Peripheral_Def_t const * BLE_Peripheral_State_t::ActiveDef = nullptr;


uint32_t BLE_Peripheral_Def_Base_t::InitService(const BLE_Service_Def_Base_t& service_def) const {
	seLib_Trace(TraceVal_EnterFunction, (void*)&__FUNCTION__, nullptr);
	uint32_t       err_code;
	auto& svc_state = *service_def.GetState();

	uint8_t uuid_type = BLE_UUID_TYPE_VENDOR_BEGIN;
	// Add custom base UUID
	err_code = sd_ble_uuid_vs_add(base_uuid, &uuid_type);
	if (err_code != NRF_SUCCESS) {
		APP_ERROR_CHECK(err_code);
		return err_code;
	}

	err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &service_def.uuid, &svc_state.service_handle);
	APP_ERROR_CHECK(err_code);
	if (err_code != NRF_SUCCESS)
		return err_code;

	for (size_t i = 0; i < service_def.CharacteristicCount; i++) {
		err_code = service_def.AddCharacteristic(i);
		//mimicmotion_char_add(&MM_BLE_CharHandles[i], &MM_BLE_CharData[i]);
		if (err_code != NRF_SUCCESS) {
			seLib_Trace2(TraceVal_Error, (void*)&__FUNCTION__, nullptr, (uint8_t)i);
			return err_code;
		}
	}

	service_def.Init();
}


/**
 @fn	void BLE_Peripheral_Def_t::Init() const

 @brief	Initializes this object

 @author	Searly
 @date	5/11/2018
 */
void BLE_Peripheral_Def_Base_t::Init() const {
	seLib_Trace(TraceVal_EnterFunction, (void*)&__FUNCTION__, nullptr);

	ret_code_t err_code;
	ble_stack_init();
    //peer_manager_init(erase_bonds);
	gap_params_init();

	// init gatt library
	err_code = nrf_ble_gatt_init(&m_gatt, BLE_Peripheral_Handlers::gatt_evt_handler);
	APP_ERROR_CHECK(err_code);
	//err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, 64);
	//APP_ERROR_CHECK(err_code);

	// Add custom base UUID
	/*err_code = sd_ble_uuid_vs_add(&nus_base_uuid, &uuid_type);
	if (err_code != NRF_SUCCESS) {
		return err_code;
	}*/

	services_init(); // init standard services

	// init each custom service
	assert(ServiceCount == 0 || ServiceList != nullptr);
	for (size_t i = 0; i < ServiceCount; i++) {
		const auto& service_def = ServiceList[i];
		InitService(*service_def);
	}

	advertising_init();
	conn_params_init();
}

uint32_t BLE_Peripheral_Def_Base_t::UpdateChar(BLE_Service_Def_Base_t const * service, BLE_Characteristic_t const * characteristic, uint8_t const * value, uint8_t length) const {
	uint32_t err_code;
	auto state = GetState();

	// Send value if connected and notifying
	if (state->conn_handle == BLE_CONN_HANDLE_INVALID)
		return NRF_ERROR_INVALID_STATE;

	uint8_t char_index = (uint8_t)(characteristic - service->CharacteristicList);;
	assert(char_index < service->CharacteristicCount);

	uint16_t hvx_len = length;
	ble_gatts_hvx_params_t hvx_params = { 0 };

	hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
	hvx_params.offset = 0;
	hvx_params.p_len  = &hvx_len;
	hvx_params.p_data = (characteristic->Stack) ? const_cast<uint8_t*>((const uint8_t*)value) : nullptr;
	hvx_params.handle = service->GetState()->characteristic_handles[char_index].value_handle;

	err_code = sd_ble_gatts_hvx(state->conn_handle, &hvx_params);
	if ((err_code == NRF_SUCCESS) && (hvx_len != length))
		err_code = NRF_ERROR_DATA_SIZE;

	if (err_code == BLE_GATTS_EVT_SYS_ATTR_MISSING) {
		err_code = sd_ble_gatts_sys_attr_set(state->conn_handle, NULL, 0, 0);
		return err_code;
	}

	if (err_code == NRF_ERROR_INVALID_STATE) {
		return err_code;
	} else if (err_code == NRF_ERROR_RESOURCES) {
		return err_code;
	} else if (err_code != NRF_SUCCESS) {
		APP_ERROR_CHECK(err_code);
		return err_code;
	}

	return NRF_SUCCESS;
}



/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
void BLE_Peripheral_Def_Base_t::gap_params_init(void) const {
	uint32_t                err_code;
	ble_gap_conn_params_t   gap_conn_params;
	ble_gap_conn_sec_mode_t sec_mode;

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

	err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *) DeviceName, strlen((const char*) DeviceName));
	APP_ERROR_CHECK(err_code);

	memset(&gap_conn_params, 0, sizeof(gap_conn_params));

	gap_conn_params.min_conn_interval = min_conn_interval;
	gap_conn_params.max_conn_interval = max_conn_interval;
	gap_conn_params.slave_latency     = SLAVE_LATENCY;
	gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

	err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
	APP_ERROR_CHECK(err_code);
}



/**
 @fn	void BLE_Peripheral_Def_t::services_init(void) const

 @brief	Function for initializing services that will be used by the application.

 @author	Searly
 @date	5/11/2018
 */
void BLE_Peripheral_Def_Base_t::services_init(void) const {
	uint32_t       err_code;

	err_code = ble_dis_init(&seLib::BLE_Module::ble_dis_config);
	APP_ERROR_CHECK(err_code);

#ifdef BLE_DFU_APP_SUPPORT
	if (Svc_DFU_Enabled) {
		// Initialize the async SVCI interface to bootloader.
		err_code = ble_dfu_buttonless_async_svci_init();
		APP_ERROR_CHECK(err_code);

		ble_dfu_buttonless_init_t dfus_init = {0};
		dfus_init.evt_handler = BLE_Peripheral_Handlers::ble_dfu_evt_handler;
		err_code = ble_dfu_buttonless_init(&dfus_init);
		APP_ERROR_CHECK(err_code);
	}
#endif
	
#if BLE_NUS_ENABLED==1
	ble_nus_init_t nus_init;

	memset(&nus_init, 0, sizeof(nus_init));

	nus_init.data_handler = nus_data_handler;

	err_code = ble_nus_init(&m_nus, &nus_init);
	APP_ERROR_CHECK(err_code);
#endif
}



/**
 @fn	void BLE_Peripheral_Def_t::conn_params_init(void) const

 @brief	Function for initializing the Connection Parameters module.

 @author	Searly
 @date	5/11/2018
 */
void BLE_Peripheral_Def_Base_t::conn_params_init(void) const {
	uint32_t               err_code;
	ble_conn_params_init_t cp_init;

	memset(&cp_init, 0, sizeof(cp_init));

	cp_init.p_conn_params                  = NULL;
	cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
	cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
	cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
	cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
	cp_init.disconnect_on_fail             = false;
	cp_init.evt_handler                    = BLE_Peripheral_Handlers::on_conn_params_evt;
	cp_init.error_handler                  = BLE_Peripheral_Handlers::conn_params_error_handler;

	static_assert(APP_TIMER_ENABLED);
	err_code = ble_conn_params_init(&cp_init);
	APP_ERROR_CHECK(err_code);
}



/**
 @fn	void BLE_Peripheral_Def_t::ble_stack_init(void) const

 @brief	This function initializes the SoftDevice and the BLE event interrupt.

 @author	Searly
 @date	5/11/2018
 */
void BLE_Peripheral_Def_Base_t::ble_stack_init(void) const {
	ret_code_t err_code;
	uint32_t ram_start = 0;

	if (!nrf_sdh_is_enabled()) {
		err_code = nrf_sdh_enable_request();
		APP_ERROR_CHECK(err_code);
	}

	// Configure the BLE stack using the default settings.
	// Fetch the start address of the application RAM.
	err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
	APP_ERROR_CHECK(err_code);

	// Enable BLE stack.
	err_code = nrf_sdh_ble_enable(&ram_start);
	APP_ERROR_CHECK(err_code);

	// Register a handler for BLE events.
	NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, BLE_Peripheral_Handlers::ble_evt_handler, NULL);
}



/**
 @fn	void BLE_Peripheral_Def_t::advertising_init(void) const

 @brief	Function for initializing the Advertising functionality.

 @author	Searly
 @date	5/11/2018
 */
void BLE_Peripheral_Def_Base_t::advertising_init(void) const {
	uint32_t               err_code;
	ble_advertising_init_t init;

	memset(&init, 0, sizeof(init));

	init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
	init.advdata.include_appearance = false;
	init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
	//init.advdata.

	init.srdata.uuids_complete.uuid_cnt = adv_uuid_count;// sizeof(adv_uuids) / sizeof(m_adv_uuids[0]);
	init.srdata.uuids_complete.p_uuids  = (ble_uuid_t*)adv_uuids;

	init.config.ble_adv_fast_enabled  = adv_fast_enabled;
	init.config.ble_adv_fast_interval = adv_fast_interval;
	init.config.ble_adv_fast_timeout  = adv_fast_timeout;

	init.config.ble_adv_slow_enabled  = adv_slow_enabled;
	init.config.ble_adv_slow_interval = adv_slow_interval;
	init.config.ble_adv_slow_timeout  = adv_slow_timeout;
	
	init.config.ble_adv_extended_enabled = adv_extended_enabled;

	init.evt_handler = BLE_Peripheral_Handlers::on_adv_evt;

	err_code = ble_advertising_init(&m_advertising, &init);
	APP_ERROR_CHECK(err_code);

	ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

void BLE_Peripheral_Def_Base_t::on_ble_evt(ble_evt_t const * p_ble_evt) const {

	uint32_t err_code;
	auto& periph_state = *GetState();

	switch (p_ble_evt->header.evt_id) {
		case BLE_GAP_EVT_CONNECTED:
			seLib_Log(seLib_TraceLevel_t::TraceLevel_Verbose, "BLE_GAP_EVT_CONNECTED");
			//NRF_LOG_INFO("Connected");
			//err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
			//APP_ERROR_CHECK(err_code);
			periph_state.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
			err_code = sd_ble_gatts_sys_attr_set(periph_state.conn_handle, NULL, 0, 0);
			break;

		case BLE_GAP_EVT_DISCONNECTED:
			seLib_Log(seLib_TraceLevel_t::TraceLevel_Verbose, "BLE_GAP_EVT_DISCONNECTED");
			//NRF_LOG_INFO("Disconnected");
			// LED indication will be changed when advertising starts.
			periph_state.conn_handle = BLE_CONN_HANDLE_INVALID;
			break;

			#ifndef S140
		case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
			{
				NRF_LOG_DEBUG("PHY update request.");
				ble_gap_phys_t const phys =
				{
					.tx_phys = BLE_GAP_PHY_AUTO,
					.rx_phys = BLE_GAP_PHY_AUTO,
				};
				err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
				APP_ERROR_CHECK(err_code);
			} break;
			#endif

		case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
		    // Pairing not supported
			seLib_Log(seLib_TraceLevel_t::TraceLevel_Verbose, "BLE_GAP_EVT_SEC_PARAMS_REQUEST");
			err_code = sd_ble_gap_sec_params_reply(periph_state.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
			APP_ERROR_CHECK(err_code);
			break;

			#if !defined (S112)
		case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
			{
				ble_gap_data_length_params_t dl_params;

				// Clearing the struct will effectivly set members to @ref BLE_GAP_DATA_LENGTH_AUTO
				memset(&dl_params, 0, sizeof(ble_gap_data_length_params_t));
				err_code = sd_ble_gap_data_length_update(p_ble_evt->evt.gap_evt.conn_handle, &dl_params, NULL);
				APP_ERROR_CHECK(err_code);
			} break;
			#endif //!defined (S112)
			
		case BLE_GAP_EVT_ADV_REPORT:
			seLib_Log(seLib_TraceLevel_t::TraceLevel_Verbose, "BLE_GAP_EVT_ADV_REPORT");
			if (p_ble_evt->evt.gap_evt.params.adv_report.type.scan_response)
				on_scan_resp(p_ble_evt);
			break;

		case BLE_GAP_EVT_SCAN_REQ_REPORT:
			// remote device has sent a scan request
			seLib_Log(seLib_TraceLevel_t::TraceLevel_Verbose, "BLE_GAP_EVT_SCAN_REQ_REPORT");
			on_scan_req(p_ble_evt);
			break;

		case BLE_GATTS_EVT_SYS_ATTR_MISSING:
		    // No system attributes have been stored.
			seLib_Log(seLib_TraceLevel_t::TraceLevel_Verbose, "BLE_GATTS_EVT_SYS_ATTR_MISSING");
			err_code = sd_ble_gatts_sys_attr_set(periph_state.conn_handle, NULL, 0, 0);
			APP_ERROR_CHECK(err_code);
			break;

		case BLE_GATTC_EVT_TIMEOUT:
		    // Disconnect on GATT Client timeout event.
			seLib_Log(seLib_TraceLevel_t::TraceLevel_Verbose, "BLE_GATTC_EVT_TIMEOUT");
			err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
			APP_ERROR_CHECK(err_code);
			break;

		case BLE_GATTS_EVT_TIMEOUT:
		    // Disconnect on GATT Server timeout event.
			seLib_Log(seLib_TraceLevel_t::TraceLevel_Verbose, "BLE_GATTS_EVT_TIMEOUT");
			err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
			APP_ERROR_CHECK(err_code);
			break;

		case BLE_GATTS_EVT_WRITE:
			{
				seLib_Log(seLib_TraceLevel_t::TraceLevel_Verbose, "BLE_GATTS_EVT_WRITE");
				ble_gatts_evt_write_t const & p_evt_write = p_ble_evt->evt.gatts_evt.params.write;
				//auto& t = p_ble_evt->evt.gatts_evt.params;
				//p_evt_write.
				//p_evt_write.handle;
				//auto& wr_uuid = p_evt_write.uuid;
				//APP_ERROR_CHECK(err_code);

				// todo: identify service owning characteristic and call only that service
				for (size_t i = 0; i < ServiceCount; i++) {
					const auto& service_def = ServiceList[i];
					service_def->on_write(p_ble_evt);
				}
			}
			break;

		case BLE_EVT_USER_MEM_REQUEST:
				seLib_Log(seLib_TraceLevel_t::TraceLevel_Verbose, "BLE_EVT_USER_MEM_REQUEST");
			err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
			APP_ERROR_CHECK(err_code);
			break;

		case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
			{
				seLib_Log(seLib_TraceLevel_t::TraceLevel_Verbose, "BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST");
				ble_gatts_evt_rw_authorize_request_t  req;
				ble_gatts_rw_authorize_reply_params_t auth_reply;

				req = p_ble_evt->evt.gatts_evt.params.authorize_request;

				if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID) {
					if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
					    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
					    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
					{
						if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE) {
							auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
						} else {
							auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
						}
						auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
						err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle, &auth_reply);
						APP_ERROR_CHECK(err_code);
					}
				}
			} break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

		default:
		    // No implementation needed.
			break;
	}
}

void BLE_Peripheral_Def_Base_t::on_adv_timeout() const {
	uint32_t err_code;
	if (adv_slow_enabled) {
		err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_SLOW);
		APP_ERROR_CHECK(err_code);
	}
}


#pragma endregion


#pragma region ** BLE_Service_Def_t ***************************************************************

/**
 @fn	void BLE_Service_Def_t::Init() const

 @brief	Initializes this object

 @author	Searly
 @date	5/11/2018
 */
void BLE_Service_Def_Base_t::Init() const {
}


/**
 @fn	uint32_t BLE_Service_Def_t::AddCharacteristic(const BLE_Characteristic_t* char_data) const

 @brief	Function for adding the MimicMotion test characteristic.

 @author	Searly
 @date	5/11/2018

 @param	char_data	Information describing the character.

 @return	NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t BLE_Service_Def_Base_t::AddCharacteristic(uint8_t charindex) const {
	const BLE_Characteristic_t* char_data = &CharacteristicList[charindex];
	uint32_t   err_code;
	ble_gatts_attr_md_t     cccd_md = { 0 };
	ble_gatts_char_md_t     char_md = { 0 };
	ble_gatts_attr_t        attr_char_value = { 0 };
	ble_uuid_t              ble_uuid;
	ble_gatts_attr_md_t     attr_md = { 0 };
	//ble_gatts_char_pf_t     char_pf;

	cccd_md.vloc = BLE_GATTS_VLOC_STACK;
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

	char_md.char_props.read     = (char_data->Read) ? 1 : 0;
	char_md.char_props.write    = (char_data->Write) ? 1 : 0;
	char_md.char_props.notify   = (char_data->Notify) ? 1 : 0;
	char_md.p_char_user_desc    = const_cast<uint8_t*>(char_data->Description); // API erroneously uses non-const
	char_md.char_user_desc_size = (char_data->Description != 0) ? strlen((const char*)char_data->Description) : 0;
	char_md.char_user_desc_max_size = char_md.char_user_desc_size;
	char_md.p_char_pf = char_data->PresentationFormat;

	//char_pf.
	//char_md.p_char_pf           = char_pf;
	char_md.p_user_desc_md      = NULL;
	char_md.p_cccd_md           = &cccd_md;
	char_md.p_sccd_md           = NULL;

	ble_uuid.type = BLE_UUID_TYPE_VENDOR_BEGIN;
	ble_uuid.uuid = char_data->UUID;

	attr_md.vloc       = (char_data->Stack) ? BLE_GATTS_VLOC_STACK : BLE_GATTS_VLOC_USER;
	attr_md.rd_auth    = 0;
	attr_md.wr_auth    = 0;
	attr_md.vlen       = 1;
	if (char_data->Read)    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
	else                    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
	if (char_data->Write)   BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
	else                    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);


	attr_char_value.p_uuid    = &ble_uuid;
	attr_char_value.p_attr_md = &attr_md;
	attr_char_value.init_len  = char_data->InitialLength;
	attr_char_value.init_offs = 0;
	attr_char_value.max_len   = char_data->MaxLength;
	attr_char_value.p_value   = char_data->Buffer;

	err_code = sd_ble_gatts_characteristic_add(GetState()->service_handle, &char_md, &attr_char_value, &(GetState()->characteristic_handles[charindex]));
	if (err_code != NRF_SUCCESS)
		return err_code;
	if (char_data->Notify) {
		err_code = sd_ble_gatts_sys_attr_set(GetState()->characteristic_handles[charindex].cccd_handle, nullptr, 0, 0);
		//APP_ERROR_CHECK(err_code);
	}

	return NRF_SUCCESS;
}


/**
 @fn	uint32_t BLE_Service_Def_t::Send(const uint8_t* data, uint8_t datalen, uint8_t charindex) const

 @brief	Send this message

 @author	Searly
 @date	5/11/2018

 @param	data	 	The data.
 @param	datalen  	The datalen.
 @param	charindex	The charindex.

 @return	An uint32_t.
 */
uint32_t BLE_Service_Def_Base_t::Send(const uint8_t* data, uint8_t datalen, uint8_t charindex) const {
	uint32_t err_code;

	auto& conn_handle = BLE_Active_Peripheral->GetState()->conn_handle;
	
	// Send value if connected and notifying
	if (conn_handle == BLE_CONN_HANDLE_INVALID)
		return NRF_ERROR_INVALID_STATE;

	uint16_t                len;
	uint16_t                hvx_len;
	ble_gatts_hvx_params_t  hvx_params;
	uint8_t                 char_index;

	memset(&hvx_params, 0, sizeof(hvx_params));

	hvx_params.handle = GetState()->characteristic_handles[charindex].value_handle;
	if (hvx_params.handle == 0)
		return NRF_ERROR_INVALID_STATE;

	hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
	hvx_params.offset = 0;
	hvx_params.p_len  = &hvx_len;
	hvx_params.p_data = NULL;

	len = datalen;
	hvx_len = len;

	hvx_params.p_data = const_cast<uint8_t*>(data);
	err_code = sd_ble_gatts_hvx(conn_handle, &hvx_params);
	if ((err_code == NRF_SUCCESS) && (hvx_len != len))
		err_code = NRF_ERROR_DATA_SIZE;

	return err_code;
}

#pragma endregion ** BLE_Service_Def_t ***************************************************************




void StopAdvertising() {
	uint32_t err_code;
	err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_IDLE);
	APP_ERROR_CHECK(err_code);
}

ServiceMessages_t BLEManagerServiceDef::Init() const {
	using namespace seLib::ServiceModel;
	//StateRef().State.AssignImmediate(ServiceState_e::Idle);
	return ServiceModel::ServiceDef<BLEManagerServiceState_t>::Init();
	
	//if (!Init())
		//return ServiceMessages_t::SvcMsg_FaultGeneric;

	//return ServiceMessages_t::SvcMsg_OK;
}

ServiceMessages_t BLEManagerServiceDef::Update() const {
	using namespace seLib::ServiceModel;
	uint32_t err_code;

	auto& state = StateRef().State;
	auto const & periph_state = BLE_Active_Peripheral->GetState();

	if (state.IsTransition(ServiceState_e::Idle, ServiceState_e::Running)) {
		BLE_Active_Peripheral->Init();
		//if (!Start())
			//return ServiceMessages_t::SvcMsg_FaultGeneric;
		if (BLE_Active_Peripheral->GetState()->Flags[BLEStatusFlags::AdvertisingEnabled]) {
			if (BLE_Active_Peripheral->adv_fast_enabled && m_advertising.adv_mode_current != BLE_ADV_MODE_FAST) {
				err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
				APP_ERROR_CHECK(err_code);
			} else if (BLE_Active_Peripheral->adv_slow_enabled && m_advertising.adv_mode_current != BLE_ADV_MODE_SLOW) {
				err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_SLOW);
				APP_ERROR_CHECK(err_code);
			}
		}
		state.Update();
	}

	if (state.IsTransition(ServiceState_e::Running, ServiceState_e::Idle)) {
		if (m_advertising.adv_mode_current != BLE_ADV_MODE_IDLE) {
			err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_IDLE);
			APP_ERROR_CHECK(err_code);
		}
		state.Update();
	}

	
	return ServiceMessages_t::SvcMsg_NoMoreTasks;
}


}
}
#endif // EnableModule_BLE
