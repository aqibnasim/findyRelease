/* mbed Microcontroller Library
 * Copyright (c) 2017-2019 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "platform/Callback.h"
#include "events/EventQueue.h"
#include "ble/BLE.h"
#include "mbed-os-ble-utils/gatt_server_process.h"
#include "power_ctrl/power_save.h"

using mbed::callback;
using namespace std::literals::chrono_literals;

/**
 * Custom Gatt profiles for Findy Device
 *
 */
class FindyDevice: public ble::GattServer::EventHandler {
public:
	FindyDevice() :
			c_VBatt("00002a19-0000-1000-8000-00805f9b34fb", 0), 
            c_Buzzer("12345679-8000-0080-5f9b-34fb12345670", 0), 
            _buzzer(LED2), 
            _Vbatt(A0) 
    {
		/* update internal pointers (value, descriptors and characteristics array) */
		_chars[0] = &c_VBatt;
		_chars[1] = &c_Buzzer;

		/* setup authorization handlers */
		c_VBatt.setWriteAuthorizationCallback(this, &FindyDevice::authorize_client_write);
		c_Buzzer.setWriteAuthorizationCallback(this, &FindyDevice::authorize_client_write);
	}

	void start(BLE &ble, events::EventQueue &event_queue) 
    {
		_server = &ble.gattServer();
		_event_queue = &event_queue;
        
		printf("Registering demo service\r\n");
		
        // Adding Battery Gatt Service and Characteristic
        _service = new GattService("0000180f-0000-1000-8000-00805f9b34fb", &_chars[0], 1);
		_server->addService(*_service);
        
        // Adding Buzzer Gatt Service and Characteristic
		_service = new GattService("12345678-0800-0008-05f9-b34fb1234567", &_chars[1], 1);
		_server->addService(*_service);

		/* register handlers */
		_server->setEventHandler(this);

		printf("Findy service registered\r\n");
		printf("Battery characteristic value handle %u\r\n", c_VBatt.getValueHandle());
		printf("Buzzer characteristic value handle %u\r\n", c_Buzzer.getValueHandle());

		// Adding periodic thead for melody play {it should be conditional and should suspend if no sound command}
		_event_queue->call_every(1000ms, callback(this, &FindyDevice::play_melody));
        
		// Adding periodic thead for battery level exchange at every 1 sec interval
		_event_queue->call_every(2000ms, callback(this, &FindyDevice::update_sensor_value));
	}

    // Play melody periodic thread that sound on buzzer Gatt command 
	void play_melody(void) {
        
		if (play_buzz) {
            
			printf("Playing Melody....\n");
            
			if (counter < 4) {
                
                // Creating buzzing sound of small beep at 4kHz
                
                //---------------- beep #1 -----------------------
				_buzzer.period(0.000253f);      // 0.25 ms period
                _buzzer.write(VOL_LEVEL_4);     // 40{+offset}% duty cycle, relative to period
				wait_us(300 * 1000);
				_buzzer.period(0);
				_buzzer.write(VOL_LEVEL_2);     // 20{+offset}% duty cycle, relative to period
				wait_us(10 * 1000);

                //---------------- beep #2 -----------------------
				_buzzer.period(0.000253f);      // 0.25 ms period
				_buzzer.write(VOL_LEVEL_4);     // 40{+offset}% duty cycle, relative to period
				wait_us(300 * 1000);
				_buzzer.period(0);
				_buzzer.write(VOL_LEVEL_2);     // 20{+offset}% duty cycle, relative to period
				wait_us(10 * 1000);

                //---------------- beep #3 -----------------------
				_buzzer.period(0.000253f);
				_buzzer.write(VOL_LEVEL_3);     // 30{+offset}% duty cycle, relative to period
				wait_us(300 * 1000);
				_buzzer.write(VOL_LEVEL_6);     // 60{+offset}% duty cycle, relative to period
				wait_us(300 * 1000);
				_buzzer.period(0);
				_buzzer.write(VOL_LEVEL_2);     // 20{+offset}% duty cycle, relative to period
				wait_us(10 * 1000);

                //---------------- beep #4 -----------------------
				_buzzer.period(0.000253f);      // 0.25 ms period
				_buzzer.write(VOL_LEVEL_4);     // 40{+offset}% duty cycle, relative to period
				wait_us(300 * 1000);
				_buzzer.period(0);
				_buzzer.write(VOL_LEVEL_2);     // 20{+offset}% duty cycle, relative to period
				wait_us(1000 * 1000);
			}
            
			counter++;
            
		}

		if (counter >= 4) {
            
            // reset counter and also flag play_buzzer
			counter = 0;
			play_buzz = false;
            
		}
        
	}

    // Update battery level periodic thread
	void update_sensor_value(void) 
    {
        
		float Vbatt = _Vbatt.read();
		
		Vbatt = Vbatt * 1800 * 2;
        printf("Vbatt :%.0f mV\n", Vbatt );
		Vbatt = Vbatt / 3600 * 100;
        
		/* and write it back */
		c_VBatt.set(*_server, Vbatt);

		printf("Vbatt :%.0f \r\n", Vbatt);
		printf("Battery Updates sending.......\n");

	}

	/* GattServer::EventHandler */
private:

	/**
	 * Handler called when a notification or an indication has been sent.
	 */
	void onDataSent(const GattDataSentCallbackParams &params) override
	{
		printf("sent updates\r\n");
	}

	/**
	 * Handler called after an attribute has been written.
	 */
	void onDataWritten(const GattWriteCallbackParams &params) override
	{

		printf("data written:\r\n");
		printf("connection handle: %u\r\n", params.connHandle);
		printf("attribute handle: %u", params.handle);

		if (params.handle == c_VBatt.getValueHandle()) 
        {
			printf(" (Battery characteristic)\r\n");
		} 
        else if (params.handle == c_Buzzer.getValueHandle()) 
        {
			printf(" (Buzzer characteristic)\r\n");
		} else 
        {
			printf("\r\n");
		}

		printf("write operation: %u\r\n", params.writeOp);
		printf("offset: %u\r\n", params.offset);
		printf("length: %u\r\n", params.len);
		printf("data: ");

		for (size_t i = 0; i < params.len; ++i) {
			printf("%02X", params.data[i]);
		}

		printf("Buzzer status received.....\n");

		if (params.data[0] == 1)
        {
			play_buzz = true;
        }

		printf("\r\n");
	}

	/**
	 * Handler called after an attribute has been read.
	 */
	void onDataRead(const GattReadCallbackParams &params) override
	{

		printf("data read:\r\n");
		printf("connection handle: %u\r\n", params.connHandle);
		printf("attribute handle: %u", params.handle);

		if (params.handle == c_VBatt.getValueHandle()) 
        {
			float Vbatt = _Vbatt.read();
			Vbatt = Vbatt * 1800 * 2;
			Vbatt = Vbatt / 3600 * 100;

			/* and write it back */
			c_VBatt.set(*_server, Vbatt);
            printf("Vbatt :%.0f mV\n", Vbatt );

		} 
        else if (params.handle == c_Buzzer.getValueHandle()) 
        {
			printf(" (Buzzer characteristic)\r\n");
		} 
        else 
        {
			printf("\r\n");
		}
	}

private:
	/**
	 * Handler called when a write request is received.
	 *
	 * This handler verify that the value submitted by the client is valid before
	 * authorizing the operation.
	 */
	void authorize_client_write(GattWriteAuthCallbackParams *e) 
    {

		printf("characteristic %u write authorization\r\n", e->handle);

		if (e->offset != 0) 
        {

			printf("Error invalid offset\r\n");
			e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_OFFSET;
			return;
		}

		if (e->len != 1) 
        {

			printf("Error invalid len\r\n");
			e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_ATT_VAL_LENGTH;
			return;
		}

		if ((e->data[0] >= 60)|| ((e->data[0] >= 24)&& (e->handle == c_VBatt.getValueHandle()))) 
        {

			printf("Error invalid data\r\n");
			e->authorizationReply =	AUTH_CALLBACK_REPLY_ATTERR_WRITE_NOT_PERMITTED;
			return;
		}

		e->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
	}

private:
	/**
	 * Read, Write, Notify, Indicate  Characteristic declaration helper.
	 *
	 * @tparam T type of data held by the characteristic.
	 */
	template<typename T>
	class ReadWriteNotifyIndicateCharacteristic: public GattCharacteristic {
	public:

		/**
		 * Construct a characteristic that can be read or written and emit
		 * notification or indication.
		 *
		 * @param[in] uuid The UUID of the characteristic.
		 * @param[in] initial_value Initial value contained by the characteristic.
		 */
		ReadWriteNotifyIndicateCharacteristic(const UUID & uuid,
				const T& initial_value) :
				GattCharacteristic(
				/* UUID */uuid,
				/* Initial value */&_value,
				/* Value size */sizeof(_value),
				/* Value capacity */sizeof(_value),
						/* Properties */GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ
								| GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE
								| GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
								| GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_INDICATE,
						/* Descriptors */nullptr,
						/* Num descriptors */0,
						/* variable len */false), _value(initial_value) {
		}

		/**
		 * Get the value of this characteristic.
		 *
		 * @param[in] server GattServer instance that contain the characteristic
		 * value.
		 * @param[in] dst Variable that will receive the characteristic value.
		 *
		 * @return BLE_ERROR_NONE in case of success or an appropriate error code.
		 */
		ble_error_t get(GattServer &server, T& dst) const {
			uint16_t value_length = sizeof(dst);
			return server.read(getValueHandle(), &dst, &value_length);
		}

		/**
		 * Assign a new value to this characteristic.
		 *
		 * @param[in] server GattServer instance that will receive the new value.
		 * @param[in] value The new value to set.
		 * @param[in] local_only Flag that determine if the change should be kept
		 * locally or forwarded to subscribed clients.
		 */
		ble_error_t set(GattServer &server, const uint8_t &value,
				bool local_only = false) const {
			return server.write(getValueHandle(), &value, sizeof(value),
					local_only);
		}

	private:
		uint8_t _value;
	};

private:
	GattServer *_server = nullptr;
	events::EventQueue *_event_queue = nullptr;

	GattService *_service;
	GattCharacteristic* _chars[2];

	PwmOut _buzzer;
	AnalogIn _Vbatt;

	uint8_t counter = 0;
	uint8_t play_buzz = false;

	ReadWriteNotifyIndicateCharacteristic<uint8_t> c_VBatt;
	ReadWriteNotifyIndicateCharacteristic<uint8_t> c_Buzzer;
};

int main() {
	//rtos::Kernel::attach_idle_hook(&sleep);
	mbed_file_handle(STDIN_FILENO)->enable_input(false);
	mbed_file_handle(STDIN_FILENO)->enable_output(false);

	power_save();

	BLE &ble = BLE::Instance();
	events::EventQueue event_queue;
	FindyDevice demo_service;

	/* this process will handle basic ble setup and advertising for us */
	GattServerProcess ble_process(event_queue, ble);

	/* once it's done it will let us continue with our demo */
	ble_process.on_init(callback(&demo_service, &FindyDevice::start));

	ble_process.start();
	while (1) {
        // turn OFF peripherals, NFC and enable DCDC converter
		power_save();

		// Enter System ON sleep mode
		ThisThread::sleep_for(2000);

	}

	return 0;
}
