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

#include <seLib_Config.h>

#if defined(EnableModule_BLE) && defined(__NRF52)

#include "ServiceModel.h"
#include "BLE_Service_Def.h"
#include <seLib/experimental/BitFlags.h>
#include <sdk_config.h>
#include <ble.h>
#include <ble_gatts.h>
#include <ble_dis.h>

namespace seLib {
namespace BLE_Module {

// ** Forward declarations *********************************************************************

class BLE_Peripheral_State_t;
class BLE_Peripheral_Def_Base_t;
template <typename State_T = BLE_Peripheral_State_t> class BLE_Peripheral_Def_t;

class BLE_Service_State_t;
class BLE_Service_Def_Base_t;
template <typename State_T = BLE_Service_State_t> class BLE_Service_Def_t;


// ** Type definitions *********************************************************************

enum class BLEState {
	Init,
	Sleep,
	Idle,
	Advertising,
	ConnectedIdle,
	ConnectedStreaming,
	AdvertisingEnabled,
};

enum class BLEStatusFlags {
	Enabled,
	Sleep,
	Idle,
	Advertising,
	ConnectedIdle,
	ConnectedStreaming,
	AdvertisingEnabled,
};

typedef struct BLE_Characteristic_s {
	uint16_t    UUID;           /**< 16-bit UUID value or octets 12-13 of 128-bit UUID. */
	uint8_t     Read : 1;         /**< Supports read. */
	uint8_t     Write : 1;        /**< Supports write. */
	uint8_t     Notify : 1;       /**< Supports notify. */
	uint8_t     Secure : 1;       /**< Enable encryption. */
	uint8_t     Stack : 1;        /**< True if value is stored by the BLE stack instead of user memory. */
	uint8_t     InitialLength;  /**< Initial length of data. */
	uint8_t     MaxLength;      /**< Maximum length of data. */
	uint8_t*    Buffer;         /**< For characteristics stored in user memory, pointer to data buffer. */
	const uint8_t*    Description;    /**< Text description of characteristic. */
	const ble_gatts_char_pf_t* PresentationFormat;
} BLE_Characteristic_t;

class BLE_Peripheral_State_t {
public:
	uint16_t conn_handle = BLE_CONN_HANDLE_INVALID;                       /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
	//static BLE_Peripheral_Def_t const * ActiveDef;
	BitFlags<BLEStatusFlags> Flags;
	
};

class BLE_Peripheral_Def_Base_t {
public:
	BLE_Service_Def_Base_t const * const * ServiceList = nullptr;
	uint8_t ServiceCount = 0;
	uint8_t const * DeviceName;
	ble_uuid128_t const * base_uuid;
	ble_uuid_t const * adv_uuids = nullptr;
	uint8_t adv_uuid_count = 0;
	uint8_t Svc_DFU_Enabled : 1;
	uint32_t adv_fast_interval = 64;
	uint32_t adv_fast_timeout  = 90;
	uint32_t adv_slow_interval = 500;
	uint32_t adv_slow_timeout  = 3600;
	uint32_t min_conn_interval = MSEC_TO_UNITS(20, UNIT_1_25_MS);
	uint32_t max_conn_interval = MSEC_TO_UNITS(75, UNIT_1_25_MS);
	bool adv_extended_enabled:1;
	bool adv_fast_enabled:1;
	bool adv_slow_enabled:1;

	constexpr BLE_Peripheral_Def_Base_t(ble_uuid128_t const * _base_uuid, uint8_t const * device_name) :
		DeviceName(device_name), base_uuid(_base_uuid), Svc_DFU_Enabled(0),
		adv_extended_enabled(false), adv_fast_enabled(true), adv_slow_enabled(true)
	{
		
	};//*/

	constexpr BLE_Peripheral_Def_Base_t(const BLE_Peripheral_Def_Base_t&) = default;
	constexpr BLE_Peripheral_Def_Base_t(BLE_Peripheral_Def_Base_t&&) = default;

	/*constexpr BLE_Peripheral_Def_t(BLE_Peripheral_State_t* state, BLE_Service_Def_t const * const * service_list, uint8_t service_count, ble_uuid128_t const * _base_uuid, ble_uuid_t * _adv_uuids, uint8_t _adv_uuid_count) :
		State(state), ServiceList(service_list), ServiceCount(service_count), base_uuid(_base_uuid), adv_uuids(_adv_uuids), adv_uuid_count(_adv_uuid_count)
	{};//*/

	virtual void on_connect(ble_evt_t * p_ble_evt) const {};
	virtual void on_disconnect(ble_evt_t * p_ble_evt) const {};
	virtual void on_write(ble_evt_t * p_ble_evt) const {};
	virtual void on_ble_evt(ble_evt_t const * p_ble_evt) const;
	virtual void on_scan_req(ble_evt_t const * p_ble_evt) const {};
	virtual void on_scan_resp(ble_evt_t const * p_ble_evt) const {};
	virtual void on_adv_timeout() const;
	virtual void Init() const;
	virtual BLE_Peripheral_State_t* GetState() const = 0;
	uint32_t UpdateChar(BLE_Service_Def_Base_t const * service, BLE_Characteristic_t const * characteristic, uint8_t const * value, uint8_t length) const;

protected:
	void ble_stack_init() const;
	void gap_params_init() const;
	void gatt_init() const;
	void services_init() const;
	//virtual ble_advertising_init_t advertising_config() const;
	virtual void advertising_init() const;
	void conn_params_init() const;

	uint32_t InitService(const BLE_Service_Def_Base_t& service_def) const;
};

template <typename State_T>
class BLE_Peripheral_Def_t : public BLE_Peripheral_Def_Base_t {
public:
	State_T* State;

	constexpr BLE_Peripheral_Def_t(State_T* state, ble_uuid128_t const * _base_uuid, uint8_t const * device_name) :
		State(state), BLE_Peripheral_Def_Base_t(_base_uuid, device_name)
	{
	};//*/

	constexpr BLE_Peripheral_Def_t(const BLE_Peripheral_Def_t<State_T>&) = default;
	constexpr BLE_Peripheral_Def_t(BLE_Peripheral_Def_t<State_T>&&) = default;

	BLE_Peripheral_State_t* GetState() const override {
		return State;
	}

protected:
};


class BLE_Service_State_t {
public:
	ble_gatts_char_handles_t*	characteristic_handles;
	uint16_t                     service_handle;                    /**< Handle of Iantech301 Service (as provided by the BLE stack). */
	//uint16_t                     conn_handle = BLE_CONN_HANDLE_INVALID;                       /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
};

class BLE_Service_Def_Base_t {
public:
	friend class BLE_Peripheral_Def_Base_t;
	friend class BLE_Peripheral_Def_t<>;

	BLE_Characteristic_t const * CharacteristicList;
	uint8_t CharacteristicCount;
	ble_uuid_t uuid;
	//BLE_Peripheral_Def_t const * PeripheralDef;
	//ble_i301s_evt_handler_t        evt_handler;                       /**< Event handler to be called for handling events in the Iantech301 Service. */
	//uint8_t                      uuid_type;                         /**< UUID type for Custom Service Base UUID. */

	constexpr BLE_Service_Def_Base_t(BLE_Characteristic_t const * char_list, uint8_t char_count, ble_uuid_t _uuid) :
		CharacteristicList(char_list), CharacteristicCount(char_count), uuid(_uuid)
	{}//*/

	virtual BLE_Service_State_t* GetState() const = 0;

	virtual void Init() const;
	virtual void on_connect(ble_evt_t const * p_ble_evt) const {};
	virtual void on_disconnect(ble_evt_t const * p_ble_evt) const {};
	virtual void on_write(ble_evt_t const * p_ble_evt) const {};
	virtual void on_ble_evt(ble_evt_t const * p_ble_evt) const {};

	virtual uint32_t Send(const uint8_t* data, uint8_t datalen, uint8_t charindex) const;

protected:
	virtual uint32_t AddCharacteristic(uint8_t charindex) const;
};

template <typename State_T>
class BLE_Service_Def_t : public BLE_Service_Def_Base_t {
public:
	friend class BLE_Peripheral_Def_t<>;

	State_T * State;

	constexpr BLE_Service_Def_t(State_T * _state, BLE_Characteristic_t const * char_list, uint8_t char_count, ble_uuid_t _uuid) :
		State(_state), BLE_Service_Def_Base_t(char_list, char_count, _uuid)
	{}//*/

	BLE_Service_State_t* GetState() const override {
		return State;
	}

	//uint32_t Send(const uint8_t* data, uint8_t datalen, uint8_t charindex) const override;

protected:
};


class BLEManagerServiceState_t : public ServiceModel::ServiceState_t {
public:
};

class BLEManagerServiceDef : public ServiceModel::ServiceDef<BLEManagerServiceState_t> {
public:
	constexpr BLEManagerServiceDef(BLEManagerServiceState_t* _stateptr, ServiceModel::ServiceManagerDef const & manager, bool _autostart) :
		ServiceDef(_stateptr, manager, _autostart)
    {
	    AlwaysPoll = true;
    }

    ServiceMessages_t Init() const override;
	ServiceMessages_t Update() const override;
};

// ** Variable declarations *********************************************************************

extern BLE_Peripheral_Def_Base_t const * BLE_Active_Peripheral;

extern const ble_dis_init_t ble_dis_config;


// ** Function declarations *********************************************************************

bool Init();
void StopAdvertising();

}
}

#endif // EnableModule_BLE
