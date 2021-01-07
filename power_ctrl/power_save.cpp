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
 
#include "power_save.h"

static void configure_gpio(void) 
{
    for(int i = 0; i < 32; i++) 
    {
        /** P0 */
        // Disable sense on all pins
        NRF_P0->PIN_CNF[i] &= ~(0x30000);

        // Set unused pins to inputs to reduce current consumption
        NRF_P0->PIN_CNF[i] &= ~(0xD);   // Set to input, no pull
        NRF_P0->PIN_CNF[i] |= 0x2;      // Disconnect input buffer
    }     
    // Enter CONSTLAT mode if desired, otherwise TASKS_LOWPWR mode will be used (LOWPWR is recommended for most applications)
    NRF_UARTE0->ENABLE = 0;     //disable UART
    NRF_TWIM1->ENABLE = 0;     //disable TWI Master
    NRF_I2S->ENABLE = 0;
    NRF_SPI0->ENABLE = 0;     //disable SPI
    NRF_SPI1->ENABLE = 0;     //disable SPI
    NRF_SPI2->ENABLE = 0;     //disable SPI
    NRF_POWER->TASKS_LOWPWR = 1;
    //NRF_POWER->SYSTEMOFF = 1;
    NRF_CLOCK->TASKS_HFCLKSTOP = 1;
    NRF_CLOCK->TASKS_HFCLKSTART = 0;
}

// Enables the DC/DC converter (note: must have external hardware)
static void dcdc_en(void) 
{
    NRF_POWER->DCDCEN = 1;
}

// Disable NFC
static void disable_nfc(void) 
{
    NRF_UICR->NFCPINS = 0x00000000;
}

void power_save(void) 
{
    disable_nfc();
    dcdc_en();
    configure_gpio();

#if defined(TARGET_EP_AGORA)
    /** Disable certain power domains */

    // Disable battery voltage divider
    static mbed::DigitalOut battery_mon_en(PIN_NAME_BATTERY_MONITOR_ENABLE, 0);

    // Disable board ID voltage divider
    static mbed::DigitalOut board_id_disable(PIN_NAME_BOARD_ID_DISABLE, 1);

    // Disable sensor power domain
    static mbed::DigitalOut sensor_pwr_en(PIN_NAME_SENSOR_POWER_ENABLE, 0);

    // Disable cellular power domain
    // This should be enough to completely power down the cell module
    static mbed::DigitalOut cell_pwr_en(PIN_NAME_CELL_POWER_ENABLE, 0);
#endif
}
